import Foundation
import LibPNG
import Testing

@Test
func testLibPNG() async throws {
    print("about libpng \(String(cString: png_get_copyright(nil)))")
}
