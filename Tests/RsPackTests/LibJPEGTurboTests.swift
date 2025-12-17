import Foundation
import Testing
import LibJPEGTurbo

@Test
func testLibJPEGTurbo() async throws {
    let tj = tj3Init(Int32(TJINIT_COMPRESS.rawValue))
    tj3Destroy(tj)
    
    let tj2 = tj3Init(Int32(TJINIT_DECOMPRESS.rawValue))
    tj3Destroy(tj2)

    let rgb = [UInt8](repeating: 0, count: 345 * 678 * 3)
    let jpg = tjCompress(rgb, TJPF_RGB, 345, 678);
    let (w, h) = tjDecompressHeader(jpg)
    #expect(w == 345 && h == 678)
}
