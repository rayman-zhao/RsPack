@_exported import CLibJPEGTurbo
import Foundation

public func tjGetErrorStr(_ handle: tjhandle?) -> String {
    return String(cString: tj3GetErrorStr(handle))
}

public func tjCompress<T>(
    _ srcBuf: [T], _ pixelFormat: TJPF, _ width: Int, _ height: Int, _ pitch: Int = 0,
    _ quality: Int = 85, _ flip: Bool = false
) -> [UInt8]? {
    // TJSAMP_GRAY is the only subsampling level libjpeg-turbo accepts for
    // TJPF_GRAY input; any other level fails with "Unsupported color
    // conversion request".
    let subsamp = pixelFormat == TJPF_GRAY ? TJSAMP_GRAY : TJSAMP_420

    let tj = tj3Init(Int32(TJINIT_COMPRESS.rawValue))
    defer { tj3Destroy(tj) }
    tj3Set(tj, Int32(TJPARAM_QUALITY.rawValue), Int32(quality))
    tj3Set(tj, Int32(TJPARAM_SUBSAMP.rawValue), subsamp.rawValue)
    tj3Set(tj, Int32(TJPARAM_BOTTOMUP.rawValue), flip ? 1 : 0)

    var jpegBuf: UnsafeMutablePointer<UInt8>?
    defer { tj3Free(jpegBuf) }
    var jpegSize: Int = 0

    return srcBuf.withUnsafeBytes { buf in
        guard
            tj3Compress8(
                tj, buf.baseAddress, Int32(width), Int32(pitch), Int32(height),
                pixelFormat.rawValue, &jpegBuf, &jpegSize) == 0
        else { return nil }

        return Array(UnsafeBufferPointer(start: jpegBuf, count: jpegSize))
    }
}

/// Decompresses a JPEG image into a caller-provided buffer — no intermediate
/// allocation.
///
/// - Parameters:
///   - jpegBuf: The JPEG image.
///   - pixelFormat: The pixel layout of the output.
///   - dstBuf: A buffer of at least `pitch * height` bytes (height comes from
///     the JPEG header; probe with `tjDecompressHeader` first).
///   - pitch: Bytes per output row; 0 means tightly packed
///     (`width * pixel size`).
///   - flip: Emit rows bottom-up (Windows bitmap raster order).
/// - Returns: The decompressed image dimensions, or `nil` on failure.
@discardableResult
public func tjDecompress(
    _ jpegBuf: [UInt8], _ pixelFormat: TJPF, into dstBuf: UnsafeMutableRawPointer, _ pitch: Int,
    _ flip: Bool = false
) -> (width: Int, height: Int)? {
    let tj = tj3Init(Int32(TJINIT_DECOMPRESS.rawValue))
    defer { tj3Destroy(tj) }

    tj3Set(tj, Int32(TJPARAM_BOTTOMUP.rawValue), flip ? 1 : 0)

    let ok = jpegBuf.withUnsafeBytes { src in
        tj3Decompress8(
            tj, src.baseAddress, src.count,
            dstBuf.assumingMemoryBound(to: UInt8.self), Int32(pitch),
            pixelFormat.rawValue) == 0
    }
    guard ok else { return nil }
    return (
        Int(tj3Get(tj, TJPARAM_JPEGWIDTH.rawValue)),
        Int(tj3Get(tj, TJPARAM_JPEGHEIGHT.rawValue))
    )
}

/// Decompresses a JPEG image into a fresh pixel array.
///
/// - Parameters:
///   - jpegBuf: The JPEG image.
///   - pixelFormat: The pixel layout of the output.
///   - pitch: Bytes per output row; 0 means tightly packed
///     (`width * pixel size`).
///   - flip: Emit rows bottom-up (Windows bitmap raster order).
/// - Returns: The pixel buffer and dimensions, or `nil` on failure.
public func tjDecompress(
    _ jpegBuf: [UInt8], _ pixelFormat: TJPF, _ pitch: Int = 0, _ flip: Bool = false
) -> (pixels: [UInt8], width: Int, height: Int)? {
    let header = tjDecompressHeader(jpegBuf)
    guard header.width > 0, header.height > 0 else { return nil }

    let pixelSize: Int
    switch pixelFormat {
    case TJPF_GRAY: pixelSize = 1
    case TJPF_RGB, TJPF_BGR: pixelSize = 3
    default: pixelSize = 4
    }
    let rowBytes = pitch > 0 ? pitch : header.width * pixelSize
    var pixels = [UInt8](repeating: 0, count: rowBytes * header.height)
    guard let dims = tjDecompress(jpegBuf, pixelFormat, into: &pixels, rowBytes, flip) else {
        return nil
    }
    return (pixels, dims.width, dims.height)
}

public func tjDecompressHeader(_ jpegBuf: [UInt8]) -> (width: Int, height: Int) {
    let tj = tj3Init(Int32(TJINIT_DECOMPRESS.rawValue))
    defer { tj3Destroy(tj) }

    return jpegBuf.withUnsafeBytes { buf in
        guard tj3DecompressHeader(tj, buf.baseAddress, buf.count) == 0 else { return (-1, -1) }

        return (
            Int(tj3Get(tj, TJPARAM_JPEGWIDTH.rawValue)),
            Int(tj3Get(tj, TJPARAM_JPEGHEIGHT.rawValue))
        )
    }
}

/// Losslessly rotates a JPEG image and returns the rotated JPEG data.
///
/// The rotation is applied directly to the compressed data, so the image is
/// never recompressed and does not lose quality. The width and height swap
/// when rotating by an odd multiple of 90 degrees.
///
/// - Parameters:
///   - jpegBuf: The JPEG image to rotate.
///   - degrees: The clockwise rotation in degrees, normalized modulo 360, so
///     `-90` rotates counter-clockwise by 90 degrees. Must be a multiple of 90.
/// - Returns: The rotated JPEG image, or `nil` if `jpegBuf` could not be
///   transformed or `degrees` is not a multiple of 90.
public func tjRotate(_ jpegBuf: consuming [UInt8], degrees: Int) -> [UInt8]? {
    let op: Int32
    switch ((degrees % 360) + 360) % 360 {
    case 0: return jpegBuf
    case 90: op = Int32(TJXOP_ROT90.rawValue)
    case 180: op = Int32(TJXOP_ROT180.rawValue)
    case 270: op = Int32(TJXOP_ROT270.rawValue)
    default: return nil
    }

    let tj = tj3Init(Int32(TJINIT_TRANSFORM.rawValue))
    defer { tj3Destroy(tj) }

    var transform = tjtransform()
    transform.op = op

    var dstBuf: UnsafeMutablePointer<UInt8>?
    defer { tj3Free(dstBuf) }
    var dstSize: Int = 0

    return jpegBuf.withUnsafeBytes { buf -> [UInt8]? in
        guard
            tj3Transform(
                tj, buf.baseAddress, buf.count, 1, &dstBuf, &dstSize, &transform) == 0
        else { return nil }

        return Array(UnsafeBufferPointer(start: dstBuf, count: dstSize))
    }
}
