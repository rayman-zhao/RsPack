@_exported import CLibPNG
import Foundation
import Zlib

/// In-memory pixel layouts `pngCompress` accepts, matching libpng's 8-bit
/// simplified write formats (PNG_FORMAT_*); the raw value is the format flag.
public enum PNGPixelFormat: png_uint_32 {
    case gray = 0
    case grayAlpha = 0x01
    case rgb = 0x02
    case bgr = 0x12
    case rgba = 0x03
    case bgra = 0x13

    /// The number of 8-bit channels per pixel.
    public var channels: Int {
        switch self {
        case .gray: 1
        case .grayAlpha: 2
        case .rgb, .bgr: 3
        case .rgba, .bgra: 4
        }
    }
}

private extension PNGPixelFormat {
    // PNG_COLOR_TYPE_RGB and friends are expression macros the clang importer
    // drops, so compose the file color type from the mask literals instead.
    var colorType: Int32 {
        switch self {
        case .gray: PNG_COLOR_TYPE_GRAY
        case .grayAlpha: PNG_COLOR_MASK_ALPHA
        case .rgb, .bgr: PNG_COLOR_MASK_COLOR
        case .rgba, .bgra: PNG_COLOR_MASK_COLOR | PNG_COLOR_MASK_ALPHA
        }
    }
}

// A class so the C write callback can recover it from png_get_io_ptr's void*.
private final class PngByteSink {
    var bytes = [UInt8]()
}

private func pngWriteToSink(png: png_structp?, data: png_bytep?, size: Int) {
    guard let png, let data, let ioPtr = png_get_io_ptr(png) else { return }
    let sink = Unmanaged<PngByteSink>.fromOpaque(ioPtr).takeUnretainedValue()
    sink.bytes.append(contentsOf: UnsafeBufferPointer(start: data, count: size))
}

/// Compresses an 8-bit image buffer to a PNG.
///
/// - Parameters:
///   - srcBuf: The pixel data; rows are top-down unless `flip` is true.
///   - pixelFormat: The in-memory pixel layout.
///   - width: The image width in pixels.
///   - height: The image height in pixels.
///   - pitch: The bytes per row; 0 means tightly packed.
///   - flip: Pass true when the rows in `srcBuf` are stored bottom-up.
/// - Returns: The PNG data, or `nil` if compression failed.
public func pngCompress<T>(
    _ srcBuf: [T], _ pixelFormat: PNGPixelFormat, _ width: Int, _ height: Int, _ pitch: Int = 0,
    _ flip: Bool = false
) -> [UInt8]? {
    let rowBytes = width * pixelFormat.channels
    // libpng only sees row_stride, not the buffer length, so reject mismatched
    // buffers up front instead of letting it read out of bounds.
    guard width > 0, height > 0,
        srcBuf.count * MemoryLayout<T>.stride >= height * max(abs(pitch), rowBytes)
    else { return nil }

    var image = png_image()
    image.version = png_uint_32(PNG_IMAGE_VERSION)
    image.width = png_uint_32(width)
    image.height = png_uint_32(height)
    image.format = pixelFormat.rawValue
    defer { png_image_free(&image) }

    // A zero row_stride lets libpng derive the tight stride, but -0 == 0 would
    // silently drop the flip, so spell out the row width in that case.
    let rowStride = png_int_32(flip ? -(pitch > 0 ? pitch : rowBytes) : pitch)

    return srcBuf.withUnsafeBytes { src in
        // A NULL destination makes the call report the exact output size.
        var size = png_alloc_size_t(0)
        guard
            png_image_write_to_memory(&image, nil, &size, 0, src.baseAddress, rowStride, nil) != 0,
            size > 0
        else { return nil }

        var png = [UInt8](repeating: 0, count: Int(size))
        let written = png.withUnsafeMutableBytes { dst in
            png_image_write_to_memory(
                &image, dst.baseAddress, &size, 0, src.baseAddress, rowStride, nil) != 0
        }
        guard written else { return nil }
        if size < png.count { png.removeSubrange(Int(size)...) }
        return png
    }
}

