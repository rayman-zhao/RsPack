import CMBL

public func scaleImage(_ srcRGB: [UInt8], _ srcWidth: Int, _ srcHeight: Int, _ destWidth: Int, _ destHeight: Int) -> [UInt8] {
    var destRGB = [UInt8](repeating: 0, count: destWidth * destHeight * 3)
    
    destRGB.withUnsafeMutableBytes { destBuf in
        srcRGB.withUnsafeBytes { srcBuf in
            ScaleImage(srcBuf.baseAddress, Int32(srcWidth), Int32(srcHeight), destBuf.baseAddress, Int32(destWidth), Int32(destHeight))
        }
    }

    return destRGB
}
