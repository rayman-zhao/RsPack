#ifndef __IMAGETRANSFORM_H__
#define __IMAGETRANSFORM_H__

/**
 * @file
 *
 * @brief 本文件包含一些二维图像几何变换函数，如平移、镜像、旋转、缩放等。
 */

#include <memory.h>

namespace MBL
{
  namespace Image2D
  {
    ///将整个给定的图像垂直翻转。
    /**
     * 用户输入一个图像，该函数将整个图像垂直翻转，即第一行与最后一行对调。执行结束后该图像在内存中的数据已经变化。
     *
     * @param image 欲处理的图像结构指针，必须包含有效的内存。
     *
     * @author 赵宇
     */
    template <class T>
    void FlipImage(ImageDef<T> *image)
    {
      int w = GetUnitsPerRow(image);
      int wb = GetBytesPerRow(image);
      T *temp = new T[w];
      T *buf1 = image->Pixels,
        *buf2 = image->Pixels + w * (image->Height - 1);

      for (int i = 0, h = image->Height / 2; i < h; ++i, buf1 += w, buf2 -= w)
      {
          memcpy(temp, buf1, wb);
          memcpy(buf1, buf2, wb);
          memcpy(buf2, temp, wb);
      }

      delete [] temp;
    }

    ///将给定的图像垂直翻转。
    /**
     * 用户输入一个图像，该函数将图像垂直翻转，即第一行与最后一行对调。执行结束后该图像在内存中的数据已经变化。
     *
     * @param image 欲处理的图像结构指针，必须包含有效的内存。
     * @param sub_area 该图像的子区指针。为0，表示没有子区，全图处理。
     *
     * @author 赵宇
     */
    template <class T>
    void FlipImage(ImageDef<T> *image, ImageSubArea *sub_area)
    {
      /*if (image->Pixels == 0) throw NullPointerException();

      if (sub_area != 0)
      {
        ImageDef<T> *img = CutImage(image, sub_area->Left, sub_area->Top, sub_area->Width, sub_area->Height);
        FlipImage(img);

        int b = GetUnitsPerPixel(image);
        for (int y1 = sub_area->Top, y2 = 0; y2 < img->Height; y1++, y2++)
        {
          for (int x1 = sub_area->Left, x2 = 0; x2 < img->Width; x1++, x2++)
          {
            if (sub_area->IsFill(x1, y1))
            {
              T *p1 = &(image->Pixels[(x1 + y1 * image->Width) * b]),
                *p2 = &(img->Pixels[(x2 + y2 *img->Width) * b]);

              for (int i = 0; i < b; i++)
              {
                *p1++ = *p2++;
              }
            }
          }
        }

        delete img;
      }
      else
      {
        FlipImage(image);
      }*/
    }

    ///将整个给定的图像水平镜像。
    /**
     * 用户输入一个图像，该函数将整个图像做水平镜像处理。执行结束后该图像在内存中的数据已经变化。
     *
     * @param image 欲处理的图像结构指针，必须包含有效的内存。
     *
     * @author 赵宇
     */
    template <class T>
    void MirrorImage(ImageDef<T> *image)
    {
      T buf1[8], buf2[8]; //足够任何图像类型数据的缓冲区。
      int hw = image->Width / 2;

      for (int y = 0; y < image->Height; y++)
      {
        for (int x1 = 0, x2 = image->Width - 1; x1 < hw; x1++, x2--)
        {
          ReadPixel(image, x1, y, buf1);
          ReadPixel(image, x2, y, buf2);
          WritePixel(image, x2, y, buf1);
          WritePixel(image, x1, y, buf2);
        }
      }
    }

    /**
     * @brief 逆时针旋转图像90。
     *
     * @param image 源图像。
     * @return 旋转后的图像，该图像在堆中创建，客户使用完毕后请删除。
     */
    template <class T>
    ImageDef<T> * RotateImageAnticlockwise90Deg(ImageDef<T> *image)
    {
      ImageDef<T> *rimage = ImageDef<T>::CreateSameFormatInstance(image, image->Height, image->Width);
      T buf[8]; //足够任何图像类型数据的缓冲区。

      for (int y = 0; y < image->Height; y++)
      {
        for (int x = 0; x < image->Width; x++)
        {
          ReadPixel(image, x, y, buf);
          WritePixel(rimage, y, image->Width - 1 - x, buf);
        }
      }

      return rimage;
    }

