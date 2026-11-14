@_exported import COpenJPEG
import Foundation

private final class OpjReadStream {
    let base: UnsafeRawPointer
    let count: Int
    var pos = 0

    init(buffer: UnsafeRawBufferPointer) {
        base = buffer.baseAddress!
        count = buffer.count
    }
}

private final class OpjWriteStream {
    var data = [UInt8]()
    var pos = 0

    func write(_ src: UnsafeRawPointer, _ count: Int) {
        if pos > data.count {
            data.append(contentsOf: [UInt8](repeating: 0, count: pos - data.count))
        }
        if pos == data.count {
            data.append(
                contentsOf: UnsafeBufferPointer(start: src.assumingMemoryBound(to: UInt8.self), count: count))
        } else {
            if pos + count > data.count {
                data.append(contentsOf: [UInt8](repeating: 0, count: pos + count - data.count))
            }
            data.withUnsafeMutableBytes { $0.baseAddress!.advanced(by: pos).copyMemory(from: src, byteCount: count) }
        }
        pos += count
    }
}

// Returning -1 from a size_t callback is (OPJ_SIZE_T)-1 in C, the error signal.
// opj_stream_read_data's refill loop only stops on -1 or a full read; a 0 at
// end-of-data would spin it forever.
private func opjReadFn(_ buffer: UnsafeMutableRawPointer?, _ count: Int, _ userData: UnsafeMutableRawPointer?) -> Int {
    guard let buffer, let userData else { return -1 }
    let s = Unmanaged<OpjReadStream>.fromOpaque(userData).takeUnretainedValue()
    guard s.pos < s.count else { return -1 }
    let n = min(count, s.count - s.pos)
    buffer.copyMemory(from: s.base + s.pos, byteCount: n)
    s.pos += n
    return n
}

// The skip callbacks must return the number of bytes skipped, not the new
// position — the library tracks the absolute offset itself.
private func opjSkipReadFn(_ count: Int64, _ userData: UnsafeMutableRawPointer?) -> Int64 {
    guard let userData, count >= 0 else { return -1 }
    let s = Unmanaged<OpjReadStream>.fromOpaque(userData).takeUnretainedValue()
    guard s.pos + Int(count) <= s.count else { return -1 }
    s.pos += Int(count)
    return count
}

private func opjSeekReadFn(_ target: Int64, _ userData: UnsafeMutableRawPointer?) -> Int32 {
    guard let userData, target >= 0 else { return 0 }
    let s = Unmanaged<OpjReadStream>.fromOpaque(userData).takeUnretainedValue()
    guard target <= Int64(s.count) else { return 0 }
    s.pos = Int(target)
    return 1
}

private func opjWriteFn(_ buffer: UnsafeMutableRawPointer?, _ count: Int, _ userData: UnsafeMutableRawPointer?) -> Int {
    guard let buffer, let userData else { return -1 }
    let s = Unmanaged<OpjWriteStream>.fromOpaque(userData).takeUnretainedValue()
    s.write(buffer, count)
    return count
}

private func opjSkipWriteFn(_ count: Int64, _ userData: UnsafeMutableRawPointer?) -> Int64 {
    guard let userData, count >= 0 else { return -1 }
    let s = Unmanaged<OpjWriteStream>.fromOpaque(userData).takeUnretainedValue()
    s.pos += Int(count)
    return count
}

private func opjSeekWriteFn(_ target: Int64, _ userData: UnsafeMutableRawPointer?) -> Int32 {
    guard let userData, target >= 0 else { return 0 }
    let s = Unmanaged<OpjWriteStream>.fromOpaque(userData).takeUnretainedValue()
    s.pos = Int(target)
    return 1
}

private let opjJP2Signature: [UInt8] = [
    0x00, 0x00, 0x00, 0x0C, 0x6A, 0x50, 0x20, 0x20, 0x0D, 0x0A, 0x87, 0x0A,
]

private func opjSniffFormat(_ data: [UInt8]) -> OPJ_CODEC_FORMAT? {
    if data.starts(with: opjJP2Signature) { return OPJ_CODEC_JP2 }
    if data.starts(with: [0xFF, 0x4F]) { return OPJ_CODEC_J2K }
    return nil
}

