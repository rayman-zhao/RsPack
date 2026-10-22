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

    let png = try #require(pngCompress(padded, .rgb, width, height, pitch))
    #expect(png.starts(with: [0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A]))

    // PNG is lossless, so the round-trip must be byte-exact.
    var pixels = [UInt8](repeating: 0, count: width * height * 3)
    #expect(pngDecompress(png, &pixels, .rgb))
    #expect(pixels == rgb)

    // A padded destination pitch must place each row at the strided offset.
    let dstPitch = rowBytes + 3
    var pitched = [UInt8](repeating: 0xFF, count: dstPitch * height)
    #expect(pngDecompress(png, &pitched, .rgb, dstPitch))
    var strided = [UInt8](repeating: 0, count: rowBytes * height)
    for row in 0..<height {
        let dst = row * dstPitch
        let src = row * rowBytes
        strided.replaceSubrange(src..<src + rowBytes, with: pitched[dst..<dst + rowBytes])
    }
    #expect(strided == rgb)
    #expect(Array(pitched[rowBytes..<dstPitch]) == [0xFF, 0xFF, 0xFF])
}

@Test
func testPngCompressGray() throws {
    let width = 345, height = 678
    let row = (0..<width).map { UInt8(($0 * 255) / max(width - 1, 1)) }
    let gray = [UInt8]((0..<height).flatMap { _ in row })

    let png = try #require(pngCompress(gray, .gray, width, height))

    var pixels = [UInt8](repeating: 0, count: width * height)
    #expect(pngDecompress(png, &pixels, .gray))
    #expect(pixels == gray)
}

@Test
func testPngCompress2() throws {
    let width = 7, height = 5
    let rowBytes = width * 3
    let rgb = [UInt8]((0..<width * height * 3).map { UInt8(($0 * 11 + 5) % 256) })

    // A padded pitch exercises the row-stride path of the low-level writer.
    let pitch = rowBytes + 2
    var padded = [UInt8](repeating: 0xA5, count: pitch * height)
    for row in 0..<height {
        let dst = row * pitch, src = row * rowBytes
        padded.replaceSubrange(dst..<(dst + rowBytes), with: rgb[src..<(src + rowBytes)])
    }

    let png = try #require(pngCompress2(padded, .rgb, width, height, pitch))
    var pixels = [UInt8](repeating: 0, count: rowBytes * height)
    #expect(pngDecompress(png, &pixels, .rgb))
    #expect(pixels == rgb)

    // Bottom-up source rows come out top-down.
    let flipped = try #require(pngCompress2(rgb, .rgb, width, height, 0, true))
    var out = [UInt8](repeating: 0, count: rowBytes * height)
    #expect(pngDecompress(flipped, &out, .rgb))
    var reversed = [UInt8]()
    for row in 0..<height {
        let src = (height - 1 - row) * rowBytes
        reversed.append(contentsOf: rgb[src..<(src + rowBytes)])
    }
    #expect(out == reversed)
}

@Test
func testPngCompressBGRA() throws {
    let width = 64, height = 32
    let rgba = [UInt8]((0..<width * height * 4).map { UInt8(($0 * 7 + 13) % 256) })

    var bgra = rgba
    for px in 0..<(width * height) {
        let i = px * 4
        bgra[i] = rgba[i + 2]
        bgra[i + 2] = rgba[i]
    }

    let png = try #require(pngCompress2(bgra, .bgra, width, height))

    var pixels = [UInt8](repeating: 0, count: width * height * 4)
    #expect(pngDecompress(png, &pixels, .rgba))
    #expect(pixels == rgba)
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

    let png = try #require(pngCompress(bgr, .bgr, width, height))

    // Reading back as RGB must undo the BGR channel order.
    var pixels = [UInt8](repeating: 0, count: width * height * 3)
    #expect(pngDecompress(png, &pixels, .rgb))
    #expect(pixels == rgb)
}

@Test
func testPngCompressFlip() throws {
    let width = 8, height = 6
    // Each row is a uniform distinct value so the row order is directly visible.
    let bottomUp = [UInt8](
        (0..<height).flatMap { row in [UInt8](repeating: UInt8(row * 40), count: width) })

    let png = try #require(pngCompress(bottomUp, .gray, width, height, 0, true))

    let topDown = [UInt8](
        (0..<height).flatMap { row in
            [UInt8](repeating: UInt8((height - 1 - row) * 40), count: width)
        })
    var pixels = [UInt8](repeating: 0, count: width * height)
    #expect(pngDecompress(png, &pixels, .gray))
    #expect(pixels == topDown)
}

@Test
func testPngDecompressHeader() throws {
    let width = 345, height = 678
    let rgb = [UInt8](repeating: 128, count: width * height * 3)
    let png = try #require(pngCompress(rgb, .rgb, width, height))

    let (w, h) = pngDecompressHeader(png)
    #expect(w == width && h == height)

    let corrupt = [UInt8](repeating: 0xFF, count: 100)
    let (cw, ch) = pngDecompressHeader(corrupt)
    #expect(cw == -1 && ch == -1)
}

@Test
func testPngDecompressGrayAsRGB() throws {
    let width = 20, height = 10
    let row = (0..<width).map { UInt8(($0 * 255) / max(width - 1, 1)) }
    let gray = [UInt8]((0..<height).flatMap { _ in row })

    let png = try #require(pngCompress(gray, .gray, width, height))

    // The requested layout applies regardless of the source color type:
    // grayscale expands to R = G = B per pixel.
    var pixels = [UInt8](repeating: 0, count: width * height * 3)
    #expect(pngDecompress(png, &pixels, .rgb))
    #expect(pixels == [UInt8](gray.flatMap { g in [g, g, g] }))
}

@Test
func testPngDecompressAtOffset() throws {
    // Two 4x2 gray tiles stitched side by side into an 8x2 layer: decoding
    // at an offset with a layer-wide pitch must place the rows in situ, the
    // way tj3Decompress8 places JPEG tiles.
    let pngL = try #require(pngCompress([UInt8]([10, 11, 12, 13, 14, 15, 16, 17]), .gray, 4, 2))
    let pngR = try #require(pngCompress([UInt8]([20, 21, 22, 23, 24, 25, 26, 27]), .gray, 4, 2))

    var layer = [UInt8](repeating: 0, count: 8 * 2)
    #expect(pngDecompress(pngL, &layer[0], .gray, 8))
    #expect(pngDecompress(pngR, &layer[4], .gray, 8))
    #expect(layer == [10, 11, 12, 13, 20, 21, 22, 23, 14, 15, 16, 17, 24, 25, 26, 27])
}

@Test
func testPngDecompressInvalidInput() {
    var pixels = [UInt8](repeating: 0, count: 100)

    let corrupt = [UInt8](repeating: 0xFF, count: 100)
    #expect(!pngDecompress(corrupt, &pixels, .rgb))
}
