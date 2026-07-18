import Foundation
import MBL
import Testing

@Test
func testMBL() async throws {
    let width = 42496
    let height = 45056
    let img = [UInt8](repeating: 255, count: width * height * 3)
    let img2 = scaleImage(img, width, height, 512, 512)
    #expect(img2.count == 512 * 512 * 3)
}