private func opjInterleave(
    _ img: opj_image_t
) -> (pixels: [UInt8], width: Int, height: Int, components: Int)? {
    let width = Int(img.x1 - img.x0)
    let height = Int(img.y1 - img.y0)
    let count = Int(img.numcomps)
    guard width > 0, height > 0, count > 0 else { return nil }

    var pixels = [UInt8](repeating: 0, count: width * height * count)
    for c in 0..<count {
        let comp = img.comps[c]
        guard let plane = comp.data else { return nil }
        let cw = Int(comp.w)
        let ch = Int(comp.h)
        let prec = Int(comp.prec)
        guard cw > 0, ch > 0, (1...31).contains(prec) else { return nil }

        // Signed samples center on zero; shift them into the unsigned range
        // before rescaling to 8 bits. Oversized/downsized components (dx/dy
        // subsampling) map back to the image grid with nearest-neighbor.
        let offset = comp.sgnd != 0 ? (1 << (prec - 1)) : 0
        let down = max(0, prec - 8)
        let up = max(0, 8 - prec)
        for y in 0..<height {
            let planeRow = (y * ch / height) * cw
            for x in 0..<width {
                let value = Int(plane[planeRow + x * cw / width]) + offset
                pixels[(y * width + x) * count + c] = UInt8(clamping: (value >> down) << up)
            }
        }
    }
    return (pixels, width, height, count)
}

/// Decompresses a JPEG 2000 image — either a `.jp2` container or a `.j2k`
/// raw codestream, auto-detected from the signature.
///
/// Samples are interleaved to 8 bits per pixel: precisions above 8 bits are
/// right-shifted, below 8 bits left-shifted, and subsampled components are
/// upsampled with nearest-neighbor sampling.
///
/// - Returns: The interleaved pixels, dimensions and component count, or `nil`
///   if `data` is not a decodable JPEG 2000 image.
public func opjDecompress(_ data: [UInt8]) -> (pixels: [UInt8], width: Int, height: Int, components: Int)? {
    guard let format = opjSniffFormat(data) else { return nil }
    guard let codec = opj_create_decompress(format) else { return nil }
    defer { opj_destroy_codec(codec) }
    opj_set_info_handler(codec, { _, _ in }, nil)
    opj_set_warning_handler(codec, { _, _ in }, nil)
    opj_set_error_handler(codec, { _, _ in }, nil)

    var parameters = opj_dparameters_t()
    opj_set_default_decoder_parameters(&parameters)
    guard opj_setup_decoder(codec, &parameters) == 1 else { return nil }
    opj_codec_set_threads(codec, Int32(ProcessInfo.processInfo.activeProcessorCount))

    return data.withUnsafeBytes { raw in
        let reader = OpjReadStream(buffer: raw)
        return withExtendedLifetime(reader) { () -> (pixels: [UInt8], width: Int, height: Int, components: Int)? in
            guard let stream = opj_stream_default_create(1) else { return nil }
            defer { opj_stream_destroy(stream) }
            opj_stream_set_user_data(stream, Unmanaged.passUnretained(reader).toOpaque(), nil)
            // Without an explicit length the stream treats any unbuffered
            // skip as end-of-stream and tile reading fails.
            opj_stream_set_user_data_length(stream, UInt64(reader.count))
            opj_stream_set_read_function(stream, opjReadFn)
            opj_stream_set_skip_function(stream, opjSkipReadFn)
            opj_stream_set_seek_function(stream, opjSeekReadFn)

            var image: UnsafeMutablePointer<opj_image_t>? = nil
            guard opj_read_header(stream, codec, &image) == 1, let img = image else { return nil }
            defer { opj_image_destroy(img) }
            guard opj_decode(codec, stream, img) == 1 else { return nil }
            return opjInterleave(img.pointee)
        }
    }
}

