#ifndef __BAYER_H__
#define __BAYER_H__

/**
 * @file
 *
 * @brief 包含Bayer图像处理的函数。
 */

namespace MBL
{
  namespace Image2D
  {
    /**
     * Bayer图像每个象素的格式。
     */
    typedef enum
    {
      BAYER_PIXEL_FORMAT_R_BGB_R,     ///**< 中间为G的象素格式。*/
      BAYER_PIXEL_FORMAT_RGR_GBG_RGR, ///**< 四周为G的象素格式。*/
      BAYER_PIXEL_FORMAT_B_RGR_B,     ///**< 中间为G的象素格式。*/
      BAYER_PIXEL_FORMAT_BGB_GRG_BGB  ///**< 四周为G的象素格式。*/
    } BayerPixelFormat;
    /**
     * Bayer转化是否做Flip或Mirror
     */
    typedef enum
    {
      BAYER_CONVERT_NORMAL = 0x00, /*普通的BAYER转化*/
      BAYER_CONVERT_FLIP = 0x01,   /*BAYER转化的同时做Flip操作*/
      BAYER_CONVERT_MIRROR = 0x02  /*BAYER转化的同时做Mirror操作*/
    } BayerConvertFlag;
    /**
     * @brief Bayer象素转换RGB格式的函数。
     * using the following 3x3 kernel interpolation:
     *
     *           R G R     R      B G B     B
     *            \|/.     |       \|/      |
     *           G-B-G   B-G1-B   G-R-G   R-G2-R
     *            /|\      |       /|\      |
     *           R G R     R      B G B     B
     *
     * @param p_in 输入的Bayer象素指针。
     * @param p_out 输出的RGB象素指针，必须是按RGB排列的彩色图像。
     * @param row Bayer图像宽度。
     * @param fmt Bayer象素格式。
     *
     * @deprecated 该函数已经过时，因为使用该函数时，发现实际不能被编译器内联，建议使用拆分的四个更短的内联函数。
     * @see _ConvertBayer2RGBPixel_R_BGB_R, _ConvertBayer2RGBPixel_RGR_GBG_RGR,
     *      _ConvertBayer2RGBPixel_B_RGR_B, _ConvertBayer2RGBPixel_BGB_GRG_BGB
     */
    template<class T>
    inline void _ConvertBayer2RGBPixel(T *p_in, T *p_out, int row, BayerPixelFormat fmt)
    {
      const int B_OFFSET = 2;
      const int G_OFFSET = 1;
      const int R_OFFSET = 0;

      switch (fmt)
      {
        case BAYER_PIXEL_FORMAT_R_BGB_R:
          *(p_out + B_OFFSET) = (*(p_in - 1) + *(p_in + 1)) >> 1;
          *(p_out + G_OFFSET) = *p_in;
          *(p_out + R_OFFSET) = (*(p_in - row) + *(p_in + row)) >> 1;
          break;
        case BAYER_PIXEL_FORMAT_RGR_GBG_RGR:
          *(p_out + B_OFFSET) = *p_in;
          *(p_out + G_OFFSET) = (*(p_in - 1) + *(p_in + 1) + *(p_in - row) + *(p_in + row)) >> 2;
          *(p_out + R_OFFSET) = (*(p_in - 1 - row) + *(p_in + 1 - row) + *(p_in - 1 + row) + *(p_in + 1 + row)) >> 2;
          break;
        case BAYER_PIXEL_FORMAT_B_RGR_B:
          *(p_out + B_OFFSET) = (*(p_in - row) + *(p_in + row)) >> 1;
          *(p_out + G_OFFSET) = *p_in;
          *(p_out + R_OFFSET) = (*(p_in - 1) + *(p_in + 1)) >> 1;
          break;
        case BAYER_PIXEL_FORMAT_BGB_GRG_BGB:
          *(p_out + B_OFFSET) = (*(p_in - 1 - row) + *(p_in + 1 - row) + *(p_in - 1 + row) + *(p_in + 1 + row)) >> 2;
          *(p_out + G_OFFSET) = (*(p_in - 1) + *(p_in + 1) + *(p_in - row) + *(p_in + row)) >> 2;
          *(p_out + R_OFFSET) = *p_in;
          break;
        default:
          break;
      }
    }

