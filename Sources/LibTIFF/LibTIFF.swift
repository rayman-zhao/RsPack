@_exported import CLibTIFF
import Foundation
import LibJPEGTurbo

public typealias TIFFWarningHandler = (String, String) -> Void
public typealias TIFFErrorHandler = (String, String) -> Void

@MainActor
private var warningHandler: TIFFWarningHandler?
@MainActor
private var errorHandler: TIFFErrorHandler?

private func tiffMessage(
    _ module: UnsafePointer<CChar>?, _ fmt: UnsafePointer<CChar>?, _ ap: CVaListPointer?
) -> (module: String, message: String)? {
    guard let module, let fmt, let ap else { return nil }
    return (String(cString: module), NSString(format: String(cString: fmt), arguments: ap) as String)
}

private func cWarningHandler(
    _ module: UnsafePointer<CChar>?, _ fmt: UnsafePointer<CChar>?, _ ap: CVaListPointer?
) {
    guard let msg = tiffMessage(module, fmt, ap) else { return }
    Task { @MainActor in
        warningHandler?(msg.module, msg.message)
    }
}

private func cErrorHandler(
    _ module: UnsafePointer<CChar>?, _ fmt: UnsafePointer<CChar>?, _ ap: CVaListPointer?
) {
    guard let msg = tiffMessage(module, fmt, ap) else { return }
    Task { @MainActor in
        errorHandler?(msg.module, msg.message)
    }
}

@MainActor
public func TIFFSetWarningHandler(_ handler: @escaping TIFFWarningHandler) {
    warningHandler = handler
    TIFFSetWarningHandler(cWarningHandler)
}

@MainActor
public func TIFFSetErrorHandler(_ handler: @escaping TIFFErrorHandler) {
    errorHandler = handler
    TIFFSetErrorHandler(cErrorHandler)
}

public func TIFFSetDirectory(_ tiff: OpaquePointer?, _ dirnum: UInt32, _ diroffset: UInt64? = nil)
    -> Bool
{
    if let diroffset {
        guard TIFFCurrentDirOffset(tiff) != diroffset else { return true }
        guard TIFFCurrentDirectory(tiff) != dirnum else {
            return TIFFSetSubDirectory(tiff, diroffset) == 1
        }
        return TIFFSetDirectory(tiff, dirnum) == 1 && TIFFSetSubDirectory(tiff, diroffset) == 1
    } else if dirnum == 0 {
        guard TIFFCurrentDirectory(tiff) != dirnum || TIFFCurrentDirOffset(tiff) != 8 else {
            return true
        }
        return TIFFSetDirectory(tiff, dirnum) == 1
    } else {
        guard TIFFCurrentDirectory(tiff) != dirnum else { return true }
        return TIFFSetDirectory(tiff, dirnum) == 1
    }
}

public func TIFFGetField<T>(_ tiff: OpaquePointer?, _ tag: Int32) -> T? {
    withUnsafeTemporaryAllocation(of: T.self, capacity: 1) { pv in
        let success = withVaList([pv.baseAddress!]) { args in
            return TIFFVGetField(tiff, UInt32(tag), args) == 1
        }
        return success ? pv[0] : nil
    }
}

public func TIFFGetField<T, P>(_ tiff: OpaquePointer?, _ tag: Int32) -> (T?, P?) {
    withUnsafeTemporaryAllocation(of: T.self, capacity: 1) { pv in
        withUnsafeTemporaryAllocation(of: P.self, capacity: 1) { pv2 in
            let success = withVaList([pv.baseAddress!, pv2.baseAddress!]) { args in
                return TIFFVGetField(tiff, UInt32(tag), args) == 1
            }
            return success ? (pv[0], pv2[0]) : (nil, nil)
        }
    }
}

public func TIFFSetField<T: CVarArg>(_ tiff: OpaquePointer?, _ tag: Int32, _ value: T) -> Bool {
    let success = withVaList([value]) { args in
        return TIFFVSetField(tiff, UInt32(tag), args)
    }
    return success == 1
}

public func TIFFSetField<T: CVarArg, P: CVarArg>(
    _ tiff: OpaquePointer?, _ tag: Int32, _ value: T, _ value2: P
) -> Bool {
    let success = withVaList([value, value2]) { args in
        return TIFFVSetField(tiff, UInt32(tag), args)
    }
    return success == 1
}

public func TIFFReadJPEGImage(
    _ tiff: OpaquePointer?, _ dirnum: UInt32? = nil, _ diroffset: UInt64? = nil
) -> [UInt8]? {
    if let dirnum {
        guard TIFFSetDirectory(tiff, dirnum, diroffset) else { return nil }
    }

    let tileWidth: UInt32? = TIFFGetField(tiff, TIFFTAG_TILEWIDTH)
    guard tileWidth == nil else { return nil }

    if let comp: UInt16 = TIFFGetField(tiff, TIFFTAG_COMPRESSION),
        comp == UInt16(COMPRESSION_JPEG),
        let photometric: UInt16 = TIFFGetField(tiff, TIFFTAG_PHOTOMETRIC),
        photometric == UInt16(PHOTOMETRIC_YCBCR)
    {
        let bufSize = Int(TIFFRawStripSize(tiff, 0))
        var buf = [UInt8](repeating: 0, count: bufSize)
        let stripSize = buf.withUnsafeMutableBytes {
            Int(TIFFReadRawStrip(tiff, 0, $0.baseAddress, tmsize_t(bufSize)))
        }

        var jpeg = stripSize == bufSize ? buf : Array(buf[..<stripSize])

        if jpeg.starts(with: [0xFF, 0xD8, 0xFF, 0xC0]) {
            let dqt: (count: UInt32?, data: UnsafeMutableRawPointer?) = TIFFGetField(
                tiff, TIFFTAG_JPEGTABLES)
            if let count = dqt.count, let data = dqt.data {
                // The DQT data is 0xFFDB...0xFFD9
                jpeg.replaceSubrange(
                    0..<2,
                    with: UnsafeBufferPointer(
                        start: data.assumingMemoryBound(to: UInt8.self), count: Int(count) - 2))
            }
        }

        return jpeg
    }

    if let w: UInt32 = TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH),
        let h: UInt32 = TIFFGetField(tiff, TIFFTAG_IMAGELENGTH)
    {
        let bufSize = Int(w * h)
        var buf = [UInt32](repeating: 0, count: bufSize)

        let read = buf.withUnsafeMutableBufferPointer {
            TIFFReadRGBAImageOriented(tiff, w, h, $0.baseAddress, ORIENTATION_TOPLEFT, 0) == 1
        }
        if read {
            return tjCompress(buf, TJPF_RGBA, Int(w), Int(h))
        }
    }

    return nil
}