    /**
     * @brief 顺时针旋转图像90。
     *
     * @param image 源图像。
     * @return 旋转后的图像，该图像在堆中创建，客户使用完毕后请删除。
     */
    template <class T>
    ImageDef<T> * RotateImageClockwise90Deg(ImageDef<T> *image)
    {
      ImageDef<T> *rimage = ImageDef<T>::CreateSameFormatInstance(image, image->Height, image->Width);
      T buf[8]; //足够任何图像类型数据的缓冲区。

      for (int y = 0; y < image->Height; y++)
      {
        for (int x = 0; x < image->Width; x++)
        {
          ReadPixel(image, x, y, buf);
          WritePixel(rimage, image->Height - 1 -y, x, buf);
        }
      }

      return rimage;
    }

    /// 将图像转由按行4字节对齐的格式转换为真实数据格式。
    /**
     * 对于位图文件，以及Windows系统显示的内存图像，都必须是按行4字节对齐的格式，不足的象素用0补全。该函数将一幅图像
     * 的内存指针重新分配，驱除每行多余的字节，只保留真实数据。对于已经为真实数据的图像，该函数不会产生副作用。该函数
     * 执行结束后，图像内存中的数据已经改变。
     *
     * 对于非单字节（如16位）的图像，该函数没有经过测试，使用时请注意。
     *
     * @param image 欲处理的图像结构，必须包含有效的内存。
     *
     * @see ConvertImage2Aligned
     *
     * @author 袁天云 赵宇
     */
    template <class T>
    void ConvertImage2Nonaligned(ImageDef<T> *image)
    {
      if (image->Pixels == 0) throw NullPointerException();

      int b = GetBytesPerPixel(image);

      if ((image->Width * b) % 4 != 0)
      {
        int newbyte = (image->Width * b * sizeof(T) + 3) / 4 * 4;
        int number = image->Width * b * sizeof(T);
        T *pdata = image->Pixels;
        T *pdatanew = new T[image->Width * b * image->Height];
        if (pdatanew == 0) throw OutOfMemoryException();
        T *pstart = pdata;
        T *pnewstart = pdatanew;

        for (int i = 0; i < image->Height; i++)
        {
          memcpy(pdatanew, pdata, number);
          pdata += newbyte;
          pdatanew += number;
        }

        delete [] pstart;
        image->Pixels = pnewstart;
      }
    }

    /// 将图像转由真实数据格式转换为按行4字节对齐的格式。
    /**
     * 对于位图文件，以及Windows系统显示的内存图像，都必须是按行4字节对齐的格式，不足的象素用0补全。该函数将一幅图像
     * 的内存指针重新分配，以使其按行4字节对齐。对于已经按行对齐的图像，该函数不会产生副作用。该函数执行结束后，图像
     * 内存中的数据已经改变。
     *
     * 对于非单字节（如16位）的图像，该函数没有经过测试，使用时请注意。
     *
     * @param image 欲处理的图像结构，必须包含有效的内存。
     *
     * @see ConvertImage2Nonaligned
     *
     * @author 袁天云 赵宇
     */
    template <class T>
    void ConvertImage2Aligned(ImageDef<T> *image)
    {
      if (image->Pixels == 0) throw NullPointerException();

      int b = GetBytesPerPixel(image);

      if ((image->Width * b) % 4 != 0)
      {
        int newbyte = (image->Width * b * sizeof(T) + 3) / 4 * 4;
        int number = image->Width * b * sizeof(T);
        int n = image->Height * newbyte;
        T *pdatanew = new T[n];
        if (pdatanew == 0) throw OutOfMemoryException();
        memset(pdatanew, 0, n);
        T *pdata = image->Pixels;
        T *pnewstart = pdatanew;
        T *pstart = pdata;

        for (int i = 0; i < image->Height; i++)
        {
          memcpy(pdatanew, pdata, number);
          pdatanew += newbyte;
          pdata += number;
        }

        delete [] pstart;
        image->Pixels = pnewstart;
      }
    }