    /**
     * @brief 将 BAYER_PIXEL_FORMAT_R_BGB_R 象素格式的Bayer象素转换RGB格式的函数。
     *
     * @param p_in 输入的Bayer象素指针。
     * @param p_out 输出的RGB象素指针，必须是按RGB排列的彩色图像。
     * @param row Bayer图像宽度。
     *
     * @author 陈伟卿
     */
    template<class T>
    inline void _ConvertBayer2RGBPixel_R_BGB_R(T *p_in, T *p_out, int row)
    {
      *(p_out + 2) = (*(p_in - 1) + *(p_in + 1)) >> 1;
      *(p_out + 1) = (*p_in * 4 + *(p_in - row - 1) + *(p_in - row + 1) + *(p_in + row + 1) + *(p_in + row - 1)) >> 3;
      *(p_out + 0) = (*(p_in - row) + *(p_in + row)) >> 1;
    }

    /**
     * @brief 将 BAYER_PIXEL_FORMAT_RGR_GBG_RGR 象素格式的Bayer象素转换RGB格式的函数。
     *
     * @param p_in 输入的Bayer象素指针。
     * @param p_out 输出的RGB象素指针，必须是按RGB排列的彩色图像。
     * @param row Bayer图像宽度。
     *
     * @author 陈伟卿
     */
    template<class T>
    inline void _ConvertBayer2RGBPixel_RGR_GBG_RGR(T *p_in, T *p_out, int row)
    {
      *(p_out + 2) = *p_in;
      *(p_out + 1) = (*(p_in - 1) + *(p_in + 1) + *(p_in - row) + *(p_in + row)) >> 2;
      *(p_out + 0) = (*(p_in - 1 - row) + *(p_in + 1 - row) + *(p_in - 1 + row) + *(p_in + 1 + row)) >> 2;
    }

    /**
     * @brief 将 BAYER_PIXEL_FORMAT_B_RGR_B 象素格式的Bayer象素转换RGB格式的函数。
     *
     * @param p_in 输入的Bayer象素指针。
     * @param p_out 输出的RGB象素指针，必须是按RGB排列的彩色图像。
     * @param row Bayer图像宽度。
     *
     * @author 陈伟卿
     */
    template<class T>
    inline void _ConvertBayer2RGBPixel_B_RGR_B(T *p_in, T *p_out, int row)
    {
      *(p_out + 2) = (*(p_in - row) + *(p_in + row)) >> 1;
      *(p_out + 1) = (*p_in * 4 + *(p_in - row - 1) + *(p_in - row + 1) + *(p_in + row + 1) + *(p_in + row - 1)) >> 3;//*p_in;
      *(p_out + 0) = (*(p_in - 1) + *(p_in + 1)) >> 1;
    }

    /**
     * @brief 将 BAYER_PIXEL_FORMAT_BGB_GRG_BGB 象素格式的Bayer象素转换RGB格式的函数。
     *
     * @param p_in 输入的Bayer象素指针。
     * @param p_out 输出的RGB象素指针，必须是按RGB排列的彩色图像。
     * @param row Bayer图像宽度。
     *
     * @author 陈伟卿
     */
    template<class T>
    inline void _ConvertBayer2RGBPixel_BGB_GRG_BGB(T *p_in, T *p_out, int row)
    {
      *(p_out + 2) = (*(p_in - 1 - row) + *(p_in + 1 - row) + *(p_in - 1 + row) + *(p_in + 1 + row)) >> 2;
      *(p_out + 1) = (*(p_in - 1) + *(p_in + 1) + *(p_in - row) + *(p_in + row)) >> 2;
      *(p_out + 0) = *p_in;
    }

