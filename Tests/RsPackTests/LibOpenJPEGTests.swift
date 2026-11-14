import Foundation
import LibOpenJPEG
import Testing

private func gradientRGB(_ width: Int, _ height: Int, _ components: Int) -> [UInt8] {
    (0..<(width * height * components)).map { i in
        let p = i / components
        let c = i % components
        return UInt8(((p % width) * (c + 1) + (p / width) * 3) & 0xFF)
    }
}

@Test
func testOpjLosslessRoundtrip() throws {
    print("OpenJPEG version \(String(cString: opj_version()))")

    let width = 345, height = 678
    let rgb = gradientRGB(width, height, 3)

    let jp2 = try #require(opjCompress(rgb, width, height, 3))
    #expect(jp2.starts(with: [0x00, 0x00, 0x00, 0x0C, 0x6A, 0x50, 0x20, 0x20, 0x0D, 0x0A, 0x87, 0x0A]))

    let decoded = try #require(opjDecompress(jp2))
    #expect(decoded.width == width && decoded.height == height)
    #expect(decoded.components == 3)
    // Reversible 5/3 wavelet coding round-trips bit-exactly.
    #expect(decoded.pixels == rgb)
}

@Test
func testOpjJ2KGrayRoundtrip() throws {
    let width = 345, height = 678
    let row = (0..<width).map { UInt8($0 * 255 / max(width - 1, 1)) }
    let gray = [UInt8]((0..<height).flatMap { _ in row })

    let j2k = try #require(opjCompress(gray, width, height, 1, format: OPJ_CODEC_J2K))
    #expect(j2k.starts(with: [0xFF, 0x4F, 0xFF, 0x51]))

    let decoded = try #require(opjDecompress(j2k))
    #expect(decoded.width == width && decoded.height == height)
    #expect(decoded.components == 1)
    #expect(decoded.pixels == gray)
}

@Test
func testOpjRGBALosslessRoundtrip() throws {
    let width = 128, height = 64
    let rgba = gradientRGB(width, height, 4)

    let jp2 = try #require(opjCompress(rgba, width, height, 4))
    let decoded = try #require(opjDecompress(jp2))
    #expect(decoded.components == 4)
    #expect(decoded.pixels == rgba)
}

@Test
func testOpjSmallImageRoundtrip() throws {
    // 4×4 exercises the numresolution clamp for tiny images.
    let pixels = gradientRGB(4, 4, 3)
    let jp2 = try #require(opjCompress(pixels, 4, 4, 3))
    let decoded = try #require(opjDecompress(jp2))
    #expect(decoded.pixels == pixels)
}

@Test
func testOpjLossyQuality() throws {
    let width = 345, height = 678
    let rgb = gradientRGB(width, height, 3)

    let lossless = try #require(opjCompress(rgb, width, height, 3))
    let lossy = try #require(opjCompress(rgb, width, height, 3, quality: 50))
    #expect(lossy.count < lossless.count)

    let decoded = try #require(opjDecompress(lossy))
    #expect(decoded.width == width && decoded.height == height && decoded.components == 3)

    // quality 50 targets ~30 dB PSNR, i.e. RMSE ≈ 8; allow margin.
    let sqSum = zip(decoded.pixels, rgb).reduce(0) { $0 + abs(Int($1.0) - Int($1.1)) * abs(Int($1.0) - Int($1.1)) }
    #expect(sqrt(Double(sqSum) / Double(rgb.count)) <= 16)
}

@Test
func testOpjFailures() {
    #expect(opjDecompress([UInt8](repeating: 0xAB, count: 64)) == nil)
    #expect(opjDecompress([0xFF, 0x4F]) == nil)
    #expect(opjDecompress([0xFF, 0x4F, 0xFF, 0x51] + [UInt8](repeating: 0, count: 128)) == nil)

    #expect(opjCompress([0, 0, 0], 2, 2, 3) == nil)
    #expect(opjCompress(gradientRGB(4, 4, 3), 4, 4, 3, quality: 0) == nil)
    #expect(opjCompress(gradientRGB(4, 4, 3), 4, 4, 3, quality: 101) == nil)
}
