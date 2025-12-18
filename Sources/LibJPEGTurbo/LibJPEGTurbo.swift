@_exported import CLibJPEGTurbo
import Foundation

public func tjGetErrorStr(_ handle: tjhandle?) -> String {
    return String(cString: tj3GetErrorStr(handle))
}

public func tjCompress<T>(_ srcBuf: [T], _ pixelFormat: TJPF, _ width: Int, _ height: Int, _ pitch: Int = 0) -> [UInt8] {
    let tj = tj3Init(Int32(TJINIT_COMPRESS.rawValue))
    defer { tj3Destroy(tj) }
    tj3Set(tj, Int32(TJPARAM_QUALITY.rawValue), 85)
    tj3Set(tj, Int32(TJPARAM_SUBSAMP.rawValue), TJSAMP_420.rawValue)
    
    var jpegBuf: UnsafeMutablePointer<UInt8>? = nil
    defer { tj3Free(jpegBuf) }
    var jpegSize: Int = 0
    
    return srcBuf.withUnsafeBytes { buf in
        guard tj3Compress8(tj, buf.baseAddress, Int32(width), Int32(pitch), Int32(height), pixelFormat.rawValue, &jpegBuf, &jpegSize) == 0 else { return [] }

        return Array(UnsafeBufferPointer(start: jpegBuf, count: jpegSize))
    }
}

public func tjDecompressHeader(_ jpegBuf: [UInt8]) -> (w: Int, h: Int) {
    let tj = tj3Init(Int32(TJINIT_DECOMPRESS.rawValue))
    defer { tj3Destroy(tj) }

    return jpegBuf.withUnsafeBytes { buf in
        guard tj3DecompressHeader(tj, buf.baseAddress, buf.count) == 0 else { return (-1, -1) }

        return (Int(tj3Get(tj, TJPARAM_JPEGWIDTH.rawValue)), Int(tj3Get(tj, TJPARAM_JPEGHEIGHT.rawValue)))
    }
}