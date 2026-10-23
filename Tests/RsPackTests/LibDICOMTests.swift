import Testing
import LibDICOM

@Test
func testLibDICOM() async throws {
    print("libdicom version \(String(cString: dcm_get_version()))")
}