    /// 将24位真彩色图象转化成单通道(红、绿、蓝)图象。
    /**
     * 处理真彩图时提取单波段亮度信息，参数band 取 1 传入时产生的是红色波段图象，取 2和3 传入时则分别是绿色和蓝色
     * 波段图象。注意 single 图象在函数内部分配内存，记得用完后释放。
     *
     * 对于非单字节（如16位）的图像，该函数没有经过测试，使用时请注意。
     *
     * @param color 源图，为真彩色图象，必须包含有效的内存。
     *
     * @param band 波段值取值为1，2或3。
     *
     * @return 目标图，转化后生成的单通道图象
     *
     * @author 袁天云 韩冬冰
     */
    template <class T>
    ImageDef<T> * ConvertTruecolortoSingle(ImageDef<T> *color, int band)
    {
      int longth,i;
      int setof;
      T *pcolor = 0,*psingle = 0;

      if (color->Pixels == 0 ) throw NullPointerException();
      if(color->Format!=IMAGE_FORMAT_RGB&&color->Format!=IMAGE_FORMAT_BGR ) throw IllegalArgumentException();

      ImageDef<T> *single = ImageDef<T>::CreateInstance(IMAGE_FORMAT_INDEX, color->Width, color->Height);

      switch (band)
      {
        case 1://red channel
          if( color->Format == IMAGE_FORMAT_RGB )
          {
             setof = 0;
          }
          else if( color->Format == IMAGE_FORMAT_BGR)
          {
             setof = 2;
          }
          break;
        case 2://green band
          setof = 1;
          break;
        case 3://blue band
          if( color->Format == IMAGE_FORMAT_RGB )
          {
            setof = 2;
          }
          else if( color->Format == IMAGE_FORMAT_BGR )
          {
            setof = 0;
          }
          break;
        default:
          throw IllegalArgumentException();
        }


        pcolor  = color->Pixels;
        psingle = single->Pixels;
        longth  = color->Width * color->Height;

        pcolor += setof;
        for(i = 0; i < longth; i++)
        {
          *psingle = *pcolor;
          psingle += 1;
          pcolor  += 3;
        }

        return single;
     }

    /// 将单通道(红、绿、蓝)图象合成为24位真彩色BGR图象。
    /**
     * 注意 color 图象在函数内部分配内存，记得用完后释放。
     *
     * @param R 红色分量源图，为索引图像格式，必须包含有效的内存。
     *
     * @param G 绿色分量源图，为索引图像格式，必须包含有效的内存。
     *
     * @param B 蓝色分量源图，为索引图像格式，必须包含有效的内存。
     *
     * @return 目标图，转化后生成的BGR图像。
     *
     * @author 韩冬冰
     */
    template <class T>
    ImageDef<T> * ConvertSingletoBGR(ImageDef<T> *R, ImageDef<T> *G, ImageDef<T> *B)
    {
      int i, j, j2, k;
      T *pcolor = 0,*pR = 0, *pG = 0, *pB = 0;
      int nr = R->Height;
      int nc = R->Width;

      if (R->Pixels == 0 || G->Pixels == 0 || B->Pixels == 0) throw NullPointerException();

      if(R->Format!=IMAGE_FORMAT_INDEX || G->Format!=IMAGE_FORMAT_INDEX || G->Format!=IMAGE_FORMAT_INDEX ) throw IllegalArgumentException();

      ImageDef<T> *color = ImageDef<T>::CreateInstance(IMAGE_FORMAT_BGR, nc, nr);
      pcolor = color->Pixels;
      pR = R->Pixels;
      pG = G->Pixels;
      pB = B->Pixels;

      for (i = 0; i < nr; i++, pcolor += 3 * nc, pR += nc, pG += nc, pB += nc)
      {
        for (j = 0, j2 = 0; j < nc; j++, j2++)
        {
           k = 3 * j;
           pcolor[k] = pB[j2];
           pcolor[k + 1] = pG[j2];
           pcolor[k + 2] = pR[j2];
        }
      }

      return color;
    }

    /// 3线性变换函数。
    /**
     * @param i1
     * @param i2
     * @param i3
     * @param i4
     * @param u3
     * @param u2
     * @param u
     * @return
     */
    inline double _ThreeLinearTrans(double i1, double i2, double i3, double i4, double u3, double u2, double u)
    {
      double tab;
      tab = (i4 - i3 + i2 - i1) * u3;
      tab -= (i4 - i3 + 2.0 * i2 - 2.0 * i1) * u2;
      tab += (i3 - i1) * u + i2;

      return tab;
    }

