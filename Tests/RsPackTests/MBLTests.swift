import Foundation
import MBL
import Testing

@Test
func testMBL() async throws {
    let width = 42496
    let height = 45056
    let img = [UInt8](repeating: 255, count: width * height * 3)
    let img2 = scaleImage(
        srcPixels: img, srcWidth: width, srcHeight: height, destWidth: 512, destHeight: 512)
    #expect(img2.count == 512 * 512 * 3)
}

@Test
func testScaleImageGray() throws {
    let width = 8
    let height = 8
    // Bilinear scaling to the source dimensions degenerates to a straight
    // copy, so a gradient must round-trip byte-for-byte through channels: 1.
    let row = (0..<width).map { UInt8(($0 * 255) / max(width - 1, 1)) }
    let gray = [UInt8]((0..<height).flatMap { _ in row })

    let same = scaleImage(
        srcPixels: gray, srcWidth: width, srcHeight: height, destWidth: width, destHeight: height,
        channels: 1)
    #expect(same == gray)

    // Bilinear interpolation preserves a constant field exactly.
    let flat = [UInt8](repeating: 137, count: 64 * 64)
    let small = scaleImage(
        srcPixels: flat, srcWidth: 64, srcHeight: 64, destWidth: 8, destHeight: 8, channels: 1)
    #expect(small.count == 8 * 8)
    #expect(small.allSatisfy { $0 == 137 })
}
