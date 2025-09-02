import Foundation
import Testing
import LibTIFF

@Test
func testLibTIFF() async throws {
    print("libtiff version \(String(cString: TIFFGetVersion()))")
    
    let path = Bundle.main.path(forResource: "TCGA-BR-4369-01Z-00-DX1", ofType: "svs", inDirectory: "RsPack_RsPackTests.resources")
    let f = TIFFOpen(path, "r")
    TIFFPrintDirectory(f, stdout, Int32(TIFFPRINT_NONE))
    TIFFClose(f)
}