/// Compresses interleaved 8-bit pixels into a JPEG 2000 image.
///
/// - Parameters:
///   - pixels: Interleaved samples, `width * height * components` bytes.
///   - width: Image width in pixels.
///   - height: Image height in pixels.
///   - components: Number of interleaved components (1 = grayscale,
///     3 = RGB, 4 = RGBA; the last component of a 2- or 4-component image is
///     tagged as alpha).
///   - format: `OPJ_CODEC_JP2` container (default) or `OPJ_CODEC_J2K` raw
///     codestream.
///   - quality: Omit for mathematically lossless compression (reversible 5/3
///     wavelet); pass 1–100 to compress lossily (irreversible 9/7) targeting
///     a 15–45 dB PSNR — 50 ≈ 30 dB, 85 ≈ 40.5 dB.
/// - Returns: The encoded image, or `nil` on failure.
public func opjCompress(
    _ pixels: [UInt8], _ width: Int, _ height: Int, _ components: Int,
    format: OPJ_CODEC_FORMAT = OPJ_CODEC_JP2, quality: Int? = nil
) -> [UInt8]? {
    guard width > 0, height > 0, components > 0,
        pixels.count == width * height * components
    else { return nil }
    if let quality {
        guard (1...100).contains(quality) else { return nil }
    }

    var parameters = opj_cparameters_t()
    opj_set_default_encoder_parameters(&parameters)
    parameters.tcp_numlayers = 1
    if let quality {
        parameters.irreversible = 1
        parameters.cp_fixed_quality = 1
        parameters.tcp_distoratio.0 = Float(30.0 + 0.3 * Double(quality - 50))
    } else {
        // A rate of 0 keeps every layer, which makes reversible coding bit-exact.
        parameters.tcp_rates.0 = 0
    }
    parameters.numresolution = Int32(min(6, Int(log2(Double(min(width, height)))) + 1))
    if components >= 3 {
        parameters.tcp_mct = 1
    }

    var cmptparms = (0..<components).map { _ -> opj_image_cmptparm_t in
        var p = opj_image_cmptparm_t()
        p.dx = 1
        p.dy = 1
        p.w = UInt32(width)
        p.h = UInt32(height)
        p.prec = 8
        p.sgnd = 0
        return p
    }
    let colorspace = components == 1 ? OPJ_CLRSPC_GRAY : OPJ_CLRSPC_SRGB
    guard let image = opj_image_create(UInt32(components), &cmptparms, colorspace) else { return nil }
    defer { opj_image_destroy(image) }
    image.pointee.x0 = 0
    image.pointee.y0 = 0
    image.pointee.x1 = UInt32(width)
    image.pointee.y1 = UInt32(height)
    if components == 2 || components == 4 {
        image.pointee.comps[components - 1].alpha = 1
    }
    pixels.withUnsafeBufferPointer { px in
        for c in 0..<components {
            guard let plane = image.pointee.comps[c].data else { return }
            var i = c
            var o = 0
            while i < px.count {
                plane[o] = Int32(px[i])
                i += components
                o += 1
            }
        }
    }

    guard let codec = opj_create_compress(format) else { return nil }
    defer { opj_destroy_codec(codec) }
    opj_set_info_handler(codec, { _, _ in }, nil)
    opj_set_warning_handler(codec, { _, _ in }, nil)
    opj_set_error_handler(codec, { _, _ in }, nil)
    guard opj_setup_encoder(codec, &parameters, image) == 1 else { return nil }
    opj_codec_set_threads(codec, Int32(ProcessInfo.processInfo.activeProcessorCount))

    let writer = OpjWriteStream()
    guard let stream = opj_stream_default_create(0) else { return nil }
    defer { opj_stream_destroy(stream) }
    opj_stream_set_user_data(stream, Unmanaged.passUnretained(writer).toOpaque(), nil)
    opj_stream_set_write_function(stream, opjWriteFn)
    opj_stream_set_skip_function(stream, opjSkipWriteFn)
    opj_stream_set_seek_function(stream, opjSeekWriteFn)

    let ok = withExtendedLifetime(writer) {
        opj_start_compress(codec, image, stream) == 1 && opj_encode(codec, stream) == 1
            && opj_end_compress(codec, stream) == 1
    }
    guard ok else { return nil }
    return writer.data
}
