@_exported import CLibTIFF
import Foundation
import LibJPEGTurbo

public typealias TIFFWarningHandler = (String, String) -> Void

@MainActor
private var warningHandler: TIFFWarningHandler? = nil

private func cWarningHandler(_ module: UnsafePointer<CChar>?, _ fmt: UnsafePointer<CChar>?, _ ap: CVaListPointer?) -> Void {
    guard let module, let fmt, let ap else { return }
    
    let md = String(cString: module)
    
    let bufSize = 256
    var buf = [CChar](repeating: 0, count: bufSize)
    // TODO: var span = buf.mutableSpan
    _ = vsnprintf(&buf, bufSize, fmt, ap)
    if let msg = String(utf8String: buf) {
        Task { @MainActor in
            warningHandler?(md, msg)
        }
    }
}

@MainActor
public func TIFFSetWarningHanlder(_ handler: @escaping TIFFWarningHandler) -> Void {
    warningHandler = handler
    TIFFSetWarningHandler(cWarningHandler)
}

public func TIFFSetDirectory(_ tif: OpaquePointer?, _ dirnum: UInt32) -> Bool {
    return TIFFCurrentDirectory(tif) == dirnum || TIFFSetDirectory(tif, dirnum) == 1
}

public func TIFFGetField<T>(_ tif: OpaquePointer?, _ tag: Int32) -> T? {
    let pv = UnsafeMutablePointer<T>.allocate(capacity: 1)
    defer {
        pv.deallocate()
    }
    
    let success = withVaList([pv]) { args in
        return TIFFVGetField(tif, UInt32(tag), args)
    }
    
    return success == 1 ? pv.pointee : nil
}

public func TIFFGetField<T, P>(_ tif: OpaquePointer?, _ tag: Int32) -> (T?, P?) {
    let pv = UnsafeMutablePointer<T>.allocate(capacity: 1)
    defer {
        pv.deallocate()
    }
    let pv2 = UnsafeMutablePointer<P>.allocate(capacity: 1)
    defer {
        pv2.deallocate()
    }
    
    let success = withVaList([pv, pv2]) { args in
        return TIFFVGetField(tif, UInt32(tag), args)
    }
    
    return success == 1 ? (pv.pointee, pv2.pointee) : (nil, nil)
}

public func TIFFReadJPEGImage(_ tif: OpaquePointer?, _ dirnum: UInt32) -> [UInt8] {
    guard TIFFSetDirectory(tif, dirnum) else { return [] }
    
    if let comp: UInt16 = TIFFGetField(tif, TIFFTAG_COMPRESSION),
       comp == UInt16(COMPRESSION_JPEG) {
        let bufSize = Int(TIFFRawStripSize(tif, 0))
        var buf = [UInt8](repeating: 0, count: bufSize)
        // TODO: var span = buf.mutableSpan
        let stripSize = Int(TIFFReadRawStrip(tif, 0, &buf, tmsize_t(bufSize)))

        var jpeg = stripSize == bufSize ? buf : Array(buf[..<stripSize])

        if buf.prefix(4) == [0xFF, 0xD8, 0xFF, 0xC0] {
            let dqt: (count: UInt32?, data: UnsafeMutableRawPointer?) = TIFFGetField(tif, TIFFTAG_JPEGTABLES)
            if let count = dqt.count, let data = dqt.data {
                // The DQT data is 0xFFDB...0xFFD9
                jpeg.replaceSubrange(0..<2, with: UnsafeBufferPointer(start: data.assumingMemoryBound(to: UInt8.self), count: Int(count) - 2)) 
            }
        }

        return jpeg
    }
    
    if let w: UInt32 = TIFFGetField(tif, TIFFTAG_IMAGEWIDTH),
       let h: UInt32 = TIFFGetField(tif, TIFFTAG_IMAGELENGTH) {
        let bufSize = Int(w * h)
        var buf = [UInt32](repeating: 0, count: bufSize)
        // TODO: var span = buf.mutableSpan

        if TIFFReadRGBAImageOriented(tif, w, h, &buf, ORIENTATION_TOPLEFT, 0) == 1 {
            return tjCompress(buf, TJPF_RGBA, Int(w), Int(h))
        }
    }
    
    return []
}