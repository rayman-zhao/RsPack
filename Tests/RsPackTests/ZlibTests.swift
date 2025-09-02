import Foundation
import Testing
import Zlib

@Test
func testZlib() async throws {
    print("zlib version \(String(cString: zlibVersion()))")
}
