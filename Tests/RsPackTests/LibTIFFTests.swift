import Foundation
import Testing
import RsHelper
import LibTIFF

@Test
func testLibTIFF() async throws {
    print("libtiff version \(String(cString: TIFFGetVersion()))")
    
    let path = Bundle.module.path(forResource: "TCGA-BR-4369-01Z-00-DX1.svs")
    try #require(path != nil, "No svs file found in \(Bundle.module.resourcePath!))")
    
    let f = TIFFOpen(path, "r")
    TIFFPrintDirectory(f, stdout, 0)
    TIFFClose(f)
}
