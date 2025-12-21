#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void ScaleImage(const unsigned char *srcRGB, int srcWidth, int srcHeight, unsigned char *destRGB, int destWidth, int destHeight);

#ifdef __cplusplus
}
#endif