/// Compresses an 8-bit image buffer to a PNG via libpng's low-level write
/// API with a speed-oriented profile pinned explicitly instead of the
/// simplified API's defaults: zlib level 1 with the Z_RLE strategy and the
/// SUB/UP row filters — the fastest combination measured to stay at or
/// below TIFF-LZW size on both graphic and photographic content.
///
/// - Parameters: same as `pngCompress`.
/// - Returns: The PNG data, or `nil` if compression failed.
public func pngCompress2<T>(
    _ srcBuf: [T], _ pixelFormat: PNGPixelFormat, _ width: Int, _ height: Int, _ pitch: Int = 0,
    _ flip: Bool = false
) -> [UInt8]? {
    let rowBytes = width * pixelFormat.channels
    // libpng only sees row_stride, not the buffer length, so reject mismatched
    // buffers up front instead of letting it read out of bounds.
    guard width > 0, height > 0,
        width <= Int(PNG_USER_WIDTH_MAX), height <= Int(PNG_USER_HEIGHT_MAX),
        srcBuf.count * MemoryLayout<T>.stride >= height * max(abs(pitch), rowBytes)
    else { return nil }

    var png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nil, nil, nil)
    guard let pngp = png else { return nil }
    var info = png_create_info_struct(pngp)
    defer { png_destroy_write_struct(&png, &info) }
    guard let infop = info else { return nil }

    // Tuned for encode speed at an LZW-class ratio (measured against TIFF
    // LZW on gradient and photo-like noise): level 1, the Z_RLE strategy
    // zlib designed for filtered PNG rows, and the two cheap row filters —
    // offering all five costs roughly half the speed while the adaptive
    // heuristic picks SUB or UP anyway on natural images.
    png_set_compression_level(pngp, 1)
    png_set_compression_strategy(pngp, Z_RLE)
    png_set_filter(pngp, PNG_FILTER_TYPE_BASE, PNG_FILTER_SUB | PNG_FILTER_UP)

    if pixelFormat == .bgr || pixelFormat == .bgra { png_set_bgr(pngp) }
    png_set_IHDR(
        pngp, infop, png_uint_32(width), png_uint_32(height), 8, pixelFormat.colorType,
        PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE)

    let sink = PngByteSink()
    png_set_write_fn(pngp, Unmanaged.passUnretained(sink).toOpaque(), pngWriteToSink, nil)

    // png_error longjmps through a jmp_buf Swift can never set up, so the
    // write sequence must be infallible: everything libpng validates (the
    // dimension caps in the guard above) is pre-checked, and the row-
    // streaming write path only allocates fixed-size zlib and row buffers.
    png_write_info(pngp, infop)
    let stride = pitch > 0 ? pitch : rowBytes
    srcBuf.withUnsafeBytes { src in
        // A negative pitch without flip meant bottom-up in the simplified API;
        // keep feeding the rows in reverse order in that case.
        let bottomUp = flip || pitch < 0
        for row in 0..<height {
            let memRow = bottomUp ? height - 1 - row : row
            png_write_row(pngp, (src.baseAddress! + memRow * stride).assumingMemoryBound(to: png_byte.self))
        }
    }
    png_write_end(pngp, infop)
    return sink.bytes
}

/// Reads the pixel dimensions from a PNG header.
///
/// - Parameters:
///   - pngBuf: The PNG data.
/// - Returns: The width and height, or (-1, -1) if `pngBuf` is not a valid PNG.
public func pngDecompressHeader(_ pngBuf: [UInt8]) -> (width: Int, height: Int) {
    var image = png_image()
    image.version = png_uint_32(PNG_IMAGE_VERSION)
    defer { png_image_free(&image) }

    return pngBuf.withUnsafeBytes { src in
        guard png_image_begin_read_from_memory(&image, src.baseAddress, src.count) != 0 else {
            return (-1, -1)
        }

        return (Int(image.width), Int(image.height))
    }
}

/// Decompresses a PNG into caller-provided memory as 8-bit pixels in the
/// requested layout.
///
/// The PNG is converted to the requested layout regardless of its source
/// color type, compositing any alpha channel onto black when the layout has
/// none. `dst` may point into the middle of a larger buffer — a tile inside
/// a stitched layer image, for example — the way `tj3Decompress8` places
/// JPEG tiles. Like `tj3Decompress8`, the destination extent is the caller's
/// responsibility; size it with `pngDecompressHeader` when the dimensions
/// are unknown.
///
/// - Parameters:
///   - pngBuf: The PNG data.
///   - dst: The destination memory.
///   - pixelFormat: The layout of the output pixels.
///   - pitch: The bytes per row at `dst`; 0 means tightly packed, and a
///     negative value stores the rows bottom-up.
/// - Returns: true on success; false if `pngBuf` is not a valid PNG.
public func pngDecompress(
    _ pngBuf: [UInt8],
    _ dst: UnsafeMutablePointer<UInt8>,
    _ pixelFormat: PNGPixelFormat, _ pitch: Int = 0
) -> Bool {
    var image = png_image()
    image.version = png_uint_32(PNG_IMAGE_VERSION)
    defer { png_image_free(&image) }

    let began = pngBuf.withUnsafeBytes { src in
        png_image_begin_read_from_memory(&image, src.baseAddress, src.count) != 0
    }
    guard began else { return false }

    image.format = pixelFormat.rawValue
    return png_image_finish_read(&image, nil, dst, png_int_32(pitch), nil) != 0
}
