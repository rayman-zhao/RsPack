#ifndef __IMAGECOLOR_H__
#define __IMAGECOLOR_H__

/**
 * @file
 *
 * @brief 包含调整、改变图像色彩的函数。
 */

#include <cmath>
#include <cassert>
#include <limits>
#include <omp.h>

namespace MBL
{
  namespace Image2D
  {
    /**
     * @brief 将一个真彩色图像变换为灰度图像。
     *
     * @param image RGB格式源图像，处理后该图像仍然是真彩色格式，但RGB分量均相等。
     * @param sub_area 子区对象，如果为0则表示处理全图。
     * @see GrayImage2
     */
    template <class T>
    void GrayImage(ImageDef<T> *image, ImageSubArea *sub_area)
    {
      if (image == 0) throw NullPointerException();
      if (image->Format != IMAGE_FORMAT_RGB) throw UnsupportedFormatException();

      T buf[3], v;
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
            ReadPixel(image, x, y, buf);
            v = (299 * buf[0] + 587 * buf[1] + 114 * buf[2]) / 1000;
            buf[0] = buf[1] = buf[2] = v;
            WritePixel(image, x, y, buf);
          }
        }
      }
    }

    /**
     * @brief 将一个真彩色图像变换为灰度图像。
     *
     * 这个函数速度更快。
     *
     * @param image RGB格式源图像，处理后该图像仍然是真彩色格式，但RGB分量均相等。
     * @param sub_area 子区对象，如果为0则表示处理全图。
     */
    template <class T>
    void GrayImage2(ImageDef<T> *image, ImageSubArea *sub_area)
    {
      if (image == 0) throw NullPointerException();
      if (image->Format != IMAGE_FORMAT_RGB) throw UnsupportedFormatException();

      static T r_lut[ImageDefTraits<T>::LengthOfLUT];
      static T g_lut[ImageDefTraits<T>::LengthOfLUT];
      static T b_lut[ImageDefTraits<T>::LengthOfLUT];
      static bool init = false;
      if (!init)
      {
        for (int i = 0; i < ImageDefTraits<T>::LengthOfLUT; ++i)
        {
          r_lut[i] = 299 * i / 1000;
          g_lut[i] = 587 * i / 1000;
          b_lut[i] = 114 * i / 1000;
        }
        init = true;
      }

      if (sub_area == 0)
      {
        T *p = image->Pixels;
        for (int i = 0, n = image->Width * image->Height; i < n; ++i)
        {
          T t = r_lut[*p] + g_lut[*(p + 1)] + b_lut[*(p + 2)];
          *p++ = t;
          *p++ = t;
          *p++ = t;
        }
      }
      else
      {
        throw UnsupportedSubAreaException();
      }
    }

    /**
     * @brief 由一幅真彩色RGB图像，创建一幅灰度图像。
     *
     * @param image 源真彩色图像，可以是RGB或BGR格式。
     * @return 同样大小的灰度索引图像，无调色板数据。使用完毕后请用delete删除。
     */
    template <class T>
    ImageDef<T> * CreateGrayImage(ImageDef<T> *image)
    {
      if (image == 0) throw NullPointerException();
      if (image->Format != IMAGE_FORMAT_RGB && image->Format != IMAGE_FORMAT_ARGB) throw UnsupportedFormatException();

      ImageDef<T> *nimage = ImageDef<T>::CreateInstance(IMAGE_FORMAT_INDEX, image->Width, image->Height, 0);
      
      T *p = nimage->Pixels, *p1 = image->Pixels;
      int cnt = image->Width * image->Height;
      
      int r, g, b;
      if (image->Format == IMAGE_FORMAT_RGB)
      {
        for (int i = 0; i < cnt; ++i)
        {
          r = *p1++;
          g = *p1++;
          b = *p1++;
          *p++ = (299 * r + 587 * g + 114 * b) / 1000;
        }
      }
      else if (image->Format == IMAGE_FORMAT_ARGB)
      {
        for (int i = 0; i < cnt; ++i)
        {
          ++p1;
          r = *p1++;
          g = *p1++;
          b = *p1++;
          *p++ = (299 * r + 587 * g + 114 * b) / 1000;
        }
      }

      return nimage;
    }

    /**
     * @brief 由一幅索引格式图像，创建一幅RGB格式图像，RGB分量值与原灰度值相同。
     *
     * @param image 源索引格式图像。
     * @return 同样大小的RGB格式图像，无调色板数据。使用完毕后请用delete删除。
     */
    template <class T>
    ImageDef<T> * CreateRGBImage(ImageDef<T> *image)
    {
      if (image == 0) throw NullPointerException();
      if (image->Format != IMAGE_FORMAT_INDEX) throw UnsupportedFormatException();

      ImageDef<T> *nimage = ImageDef<T>::CreateInstance(IMAGE_FORMAT_RGB, image->Width, image->Height, 0);
      T buf[1], v[3];

      for (int y = 0; y < image->Height; y++)
      {
        for (int x = 0; x < image->Width; x++)
        {
          ReadPixel(image, x, y, buf);
          v[0] = buf[0];
          v[1] = buf[0];
          v[2] = buf[0];
          WritePixel(nimage, x, y, v);
        }
      }

      return nimage;
    }

    /**
     * @brief 取得图像中某个象素的亮度（灰度）值。
     *
     * @param image 源图像。
     * @param x X坐标（象素）。
     * @param y Y坐标（象素）。
     * @return 该点的亮度（灰度）值。
     */
    template <class T>
    T GetIntensityOfPixel(ImageDef<T> *image, int x, int y)
    {
      T buf[8];
      T ret = 0;
      ReadPixel(image, x, y, buf);

      switch (image->Format)
      {
        case IMAGE_FORMAT_INDEX:
        case IMAGE_FORMAT_INDEX_ALPHA:
          ret = buf[0];
          break;
        case IMAGE_FORMAT_RGB:
        case IMAGE_FORMAT_RGBA:
          ret = (299 * buf[0] + 587 * buf[1] + 114 * buf[2]) / 1000;
          break;
        case IMAGE_FORMAT_BGR:
          ret = (299 * buf[2] + 587 * buf[1] + 114 * buf[0]) / 1000;
          break;
        case IMAGE_FORMAT_ARGB:
          ret = (299 * buf[1] + 587 * buf[2] + 114 * buf[3]) / 1000;
          break;
        default:
          break;
      }

      return ret;
    }

    /**
     * @brief 将一个图像变换为反色图像。
     *
     * @param image 源图像。
     * @param sub_area 子区对象，如果为0则表示处理全图。
     */
    template <class T>
    void InvertImage(ImageDef<T> *image, ImageSubArea *sub_area)
    {
      if (image == 0) throw NullPointerException();

      T buf[8];
      int b = GetUnitsPerPixel(image);

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
            ReadPixel(image, x, y, buf);
            for (int i = 0; i < b; i++)
            {
              buf[i] = ImageDefTraits<T>::MaxValue - buf[i];
            }
            WritePixel(image, x, y, buf);
          }
        }
      }
    }

    /**
     * @brief 取得反色图像算法的查找表。
     *
     * @return 查找表。由于反色算法各个通道是一致的，所以只返回单个通道的查找表。
     */
    template <class T>
    const T * GetInvertLUT()
    {
      static T lut[ImageDefTraits<T>::LengthOfLUT];
      static bool init = false;

      if (!init)
      {
        for (int i = 0; i < ImageDefTraits<T>::LengthOfLUT; ++i)
        {
          lut[i] = ImageDefTraits<T>::MaxValue - i;
        }
        init = true;
      }

      return lut;
    }

    /**
     * @brief 调整图像的RGB颜色均衡。
     *
     * @param image 欲处理图像，必须是RGB格式。
     * @param sub_area 子区对象，如果为0则表示处理全图。
     * @param r 红色分量调整值，为[-100, 100]区间，即增量占原值的百分比。
     * @param g 绿色分量调整值，为[-100, 100]区间，即增量占原值的百分比。
     * @param b 蓝色分量调整值，为[-100, 100]区间，即增量占原值的百分比。
     */
    template <class T>
    void AdjustImageRGB(ImageDef<T> *image, ImageSubArea *sub_area, int r, int g, int b)
    {
      if (image == 0) throw NullPointerException();
      if (image->Format != IMAGE_FORMAT_RGB) throw UnsupportedFormatException();

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

      T buf[3];
      for (int y = top; y < bottom; y++)
      {
        for (int x = left; x < right; x++)
        {
          if (sub_area == 0 || sub_area->IsFill(x, y))
          {
            ReadPixel(image, x, y, buf);
            buf[0] = MBL::Utility::Clamp(buf[0] + buf[0] * r / 100, ImageDefTraits<T>::MinValue, ImageDefTraits<T>::MaxValue);
            buf[1] = MBL::Utility::Clamp(buf[1] + buf[1] * g / 100, ImageDefTraits<T>::MinValue, ImageDefTraits<T>::MaxValue);
            buf[2] = MBL::Utility::Clamp(buf[2] + buf[2] * b / 100, ImageDefTraits<T>::MinValue, ImageDefTraits<T>::MaxValue);
            WritePixel(image, x, y, buf);
          }
        }
      }
    }

    /**
     * @brief 根据输入图像的子区，进行白色平衡处理，取得各个颜色分量的增益值。
     *
     * @param image 输入图像，必须是RGB格式。
     * @param sub_area 图像子区，如果为0，则表示对全图进行白平衡处理。
     * @param r_gain 函数返回后得到红色分量增益值。
     * @param g_gain 函数返回后得到绿色分量增益值。
     * @param b_gain 函数返回后得到蓝色分量增益值。
     *
     * @author 黄超，钟明亮，陈进，赵宇。
     */
    template <class T>
    void WhiteBalanceImage(ImageDef<T> *image, ImageSubArea *sub_area, float *r_gain, float *g_gain, float *b_gain)
    {
      *r_gain = *g_gain = *b_gain = 1;

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

      T buf[3];
      int pixel_cnt = 0;
      int ave_r = 0, ave_g = 0, ave_b = 0;

      for (int y = top; y < bottom; y++)
      {
        for (int x = left; x < right; x++)
        {
          if (sub_area == 0 || sub_area->IsFill(x, y))
          {
            ReadPixel(image, x, y, buf);
            pixel_cnt++;
            ave_r += buf[0];
            ave_g += buf[1];
            ave_b += buf[2];
          }
        }
      }
      ave_r /= pixel_cnt;
      ave_g /= pixel_cnt;
      ave_b /= pixel_cnt;

      float max = static_cast<float>(Utility::GetMax(Utility::GetMax(ave_r, ave_g), ave_b));
      if (image->Format == IMAGE_FORMAT_RGB)
      {
        if (ave_r != 0)
        {
          *r_gain = max / ave_r;
        }
        if (ave_g != 0)
        {
          *g_gain = max / ave_g;
        }
        if (ave_b != 0)
        {
          *b_gain = max / ave_b;
        }
      }
      else if (image->Format == IMAGE_FORMAT_BGR)
      {
        if (ave_b != 0)
        {
          *r_gain = max / ave_b;
        }
        if (ave_g != 0)
        {
          *g_gain = max / ave_g;
        }
        if (ave_r != 0)
        {
          *b_gain = max / ave_r;
        }
      }
    }

    /**
     * @brief 取得RGB颜色均衡的查找表。
     *
     * 每个颜色通道的变换公式为：value = clamp(value * gain + offset, 0, 255)。
     *
     * @param r_gain 红色分量的增益。
     * @param g_gain 绿色分量的增益。
     * @param b_gain 蓝色分量的增益。
     * @param r_offset 红色分量的偏移。
     * @param g_offset 绿色分量的偏移。
     * @param b_offset 蓝色分量的偏移。
     * @return RGB查找表，按红色、绿色、蓝色通道顺序排列。
     */
    template <class T>
    const T * GetColorBalanceLUT(float r_gain, float g_gain, float b_gain, int r_offset, int g_offset, int b_offset)
    {
      assert(r_gain >= 0 && g_gain >= 0 && b_gain >= 0);

	    static thread_local T lut[ImageDefTraits<T>::LengthOfLUT * 3];
	    static thread_local float red_gain = -1;
	    static thread_local float green_gain = -1;
	    static thread_local float blue_gain = -1;
	    static thread_local int red_offset = 0;
	    static thread_local int green_offset = 0;
	    static thread_local int blue_offset = 0;

      // 根据输入参数更新查找表。
      if (r_gain != red_gain || r_offset != red_offset)
      {
        red_gain = r_gain;
        red_offset = r_offset;
        for (int i = 0; i < ImageDefTraits<T>::LengthOfLUT; ++i)
          lut[i] = (T)Utility::Clamp(i * red_gain + red_offset, ImageDefTraits<T>::MinValue, ImageDefTraits<T>::MaxValue);
      }
      if (g_gain != green_gain || g_offset != green_offset)
      {
        green_gain = g_gain;
        green_offset = g_offset;
        for (int i = 0; i < ImageDefTraits<T>::LengthOfLUT; ++i)
          lut[i + ImageDefTraits<T>::LengthOfLUT] = (T)Utility::Clamp(i * green_gain + green_offset, ImageDefTraits<T>::MinValue, ImageDefTraits<T>::MaxValue);
      }
      if (b_gain != blue_gain || b_offset != blue_offset)
      {
        blue_gain = b_gain;
        blue_offset = b_offset;
        for (int i = 0; i < ImageDefTraits<T>::LengthOfLUT; ++i)
          lut[i + 2 * ImageDefTraits<T>::LengthOfLUT] = (T)Utility::Clamp(i * blue_gain + blue_offset, ImageDefTraits<T>::MinValue, ImageDefTraits<T>::MaxValue);
      }

      return lut;
    }

    /**
     * @brief 调整图像的RGB颜色均衡。
     *
     * 每个颜色通道的变换公式为：value = clamp(value * gain + offset, 0, 255)。
     *
     * @param image 欲处理图像。
     * @param r_gain 红色分量的增益。
     * @param g_gain 绿色分量的增益。
     * @param b_gain 蓝色分量的增益。
     * @param r_offset 红色分量的偏移。
     * @param g_offset 绿色分量的偏移。
     * @param b_offset 蓝色分量的偏移。
     *
     * @author 黄超，陈进，陈伟卿，赵宇
     */
    template <class T>
    void ColorBalanceImage(ImageDef<T> *image, float r_gain, float g_gain, float b_gain,
                                               int r_offset, int g_offset, int b_offset)
    {
      if (r_gain == 1 && g_gain == 1 && b_gain == 1 &&  r_offset == 0 && g_offset == 0 && b_offset == 0) return;

      const T *r_lut = GetColorBalanceLUT<T>(r_gain, g_gain, b_gain, r_offset, g_offset, b_offset);
      const T *g_lut = r_lut + ImageDefTraits<T>::LengthOfLUT;
      const T *b_lut = g_lut + ImageDefTraits<T>::LengthOfLUT;
      ApplyImageLUT(image, r_lut, g_lut, b_lut);
    }

    /**
     * @brief 将RGB分量转换到HSI色彩空间。
     *
     * http://ilab.usc.edu/wiki/index.php/HSV_And_H2SV_Color_Space
     *
     * @param R 红色分量，0～255。
     * @param G 绿色分量，0～255。
     * @param B 蓝色分量，0～255。
     * @param H 色度变量引用，0～360。
     * @param S 饱和度变量引用，0～1。
     * @param V 亮度变量引用，0～1。
     */
    template <typename T>
    inline void PIX_RGB_TO_HSV(const T R,
                               const T G,
                               const T B,
                               double &H,
                               double &S,
                               double &V)
    {
      if ((B > G) && (B > R))
      {
        V = B;
        if (V != 0)
        {
          double min;
          if (R > G) min = G;
          else       min = R;
          
          const double delta = V - min;
          if (delta != 0)
          {
            S = (delta / V);
            H = 4 + (R - G) / delta;
          }
          else
          {
            S = 0;
            H = 4 + (R - G);
          }
          H *= 60; if (H < 0) H += 360;
          V = (V / ImageDefTraits<T>::MaxValue);
        }
        else
        {
          S = H = 0;
        }     
      }
      else if (G > R)
      {
        V = G;
        if (V != 0)
        {
          double min;
          if (R > B) min = B;
          else       min = R;
          
          const double delta = V - min;
          if (delta != 0)
          {
            S = (delta / V);
            H = 2 + (B - R) / delta;
          }
          else
          {
            S = 0;
            H = 2 + (B - R);
          }
          H *=   60; if (H < 0) H += 360;
          V =  (V / ImageDefTraits<T>::MaxValue);
        }
        else
        {
          S = H = 0;
        }
      }
      else
      {
        V = R;
        if (V != 0)
        {
          double min;
          if (G > B) min = B;
          else       min = G;
          
          const double delta = V - min;
          if (delta != 0)
          {
            S = (delta/V);
            H = (G - B) / delta;
          }
          else
          {
            S = 0;
            H = (G - B);
          }
          H *=   60; if (H < 0) H += 360;
          V =  (V / ImageDefTraits<T>::MaxValue);
        }
        else
        {
          S = H = 0;
        }                                                   
      }
    }

    /**
     * @brief 将HSI分量转换到RGB色彩空间。
     *
     * http://ilab.usc.edu/wiki/index.php/HSV_And_H2SV_Color_Space
     *
     * @param H 色度，0～360。
     * @param S 饱和度，0～1。
     * @param V 亮度，0～1。
     * @param Red 红色通道引用。
     * @param Green 绿色通道引用。
     * @param Blue 蓝色通道引用。
     */
    template <class T>
    inline void PIX_HSV_TO_RGB(const double H,
                               const double S,
                               const double V,
                               T &Red,
                               T &Green,
                               T &Blue)
    {
      double R, G, B;
      
      if (V == 0)
      {
        R = G = B = 0;
      }
      else if (S == 0)
      {
        R = V;
        G = V;
        B = V;
      }
      else
      {
        const double hf = H / 60.0;
        const int    i  = (int)floor(hf);
        const double f  = hf - i;
        const double pv  = V * (1 - S);
        const double qv  = V * (1 - S * f);
        const double tv  = V * (1 - S * (1 - f));
        
        switch (i)
        {
          case 0:
            R = V;
            G = tv;
            B = pv;
            break;
          case 1:
            R = qv;
            G = V;
            B = pv;
            break;
          case 2:
            R = pv;
            G = V;
            B = tv;
            break;
          case 3:
            R = pv;
            G = qv;
            B = V;
            break;
          case 4:
            R = tv;
            G = pv;
            B = V;
            break;
          case 5:
            R = V;
            G = pv;
            B = qv;
            break;
          case 6:
            R = V;
            G = tv;
            B = pv;
            break;
          case -1:
            R = V;
            G = pv;
            B = qv;
            break;
          default:
            R = G = B = 0;
            break;
        }
      }
      R *= ImageDefTraits<T>::MaxValue;
      G *= ImageDefTraits<T>::MaxValue;
      B *= ImageDefTraits<T>::MaxValue;
      
      Red = MBL::Utility::Clamp(R, ImageDefTraits<T>::MinValue, ImageDefTraits<T>::MaxValue);
      Green = MBL::Utility::Clamp(G, ImageDefTraits<T>::MinValue, ImageDefTraits<T>::MaxValue);
      Blue = MBL::Utility::Clamp(B, ImageDefTraits<T>::MinValue, ImageDefTraits<T>::MaxValue);
    }

    /**
     * @brief 调整图像的HSI分量。
     *
     * @param image 欲处理图像，必须是RGB格式。
     * @param sub_area 子区对象，如果为0则表示处理全图。
     * @param h 色度分量调整值，为[-180, 180]区间，即调整的角度值色。
     * @param s 饱和度分量调整值，为[-100, 100]区间，即增量占原值的百分比。
     * @param i 亮度分量调整值，为[-100, 100]区间，即增量占原值的百分比。
     */
    template <class T>
    void AdjustImageHSI(ImageDef<T> *image, ImageSubArea *sub_area, int h, int s, int i)
    {
      if (image == 0) throw NullPointerException();
      if (image->Format != IMAGE_FORMAT_RGB) throw UnsupportedFormatException();

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

      T buf[3];
      double H, S, I;
      for (int y = top; y < bottom; y++)
      {
        for (int x = left; x < right; x++)
        {
          if (sub_area == 0 || sub_area->IsFill(x, y))
          {
            ReadPixel(image, x, y, buf);
            PIX_RGB_TO_HSV(buf[0], buf[1], buf[2], H, S, I);
            H = H + h;
            //将H限制在[0, 360)区间。
            while (H < 0) H += 360;
            while (H >= 360) H -= 360;
            S = MBL::Utility::Clamp(S + s / 100.0, 0, 1);
            I = MBL::Utility::Clamp(I + i / 100.0, 0, 1);
            PIX_HSV_TO_RGB(H, S, I, (*buf), (*(buf + 1)), (*(buf + 2)));
            WritePixel(image, x, y, buf);
          }
        }
      }
    }

    /**
     * @brief 调整图像的亮度和对比度。
     *
     * @param image 欲处理图像，必须是RGB格式。
     * @param bright 亮度分量调整值，为[-100, 100]区间，即增量占原值的百分比。
     * @param contrast 对比度分量调整值，为[-100, 100]区间，即增量占原值的百分比。
     */
    template <class T>
    void AdjustImageBrightContrast(ImageDef<T> *image, int bright, int contrast)
    {
      if (image == 0) throw NullPointerException();
      if (image->Format != IMAGE_FORMAT_RGB) throw UnsupportedFormatException();

      int *intensity_level = new int[ImageDefTraits<T>::LengthOfLUT];
      memset(intensity_level, 0, sizeof(int) * (ImageDefTraits<T>::LengthOfLUT));
      ImageDef<T> *intensity_image = CreateGrayImage(image);

      // Calculate max_intensity, min_intensity, mean_Intensity
      double sum = 0, mean_intensity = 0;
      T max_intensity = 0, min_intensity = ImageDefTraits<T>::MaxValue, buf, rgb_buf[3];
      for (int y = 0; y < image->Height; y++)
      {
        for (int x = 0; x < image->Width; x++)
        {
          ReadPixel(intensity_image, x, y, &buf);

          if (buf < min_intensity) min_intensity = buf;
          else if (buf > max_intensity) max_intensity = buf;

          intensity_level[buf]++;
        }
      }
      for (int i = 0; i < ImageDefTraits<T>::LengthOfLUT; i++)
      {
        sum += i * intensity_level[i];
      }
      mean_intensity = sum / image->Width / image->Height;

      // adjust intensity
      double slope = tan(MBL::Utility::Clamp(contrast * 9.0 / 20.0 + 45.0, 0.0, 90.0) * 3.14159265359 / 180);
      double kk;
      for (int y = 0; y < image->Height; y++)
      {
        for (int x = 0; x < image->Width; x++)
        {
          ReadPixel(intensity_image, x, y, &buf);
          kk = (buf - mean_intensity ) * slope + mean_intensity - buf + bright;

          ReadPixel(image, x, y, rgb_buf);
          // Deal with color balance.
          for (int i = 0; i < 3; i++)
          {
            kk = MBL::Utility::Clamp(kk, 255.0 - rgb_buf[i], (double)-rgb_buf[i]);
          }
          for (int i = 0; i < 3; i++)
          {
            rgb_buf[i] += kk;
          }
          WritePixel(image, x, y, rgb_buf);
        }
      }

      delete [] intensity_level;
      delete intensity_image;
    }

    /**
     * @brief 取得对比度拉伸算法的查找表。
     *
     * @param contrast 对比度拉伸参数，在[-100, 100]区间，0表示不拉伸。
     * @return 单个通道查找表。
     */
    template <class T>
    const T * GetContrastLUT(int contrast)
    {
      assert(-100 <= contrast && contrast <= 100);

      static T lut[ImageDefTraits<T>::LengthOfLUT];

      int val = contrast * ImageDefTraits<T>::MidValueRoundDown / 100;
      if (val >= 0)
      {
        float gain = 255.0f / (255 - 2.0f * val);

        int i = 0;
        for (; i < val; ++i)
        {
          lut[i] = 0;
        }
        for (; i <= ImageDefTraits<T>::MaxValue - val; ++i)
        {
          lut[i] = static_cast<T>(MBL::Utility::Clamp((i - val) * gain, ImageDefTraits<T>::MinValue, ImageDefTraits<T>::MaxValue));
        }
        for (; i < ImageDefTraits<T>::LengthOfLUT; ++i)
        {
          lut[i] = ImageDefTraits<T>::MaxValue;
        }
      }
      else
      {
        val = -val;
        int i = 0;
        for (; i < ImageDefTraits<T>::MidValueRoundUp; ++i)
        {
          lut[i] = MBL::Utility::GetMin(i + val, (int)ImageDefTraits<T>::MidValueRoundDown);
        }
        for (; i < ImageDefTraits<T>::LengthOfLUT; ++i)
        {
          lut[i] = MBL::Utility::GetMax(i - val, (int)ImageDefTraits<T>::MidValueRoundUp);
        }
      }

      return lut;
    }

    /**
    * @brief 取得对比度拉伸算法的查找表。
    *
    * @param min 最小亮度值。
    * @param max 最大亮度值。
    * @return 单个通道查找表。
    */
    template <class T>
    const T * GetContrastLUT(T min, T max)
    {
      static thread_local T lut[ImageDefTraits<T>::LengthOfLUT];

      float scale = max > min ? (float)ImageDefTraits<T>::MaxValue / (max - min) : 0;

      int i = 0;
      for (; i < min; ++i)
      {
          lut[i] = 0;
      }
      for (; i <= max; ++i)
      {
          lut[i] = static_cast<T>(MBL::Utility::Clamp((i - min) * scale, ImageDefTraits<T>::MinValue, ImageDefTraits<T>::MaxValue));
      }
      for (; i < ImageDefTraits<T>::LengthOfLUT; ++i)
      {
          lut[i] = ImageDefTraits<T>::MaxValue;
      }
      
      return lut;
    }

    /**
     * @brief 校正图像色彩。
     *
     * @param image 欲处理图像，必须是RGB或BGR格式。
     * @param level 校正级别参数，为1～10。
     *
     * @author 陈进
     */
    template <class T>
    void CorrectImageColor(ImageDef<T> *image, int level)
    {
      if (image == 0 || (image->Format != IMAGE_FORMAT_RGB && image->Format != IMAGE_FORMAT_BGR)) throw UnsupportedFormatException();
      if (level < 1 || level > 10) throw IllegalArgumentException();

      int itemp1;
      int i,j;
      int f,f1,f2,f3;
      int s,s1,s2,s3;

      static double c1 = 0;
      static int tab[256];
      static int c = 0;

      if (level != c)
      {
        c = level;
        c1 = 0.15*(c-10)+2.5;

        for(i=0;i<256;i++)
        {
          tab[i] = (int)pow((double)i,c1);
        }
      }

      int n = image->Width * image->Height;
      T *lpBits = image->Pixels;
      for(i=0;i<n;i++)
      {
        j=i*3;
        f1 = lpBits[j];
        f2 = lpBits[j+1];
        f3 = lpBits[j+2];
        f = f1+f2+f3;
        s1 = tab[f1];
        s2 = tab[f2];
        s3 = tab[f3];
        s = s1+s2+s3;
        if (s ==0) s =1;
        itemp1 = (f*s1)/s;
        if (itemp1 > 255)
          lpBits[j] = 255;
        else
          lpBits[j] = itemp1;

        itemp1 = (f*s2)/s;
        if (itemp1 > 255)
          lpBits[j+1] = 255;
        else
          lpBits[j+1] = itemp1;

        itemp1 = (f*s3)/s;
        if (itemp1 > 255)
          lpBits[j+2] = 255;
        else
          lpBits[j+2] = itemp1;
      }
    }

    /**
     * @brief 校正图像色彩。
     *
     * 该函数比CorrectImageColor速度更快，效果更好，且支持色彩减弱。
     *
     * @param image 处理图像。
     * @param level 校正因子，在[-10, 10]区间， 0表示不做处理。
     * @author 陈进
     */
    template <class T>
    void CorrectImageColor2(ImageDef<T> *image, int level)
    {
      assert(image->Format == IMAGE_FORMAT_RGB && -10 <= level && level <= 10);

      static thread_local int old_level = 100;
      static thread_local int table[10 * ImageDefTraits<T>::LengthOfLUT];

      if (old_level != level)
      {
        old_level = level;

        double K = level;
        int m[9];

        // Calculate matrix
        if (K <= 0) K = (K + 10) / 10;
        if (K < 1) // reduce color
        {
          m[1] = int(1024 * (0.587 * (1 - K)));
          m[2] = int(1024 * (0.114 * (1 - K)));
          m[3] = int(1024 * (0.299 * (1 - K)));
          m[5] = int(1024 * (0.114 * (1 - K)));
          m[6] = int(1024 * (0.299 * (1 - K)));
          m[7] = int(1024 * (0.587 * (1 - K)));
          m[0] = int(1024 * (0.299 + 0.701 * K));
          m[4] = int(1024 * (0.587 + 0.413 * K));
          m[8] = int(1024 * (0.114 + 0.866 * K));
        }
        else // enhance color
        {
          if (level == 0)
          {
            m[1] = int(1024 * ((-K) / 20));
            m[2] = int(1024 * ((-K) / 20));
            m[3] = int(1024 * ((-K) / 20));
            m[5] = int(1024 * ((-K) / 20));
            m[6] = int(1024 * ((-K) / 20));
            m[7] = int(1024 * ((-K) / 20));
            m[0] = int(1024 - m[1] * 2);
            m[4] = int(1024 - m[1] * 2);
            m[8] = int(1024 - m[1] * 2);
          }
          else
          {
            m[1] = int((1024 * ((-K) / 10))); // simulate mciron matrix
            m[2] = int((1024 * ((-K) / 50)));
            m[3] = int((1024 * ((-K) / 50)));
            m[5] = int((1024 * ((-K) / 20)));
            m[6] = int((1024 * ((-K) / 50)));
            m[7] = int((1024 * ((-K) / 50)));
            m[0] = int((1024 - m[1] - m[2]));
            m[4] = int((1024 - m[3] - m[5]));
            m[8] = int((1024 - m[6] - m[7]));
          }
        }

        // Calculate hash table
        for (int j = 0; j < 9; j++)
        {
          for (int i = 0; i < ImageDefTraits<T>::LengthOfLUT; i++)
          {
            table[j * ImageDefTraits<T>::LengthOfLUT + i] = m[j] * i;
          }
        }
      }

      const int t0 = 0, t1 = 256, t2 = 512, t3 = 768, t4 = 1024, t5 = 1280, t6 = 1536, t7 = 1792, t8 = 2048;
	    T *p = nullptr;
      int r, g, b;
      int r1, g1, b1;
	    const int n = image->Width * image->Height;
      const int *pt = table; // Avoid parallel for cause another local storage of LUT.

#pragma omp parallel for firstprivate(p) private(r, g, b, r1, g1, b1)
      for (int i = 0; i < n; ++i)
      {
        if (p == nullptr)
        {
          p = image->Pixels + i * 3;
        }

        r1 = *p;
        g1 = *(p + 1);
        b1 = *(p + 2);
        r = (pt[t0 + r1] + pt[t1 + g1] + pt[t2 + b1]);
        g = (pt[t3 + r1] + pt[t4 + g1] + pt[t5 + b1]);
        b = (pt[t6 + r1] + pt[t7 + g1] + pt[t8 + b1]);
        if (r < ImageDefTraits<T>::MinValue) r = ImageDefTraits<T>::MinValue;
        if (g < ImageDefTraits<T>::MinValue) g = ImageDefTraits<T>::MinValue;
        if (b < ImageDefTraits<T>::MinValue) b = ImageDefTraits<T>::MinValue;
        r = r >> 10;
        g = g >> 10;
        b = b >> 10;
        if (r > ImageDefTraits<T>::MaxValue) r = ImageDefTraits<T>::MaxValue;
        if (g > ImageDefTraits<T>::MaxValue) g = ImageDefTraits<T>::MaxValue;
        if (b > ImageDefTraits<T>::MaxValue) b = ImageDefTraits<T>::MaxValue;
        *p++ = r;
        *p++ = g;
        *p++ = b;
      }
    }

    /**
     * @brief 取得校正图像Gamma的查找表。
     *
     * @param m_iGamma Gamma参数，为[0, 100]，50为无效果。
     * @param m_iShift Shift参数，为[0, 255]，0为无效果。
     *
     * @author 陈进
     */
    template <class T>
    const T * GetGammaLUT(int m_iGamma, int m_iShift)
    {
      assert (0 <= m_iGamma && m_iGamma <= 100 && 0 <= m_iShift && m_iShift <= 255);

      static thread_local T m_Table[ImageDefTraits<T>::LengthOfLUT];
      static thread_local int old_gamma = -1, old_shift = -1;

      if (m_iGamma != old_gamma || m_iShift != old_shift)
      {
        int shift;
        double scale, temp;
        int i, j;

        if (m_iGamma == 50)
        {
          scale = 1.0;
        }
        else if (m_iGamma > 50) // for gamma
        {
          scale = 0.01 - 0.99 / 50 * (m_iGamma - 100);
        }
        else // for reverse gamma
        {
          scale = 1 - 9.0 / 50 * (m_iGamma - 50);
        }
        if (scale < 0.12) scale = 0.12; // lower than 0.12 may cause numerical problem for pow function

        shift = (int) (ImageDefTraits<T>::MaxValue * (m_iShift - 99) / 99.0 + ImageDefTraits<T>::MaxValue);
        shift = MBL::Utility::Clamp(shift, ImageDefTraits<T>::MinValue, ImageDefTraits<T>::MaxValue);
        if (m_iGamma == 50)
        {
          for (i = 0; i < shift; i++)
          {
            m_Table[i] = 0;
          }
          for (i = shift; i < ImageDefTraits<T>::LengthOfLUT; i++)
          {
            m_Table[i] = i - shift;
          }
        }
        else
        {
          for (i = 0; i < shift; i++)
          {
            m_Table[i] = 0;
          }
          for (i = shift; i < ImageDefTraits<T>::LengthOfLUT; i++)
          {
            temp = (double)(i - shift) / ImageDefTraits<T>::LengthOfLUT;
            j = (int) (pow(temp, scale) * ImageDefTraits<T>::LengthOfLUT + 0.0001);
            m_Table[i] = MBL::Utility::Clamp(j, ImageDefTraits<T>::MinValue, ImageDefTraits<T>::MaxValue);
          }
        }

        old_gamma = m_iGamma;
        old_shift = m_iShift;
      }

      return m_Table;
    }

    /**
     * @brief 与SlideViewer.ImageProcessing.js一致的校正图像Gamma的查找表。
     *
     * @param gamma Gamma参数，为[0, infinity)，1.0为无效果。
     * @param shift Shift参数，为[0, 255]，0为无效果。
     *
     * @author 陈进
     */
    template <class T>
    const T * GetGammaLUT2(float gamma, int shift)
    {
        assert(0 <= gamma && 0 <= shift && shift <= 255);

        static thread_local T m_table[ImageDefTraits<T>::LengthOfLUT];
        static thread_local T m_tableGamma[ImageDefTraits<T>::LengthOfLUT];
        static thread_local T m_tableShift[ImageDefTraits<T>::LengthOfLUT];
        static thread_local float old_gamma = -1;
        static thread_local int old_shift = -1;

        if (gamma != old_gamma || shift != old_shift)
        {
            if (gamma == 1.0)
            {
                for (int i = 0; i < ImageDefTraits<T>::LengthOfLUT; i++)
                {
                    m_tableGamma[i] = i;
                }
            }
            else
            {
                for (int i = 0; i < ImageDefTraits<T>::LengthOfLUT; i++)
                {
                    double temp = i / 256.0 ;
                    double v = pow(temp, gamma) * 256.0 + 0.0001;
                    if (v > 255 ) v = 255;
                    else if (v < 0) v = 0;
                    m_tableGamma[i] = (int)(v);
                }
            }

            int rest = 255 - shift;
            if (gamma < 1.0) // enhance lower part
            {
                int i = 0;
                for (i = 0; i <= shift; i++)
                {
                    m_tableShift[i] = 0;
                }
                for (i = shift + 1; i < 256; i++)
                {
                    m_tableShift[i] = (int)((((i - shift) / (rest * 1.0)) * 255));
                }
            }
            else // enhance higher part
            {
                int i;
                for (i = 255; i >= rest; i--)
                {
                    m_tableShift[i] = 255;
                }
                for (i = rest - 1; i >= 0; i--)
                {
                    m_tableShift[i] = (int)(((i * 255) / rest));
                }
            }

            for (int i = 0; i < 256; i++)
            {
                m_table[i] = m_tableShift[m_tableGamma[i]];
            }

            old_gamma = gamma;
            old_shift = shift;
        }

        return m_table;
    }

    /**
     * @brief 校正图像Gamma值。
     *
     * @param image 欲处理图像。
     * @param m_iGamma Gamma参数，为0～100，缺省为50。
     * @param m_iShift Shift参数，为0～255，缺省为0。
     *
     * @author 陈进
     */
    template <class T>
    void CorrectImageGamma(ImageDef<T> *image, int m_iGamma, int m_iShift)
    {
      const T *lut = GetGammaLUT<T>(m_iGamma, m_iShift);
      ApplyImageLUT(image, lut, lut, lut);
    }

    /**
     * @brief 将一个灰度索引图像转换成彩色图像。
     *
     * 该函数将一个单通道的索引图像转换为彩色图像，RGB各个通道的值相同。灰度图像可以存储在彩色图像的前1/3内存空间。
     *
     * @param gray 灰度索引图像。
     * @param rgb 转换后的彩色图像。
     */
    template <class T>
    void ConvertImageGray2RGB(ImageDef<T> *gray, ImageDef<T> *rgb)
    {
      assert(gray->Format == IMAGE_FORMAT_INDEX && (rgb->Format == IMAGE_FORMAT_RGB || rgb->Format == IMAGE_FORMAT_BGR));
      assert(gray->Width == rgb->Width && gray->Height == rgb->Height);

      T *p0 = gray->Pixels;
      T *p1 = gray->Pixels + gray->Width * gray->Height - 1;
      T *p2 = rgb->Pixels + rgb->Width * rgb->Height * 3 - 1;

      while (p1 >= p0)
      {
        *p2-- = *p1;
        *p2-- = *p1;
        *p2-- = *p1--;
      }
    }

    /**
     * @brief 将一个YUV格式的图像转换成RGB/BGR格式的图像。
     *
     * 计算公式如下，采用的是YUV颜色空间派生的一种颜色空间 YCbCr ITU-R BT.601：
     *
     * <PRE>
	   * R = 1.164(Y - 16) + 1.596(Cr - 128)
     * G = 1.164(Y - 16) - 0.813(Cr - 128) - 0.391(Cb - 128)
     * B = 1.164(Y - 16) + 2.018(Cb - 128)
     * 其中 16 < Y < 235，16 < Cb、Cr <240，  0 < = RGB < = 255。
     * </PRE>
     *
     * @param yuv 输入YUV图像，目前只支持YUV422格式。
     * @param rgb 输出RGB/BGR图像，为RGB 8:8:8格式。
     *
     * 转换时，YUV 和 RGB/BGR 可以共用一段缓冲内存，假设图像高为height, 宽为width，则分配一段缓冲内存3 * width * height。
     * YUV的一帧图像可以读入到缓冲内存的后 2/3 处，生成的RGB/BGR图像则可以再存储在这段内存中。例如：
     *
     * @code
	   * ImageDef *yuv = CreateWrapperInstance(IMAGE_FORMAT_YUYV, buf + width * height, width, height);
	   * ImageDef *rgb = CreateWrapperInstance(IMAGE_FORMAT_RGB, buf, width, height);
	   * ConvertImageYUYV2RGB(yuv, rgb);
     * @endcode
	   *
     * @author 陈伟卿
     */
    template <class T>
    void ConvertImageYUYV2RGB(ImageDef<T> *yuv, ImageDef<T> *rgb)
    {
      assert(yuv->Format == IMAGE_FORMAT_YUV422_PACKED);

      int width = yuv->Width;
      int height = yuv->Height;
      T *pin = yuv->Pixels;
      T *pout = rgb->Pixels;

      int precalc_xy = 0, precalc_xy_raw = 0;
	    int uc0, uc1, uc2, uc3, Ig;
	    static bool flag = true;
	    static int u308[256], u256[256];

	    static unsigned char YCr2R[256 * 256];
	    static unsigned char YCb2B[256 * 256];
	    static short CrCb2Ig[256 * 256];
	    static unsigned char YIg2G[256 * 308];

      if (flag == true)
  	  {
        int m, n,row,row1;
        float ft, f1, f2, f3;
        float f164[256];

    		for (m = 0; m < 256; m++)
    			f164[m] = 1.164f * (m - 16);

    		f2= (0.813f + 0.391f) * 127.0f;

    		//建立YCr2R表, YCb2B表, CrCb2Ig表, YIg2G表
    		for (m = 0; m < 256; m++)
    		{
    			u308[m]= m * 308;
    			u256[m]= m * 256;

    			row = m * 256;
    			row1 = m * 308;
    			f1 = f164[m];
    			f3 = -0.813f * (m - 128);
    			for (n = 0; n < 256; n++)
    			{
    				ft = f1 + 1.596f * (n - 128);
    				if (ft < 0.0f ) ft = 0.0f;
    				if (ft > 255.0f) ft = 255.0f;
    				*(YCr2R + row + n) = (unsigned char)(ft+0.5f);// +0.5f 是避免向下取整的偏差

    				ft = f1 + 2.018f * (n - 128);
    				if (ft < 0.0f) ft = 0.0f;
    				if (ft > 255.0f) ft = 255.0f;
    				*(YCb2B + row + n) = (unsigned char)(ft+0.5f);

    				ft = f3 - 0.391f * (n - 128) + f2;
    				*(CrCb2Ig + row + n) = (short)(ft+0.5f); //*(CrCb2Ig+i) 的值范围为 0 到307.
    			}

    			for (n = 0; n < 308; n++)
    			{
    				ft = f1 + n - f2;
    				if (ft < 0.0f)	ft = 0.0f;
    				if (ft > 255.0f)	ft = 255.0f;
    				*(YIg2G + row1 + n) = (unsigned char)(ft+0.5f);
    			}
    		}

  		  flag = false;
  	  }

  	  //读取源图像并做转换
      if (rgb->Format == IMAGE_FORMAT_RGB)
      {
        for (int i = 0, k = width * height / 2; i< k; i++)
        {
          uc0 = pin[precalc_xy_raw++];
          uc1 = pin[precalc_xy_raw++];
          uc2 = pin[precalc_xy_raw++];
          uc3 = pin[precalc_xy_raw++];
          Ig = *(CrCb2Ig + u256[uc2] + uc0);

          pout[precalc_xy++] = *(YCr2R + u256[uc1] + uc2);
          pout[precalc_xy++] = *(YIg2G+ u308[uc1] + Ig);
          pout[precalc_xy++] = *(YCb2B + u256[uc1] + uc0);

          pout[precalc_xy++] = *(YCr2R + u256[uc3] + uc2);
          pout[precalc_xy++] = *(YIg2G + u308[uc3] + Ig);
          pout[precalc_xy++] = *(YCb2B + u256[uc3] + uc0);
        }
      }
      else
      {
        for (int i = 0, k = width * height / 2; i< k; i++)
        {
          uc0 = pin[precalc_xy_raw++];
          uc1 = pin[precalc_xy_raw++];
          uc2 = pin[precalc_xy_raw++];
          uc3 = pin[precalc_xy_raw++];
          Ig = *(CrCb2Ig + u256[uc2] + uc0);

          pout[precalc_xy++] = *(YCb2B + u256[uc1] + uc0);
          pout[precalc_xy++] = *(YIg2G+ u308[uc1] + Ig);
          pout[precalc_xy++] = *(YCr2R + u256[uc1] + uc2);

          pout[precalc_xy++] = *(YCb2B + u256[uc3] + uc0);
          pout[precalc_xy++] = *(YIg2G + u308[uc3] + Ig);
          pout[precalc_xy++] = *(YCr2R + u256[uc3] + uc2);
        }
      }
    }
    
    /**
     * @brief 将24位RGB数据转换为YUV420数据块。
     *
     * 在DVD、摄像机、数字电视等消费类视频产品中，常用的色彩编码方案是YCbCr，其中Y是指亮度分量，Cb指蓝色色度分量，而Cr指红色色度分量。
     * 人的肉眼对视频的Y分量更敏感，因此在通过对色度分量进行子采样来减少色度分量后，肉眼将察觉不到的图像质量的变化。
     * 主要的子采样格式有YCbCr 4:2:0、YCbCr 4:2:2 和 YCbCr 4:4:4。
     * 4:2:0表示每4个像素有4个亮度分量，2个色度分量(YYYYCbCr)，仅采样奇数扫描线，是便携式视频设备(MPEG-4)以及电视会议(H.263)最常用格式；
     * 4：2：2表示每4个像素有4个亮度分量，4个色度分量(YYYYCbCrCbCr)，是DVD、数字电视、HDTV 以及其它消费类视频设备的最常用格式；
     * 4：4：4表示全像素点阵(YYYYCbCrCbCrCbCrCbCr)，用于高质量视频应用、演播室以及专业视频产品。
     *
     * Y = (0.257 * R) + (0.504 * G) + (0.098 * B) + 16
     * Cr = V = (0.439 * R) - (0.368 * G) - (0.071 * B) + 128
     * Cb = U = -(0.148 * R) - (0.291 * G) + (0.439 * B) + 128
     *
     * @param rgb RGB数据块,目前必须是BGR格式。
     * @param yuv 输出YUV数据缓冲区，外部必须保证缓冲区的长度可以容纳转换后的数据长度。
     *
     * @author 栗远
     */
	template <class T>
	void ConvertImageRGB2YUV420(ImageDef<T> *rgb, ImageDef<T> *yuv)
	{
		assert(rgb->Format == IMAGE_FORMAT_BGR);
		assert(yuv->Format == IMAGE_FORMAT_YUV420_PLANAR);
		
		T *RgbBuf = rgb->Pixels;
		T *bufY = yuv->Pixels;
		T *bufV = yuv->Pixels + rgb->Width * rgb->Height;
		T *bufU = bufV + ((rgb->Width + 1) / 2) * (rgb->Height / 2);
		T *bufRGB = RgbBuf;
		T r, g, b;
		
		for (int j = 0; j < rgb->Height; j++)
		{
		bufRGB = RgbBuf + rgb->Width * j * 3;
		for (int i = 0; i < rgb->Width; i++)
		{
		    b = *(bufRGB++);
		    g = *(bufRGB++);
		    r = *(bufRGB++);
		    *(bufY++) = ((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
		      
		    if (i % 2 == 0)
		    {
		    if (j % 2 == 0)
		        *(bufU++) = ((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
		    else
		        *(bufV++) = ((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
		    }		
		}
		}
	}

    /**
     * @brief 计算一幅图像的平均亮度。
     *
     * 该函数一般用于自动曝光过程中评估图像的亮度。
     *
     * @param image 图像指针，目前只支持计算 Bayer 图像和 RGB，BGR图像的平均亮度。
     * @return 该图像的平均亮度。
	   *
     * @author 陈伟卿
     */
    template <class T>
    T GetImageAverageBrightness(ImageDef<T> *image)
    {
      switch (image->Format)
      {
        case IMAGE_FORMAT_BAYER_GR_BG:
        case IMAGE_FORMAT_BAYER_BG_GR:
        case IMAGE_FORMAT_BAYER_GB_RG:
        case IMAGE_FORMAT_BAYER_RG_GB:
          return GetBayerAverageBrightness(image); //Byer 格式图像计算平均亮度。
          break;
        case IMAGE_FORMAT_RGB:
        case IMAGE_FORMAT_BGR:
          break;
        default:
          throw UnsupportedFormatException();
          break;
      }
      // RGB 或 BGR 格式计算平均亮度。
      T *ptr = image->Pixels;
      long R = 0, G = 0, B = 0;

      int n = image->Width * image->Height;
      for (int i = 0; i < n; i++)
      {
        R += *(ptr++);
        G += *(ptr++);
        B += *(ptr++);
      }

      R /= n;
      G /= n;
      B /= n;
      if (image->Format == IMAGE_FORMAT_BGR) MBL::Utility::Swap(&R, &B);

      long avg = (299 * R + 587 * G + 114 * B) / 1000;
      return MBL::Utility::GetMin(ImageDefTraits<T>::MaxValue, static_cast<T>(avg));
    }

    /**
     * @brief 将一副索引图像的调色板设置为伪彩色，以方便显示。
     *
     * 调色板的第一个颜色为黑色。最后一个颜色为白色。
     *
     * @param img 待处理的索引图像，该图像原有的调色板会被改变。
     */
    template <class T>
    void SetPesudoColorPalette(ImageDef<T> *img)
    {
      assert(img->Format == IMAGE_FORMAT_INDEX);

      //由PhotoShop转换来的Windows系统调色板。
      unsigned char pal[256 * 3] = {0x00, 0x00, 0x00,0x80, 0x00, 0x00,0x00, 0x80, 0x00,0x80, 0x80, 0x00,
                                    0x00, 0x00, 0x80,0x80, 0x00, 0x80,0x00, 0x80, 0x80,0x80, 0x80, 0x80,
                                    0xC0, 0xDC, 0xC0,0xA6, 0xCA, 0xF0,0x2A, 0x3F, 0xAA,0x2A, 0x3F, 0xFF,
                                    0x2A, 0x5F, 0x00,0x2A, 0x5F, 0x55,0x2A, 0x5F, 0xAA,0x2A, 0x5F, 0xFF,
                                    0x2A, 0x7F, 0x00,0x2A, 0x7F, 0x55,0x2A, 0x7F, 0xAA,0x2A, 0x7F, 0xFF,
                                    0x2A, 0x9F, 0x00,0x2A, 0x9F, 0x55,0x2A, 0x9F, 0xAA,0x2A, 0x9F, 0xFF,
                                    0x2A, 0xBF, 0x00,0x2A, 0xBF, 0x55,0x2A, 0xBF, 0xAA,0x2A, 0xBF, 0xFF,
                                    0x2A, 0xDF, 0x00,0x2A, 0xDF, 0x55,0x2A, 0xDF, 0xAA,0x2A, 0xDF, 0xFF,
                                    0x2A, 0xFF, 0x00,0x2A, 0xFF, 0x55,0x2A, 0xFF, 0xAA,0x2A, 0xFF, 0xFF,
                                    0x55, 0x00, 0x00,0x55, 0x00, 0x55,0x55, 0x00, 0xAA,0x55, 0x00, 0xFF,
                                    0x55, 0x1F, 0x00,0x55, 0x1F, 0x55,0x55, 0x1F, 0xAA,0x55, 0x1F, 0xFF,
                                    0x55, 0x3F, 0x00,0x55, 0x3F, 0x55,0x55, 0x3F, 0xAA,0x55, 0x3F, 0xFF,
                                    0x55, 0x5F, 0x00,0x55, 0x5F, 0x55,0x55, 0x5F, 0xAA,0x55, 0x5F, 0xFF,
                                    0x55, 0x7F, 0x00,0x55, 0x7F, 0x55,0x55, 0x7F, 0xAA,0x55, 0x7F, 0xFF,
                                    0x55, 0x9F, 0x00,0x55, 0x9F, 0x55,0x55, 0x9F, 0xAA,0x55, 0x9F, 0xFF,
                                    0x55, 0xBF, 0x00,0x55, 0xBF, 0x55,0x55, 0xBF, 0xAA,0x55, 0xBF, 0xFF,
                                    0x55, 0xDF, 0x00,0x55, 0xDF, 0x55,0x55, 0xDF, 0xAA,0x55, 0xDF, 0xFF,
                                    0x55, 0xFF, 0x00,0x55, 0xFF, 0x55,0x55, 0xFF, 0xAA,0x55, 0xFF, 0xFF,
                                    0x7F, 0x00, 0x00,0x7F, 0x00, 0x55,0x7F, 0x00, 0xAA,0x7F, 0x00, 0xFF,
                                    0x7F, 0x1F, 0x00,0x7F, 0x1F, 0x55,0x7F, 0x1F, 0xAA,0x7F, 0x1F, 0xFF,
                                    0x7F, 0x3F, 0x00,0x7F, 0x3F, 0x55,0x7F, 0x3F, 0xAA,0x7F, 0x3F, 0xFF,
                                    0x7F, 0x5F, 0x00,0x7F, 0x5F, 0x55,0x7F, 0x5F, 0xAA,0x7F, 0x5F, 0xFF,
                                    0x7F, 0x7F, 0x00,0x7F, 0x7F, 0x55,0x7F, 0x7F, 0xAA,0x7F, 0x7F, 0xFF,
                                    0x7F, 0x9F, 0x00,0x7F, 0x9F, 0x55,0x7F, 0x9F, 0xAA,0x7F, 0x9F, 0xFF,
                                    0x7F, 0xBF, 0x00,0x7F, 0xBF, 0x55,0x7F, 0xBF, 0xAA,0x7F, 0xBF, 0xFF,
                                    0x7F, 0xDF, 0x00,0x7F, 0xDF, 0x55,0x7F, 0xDF, 0xAA,0x7F, 0xDF, 0xFF,
                                    0x7F, 0xFF, 0x00,0x7F, 0xFF, 0x55,0x7F, 0xFF, 0xAA,0x7F, 0xFF, 0xFF,
                                    0xAA, 0x00, 0x00,0xAA, 0x00, 0x55,0xAA, 0x00, 0xAA,0xAA, 0x00, 0xFF,
                                    0xAA, 0x1F, 0x00,0xAA, 0x1F, 0x55,0xAA, 0x1F, 0xAA,0xAA, 0x1F, 0xFF,
                                    0xAA, 0x3F, 0x00,0xAA, 0x3F, 0x55,0xAA, 0x3F, 0xAA,0xAA, 0x3F, 0xFF,
                                    0xAA, 0x5F, 0x00,0xAA, 0x5F, 0x55,0xAA, 0x5F, 0xAA,0xAA, 0x5F, 0xFF,
                                    0xAA, 0x7F, 0x00,0xAA, 0x7F, 0x55,0xAA, 0x7F, 0xAA,0xAA, 0x7F, 0xFF,
                                    0xAA, 0x9F, 0x00,0xAA, 0x9F, 0x55,0xAA, 0x9F, 0xAA,0xAA, 0x9F, 0xFF,
                                    0xAA, 0xBF, 0x00,0xAA, 0xBF, 0x55,0xAA, 0xBF, 0xAA,0xAA, 0xBF, 0xFF,
                                    0xAA, 0xDF, 0x00,0xAA, 0xDF, 0x55,0xAA, 0xDF, 0xAA,0xAA, 0xDF, 0xFF,
                                    0xAA, 0xFF, 0x00,0xAA, 0xFF, 0x55,0xAA, 0xFF, 0xAA,0xAA, 0xFF, 0xFF,
                                    0xD4, 0x00, 0x00,0xD4, 0x00, 0x55,0xD4, 0x00, 0xAA,0xD4, 0x00, 0xFF,
                                    0xD4, 0x1F, 0x00,0xD4, 0x1F, 0x55,0xD4, 0x1F, 0xAA,0xD4, 0x1F, 0xFF,
                                    0xD4, 0x3F, 0x00,0xD4, 0x3F, 0x55,0xD4, 0x3F, 0xAA,0xD4, 0x3F, 0xFF,
                                    0xD4, 0x5F, 0x00,0xD4, 0x5F, 0x55,0xD4, 0x5F, 0xAA,0xD4, 0x5F, 0xFF,
                                    0xD4, 0x7F, 0x00,0xD4, 0x7F, 0x55,0xD4, 0x7F, 0xAA,0xD4, 0x7F, 0xFF,
                                    0xD4, 0x9F, 0x00,0xD4, 0x9F, 0x55,0xD4, 0x9F, 0xAA,0xD4, 0x9F, 0xFF,
                                    0xD4, 0xBF, 0x00,0xD4, 0xBF, 0x55,0xD4, 0xBF, 0xAA,0xD4, 0xBF, 0xFF,
                                    0xD4, 0xDF, 0x00,0xD4, 0xDF, 0x55,0xD4, 0xDF, 0xAA,0xD4, 0xDF, 0xFF,
                                    0xD4, 0xFF, 0x00,0xD4, 0xFF, 0x55,0xD4, 0xFF, 0xAA,0xD4, 0xFF, 0xFF,
                                    0xFF, 0x00, 0x55,0xFF, 0x00, 0xAA,0xFF, 0x1F, 0x00,0xFF, 0x1F, 0x55,
                                    0xFF, 0x1F, 0xAA,0xFF, 0x1F, 0xFF,0xFF, 0x3F, 0x00,0xFF, 0x3F, 0x55,
                                    0xFF, 0x3F, 0xAA,0xFF, 0x3F, 0xFF,0xFF, 0x5F, 0x00,0xFF, 0x5F, 0x55,
                                    0xFF, 0x5F, 0xAA,0xFF, 0x5F, 0xFF,0xFF, 0x7F, 0x00,0xFF, 0x7F, 0x55,
                                    0xFF, 0x7F, 0xAA,0xFF, 0x7F, 0xFF,0xFF, 0x9F, 0x00,0xFF, 0x9F, 0x55,
                                    0xFF, 0x9F, 0xAA,0xFF, 0x9F, 0xFF,0xFF, 0xBF, 0x00,0xFF, 0xBF, 0x55,
                                    0xFF, 0xBF, 0xAA,0xFF, 0xBF, 0xFF,0xFF, 0xDF, 0x00,0xFF, 0xDF, 0x55,
                                    0xFF, 0xDF, 0xAA,0xFF, 0xDF, 0xFF,0xFF, 0xFF, 0x55,0xFF, 0xFF, 0xAA,
                                    0xCC, 0xCC, 0xFF,0xFF, 0xCC, 0xFF,0x33, 0xFF, 0xFF,0x66, 0xFF, 0xFF,
                                    0x99, 0xFF, 0xFF,0xCC, 0xFF, 0xFF,0x00, 0x7F, 0x00,0x00, 0x7F, 0x55,
                                    0x00, 0x7F, 0xAA,0x00, 0x7F, 0xFF,0x00, 0x9F, 0x00,0x00, 0x9F, 0x55,
                                    0x00, 0x9F, 0xAA,0x00, 0x9F, 0xFF,0x00, 0xBF, 0x00,0x00, 0xBF, 0x55,
                                    0x00, 0xBF, 0xAA,0x00, 0xBF, 0xFF,0x00, 0xDF, 0x00,0x00, 0xDF, 0x55,
                                    0x00, 0xDF, 0xAA,0x00, 0xDF, 0xFF,0x00, 0xFF, 0x55,0x00, 0xFF, 0xAA,
                                    0x2A, 0x00, 0x00,0x2A, 0x00, 0x55,0x2A, 0x00, 0xAA,0x2A, 0x00, 0xFF,
                                    0x2A, 0x1F, 0x00,0x2A, 0x1F, 0x55,0x2A, 0x1F, 0xAA,0x2A, 0x1F, 0xFF,
                                    0x2A, 0x3F, 0x00,0x2A, 0x3F, 0x55,0xFF, 0xFB, 0xF0,0xA0, 0xA0, 0xA4,
                                    0x80, 0x80, 0x80,0xFF, 0x00, 0x00,0x00, 0xFF, 0x00,0xFF, 0xFF, 0x00,
                                    0x00, 0x00, 0xFF,0xFF, 0x00, 0xFF,0x00, 0xFF, 0xFF,0xFF, 0xFF, 0xFF};

      if (img->Palette == 0)
      {
        if (img->UsedColor == 0)
        {
          img->UsedColor = ImageDefTraits<T>::LengthOfLUT;
        }
        img->Palette = new ImageRGBQUAD[img->UsedColor];
      }
      ImageRGBQUAD *p = img->Palette;
      for (int i = 0, j = 0, n = ImageDefTraits<T>::LengthOfLUT; i < n; i++)
      {
        j = 3 * i * 256 / n;
        p[i].Red = pal[j];
        p[i].Green = pal[j + 1];
        p[i].Blue = pal[j + 2];
        p[i].Reserved = 0;
      }
    }

    /**
     * @brief 对图像进行查找表映射处理。
     *
     * @param img 图像指针。
     * @param r_lut 红色通道、或者单色索引通道的查找表。
     * @param g_lut 绿色通道查找表。
     * @param b_lut 蓝色通道查找表。
     */
    template <class T>
    void ApplyImageLUT(ImageDef<T> *img, const T *r_lut, const T *g_lut = 0, const T *b_lut = 0)
    {
      T *p = img->Pixels;
      const int n = img->Width * img->Height;

      switch (img->Format)
      {
        case IMAGE_FORMAT_RGB:
          p = nullptr;
#pragma omp parallel for firstprivate(p)
          for (int i = 0; i < n; ++i)
          {
            if (p == nullptr)
            {
              p = img->Pixels + i * 3;
            }
            *p = r_lut[*p]; ++p;
            *p = g_lut[*p]; ++p;
            *p = b_lut[*p]; ++p;
          }
          break;
        case IMAGE_FORMAT_BGR:
          for (int i = 0; i < n; ++i)
          {
            *p = b_lut[*p]; ++p;
            *p = g_lut[*p]; ++p;
            *p = r_lut[*p]; ++p;
          }
          break;
        case IMAGE_FORMAT_RGBA:
          for (int i = 0; i < n; ++i)
          {
            *p = r_lut[*p]; ++p;
            *p = g_lut[*p]; ++p;
            *p = b_lut[*p]; ++p;
            p++;
          }
          break;
        case IMAGE_FORMAT_ARGB:
          for (int i = 0; i < n; ++i)
          {
            p++;
            *p = r_lut[*p]; ++p;
            *p = g_lut[*p]; ++p;
            *p = b_lut[*p]; ++p;
          }
          break;
        case IMAGE_FORMAT_BAYER_GR_BG:
          for (int y = 0, x_end = img->Width / 2,  y_end = img->Height / 2; y < y_end; ++y)
          {
            for(int x = 0; x < x_end; ++x)
            {
              *p = g_lut[*p]; ++p;
              *p = r_lut[*p]; ++p;
            }
            for(int x = 0; x < x_end; ++x)
            {
              *p = b_lut[*p]; ++p;
              *p = g_lut[*p]; ++p;
            }
          }
          break;
        case IMAGE_FORMAT_BAYER_BG_GR:
          for (int y = 0, x_end = img->Width / 2,  y_end = img->Height / 2; y < y_end; ++y)
          {
            for(int x = 0; x < x_end; ++x)
            {
              *p = b_lut[*p]; ++p;
              *p = g_lut[*p]; ++p;
            }
            for(int x = 0; x < x_end; ++x)
            {
              *p = g_lut[*p]; ++p;
              *p = r_lut[*p]; ++p;
            }
          }
          break;
        case IMAGE_FORMAT_BAYER_GB_RG:
          for (int y = 0, x_end = img->Width / 2,  y_end = img->Height / 2; y < y_end; ++y)
          {
            for(int x = 0; x < x_end; ++x)
            {
              *p = g_lut[*p]; ++p;
              *p = b_lut[*p]; ++p;
            }
            for(int x = 0; x < x_end; ++x)
            {
              *p = r_lut[*p]; ++p;
              *p = g_lut[*p]; ++p;
            }
          }
          break;
        case IMAGE_FORMAT_BAYER_RG_GB:
          for (int y = 0, x_end = img->Width / 2,  y_end = img->Height / 2; y < y_end; ++y)
          {
            for(int x = 0; x < x_end; ++x)
            {
              *p = r_lut[*p]; ++p;
              *p = g_lut[*p]; ++p;
            }
            for(int x = 0; x < x_end; ++x)
            {
              *p = g_lut[*p]; ++p;
              *p = b_lut[*p]; ++p;
            }
          }
          break;
        default:
          throw UnsupportedFormatException();
      }
    }

    /**
    * @brief 合并查找表。
    *
    * @param src_r 源红色通道查找表。
    * @param src_g 源绿色通道查找表。
    * @param src_b 源蓝色通道查找表。
    * @param src_r2 源红色通道查找表。
    * @param src_g2 源绿色通道查找表。
    * @param src_b2 源蓝色通道查找表。
    * @param dst_r 结果红色通道查找表。
    * @param dst_g 结果绿色通道查找表。
    * @param dst_b 结果蓝色通道查找表。
    */
    template <class T>
    void CombineImageLUT(const T *src_r, const T *src_g, const T *src_b, const T *src_r2, const T *src_g2, const T *src_b2, T *dst_r, T *dst_g, T *dst_b)
    {
        for (int i = 0; i < ImageDefTraits<T>::LengthOfLUT; i++)
        {
            dst_r[i] = src_r2[src_r[i]];
            dst_g[i] = src_g2[src_g[i]];
            dst_b[i] = src_b2[src_b[i]];
        }
    }
  }
}

#endif // __IMAGECOLOR_H__
