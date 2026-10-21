import Foundation
import LibPNG
import Testing

@Test
func testLibPNG() async throws {
    print("about libpng \(String(cString: png_get_copyright(nil)))")
}

@Test
func testPngCompressRGB() throws {
    let width = 345, height = 678
    let rgb = [UInt8]((0..<width * height * 3).map { UInt8(($0 * 7 + 13) % 256) })

    // A padded pitch exercises the row-stride path.
    let rowBytes = width * 3
    let pitch = rowBytes + 3
    var padded = [UInt8](repeating: 0xFF, count: pitch * height)
    for row in 0..<height {
        let dst = row * pitch
        let src = row * rowBytes
        padded.replaceSubrange(dst..<dst + rowBytes, with: rgb[src..<src + rowBytes])
    }

    let png = pngCompress(padded, .rgb, width, height, pitch)
    #expect(png.starts(with: [0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A]))

    // PNG is lossless, so the round-trip must be byte-exact.
    let decoded = try #require(pngDecompress(png, .rgb))
    #expect(decoded.width == width && decoded.height == height)
    #expect(decoded.pixels == rgb)
}

@Test
func testPngCompressGray() throws {
    let width = 345, height = 678
    let row = (0..<width).map { UInt8(($0 * 255) / max(width - 1, 1)) }
    let gray = [UInt8]((0..<height).flatMap { _ in row })

    let png = pngCompress(gray, .gray, width, height)
    #expect(!png.isEmpty)

    let decoded = try #require(pngDecompress(png, .gray))
    #expect(decoded.width == width && decoded.height == height)
    #expect(decoded.pixels == gray)
}

@Test
func testPngCompressBGR() throws {
    let width = 64, height = 32
    let rgb = [UInt8]((0..<width * height * 3).map { UInt8(($0 * 7 + 13) % 256) })

    var bgr = rgb
    for px in 0..<(width * height) {
        let i = px * 3
        bgr[i] = rgb[i + 2]
        bgr[i + 2] = rgb[i]
    }

    let png = pngCompress(bgr, .bgr, width, height)

    // Reading back as RGB must undo the BGR channel order.
    let decoded = try #require(pngDecompress(png, .rgb))
    #expect(decoded.pixels == rgb)
}

@Test
func testPngCompressFlip() throws {
    let width = 8, height = 6
    // Each row is a uniform distinct value so the row order is directly visible.
    let bottomUp = [UInt8](
        (0..<height).flatMap { row in [UInt8](repeating: UInt8(row * 40), count: width) })

    let png = pngCompress(bottomUp, .gray, width, height, 0, true)

    let topDown = [UInt8](
        (0..<height).flatMap { row in
            [UInt8](repeating: UInt8((height - 1 - row) * 40), count: width)
        })
    let decoded = try #require(pngDecompress(png, .gray))
    #expect(decoded.pixels == topDown)
}

@Test
func testPngDecompressGrayAsRGB() throws {
    let width = 20, height = 10
    let row = (0..<width).map { UInt8(($0 * 255) / max(width - 1, 1)) }
    let gray = [UInt8]((0..<height).flatMap { _ in row })

    let png = pngCompress(gray, .gray, width, height)

    // The requested layout applies regardless of the source color type:
    // grayscale expands to R = G = B per pixel.
    let decoded = try #require(pngDecompress(png, .rgb))
    #expect(decoded.width == width && decoded.height == height)
    #expect(decoded.pixels == [UInt8](gray.flatMap { g in [g, g, g] }))
}

@Test
func testPngDecompressInvalidInput() {
    let corrupt = [UInt8](repeating: 0xFF, count: 100)
    #expect(pngDecompress(corrupt, .rgb) == nil)
}

@Test
func testPngCompressInvalidInput() {
    let rgb = [UInt8](repeating: 0, count: 100)
    #expect(pngCompress(rgb, .rgb, 0, 10).isEmpty)
    #expect(pngCompress(rgb, .rgb, 10, 10).isEmpty)
}
