import Foundation
import Testing
import RsHelper
import POLE

@Test
func testPOLE() throws {
    let path = Bundle.module.path(forResource: "Workbook.xls")
    try #require(path != nil, "No xls file found in \(Bundle.module.resourcePath!))")
    
    let s = Storage(path!)
    let ret = s.open()
    #expect(ret)
    #expect(s.result() == .ok)
    
    let e = s.entries("/")
    #expect(e.count > 0)
    #expect(s.isDirectory("Workbook") == false)
    #expect(s.exists("Workbook"))

    e.forEach { e in
        print("\(e)")
    }

    let streams = s.getAllStreams("/")
    #expect(streams.count == e.count)

    let stream = Stream(s, "/Workbook")
    #expect(stream.fullName() == "/Workbook")
    #expect(stream.size() > 0)
    #expect(stream.read(1).count == 1)
    #expect(stream.read().count > 1)
    #expect(stream.fail() == false)
    
    s.close()
}