    /**
     * @brief 转换一幅Bayer图像到彩色图像。
     *
     * @param bayer Bayer索引图像，其宽度必须是偶数。
     * @param rgb 彩色图像，可以是RGB或者BGR格式，必须与Bayer图像同尺寸。
     * @param flag 转化时是否做Flip或(与)mirror,参见BayerConvertFlag定义
     *
     * @author 黄超 刘瑞北 赵宇 陈伟卿 陈德敏
     */
    template<class T>
    void ConvertBayer2Color(ImageDef<T> *bayer, ImageDef<T> *rgb, BayerConvertFlag flag = BAYER_CONVERT_NORMAL)
    {
      int bayer_row_units = bayer->Width;
      int rgb_row_units = rgb->Width * 3;
      T *p_in = bayer->Pixels + bayer_row_units + 1;
      T *p_out = rgb->Pixels + rgb_row_units + 3;
      int x_end = (bayer->Width - 2) / 2;
      int y_end = (bayer->Height - 2) / 2;
      int x, y;
      ImageFormat fmt = bayer->Format;

      int rgb_row_offset = rgb_row_units;
      int rgb_pixel_offset = 3;
      if (flag & BAYER_CONVERT_FLIP)
      {
        rgb_row_offset = -rgb_row_offset;
        p_out = rgb->Pixels + rgb_row_units * (rgb->Height - 2) + 3;
      }
      if (flag & BAYER_CONVERT_MIRROR)
      {
        rgb_pixel_offset = -rgb_pixel_offset;
        p_out += rgb_row_units + 3 * rgb_pixel_offset;
      }

      for (y = 0; y < y_end; y++)
      {
        T* p_out_line = p_out;
        for (x = 0; x < x_end; x++)
        {
          switch (fmt)
          {
            case IMAGE_FORMAT_BAYER_GR_BG:
              // 1 - 1
              _ConvertBayer2RGBPixel_R_BGB_R(p_in, p_out, bayer_row_units);
              // 1 - 2
              p_in++;
              p_out += rgb_pixel_offset;
              _ConvertBayer2RGBPixel_RGR_GBG_RGR(p_in, p_out, bayer_row_units);
              // 2 - 2
              p_in += bayer_row_units;
              p_out += rgb_row_offset;
              _ConvertBayer2RGBPixel_B_RGR_B(p_in, p_out, bayer_row_units);
              // 2 - 1
              p_in--;
              p_out -= rgb_pixel_offset;
              _ConvertBayer2RGBPixel_BGB_GRG_BGB(p_in, p_out, bayer_row_units);
              break;

            case IMAGE_FORMAT_BAYER_BG_GR:
              // 1 - 1
              _ConvertBayer2RGBPixel_BGB_GRG_BGB(p_in, p_out, bayer_row_units);
              // 1 - 2
              p_in++;
              p_out += rgb_pixel_offset;
              _ConvertBayer2RGBPixel_B_RGR_B(p_in, p_out, bayer_row_units);
              // 2 - 2
              p_in += bayer_row_units;
              p_out += rgb_row_offset;
              _ConvertBayer2RGBPixel_RGR_GBG_RGR(p_in, p_out, bayer_row_units);
              // 2 - 1
              p_in--;
              p_out -= rgb_pixel_offset;
              _ConvertBayer2RGBPixel_R_BGB_R(p_in, p_out, bayer_row_units);
              break;

            case IMAGE_FORMAT_BAYER_GB_RG:
              // 1 - 1
              _ConvertBayer2RGBPixel_B_RGR_B(p_in, p_out, bayer_row_units);
              // 1 - 2
              p_in++;
              p_out += rgb_pixel_offset;
              _ConvertBayer2RGBPixel_BGB_GRG_BGB(p_in, p_out, bayer_row_units);
              // 2 - 2
              p_in += bayer_row_units;
              p_out += rgb_row_offset;
              _ConvertBayer2RGBPixel_R_BGB_R(p_in, p_out, bayer_row_units);
              // 2 - 1
              p_in--;
              p_out -= rgb_pixel_offset;
              _ConvertBayer2RGBPixel_RGR_GBG_RGR(p_in, p_out, bayer_row_units);
              break;

            case IMAGE_FORMAT_BAYER_RG_GB:
              // 1 - 1
              _ConvertBayer2RGBPixel_RGR_GBG_RGR(p_in, p_out, bayer_row_units);
              // 1 - 2
              p_in++;
              p_out += rgb_pixel_offset;
              _ConvertBayer2RGBPixel_R_BGB_R(p_in, p_out, bayer_row_units);
              // 2 - 2
              p_in += bayer_row_units;
              p_out += rgb_row_offset;
              _ConvertBayer2RGBPixel_BGB_GRG_BGB(p_in, p_out, bayer_row_units);
              // 2 - 1
              p_in--;
              p_out -= rgb_pixel_offset;
              _ConvertBayer2RGBPixel_B_RGR_B(p_in, p_out, bayer_row_units);
              break;

            default:
              break;
          }
          // 到下一个象素。
          p_in += 2 - bayer_row_units;
          p_out += 2 * rgb_pixel_offset - rgb_row_offset;
        }
        // 到下一行开头。
        p_in += 2 + bayer_row_units;
        //p_out += 2*rgb_pixel_offset + rgb_row_offset*offset;
        p_out = p_out_line + rgb_row_offset * 2;
      }

      //border cases
      //2005-8-12: fill it in with the near pixel

      // First line & last line, 将第二行的数据写到第一行，将倒数第二行的数据写到最后一行
      int x_max = rgb->Width - 1;
      WriteRow(rgb, 0, x_max, 0, rgb->Pixels + rgb_row_units);
      WriteRow(rgb, 0, x_max, rgb->Height - 1, rgb->Pixels + rgb_row_units * (rgb->Height - 2));
      // Left & Right
      T *ptr = rgb->Pixels;
      int b = GetUnitsPerPixel(rgb);
      int b2 = b * 2;
      for (y = 0; y < rgb->Height; y++)
      {
        WritePixel(rgb, 0, y, ptr + b);
        ptr += rgb_row_units;
        WritePixel(rgb, x_max, y, ptr - b2);
      }
      
      if (rgb->Format == IMAGE_FORMAT_BGR) ExchangeBand(rgb, 0, 2);
    }

