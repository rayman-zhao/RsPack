import CPOLE
import Foundation

public class Storage {
    public enum Result : Int32 {
        case ok
        case openFailed
        case notOLE
        case badOLE
        case unknownError
    }

    fileprivate let cstorage: UnsafeMutableRawPointer

    public init(_ path: String) {
        cstorage = CStorage_create(path)
    }

    deinit {
        CStorage_destroy(cstorage)
    }

    public func open() -> Bool {
        return CStorage_open(cstorage)
    }

    public func close() {
        CStorage_close(cstorage)
    }

    public func result() -> Result {
        return Result(rawValue: CStorage_result(cstorage))!
    }

    public func entries(_ path: String = "/") -> [String] {
        return consumeCStrings2SwiftStrings(CStorage_entries(cstorage, path))
    }

    public func isDirectory(_ name: String) -> Bool {
        return CStorage_isDirectory(cstorage, name);
    }

    public func exists(_ name: String) -> Bool {
        return CStorage_exists(cstorage, name);
    }

    public func getAllStreams(_ storageName: String) -> [String] {
        return consumeCStrings2SwiftStrings(CStorage_getAllStreams(cstorage, storageName))
    }
}

public class Stream {
    private let cstream: UnsafeMutableRawPointer

    public init(_ storage: Storage, _ name: String) {
        cstream = CStream_create(storage.cstorage, name)
    }

    deinit {
        CStream_destroy(cstream)
    }

    public func fullName() -> String {
        return consumeCString2SwiftString(CStream_fullName(cstream))
    }

    public func size() -> Int {
        return CStream_size(cstream)
    }

    public func read(_ maxlen: Int = Int.max) -> [UInt8] {
        let bufSize = min(size(), maxlen)
        guard bufSize > 0 else { return [] }

        var buf = [UInt8](repeating: 0, count: bufSize)
        let readSize = CStream_read(cstream, &buf, bufSize)

        return readSize == bufSize ? buf : Array(buf[..<readSize])
    }

    public func fail() -> Bool {
        return CStream_fail(cstream)
    }
}

fileprivate func consumeCStrings2SwiftStrings(_ cStrings: UnsafeMutablePointer<UnsafeMutablePointer<CChar>?>?) -> [String] {
    guard let cStrings else { return [] }
    
    var sStrings: [String] = []
    var index = 0
    while let cstr = cStrings[index] {
        if let sstr = String(utf8String: cstr) {
            sStrings.append(sstr)
        }
        free(cstr)
        index += 1
    }

    free(cStrings)
    return sStrings
}

fileprivate func consumeCString2SwiftString(_ cString: UnsafeMutablePointer<CChar>?) -> String {
    guard let cString else { return "" }
    
    let sString = String(utf8String: cString)
    free(cString)
    return sString ?? ""
}