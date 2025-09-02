import Foundation
import Testing
@testable import POLE

@Test
func testPOLE() {
	let path = Bundle.main.path(forResource: "Workbook", ofType: "xls", inDirectory: "RsPack_RsPackTests.resources")
    let s = CStorage(path!)
    let ret = s.open()
    #expect(ret)
    
    let e = s.entries("/")
    #expect(e.size() > 0)

    e.forEach { e in
        print("\(e)")
    }
    
    s.close()
}
