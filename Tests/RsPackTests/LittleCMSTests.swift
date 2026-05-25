import Foundation
import Testing
import LittleCMS

@Test
func testLittleCMS() async throws {
    #expect(cmsGetEncodedCMMversion() >= 2190)

    let hsRGB = cmsCreate_sRGBProfile()
    let xform = cmsCreateTransform(hsRGB, CMS_TYPE_RGB_8, hsRGB, CMS_TYPE_RGB_8, cmsUInt32Number(INTENT_PERCEPTUAL), 0)

    let bufIn: [UInt8] = [255, 0, 0]
    var bufOut: [UInt8] = [0, 0, 0]
    cmsDoTransform(xform, bufIn, &bufOut, 1)
    #expect(bufIn == bufOut)

    cmsCloseProfile(hsRGB)
    cmsDeleteTransform(xform)
}