    /// 用双线性算法缩放一幅图像。
    /**
     * @param image1 源图像。
     * @param dest_width 欲缩放的图像宽度（象素）。
     * @param dest_height 欲缩放的图像高度（象素）。
     * @return 缩放后的图像，使用完毕后请用delete删除。
     */
    template <class T>
    ImageDef<T> * ScaleImage2Linear(ImageDef<T> *image, int dest_width, int dest_height)
    {
      ImageDef<T> *ret = ImageDef<T>::CreateSameFormatInstance(image, dest_width, dest_height);

      const int sw = image->Width - 1, sh = image->Height - 1, dw = ret->Width - 1, dh = ret->Height - 1;
      const int nPixelSize = GetUnitsPerPixel(ret);
      const int nSrcRowSize = GetUnitsPerRow(image);
      const int nDestRowSize = GetUnitsPerRow(ret);

      int B, N, x, y;
      T *pLinePrev, *pLineNext;
      T *pDest;
      T *pA, *pB, *pC, *pD;

      for (int i = 0; i <= dh; ++i)
      {
        pDest = ret->Pixels + i * nDestRowSize;
        y = i * sh / dh;
        N = dh - i * sh % dh;
        pLinePrev = image->Pixels + (y++) * nSrcRowSize;
        pLineNext = (N == dh) ? pLinePrev : image->Pixels + y * nSrcRowSize;

        for (int j = 0; j <= dw; ++j)
        {
          x = j * sw / dw * nPixelSize;
          B = dw - j * sw % dw;
          pA = pLinePrev + x;
          pB = pA + nPixelSize;
          pC = pLineNext + x;
          pD = pC + nPixelSize;
          if (B == dw)
          {
            pB = pA;
            pD = pC;
          }

          for (int k = 0; k < nPixelSize; ++k)
          {
            *pDest++ = MBL::Utility::Clamp(
            (B * N * (*pA - *pB - *pC + *pD) + dw * N * (*pB) + dh * B * (*pC) + (dw * dh - dh * B - dw * N) * (*pD) + dw * dh / 2)
            / (dw * dh),
            ImageDefTraits<T>::MinValue, ImageDefTraits<T>::MaxValue);
            pA++; pB++; pC++; pD++;
          }
        }
      }

      return ret;
    }

