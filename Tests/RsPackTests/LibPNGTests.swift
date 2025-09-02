import Foundation
import Testing
import LibPNG

@Test
func testLibPNG() async throws {
    print("about libpng \(String(cString: png_get_copyright(nil)))")
}
