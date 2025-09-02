import Foundation
import Testing
import LibJPEGTurbo

@Test
func testLibJPEGTurbo() async throws {
    let tj = tj3Init(Int32(TJINIT_COMPRESS.rawValue))
    tj3Destroy(tj)
    
    let tj2 = tj3Init(Int32(TJINIT_DECOMPRESS.rawValue))
    tj3Destroy(tj2)
}
