import Foundation
import Testing
@testable import POLE

@Test
func testPOLE() throws {
    let path = Bundle.module.path(forResource: "Workbook", ofType: "xls") ?? // For test in SPM
               Bundle.module.path(forResource: "Workbook", ofType: "xls", inDirectory: "Resources") // For test in XCode
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
