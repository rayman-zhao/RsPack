@_exported import CPOLE
import Foundation

extension CStream {
    public func readAll() -> Data {
        let bufSize = Int(self.size())
        var buf = [UInt8](repeating: 0, count: bufSize)
        // TODO: var span = buf.mutableSpan
        let readSize = Int(self.read(&buf, UInt64(bufSize)))
        
        return Data(buf[..<readSize])
    }
}
