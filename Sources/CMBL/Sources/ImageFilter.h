#ifndef __IMAGEFILTER_H__
#define __IMAGEFILTER_H__

/**
 * @file
 *
 * @brief 包含图像卷积滤波的函数。
 */

namespace MBL
{
  namespace Image2D
  {
    /**
     * @brief 5×5自定义滤波。
     *
     * 该函数改写自许俊的IShop::UserFilter函数。
     *
     * @param image 源图像，处理后该图像会被更新。
     * @param core 5×5卷积核。
     * @param div 除数。
     * @param bias 偏移量。
     * @param sub_area 处理子区。
     */
    template <class T>
    void CustomFilterImage(ImageDef<T> *image, ImageSubArea *sub_area, int core[5][5], int div, int bias)
    {
      T max_T;
      MBL::Utility::GetMaxValue(&max_T);
      int b = GetUnitsPerPixel(image), rb = GetUnitsPerRow(image);
      ImageDef<T> *nimage = DuplicateImage(image);
      int x1 = 0, y1 = 0;
      int core_v;
      T *p;

      int left, top, right, bottom;
      if (sub_area == 0)
      {
        left = 0;
        top = 0;
        right = image->Width;
        bottom = image->Height;
      }
      else
      {
        left = sub_area->Left;
        top = sub_area->Top;
        right = sub_area->Left + sub_area->Width;
        bottom = sub_area->Top + sub_area->Height;
      }

      for (int y = top; y < bottom; y++)
      {
        for (int x = left; x < right; x++)
        {
          if (sub_area == 0 || sub_area->IsFill(x, y))
          {
            float total[8];
            memset(total, 0, sizeof(total));

            for (int dy = -2; dy < 3; dy++)
            {
              y1 = MBL::Utility::Clamp(y + dy, 0, image->Height - 1);
              for (int dx = -2; dx < 3; dx++)
              {
                x1 = MBL::Utility::Clamp(x + dx, 0, image->Width - 1);
                core_v = core[dy + 2][dx + 2];
                p = image->Pixels + y1 * rb + x1 * b;
                for (int i = 0; i < b; i++)
                {
                  total[i] += core_v * p[i];
                }
              }
            }

            p = nimage->Pixels + y * rb + x * b;
            for (int i = 0; i < b; i++)
            {
              total[i] /= div;
              total[i] += bias;
              total[i] = MBL::Utility::Clamp(total[i], (float)0, (float)max_T);

              p[i] = (T)total[i];
            }
          }
        }
      }

      CopyImage(image, nimage);
      delete nimage;
    }

    /**
     * @brief 对图像进行马赛克处理。
     *
     * @param image 欲处理的图像。
     * @param block 马赛克块的大小（正方形的高和宽象素）。
     */
    template <class T>
    void MosaicImage(ImageDef<T> *image, int block)
    {
      if (image == 0) throw NullPointerException();

      T buf[8], max_T;
      MBL::Utility::GetMaxValue(&max_T);
      int buf1[8];
      int row, col;
      int b = GetUnitsPerPixel(image);
      int block2 = block * block;
      for (int y = 0; y < image->Height; y += block)
      {
        for (int x = 0; x < image->Width; x += block)
        {
          memset(buf1, 0, sizeof(buf1));
          for (int dy = 0; dy < block; dy++)
          {
            row = MBL::Utility::Clamp(y + dy, 0, image->Height - 1);
            for (int dx = 0; dx < block; dx++)
            {
              col = MBL::Utility::Clamp(x + dx, 0, image->Width - 1);

              ReadPixel(image, col, row, buf);
              for (int i = 0; i < b; i++)
              {
                buf1[i] += buf[i];
              }
            }
          }

          for (int i = 0; i < b; i++)
          {
            buf[i] = MBL::Utility::Clamp(buf1[i] / block2, 0, (int)max_T);
          }

          for (int dy = 0; dy < block; dy++)
          {
            row = MBL::Utility::Clamp(y + dy, 0, image->Height - 1);
            for (int dx = 0; dx < block; dx++)
            {
              col = MBL::Utility::Clamp(x + dx, 0, image->Width - 1);
              WritePixel(image, col, row, buf);
            }
          }
        }
      }
    }

