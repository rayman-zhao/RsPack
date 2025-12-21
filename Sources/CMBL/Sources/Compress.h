#ifndef __COMPRESS_H__
#define __COMPRESS_H__

/**
 * @file
 *
 * @brief 图像压缩函数的头文件。
 */

namespace MBL
{
  namespace Image2D
  {
    extern int EncodeImageAsCMP(ImageDef<unsigned char> *image, unsigned char **buf, short int q_factor);
    extern ImageDef<unsigned char> * DecodeImageAsCMP(unsigned char *buf);
  }
}
#endif //__COMPRESS_H__