    /// 用3线性算法缩放一幅图像。
    /**
     * @param image1 源图像。
     * @param dest_width 欲缩放的图像宽度（象素）。
     * @param dest_height 欲缩放的图像高度（象素）。
     * @return 缩放后的图像，使用完毕后请用delete删除。
     */
    template <class T>
    ImageDef<T> * ScaleImage3Linear(ImageDef<T> *image1, int dest_width, int dest_height)
    {
      if (dest_width <= 0 || dest_height <= 0) throw IllegalArgumentException();
      ImageDef<T> *image2 = ImageDef<T>::CreateSameFormatInstance(image1, dest_width, dest_height);

      int i, j, Line, Pixel;
      int m;
      double Xzoom, Yzoom, vv3,vv2,vv, uu3, uu2,uu;
      double ii[4];
      double a[4][4], tab;  //4 * 4 sub image
      T *p, buf[9];

      Xzoom = ((double)dest_width) / image1->Width;
      Yzoom = ((double)dest_height) / image1->Height;

      // 为了解决边的插值问题，生成扩大的临时图象。
      ImageDef<T> *temp_image = ImageDef<T>::CreateSameFormatInstance(image1, image1->Width + 3, image1->Height + 3);
      WriteWindow(temp_image, 1, 1, temp_image->Width - 2 - 1, temp_image->Height - 2 - 1, image1->Pixels);

      for (Pixel = 0; Pixel < image1->Width; Pixel++)
      {
        ReadPixel(image1, Pixel, 0, buf);
        WritePixel(temp_image, Pixel + 1, 0, buf);

        ReadPixel(image1, Pixel, image1->Height - 1, buf);
        WritePixel(temp_image, Pixel + 1, temp_image->Height - 1, buf);
        WritePixel(temp_image, Pixel + 1, temp_image->Height - 2, buf);
      }

      for (Line = 0; Line < image1->Height; Line++)
      {
        ReadPixel(image1, 0, Line, buf);
        WritePixel(temp_image, 0, Line + 1, buf);
        if (Line == 0)
        {
          WritePixel(temp_image, 0, Line, buf);
        }
        else if (Line == image1->Height - 1)
        {
          WritePixel(temp_image, 0, Line + 2, buf);
          WritePixel(temp_image, 0, Line + 3, buf);
        }

        ReadPixel(image1, image1->Width - 1, Line, buf);
        WritePixel(temp_image, temp_image->Width - 2, Line + 1, buf);
        WritePixel(temp_image, temp_image->Width - 1, Line + 1, buf);
        if (Line == 0)
        {
          WritePixel(temp_image, temp_image->Width - 2, Line, buf);
          WritePixel(temp_image, temp_image->Width - 1, Line, buf);
        }
        if (Line == image1->Height - 1)
        {
          WritePixel(temp_image, temp_image->Width - 2, Line + 2, buf);
          WritePixel(temp_image, temp_image->Width - 1, Line + 2, buf);
          WritePixel(temp_image, temp_image->Width - 2, Line + 3, buf);
          WritePixel(temp_image, temp_image->Width - 1, Line + 3, buf);
        }
      }

			int up = GetUnitsPerPixel(temp_image), up2 = up * 2, ur = GetUnitsPerRow(temp_image), ur3 = ur * 3;
			for (i = 0; i < image2->Height; i++)
			{
			  Line = (int)((double)i / Yzoom);
			  uu = (double)i / Yzoom - Line;
			  uu2 = uu * uu;
			  uu3 = uu2 * uu;
			  Line += 1; // Line and Pixel is according temp_image 's position

			  for (j = 0; j < image2->Width; j++)
			  {
			    //计算坐标
			    Pixel = (int)((double)j / Xzoom);
			    vv = (double)j / Xzoom - Pixel;
			    vv2 = vv * vv;
			    vv3 = vv2 * vv;
			    Pixel += 1;

			    for (int iii = 0; iii < up; iii++)
			    {
  			    p = temp_image->Pixels + (Line * temp_image->Width + Pixel) * up + iii;
  			    a[1][0] = *(p - up); a[1][1] = *p; a[1][2] = *(p + up); a[1][3] = *(p + up2);
  			    p += ur;
  				  a[2][0] = *(p - up); a[2][1] = *p; a[2][2] = *(p + up); a[2][3] = *(p + up2);
  				  p += ur;
  				  a[3][0] = *(p - up); a[3][1] = *p; a[3][2] = *(p + up); a[3][3] = *(p + up2);
  				  p -= ur3;
  				  a[0][0] = *(p - up); a[0][1] = *p; a[0][2] = *(p + up); a[0][3] = *(p + up2);

  				  for (m = 0; m < 4; m++)
  				  {
  				    ii[m] = _ThreeLinearTrans(a[0][m], a[1][m], a[2][m], a[3][m], uu3, uu2, uu);
  				  }
  				  tab = _ThreeLinearTrans(ii[0], ii[1], ii[2], ii[3], vv3, vv2, vv) + 0.5;
  				  tab = MBL::Utility::Clamp(tab, 0.0, 255.0);
  				  *(image2->Pixels + (i * image2->Width + j) * up + iii) = (T)tab;
  				}
				}
			}

      delete temp_image;
      return image2;
    }

