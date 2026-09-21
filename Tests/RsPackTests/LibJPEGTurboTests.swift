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
    let jpg = tjCompress(rgb, TJPF_RGB, 345, 678)

    let (w, h) = tjDecompressHeader(jpg)
    #expect(w == 345 && h == 678)
}

@Test
func testTjRotate() throws {
    let rgb = [UInt8](repeating: 128, count: 345 * 678 * 3)
    let jpg = tjCompress(rgb, TJPF_RGB, 345, 678)
    #expect(!jpg.isEmpty)

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
