@_exported import CLibTIFF
import Foundation
import LibJPEGTurbo

public typealias TIFFWarningHandler = (String, String) -> Void
public typealias TIFFErrorHandler = (String, String) -> Void

@MainActor
private var warningHandler: TIFFWarningHandler? = nil
@MainActor
private var errorHandler: TIFFErrorHandler? = nil

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

private func cErrorHandler(_ module: UnsafePointer<CChar>?, _ fmt: UnsafePointer<CChar>?, _ ap: CVaListPointer?) -> Void {
    guard let module, let fmt, let ap else { return }
    
    let md = String(cString: module)
    
    let bufSize = 256
    var buf = [CChar](repeating: 0, count: bufSize)
    // TODO: var span = buf.mutableSpan
    _ = vsnprintf(&buf, bufSize, fmt, ap)
    if let msg = String(utf8String: buf) {
        Task { @MainActor in
            errorHandler?(md, msg)
        }
    }
}

@MainActor
public func TIFFSetWarningHanlder(_ handler: @escaping TIFFWarningHandler) -> Void {
    warningHandler = handler
    TIFFSetWarningHandler(cWarningHandler)
}

@MainActor
public func TIFFSetErrorHanlder(_ handler: @escaping TIFFErrorHandler) -> Void {
    errorHandler = handler
    TIFFSetErrorHandler(cErrorHandler)
}

public func TIFFSetDirectory(_ tiff: OpaquePointer?, _ dirnum: UInt32, _ diroffset: UInt64? = nil) -> Bool {
    if let diroffset {
        guard TIFFCurrentDirOffset(tiff) != diroffset else { return true }
        guard TIFFCurrentDirectory(tiff) != dirnum else { return TIFFSetSubDirectory(tiff, diroffset) == 1 }
        return TIFFSetDirectory(tiff, dirnum) == 1 && TIFFSetSubDirectory(tiff, diroffset) == 1
    } else if dirnum == 0 {
        guard TIFFCurrentDirectory(tiff) != dirnum || TIFFCurrentDirOffset(tiff) != 8 else { return true }
        return TIFFSetDirectory(tiff, dirnum) == 1
    } else {
        guard TIFFCurrentDirectory(tiff) != dirnum else { return true }
        return TIFFSetDirectory(tiff, dirnum) == 1
    }
}

public func TIFFGetField<T>(_ tiff: OpaquePointer?, _ tag: Int32) -> T? {
    let pv = UnsafeMutablePointer<T>.allocate(capacity: 1)
    defer {
        pv.deallocate()
    }
    
    let success = withVaList([pv]) { args in
        return TIFFVGetField(tiff, UInt32(tag), args)
    }
    
    return success == 1 ? pv.pointee : nil
}

public func TIFFGetField<T, P>(_ tiff: OpaquePointer?, _ tag: Int32) -> (T?, P?) {
    let pv = UnsafeMutablePointer<T>.allocate(capacity: 1)
    defer {
        pv.deallocate()
    }
    let pv2 = UnsafeMutablePointer<P>.allocate(capacity: 1)
    defer {
        pv2.deallocate()
    }
    
    let success = withVaList([pv, pv2]) { args in
        return TIFFVGetField(tiff, UInt32(tag), args)
    }
    
    return success == 1 ? (pv.pointee, pv2.pointee) : (nil, nil)
}

public func TIFFSetField<T: CVarArg>(_ tiff: OpaquePointer?, _ tag: Int32, _ value: T) -> Bool {
    let success = withVaList([value]) { args in
        return TIFFVSetField(tiff, UInt32(tag), args)
    }
    return success == 1
}

public func TIFFSetField<T: CVarArg, P: CVarArg>(_ tiff: OpaquePointer?, _ tag: Int32, _ value: T, _ value2: P) -> Bool {
    let success = withVaList([value, value2]) { args in
        return TIFFVSetField(tiff, UInt32(tag), args)
    }
    return success == 1
}

public func TIFFReadJPEGImage(_ tiff: OpaquePointer?, _ dirnum: UInt32, _ diroffset: UInt64? = nil) -> [UInt8]? {
    guard TIFFSetDirectory(tiff, dirnum, diroffset) else { return nil }

    let tileWidth: UInt32? = TIFFGetField(tiff, TIFFTAG_TILEWIDTH)
    guard tileWidth == nil else { return nil }
    
    if let comp: UInt16 = TIFFGetField(tiff, TIFFTAG_COMPRESSION),
       comp == UInt16(COMPRESSION_JPEG),
       let photometric: UInt16 = TIFFGetField(tiff, TIFFTAG_PHOTOMETRIC),
       photometric == UInt16(PHOTOMETRIC_YCBCR) {
        let bufSize = Int(TIFFRawStripSize(tiff, 0))
        var buf = [UInt8](repeating: 0, count: bufSize)
        // TODO: var span = buf.mutableSpan
        let stripSize = Int(TIFFReadRawStrip(tiff, 0, &buf, tmsize_t(bufSize)))

        var jpeg = stripSize == bufSize ? buf : Array(buf[..<stripSize])

        if buf.prefix(4) == [0xFF, 0xD8, 0xFF, 0xC0] {
            let dqt: (count: UInt32?, data: UnsafeMutableRawPointer?) = TIFFGetField(tiff, TIFFTAG_JPEGTABLES)
            if let count = dqt.count, let data = dqt.data {
                // The DQT data is 0xFFDB...0xFFD9
                jpeg.replaceSubrange(0..<2, with: UnsafeBufferPointer(start: data.assumingMemoryBound(to: UInt8.self), count: Int(count) - 2)) 
            }
        }

        return jpeg
    }
    
    if let w: UInt32 = TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH),
       let h: UInt32 = TIFFGetField(tiff, TIFFTAG_IMAGELENGTH) {
        let bufSize = Int(w * h)
        var buf = [UInt32](repeating: 0, count: bufSize)
        // TODO: var span = buf.mutableSpan

        if TIFFReadRGBAImageOriented(tiff, w, h, &buf, ORIENTATION_TOPLEFT, 0) == 1 {
            return tjCompress(buf, TJPF_RGBA, Int(w), Int(h))
        }
    }
    
    return nil
}