    /**
     * @brief 缩小一幅图像，图像可以为 RGB 或 Bayer 图像。
     *
     * 缩小图像的原理是：依据源图像和目标图像的长度比来映射 x 坐标，依据宽度比来映射 y 坐标。
     * 若要保证源图像缩小后不变形，则要对源图像按目标图像的长宽比进行裁减。
     * 该函数调用示例如下：
     *
     * @code
     * ImageDef<unsigned char> *image = ...;
     * ReduceImageSize(image, 1280, 1024, false); //对源图像进行裁减后再缩小。
     * ReduceImageSize(0, 1280, 1024, false);  //最后调用该函数清除内部资源，即所建的查找表。
     * @endcode
     *
     * 若调用该函数时，源图像的格式经常在变化，例如一会儿是 RGB格式，一会儿是 Bayer格式，这样会导致内部
     * 查找表的经常重建，建议此种情况时对本函数的查找表建立部分进行修改。
     *
     * @param img 欲处理的图像,可以是RGB、BGR和Bayer格式的图像。img为0时，清除所建的查找表。
     * @param dw 目标图像的宽度。
     * @param dh 目标图像的高度。
     * @param distortion 目标图像是否要变形，若为 false 则不变形，要对源图像进行裁减再缩小。若为 true 则要变形，源图像不做裁减就缩小。
     *
     * @author 陈伟卿
     */
    template <class T>
    void ReduceImageSize(ImageDef<T> *img, int dw, int dh, bool distortion)
    {
      static int *d2sX = 0, *d2sY = 0;
      static int dWidth = 0, dHeight = 0;
      static int sWidth = 0, sHeight = 0;
      static int format;

      if (img == 0)
      {
        MBL::Utility::SafeReleaseArray(&d2sX);
        MBL::Utility::SafeReleaseArray(&d2sY);
				dWidth = 0; dHeight = 0;
				sWidth = 0; sHeight = 0; format = 0;
        return;
      }

      int i, j, k, sw = img->Width, sh = img->Height;
      float fx, fy;
      T *pSource = img->Pixels;
      T *pTarget = img->Pixels;
      T *pt = 0, *p1 = 0;

      fx = (float)sw / (float)dw;
      fy = (float)sh / (float)dh;
      if (distortion == false)
      {
        int sw1 = dw * sh / dh;
        if (sw1 < sw)
        {
          fx = (float)sw1 / (float)dw;
        }
        else
        {
          int sh1 = dh * sw /dw;
          fy = (float)sh1 / (float)dh;
        }
      }

      if (dh != dHeight || sh != sHeight || format != img->Format)
      {
        MBL::Utility::SafeReleaseArray(&d2sY);
        dHeight = dh;
        sHeight = sh;
        format = img->Format;
        d2sY = new int[dHeight];
        if (img->Format == IMAGE_FORMAT_RGB || img->Format == IMAGE_FORMAT_BGR)
        {
          for( i = 0; i < dh; i++)
            *(d2sY + i) = (int)(i * fy) * sw * 3;
        }
        else
        {
          for (i = 0; i < dh; i++)
          {
            k = (int)(i * fy);
            if (i % 2 == 0)
              *(d2sY + i) = (k - k % 2) * sw;
            else
              *(d2sY + i) = (k - (1 - k % 2)) * sw;
          }
        }
      }

      if (dw != dWidth || sw != sWidth || format != img->Format)
      {
        MBL::Utility::SafeReleaseArray(&d2sX);
        dWidth = dw;
        sWidth = sw;
        format = img->Format;
        d2sX = new int[dWidth];
        if(img->Format == IMAGE_FORMAT_RGB || img->Format == IMAGE_FORMAT_BGR)
        {
          for (i = 0; i < dw; i++)
            *(d2sX + i) = (int)(i * fx) * 3;
        }
        else
        {
          for (i = 0; i < dw; i++)
          {
            k = (int)(i * fx);
            if(i % 2 == 0)
              *(d2sX + i) = k - k % 2;
            else
              *(d2sX + i) = k - (1 - k % 2);
          }
        }
      }

      switch (img->Format)
      {
        case IMAGE_FORMAT_RGB:
        case IMAGE_FORMAT_BGR:
          for (i = 0; i < dh; i++)
          {
            p1 = pSource + d2sY[i];
            for (j = 0; j < dw; j++)
            {
              pt = p1 + d2sX[j];
              *pTarget++ = *pt++;
              *pTarget++ = *pt++;
              *pTarget++ = *pt;
            }
          }
          break;
        case IMAGE_FORMAT_BAYER_GR_BG:
        case IMAGE_FORMAT_BAYER_BG_GR:
        case IMAGE_FORMAT_BAYER_GB_RG:
        case IMAGE_FORMAT_BAYER_RG_GB:
          for (i = 0; i < dh; i++)
          {
            p1 = pSource + d2sY[i];
            for (j = 0; j < dw; j++)
              *pTarget++ = *(p1 + d2sX[j]);
          }
          break;
        default:
          break;
      }
      img->Width = dw;
      img->Height = dh;
    }

