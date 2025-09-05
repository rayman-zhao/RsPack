import Foundation
import Testing
import RsHelper
import POLE

@Test
func testPOLE() throws {
    let path = Bundle.module.path(forResource: "Workbook.xls")
    try #require(path != nil, "No xls file found in \(Bundle.module.resourcePath!))")
    
    let s = CStorage(path)
    let ret = s.open()
    #expect(ret)
    
    let e = s.entries("/")
    #expect(e.size() > 0)

    e.forEach { e in
        print("\(e)")
    }
    
    s.close()
}