    /**
     * @brief 计算一幅Bayer图像的平均亮度。
     *
     * 该函数一般用于图像设备的自动曝光。
     *
     * @param image Bayer索引图像，目前只适用于8bit图像。
     * @return 该图像的平均亮度。
     *
     * @author 钟明亮 赵宇
     * @deprecated 该函数已经过时，请使用GetImageAverageBrightness函数，它可以处理多种格式的图像。
     * @see GetImageAverageBrightness
     */
    template<class T>
    T GetBayerAverageBrightness(ImageDef<T> *image)
    {
      long Avg = 0;
      T *p = image->Pixels;

      //Byer数据计算平均亮度。
      int x, y;
      int x_end = image->Width / 2, y_end = image->Height / 2;
      long R = 0, G = 0, B = 0;

      switch (image->Format)
      {
        case IMAGE_FORMAT_BAYER_GR_BG:
          for (y = 0; y < y_end; y++)
          {
            for (x = 0; x < x_end; x++)
            {
              G += *p++; //g分量
              R += *p++; //r分量
            }
            for (x = 0; x < x_end; x++)
            {
              B += *p++; //b分量
              G += *p++; //g分量
            }
          }
          break;
        case IMAGE_FORMAT_BAYER_BG_GR:
          for (y = 0; y < y_end; y++)
          {
            for (x = 0; x < x_end; x++)
            {
              B += *p++;
              G += *p++;
            }
            for (x = 0; x < x_end; x++)
            {
              G += *p++;
              R += *p++;
            }
          }
          break;
        case IMAGE_FORMAT_BAYER_GB_RG:
          for (y = 0; y < y_end; y++)
          {
            for (x = 0; x < x_end; x++)
            {
              G += *p++;
              B += *p++;
            }
            for (x = 0; x < x_end; x++)
            {
              R += *p++;
              G += *p++;
            }
          }
          break;
        case IMAGE_FORMAT_BAYER_RG_GB:
          for (y = 0; y < y_end; y++)
          {
            for (x = 0; x < x_end; x++)
            {
              R += *p++;
              G += *p++;
            }
            for (x = 0; x < x_end; x++)
            {
              G += *p++;
              B += *p++;
            }
          }
          break;
        default:
          break;
      }

      int n = image->Width * image->Height / 4;
      G /= 2 * n;
      B /= n;
      R /= n;

      Avg = (299 * R + 587 * G + 114 * B) / 1000;
      if (Avg > 255) Avg = 255;

      return static_cast<T> (Avg);
    }

