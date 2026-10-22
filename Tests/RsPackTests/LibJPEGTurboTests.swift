import Foundation
import LibJPEGTurbo
import Testing

@Test
func testLibJPEGTurbo() async throws {
    let tj = tj3Init(Int32(TJINIT_COMPRESS.rawValue))
    tj3Destroy(tj)

    let tj2 = tj3Init(Int32(TJINIT_DECOMPRESS.rawValue))
    tj3Destroy(tj2)

    let rgb = [UInt8](repeating: 0, count: 345 * 678 * 3)
    let jpg = try #require(tjCompress(rgb, TJPF_RGB, 345, 678))

    let (w, h) = tjDecompressHeader(jpg)
    #expect(w == 345 && h == 678)
}

@Test
func testTjCompressGray() throws {
    let width = 345, height = 678
    // Smooth horizontal gradient: a pattern with per-pixel or wrapped-row
    // jumps would be worst-case for the DCT and blur the round-trip check.
    let row = (0..<width).map { UInt8(($0 * 255) / max(width - 1, 1)) }
    let gray = [UInt8]((0..<height).flatMap { _ in row })

    let jpg = try #require(tjCompress(gray, TJPF_GRAY, width, height))

    let (w, h) = tjDecompressHeader(jpg)
    #expect(w == width && h == height)

    // The output must be a single-component grayscale JPEG that round-trips
    // back to the source pixels within lossy-compression tolerance.
    let tj = tj3Init(Int32(TJINIT_DECOMPRESS.rawValue))
    defer { tj3Destroy(tj) }

    var dst = [UInt8](repeating: 0, count: width * height)
    let result = jpg.withUnsafeBytes { src -> (decompressed: Bool, colorspace: Int32) in
        guard tj3DecompressHeader(tj, src.baseAddress, src.count) == 0 else { return (false, -1) }
        let colorspace = tj3Get(tj, TJPARAM_COLORSPACE.rawValue)
        let decompressed = dst.withUnsafeMutableBytes {
            tj3Decompress8(tj, src.baseAddress, src.count, $0.baseAddress, 0, TJPF_GRAY.rawValue) == 0
        }
        return (decompressed, colorspace)
    }
    #expect(result.decompressed)
    #expect(result.colorspace == TJCS_GRAY.rawValue)

    let maxError = zip(gray, dst).map { abs(Int($0) - Int($1)) }.max() ?? 0
    #expect(maxError <= 2)
}

@Test
func testTjRotate() throws {
    let rgb = [UInt8](repeating: 128, count: 345 * 678 * 3)
    let jpg = try #require(tjCompress(rgb, TJPF_RGB, 345, 678))

    let clockwise = try #require(tjRotate(jpg, degrees: 90))
    let (cw, ch) = tjDecompressHeader(clockwise)
    #expect(cw == 678 && ch == 345)

    let counterClockwise = try #require(tjRotate(jpg, degrees: -90))
    let (ccw, cch) = tjDecompressHeader(counterClockwise)
    #expect(ccw == 678 && cch == 345)

    let fullTurn = try #require(tjRotate(jpg, degrees: 360))
    let (fw, fh) = tjDecompressHeader(fullTurn)
    #expect(fw == 345 && fh == 678)

    // Invalid degrees and untransformable data both fail with nil.
    #expect(tjRotate(jpg, degrees: 45) == nil)
    let corrupt = [UInt8](repeating: 0xFF, count: 100)
    #expect(tjRotate(corrupt, degrees: 90) == nil)
}
