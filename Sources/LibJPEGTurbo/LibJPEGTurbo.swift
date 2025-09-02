@_exported import CLibJPEGTurbo
import Foundation

public func tjGetErrorStr(_ handle: tjhandle?) -> String {
    return String(cString: tj3GetErrorStr(handle))
}

public func tjCompress(_ srcBuf: UnsafePointer<UInt32>, _ width: Int, _ height: Int  ) -> Data {
    let tj = tj3Init(Int32(TJINIT_COMPRESS.rawValue))
    defer { tj3Destroy(tj) }
    tj3Set(tj, Int32(TJPARAM_QUALITY.rawValue), 85)
    tj3Set(tj, Int32(TJPARAM_SUBSAMP.rawValue), TJSAMP_420.rawValue)
    
    var jpegBuf: UnsafeMutablePointer<UInt8>? = nil
    defer { tj3Free(jpegBuf) }
    var jpegSize: Int = 0
    if tj3Compress8(tj, srcBuf, Int32(width), 0, Int32(height), TJPF_RGBA.rawValue, &jpegBuf, &jpegSize) == 0 {
        return Data(bytes: jpegBuf!, count: jpegSize)
    }
    else {
        return Data()
    }
}
