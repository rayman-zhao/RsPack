#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void ScaleImage(const unsigned char *srcPixels, int srcWidth, int srcHeight, unsigned char *destPixels, int destWidth, int destHeight, int channels);

#ifdef __cplusplus
}
#endif