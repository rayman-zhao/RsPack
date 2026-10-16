import CMBL

public func scaleImage(
    _ srcPixels: [UInt8], _ srcWidth: Int, _ srcHeight: Int, _ destWidth: Int, _ destHeight: Int,
    channels: Int = 3
) -> [UInt8] {
    precondition(channels == 1 || channels == 3 || channels == 4, "channels must be 1, 3, or 4")
    var destPixels = [UInt8](repeating: 0, count: destWidth * destHeight * channels)

    destPixels.withUnsafeMutableBytes { destBuf in
        srcPixels.withUnsafeBytes { srcBuf in
            ScaleImage(
                srcBuf.baseAddress, Int32(srcWidth), Int32(srcHeight), destBuf.baseAddress,
                Int32(destWidth), Int32(destHeight), Int32(channels))
        }
    }

    return destPixels
}