    /**
     * @brief 缩放一幅图像。
     *
     * 双线性插值图像缩放算法。因为输出的目标图像与源图像大小相等，所以当截取部分源图像时，源图像内容会被放大。反之，源图象内容会被缩小，并且四周加上黑边。
     *
     * 例如，当图像发生横向变形或纵向变形时，可以截取图像的一部分进行放大，来校正图象的变形。该函数调用示例如下：
     *
     * @code
     * ImageDef<unsigned char> *srcRgb = CreateInstance(IMAGE_FORMAT_RGB, 800, 600);
     * ImageDef<unsigned char> *dstRgb = CreateInstance(IMAGE_FORMAT_RGB, 800, 600);
     * ZoomImage(srcRgb, 400, 200, dstRgb); //截取源图像400*200的部分进行放大，得到结果图像dstRgb。
     * ZoomImage(srcRgb, 1600, 1200, dstRgb); //将源图像缩小一倍，填充到结果图像dstRgb的中心，空白处为0。
     * ZoomImage(0, 0, 0, 0);  //最后调用该函数并设置第一个参数为0,即可清除内部资源，即所建的查找表。
     * @endcode
     *
     * @param srcImg 输入的源图像。srcImg为0时，清除所建的查找表。目前必须是RGB或者BGR格式的图像。
     * @param cwidth 截取图像宽度（象素），当它小于源图像的宽度时，将截取源图像进行放大；当它大于源图像宽度时，源图像将被缩小。
     * @param cheight 截取图像高度（象素），当它小于源图像的高度时，将截取源图像进行放大；当它大于源图像高度时，源图像将被缩小。
     * @param dstImg 输出的目标图像，必须已经分配内存，且其尺寸必须与源图像相等。
     *
     * @author 陈伟卿
     */
    template <class T>
    void ZoomImage(ImageDef<T> *srcImg, int cwidth, int cheight, ImageDef<T> *dstImg)
    {
      static int *ulutx = 0, *vluty = 0;
      static int *byteY = 0, *byteX = 0;
      static int wid = 0, hei = 0, cw = 0, ch = 0;
      static int bx = 0, ex, by = 0, ey;
      if (srcImg == 0)
      {
        MBL::Utility::SafeReleaseArray(&ulutx);
        MBL::Utility::SafeReleaseArray(&vluty);
        MBL::Utility::SafeReleaseArray(&byteY);
        MBL::Utility::SafeReleaseArray(&byteX);
				wid = 0; hei = 0; cw = 0; ch = 0;
				bx = 0; ex = 0; by = 0; ey = 0;
        return;
      }
      T *ptem = srcImg->Pixels;
      int width = srcImg->Width;
      int height = srcImg->Height;

      T *pbuf = dstImg->Pixels;
      int bitCount = GetUnitsPerPixel(srcImg);
      float scaleX, scaleY;
      float xnew, ynew, wgap, hgap;;
      int k,xint,yint;
	    //建立双线性插值所要用的表
      if (wid != width || cw != cwidth)
      {
				MBL::Utility::SafeReleaseArray(&ulutx);
				MBL::Utility::SafeReleaseArray(&byteX);
        wid = width;
        cw = cwidth;
        ex = wid;
        scaleX = (float)cw / (float)width;
        if( cw > width)
        {
          bx = width / (scaleX * 2);
          ex = width - bx;
        }
        wgap = (width - width * scaleX) / 2;
    	  ulutx = new int[wid];
        byteX = new int[wid];
	      for ( k = 0; k < width; k++)
	      {
          //xnew 为目标图像某个像素的横坐标 k 对应的原图像的横坐标
		      xnew = (k * ((width - 2.0 * wgap) / (width - 1)) + wgap);
		      if (xnew >= width)
			      xnew = width-1;
		      else if (xnew < 0)
			      xnew = 0;
          xint = (int)xnew;
          if(xnew == width -1)
            xint = width - 2;
		      ulutx[k] = (int)((xnew - xint) * 256);//u 值所用的表
          byteX[k] = ((int)xnew) * bitCount;
	      }
      }

      if (hei != height || ch != cheight)
      {
				MBL::Utility::SafeReleaseArray(&vluty);
				MBL::Utility::SafeReleaseArray(&byteY);
        hei = height;
        ch = cheight;
        ey = hei;
        scaleY = (float)ch / (float)height;
        if(ch > height)
        {
          by = height / (scaleY * 2);
          ey = height - by;
        }
	      hgap = (height - height * scaleY) / 2;
        vluty = new int[hei];
        byteY = new int[hei];
	      for ( k = 0; k < height; k++)
	      {
		      ynew = (k * ((height - 2.0 * hgap) / (height - 1)) + hgap);
		      if (ynew >= height)
			      ynew = height-1;
		      else if (ynew <0)
			      ynew = 0;
          yint = (int)ynew; //ynew 为目标图像某个像素的纵坐标 k 对应的原图像的纵坐标
          if(ynew == height - 1)
            yint = height - 2;
		      vluty[k] = (int)((ynew - yint) * 256); //v 值所用的表
          byteY[k] = ((int)ynew) * width * bitCount;
	      }
      }
	    int xx,yy,t, v,u, skipX, a0, a1, a2, a3, a4;
      T  t1,t2,t3,t4;
      if(ch > height || cw > width)
        memset(pbuf, 0, wid * hei * bitCount);//如果图像是缩小则将边界设置为黑
      pbuf += by * width * bitCount + bx * bitCount; //缩小时才移 pbuf
      skipX = 2 * bx * bitCount;

      for (int i = by; i < ey; i++)
      {
	      v = vluty[i];
        a0 = 256 - v;
        yy = byteY[i];
	      for (int j = bx; j < ex; j++)
	      {
		      u = ulutx[j];
		      xx = yy + byteX[j];
          a1 = (256-u) * a0;
          a2 = u * a0;
          a3 = (256 - u) * v;
          a4 = u * v;
		      //R
		      t1 = ptem[xx];
		      t2 = ptem[xx + bitCount] ;
		      t3 = ptem[xx + width * bitCount] ;
		      t4 = ptem[xx + bitCount + width * bitCount] ;
          //t = t1 * (1-u) * (1-v) + t2 * (1-v) * u + t3 * v * (1-u) + t4 * u * v;
		      //t = (t1 * (256-u) * (256-v) + t2 * (256-v) * u + t3 * v * (256-u) + t4 * u * v) >> 16;//将原来的浮点乘法转化为整型的乘法
          t = (t1 * a1 + t2 * a2 + t3 * a3 + t4 * a4) >> 16;
		      *pbuf++ = t;
		      //G
		      t1 = ptem[xx + 1] ;
		      t2 = ptem[xx + bitCount + 1];
		      t3 = ptem[xx + width * bitCount + 1] ;
		      t4 = ptem[xx + bitCount + width * bitCount + 1] ;
		      t = (t1 * a1 + t2 * a2 + t3 * a3 + t4 * a4) >> 16;
		      *pbuf++ = t;
		      //B
		      t1 = ptem[xx + 2] ;
		      t2 = ptem[xx + bitCount + 2] ;
		      t3 = ptem[xx + width * bitCount + 2] ;
		      t4 = ptem[xx + bitCount + width * bitCount + 2];
          t = (t1 * a1 + t2 * a2 + t3 * a3 + t4 * a4) >> 16;
		      *pbuf++ = t;
	      }
        pbuf += skipX;
      }
    }