    /**
     * @brief 取窗口中灰度值最小和最大的象素填充当前象素，可以起到膨胀或腐蚀深色区域的作用。
     *
     * @param image 欲处理的图像。目前只处理RGB格式图像。
     * @param sub_area 子区，为0表示处理全图。
     * @param block 窗口块大小（象素）。
     * @param type true 表示取灰度最小值，即膨胀深色区域，false 表示取灰度最大值，即腐蚀深色区域。
     */
    template <class T>
    void ErodeExpandImage(MBL::Image2D::ImageDef<T> *image, ImageSubArea *sub_area, int block, bool type)
    {
      if (image->Format != IMAGE_FORMAT_RGB) throw UnsupportedFormatException();

      ImageDef<T> *nimage = DuplicateImage(image);

      int left, top, right, bottom;
      if (sub_area == 0)
      {
        left = 0;
        top = 0;
        right = image->Width;
        bottom = image->Height;
      }
      else
      {
        left = sub_area->Left;
        top = sub_area->Top;
        right = sub_area->Left + sub_area->Width;
        bottom = sub_area->Top + sub_area->Height;
      }

      T final_gray, preset_gray, gray;
      if (type == true)
      {
        MBL::Utility::GetMaxValue(&preset_gray);
      }
      else
      {
        preset_gray = 0;
      }
      T buf[3], final_buf[3];
      int ld = -block / 2, rd = ld + block;
      int x1, y1;
      for (int y = top; y < bottom; y++)
      {
        for (int x = left; x < right; x++)
        {
          if (sub_area == 0 || sub_area->IsFill(x, y))
          {
            final_gray = preset_gray;
            ReadPixel(image, x, y, final_buf);
            for (int dy = ld; dy < rd; dy++)
            {
              y1 = MBL::Utility::Clamp(y + dy, 0, image->Height - 1);
              for (int dx = ld; dx < rd; dx++)
              {
                x1 = MBL::Utility::Clamp(x + dx, 0, image->Width - 1);
                ReadPixel(image, x1, y1, buf);
                gray = (299 * buf[0] + 587 * buf[1] + 114 * buf[2]) / 1000;
                if ((type == true && gray < final_gray) || (type == false && gray > final_gray))
                {
                  final_gray = gray;
                  final_buf[0] = buf[0];
                  final_buf[1] = buf[1];
                  final_buf[2] = buf[2];
                }
              }
            }
            if (final_gray != preset_gray) WritePixel(nimage, x, y, final_buf);
          }
        }
      }

      CopyImage(image, nimage);
      delete nimage;
    }

    /**
     * @brief 取窗口中灰度值中间值象素填充当前象素，可以起到平滑去噪的作用。
     *
     * @param image 欲处理的图像。目前只处理RGB格式图像。
     * @param sub_area 子区，为0表示处理全图。
     * @param block 窗口块大小（象素）。
     */
    template <class T>
    void MiddleValueFilterImage(MBL::Image2D::ImageDef<T> *image, ImageSubArea *sub_area, int block)
    {
      if (image->Format != IMAGE_FORMAT_RGB) throw UnsupportedFormatException();

      ImageDef<T> *nimage = DuplicateImage(image);

      int left, top, right, bottom;
      if (sub_area == 0)
      {
        left = 0;
        top = 0;
        right = image->Width;
        bottom = image->Height;
      }
      else
      {
        left = sub_area->Left;
        top = sub_area->Top;
        right = sub_area->Left + sub_area->Width;
        bottom = sub_area->Top + sub_area->Height;
      }

      int block2 = block * block;
      T *buf = new T[block2 * 3], *gray = new T[block2];
      int ld = -block / 2, rd = ld + block;
      int x1, y1;
      int i, j, k, l;
      for (int y = top; y < bottom; y++)
      {
        for (int x = left; x < right; x++)
        {
          if (sub_area == 0 || sub_area->IsFill(x, y))
          {
            for (int dy = ld; dy < rd; dy++)
            {
              y1 = MBL::Utility::Clamp(y + dy, 0, image->Height - 1);
              for (int dx = ld; dx < rd; dx++)
              {
                x1 = MBL::Utility::Clamp(x + dx, 0, image->Width - 1);
                i = (dy - ld) * block + (dx - ld);
                j = i * 3;
                ReadPixel(image, x1, y1, &buf[j]);
                gray[i] = (299 * buf[j] + 587 * buf[j + 1] + 114 * buf[j + 2]) / 1000;
              }
            }
            for (i = 0; i < block2 - 1; i++)
            {
              for (j = i + 1; j < block2; j++)
              {
                if (gray[i] > gray[j])
                {
                  MBL::Utility::Swap(&gray[i], &gray[j]);
                  k = i * 3;
                  l = j * 3;
                  MBL::Utility::Swap(&buf[k], &buf[l]);
                  MBL::Utility::Swap(&buf[k + 1], &buf[l + 1]);
                  MBL::Utility::Swap(&buf[k + 2], &buf[l + 2]);
                }
              }
            }
            WritePixel(nimage, x, y, &buf[block2 / 2 * 3]);
          }
        }
      }

      CopyImage(image, nimage);
      delete nimage;
    }