    /**
     * @brief 去除Bayer图像中的坏点（Dead Pixel）。
     *
     * 该算法来自Kodak的一篇文章，称为one-line algorithm。
     *
     * @param image Bayer索引图像。
     * @param thrd 阈值，为20～60，一般可取30。
     *
     * @author 黄超 赵宇
     */
    template<class T>
    void BadBayerPixelCorrection(ImageDef<T> *image, int thrd)
    {
      thrd = Utility::Clamp(thrd, 20, 60);

      int L2, L1, R1, R2;
      T *ptr = image->Pixels + 4;

      int w = image->Width - 8;
      int h = image->Height;

      for (int i = 0; i < h; i++)
      {
        for (int j = 0; j < w; j++)
        {
          L2 = (*ptr) - (*(ptr - 4));
          L1 = (*ptr) - (*(ptr - 2));
          R1 = (*ptr) - (*(ptr + 2));
          R2 = (*ptr) - (*(ptr + 4));

          if ((L2 > thrd) && (R2 > thrd) && (R1 > thrd) && (L1 > thrd))
          {
            (*ptr) = Utility::GetMax((*(ptr + 2)), (*(ptr - 2)));
          }
          ptr++; // Next piexl
        }
        ptr += 8; // To Next Line, -----Pixels offset
      }
    }

    /**
     * @brief 获取 Binning 后的 Bayer 图像
     *
     * @param img   Bayer索引图像，处理完后，指针指向结果图像。
     * @param idx   要 Binning 的索引值，取 1时图像缩小1倍，取2时图像缩小4倍。
     *
     * @author 陈伟卿
     */
    template<class T>
    void GetBinningImage(ImageDef<T> *img, int idx)
    {
      if (idx <= 0) return;
      T *src = img->Pixels;
      T *dst = img->Pixels;

      int width = img->Width;
      int zoom = 1 << idx;
      img->Width = img->Width / zoom;
      img->Height = img->Height / zoom;
      int wid = img->Width; //最后图像的宽度
      int hei = img->Height; //最后图像的高度

      hei = hei / 2;
      for (int i = 0; i < hei; i++)
      {
        //一次处理两行
        for (int j = 0; j < wid; j++)
        {
          *dst++ = *src++;
          *dst++ = *src++;
          src = src + 2 * (zoom - 1);
        }
        src = src + 2 * (zoom - 1) * width;
      }
    }

