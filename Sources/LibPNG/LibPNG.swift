@_exported import CLibPNG
import Foundation

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

/// Compresses an 8-bit image buffer to a PNG.
///
/// - Parameters:
///   - srcBuf: The pixel data; rows are top-down unless `flip` is true.
///   - pixelFormat: The in-memory pixel layout.
///   - width: The image width in pixels.
///   - height: The image height in pixels.
///   - pitch: The bytes per row; 0 means tightly packed.
///   - flip: Pass true when the rows in `srcBuf` are stored bottom-up.
/// - Returns: The PNG data, or an empty array if compression failed.
public func pngCompress<T>(
    _ srcBuf: [T], _ pixelFormat: PNGPixelFormat, _ width: Int, _ height: Int, _ pitch: Int = 0,
    _ flip: Bool = false
)
    -> [UInt8]
{
    let rowBytes = width * pixelFormat.channels
    // libpng only sees row_stride, not the buffer length, so reject mismatched
    // buffers up front instead of letting it read out of bounds.
    guard width > 0, height > 0,
        srcBuf.count * MemoryLayout<T>.stride >= height * max(abs(pitch), rowBytes)
    else { return [] }

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
        else { return [] }

        var png = [UInt8](repeating: 0, count: Int(size))
        let written = png.withUnsafeMutableBytes { dst in
            png_image_write_to_memory(
                &image, dst.baseAddress, &size, 0, src.baseAddress, rowStride, nil) != 0
        }
        guard written else { return [] }
        if size < png.count { png.removeSubrange(Int(size)...) }
        return png
    }
}

/// Decompresses a PNG to tightly packed, top-down 8-bit pixels.
///
/// - Parameters:
///   - pngBuf: The PNG data.
///   - pixelFormat: The layout of the returned pixels; the PNG is converted to
///     it regardless of its source color type, compositing any alpha channel
///     onto black when the requested layout has none.
/// - Returns: The dimensions and pixel data, or `nil` if `pngBuf` is not a
///   valid PNG.
public func pngDecompress(_ pngBuf: [UInt8], _ pixelFormat: PNGPixelFormat)
    -> (width: Int, height: Int, pixels: [UInt8])?
{
    var image = png_image()
    image.version = png_uint_32(PNG_IMAGE_VERSION)
    defer { png_image_free(&image) }

    let began = pngBuf.withUnsafeBytes { src in
        png_image_begin_read_from_memory(&image, src.baseAddress, src.count) != 0
    }
    guard began else { return nil }

    image.format = pixelFormat.rawValue

    let width = Int(image.width), height = Int(image.height)
    var pixels = [UInt8](repeating: 0, count: width * height * pixelFormat.channels)
    let read = pixels.withUnsafeMutableBytes { dst in
        png_image_finish_read(&image, nil, dst.baseAddress, 0, nil) != 0
    }
    guard read else { return nil }

    return (width, height, pixels)
}