    /**
     * @brief 对图像进行平滑或锐化处理。
     *
     * 该函数移植自MVideo中的算法。
     *
     * @param image 欲处理的图像。目前只处理RGB格式图像。
     * @param sharpness 处理效果，为-10~10的整数。负数表示平滑图像，正数表示锐化图像，0表示不做任何处理。
     *
     * @author 陈进
     */
    template <class T>
    void SharpenImage(MBL::Image2D::ImageDef<T> *image, int sharpness)
    {
      T *lpImageData = image->Pixels;
      int xsize = image->Width;
      int ysize = image->Height;

      int i,j,k;
      int i1,i2,i3,j1;
      int itemp;
      int s1,s2,s3;
      int modein,modeout;

      //if (bits != 24) return; // currently only deal with 24 bits image
      if (sharpness == 0) return; // no sharpeness
      //if ( sharpeness < -10 || sharpeness > 10) return; // range -10 ~ 10

      if (xsize < 4 || xsize > 8196) return; // range of image resolution
      if (ysize < 5 || ysize > 8196) return; // range of image resolution
      //if (lpImageData == NULL) return;  // image data pointer

      if (sharpness > 0) // sharpen
      {
        s1 = -sharpness;
        s2 = -s1*4+8;
        s3 = 3;
      }
      else // smooth
      {
        s1 = -sharpness;
        if (s1 > 8) s1 = 8;
        s2 = 32-4*s1;
        s3 = 5;
      }

      static thread_local T *line = nullptr;
      static thread_local int xsize_3 = -1;

      if (xsize_3 != xsize * 3)
      {
        MBL::Utility::SafeRelease(&line); // Lost some memory trades for fast process
        xsize_3 = xsize * 3;
        line = new T[xsize_3 * 5]; // temp results
      }
      memcpy(line, lpImageData, xsize_3 * 5); // init line, at least 5px height

      int i2_l, i2_r;
      T *pi1, *pi2, *pi3, *pi2_l, *pi2_r, *pj1;

      for(i=0;i<ysize+2;i++)
      {
        modein = (MBL::Utility::GetMin(i, ysize - 1) % 5) * xsize_3;
        modeout = (MBL::Utility::GetMax(i - 2, 0) % 5) * xsize_3;

        i1 = MBL::Utility::GetMax(i - 2, 0)*xsize_3;
        i2 = MBL::Utility::GetMin(i, ysize - 1)*xsize_3;
        i3 = MBL::Utility::GetMin(i + 2, ysize - 1)*xsize_3;
        j1 = modein;

        pi1 = lpImageData + i1;
        pi2 = lpImageData + i2;
        pi3 = lpImageData + i3;
        pj1 = line + j1;

        for(j=0;j<xsize;j++)
        {
          i2_l = j >= 2 ? i2 - 6 : i2;
          i2_r = j < xsize -2 ? i2 + 6 : i2;

          pi2_l = lpImageData + i2_l;
          pi2_r = lpImageData + i2_r;

          for (k = 0; k < 3; ++k)
          {
              itemp = ((*pi2++) * s2 + ((*pi1++) + (*pi3++) + (*pi2_l++) + (*pi2_r++)) * s1) >> s3;
              *pj1++ = MBL::Utility::ClampFast(itemp, 0, 255);

              i2++;
          }
        } // end of j
        memcpy(&lpImageData[MBL::Utility::GetMax(i - 2, 0)*xsize_3], &line[modeout], xsize_3);
      } // end of i
    }

    /**
     * @brief 对图像进行浮雕处理。
     *
     * 该函数移植自MVideo中的算法。
     *
     * @param image 欲处理的图像。目前只处理RGB格式图像。
     *
     * @author 陈进
     */
    template <class T>
    void EmbossImage(ImageDef<T> *image)
    {
      if (image->Format != IMAGE_FORMAT_RGB) throw UnsupportedFormatException();

      T grey, grey2, diff;
      T *p = image->Pixels;
      for (int y = 0, h = image->Height; y < h; ++y)
      {
        grey2 = (*p) / 2 + (*(p + 1)) / 2;
        *p++ = ImageDefTraits<T>::MidValueRoundUp;
        *p++ = ImageDefTraits<T>::MidValueRoundUp;
        *p++ = ImageDefTraits<T>::MidValueRoundUp;
        for (int x = 1, w = image->Width; x < w; ++x)
        {
          grey = (*p) / 2 + (*(p + 1)) / 2;
          if (grey >= grey2)
          {
            diff = grey - grey2;
            if (diff > ImageDefTraits<T>::MidValueRoundDown) diff = ImageDefTraits<T>::MidValueRoundDown;
            diff += ImageDefTraits<T>::MidValueRoundUp;
          }
          else
          {
            diff = grey2 - grey;
            if (diff > ImageDefTraits<T>::MidValueRoundDown) diff = ImageDefTraits<T>::MidValueRoundDown;
            diff = ImageDefTraits<T>::MidValueRoundUp - diff;
          }
          *p++ = diff;
          *p++ = diff;
          *p++ = diff;
          grey2 = grey;
        }
      }
    }
  }
}

#endif // __IMAGEFILTER_H__