    /// 将源图像转换为另外一种数据存储格式。
    /**
     * 该函数将源图像转换为另外一种数据存储格式，如由unsigned char转换为float。在转换过程中图像格式并不改变。
     * 当由宽数据类型转换到窄数据类型时，可能会有精度损失。
     *
     * @param src 欲处理的源图像，必须包含有效的内存。
     * @param desc 指向转换后的目标图像的指针，其内容必须为0，否则其原有数据会被删除。
     * @param map 是否将源数据比例映射到0~T2max范围内。如果为是则会将源数据的最小值映射为0，最大值映射为T2
     *            可能的最大值。该映射适合于从浮点数到无符号整数的转换，并且暂时必须是灰度图像。缺省不做比
     *            例映射。
     *
     * @author 赵宇
     *
     * @todo 比例映射方法有很多值得改进的地方。
     */
    template <class T1, class T2>
    void ConvertImageStorageFormat(ImageDef<T1> *src, ImageDef<T2> **desc, bool map = false)
    {
      if (*desc != 0) MBL::Utility::SafeRelease(desc);
      *desc = ImageDef<T2>::CreateInstance(src->Format, src->Width, src->Height, src->UsedColor);

      int n = GetUnitsOfPixelData(src);
      T1 *p1 = src->Pixels;
      T2 *p2 = (*desc)->Pixels;

      if (map == true)
      {
        T1 src_min, src_max;
        src_min = src_max = *p1++;
        for (int i = 0; i < n; i++, p1++)
        {
          if (*p1 < src_min) src_min = *p1;
          if (*p1 > src_max) src_max = *p1;
        }

        T2 desc_max;
        MBL::Utility::GetMaxValue(&desc_max);
        T1 offset = -src_min, scale = desc_max / (src_max - src_min);

        p1 = src->Pixels;
        for (int i = 0; i < n; i++)
        {
          *p2++ = (T2)(((*p1++) + offset) * scale);
        }
      }
      else
      {
        for (int i = 0; i < n; i++)
        {
          *p2++ = (T2)(*p1++);
        }
      }
    }
  } // Image2D namespace
} // MBL namespace

#endif // __IMAGETRANSFORM_H__
