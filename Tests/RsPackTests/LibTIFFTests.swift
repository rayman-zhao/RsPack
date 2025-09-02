import Foundation
import Testing
import LibTIFF

@Test
func testLibTIFF() async throws {
    print("libtiff version \(String(cString: TIFFGetVersion()))")
    
//    let path = "/Users/zhaoyu/Downloads/2312399.svs"
//    let f = TIFFOpen(path, "r")
//    TIFFPrintDirectory(f, stdout, Int(TIFFPRINT_NONE))
//    TIFFClose(f)
}