    /**
     * @brief Bayer图像增强,使用锐化算法补尝因BAYER转RGB/BGR图像时引起的图像模糊
     * 
     * @param image Bayer索引图像,或0用于释放临时临时缓冲区见attention，目前只适用于8bit图像。
     * 
     * @attention 调用该函数会产生临时内存缓冲区,当不再使用后请以0为参数调用一次以释放该临时缓冲区
     *
     * @author 陈德敏
     */
    template<class T>
    void BayerEnhance(ImageDef<T> *image, int c_o = -96, int c_c = 640, int c_a= 12)
    {
      //static const int c_o = -96;//增强系数*255
      //static const int c_c = 640;
      //static const int c_a = 12;
      static unsigned char*tempBuf = 0;
      static unsigned int tempSize = 0;
      if (image == 0)//释放临时缓冲
      {
        if (tempBuf)
        {
          delete[] tempBuf;
          tempBuf = 0;
        }
        tempSize = 0;
        return;
      }
      if (tempBuf == 0 || tempSize < image->Width * image->Height * sizeof(T))
      {
        if (tempBuf)
        {
          delete[] tempBuf;
          tempBuf = 0;
        }
        tempSize = image->Width * image->Height * sizeof(T);
        tempBuf = new unsigned char[tempSize];
      }

      T* temp = (T*) tempBuf;
      if (temp == 0) return;

      long maxValue = ((1 << (sizeof(T) * 8)) - 1);

      int width = image->Width;
      int height = image->Height;
      memcpy(temp, image->Pixels, image->Width * image->Height * sizeof(T));

      T* p1, *p2, *p3, *p4;
      int pitch = width * 2;
      p1 = temp;
      p2 = temp + 1;
      p3 = temp + width;
      p4 = temp + width + 1;
      p1 += pitch + 2;
      p2 += pitch + 2;
      p3 += pitch + 2;
      p4 += pitch + 2;
      long u, d, l, r, x, a1, a2;
      T* pL1, *pL2, *pL3, *pL4;

      for (int j = 2; j < height / 2 - 2; j++)
      {
        pL1 = p1;
        pL2 = p2;
        pL3 = p3;
        pL4 = p4;
        for (int i = 2; i < width / 2 - 2; i++)
        {
          u = *(p1 - pitch);
          d = *(p1 + pitch);
          l = *(p1 - 2);
          r = *(p1 + 2);
          a1 = u - d;
          a2 = r - l;
          if (a1 > c_a || a1 < -c_a || a2 > c_a || a2 < -c_a)
          {
            x = ((c_o * (u + d + l + r) + (*(p1)) * c_c) >> 8);
            if (x < 0) x = 0;
            if (x > maxValue) x = maxValue;
            image->Pixels[p1 - temp] = x;
          }
          p1 += 2;

          u = *(p2 - pitch);
          d = *(p2 + pitch);
          l = *(p2 - 2);
          r = *(p2 + 2);
          a1 = u - d;
          a2 = r - l;
          if (a1 > c_a || a1 < -c_a || a2 > c_a || a2 < -c_a)
          {
            x = ((c_o * (u + d + l + r) + (*(p2)) * c_c) >> 8);
            if (x < 0) x = 0;
            if (x > maxValue) x = maxValue;
            image->Pixels[p2 - temp] = x;
          }
          p2 += 2;

          u = *(p3 - pitch);
          d = *(p3 + pitch);
          l = *(p3 - 2);
          r = *(p3 + 2);
          a1 = u - d;
          a2 = r - l;
          if (a1 > c_a || a1 < -c_a || a2 > c_a || a2 < -c_a)
          {
            x = ((c_o * (u + d + l + r) + (*(p3)) * c_c) >> 8);
            if (x < 0) x = 0;
            if (x > maxValue) x = maxValue;
            image->Pixels[p3 - temp] = x;
          }
          p3 += 2;

          u = *(p4 - pitch);
          d = *(p4 + pitch);
          l = *(p4 - 2);
          r = *(p4 + 2);
          a1 = u - d;
          a2 = r - l;
          if (a1 > c_a || a1 < -c_a || a2 > c_a || a2 < -c_a)
          {
            x = ((c_o * (u + d + l + r) + (*(p4)) * c_c) >> 8);
            if (x < 0) x = 0;
            if (x > maxValue) x = maxValue;
            image->Pixels[p4 - temp] = x;
          }
          p4 += 2;
        }
        p1 = pL1 + pitch;
        p2 = pL2 + pitch;
        p3 = pL3 + pitch;
        p4 = pL4 + pitch;
      }
    }
  }
}

#endif // __BAYER_H__
