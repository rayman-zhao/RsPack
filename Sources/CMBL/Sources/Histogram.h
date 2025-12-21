#ifndef __HISTOGRAM_H__
#define __HISTOGRAM_H__

#include <limits>

/**
 * @file
 *
 * @brief 包含直方图相关处理的函数。
 */

namespace MBL
{
  namespace Image2D
  {
    /**
     * @brief 取得一幅图像的直方图。
     *
     * @param image 源图像，目前必须是索引图像，不能是彩色图像。
     * @param buf 预先分配好的直方图数据缓冲区，尺寸必须足够存储数据。如8bit图像为256个元素，16bit图像为65536个元素。
     */
    template <class T>
    void GetImageHistogram(ImageDef<T> *image, int *buf)
    {
      T *pixels;
    	int units;
    	units = GetUnitsPerRow(image) * image->Height;
    	memset(buf, 0, ImageDefTraits<T>::LengthOfLUT *sizeof(int));
    	pixels = image->Pixels;

  		switch (image->Format)
  		{
  		  case IMAGE_FORMAT_INDEX:
  		    for (int i = 0; i < units; i++, pixels++)
  		    {
  		      buf[*pixels]++;
  		    }
  		    break;
        default:
          throw UnsupportedFormatException();
  			  break;
  		}
    }

    /**
     * @brief 取得一幅RGB彩色图像的直方图。
     *
     * @param image 源图像，必须是彩色图像。
     * @param sub_area 图像子区对象，如果为0则提取整副图像的直方图。
     * @param r_buf 预先分配好的直方图红色通道数据缓冲区，尺寸必须足够存储数据。如8bit图像为256个元素，16bit图像为65536个元素。
     * @param g_buf 预先分配好的直方图绿色通道数据缓冲区，尺寸必须足够存储数据。如8bit图像为256个元素，16bit图像为65536个元素。
     * @param b_buf 预先分配好的直方图蓝色通道数据缓冲区，尺寸必须足够存储数据。如8bit图像为256个元素，16bit图像为65536个元素。
     */
    template <class T>
    void GetImageRGBHistogram(ImageDef<T> *image, ImageSubArea *sub_area, int *r_buf, int *g_buf, int *b_buf)
    {
      if (image->Format != IMAGE_FORMAT_RGB && image->Format != IMAGE_FORMAT_BGR) throw UnsupportedFormatException();

    	T max_T;
    	MBL::Utility::GetMaxValue(&max_T);
    	memset(r_buf, 0, (max_T + 1) *sizeof(int));
    	memset(g_buf, 0, (max_T + 1) *sizeof(int));
    	memset(b_buf, 0, (max_T + 1) *sizeof(int));

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

      T buf[8];
      for (int y = top; y < bottom; y++)
      {
        for (int x = left; x < right; x++)
        {
          if (sub_area == 0 || sub_area->IsFill(x, y))
          {
            ReadPixel(image, x, y, buf);

            switch (image->Format)
        		{
        		  case IMAGE_FORMAT_RGB:
      		      r_buf[buf[0]]++;
       		      g_buf[buf[1]]++;
       		      b_buf[buf[2]]++;
        		    break;
        		  case IMAGE_FORMAT_BGR:
       		      b_buf[buf[0]]++;
       		      g_buf[buf[1]]++;
       		      r_buf[buf[2]]++;
        		    break;
              default:
        			  break;
        		}
          }
        }
      }
    }

    /**
     * @brief 取得一个直方图的Mode，即峰值的灰度级。
     *
     * @param histogram 直方图。
     * @param length 直方图长度，缺省为-1，包括T类型的所有可能值。
     * @return 直方图的Mode。
     */
    template <class T>
    T GetHistogramMode(int *histogram, int length = -1)
    {
      if (length == -1) length = ImageDefTraits<T>::LengthOfLUT;
      T mode = 0;

      for (int i = 1; i < length; ++i)
      {
        if (histogram[i] > histogram[mode])
        {
          mode = i;
        }
      }

      return mode;
    }

    /**
     * @brief 取得直方图的范围，即最小和最大值。
     *
     * @param histogram 直方图。
     * @param length 直方图长度，缺省为-1，包括T类型的所有可能值。
     * @return 直方图的最小和最大值。
     */
    template <class T>
    std::pair<T, T> GetHistogramRange(int *histogram, int length = -1)
    {
      if (length == -1) length = ImageDefTraits<T>::LengthOfLUT;
      std::pair<T, T> m;

      for (int i = 0; i < length; ++i)
      {
        if (histogram[i] != 0)
        {
          m.first = i;
          break;
        }
      }
      for (int i = length - 1; i >= 0; --i)
      {
        if (histogram[i] != 0)
        {
          m.second = i;
          break;
        }
      }

      return m;
    }

    /**
     * @brief 取得直方图的标准差和相关系数。
     *
     * @param histogram 直方图。
     * @param length 直方图长度，缺省包括T类型的所有可能值。
     * @return 标准差STD和相关系数CV。
     */
    template <class T>
    std::pair<double, double> GetHistogramSD_CV(int *histogram, int length = -1)
    {
      if (length == -1) length = ImageDefTraits<T>::LengthOfLUT;
      double mean = 0, dval = 0, std = 0, cv = 0;
      int cnt = 0;

      for (int i = 0; i < length; i++)
      {
        cnt += histogram[i];
        mean += histogram[i] * i;
      }
      if (cnt > 0)
      {
        mean /= cnt;
        for (int i = 0; i < length; i++)
        {
          dval = (mean - i);
          std += histogram[i] * dval * dval;
        }
        std /= (cnt - 1);
        std = sqrt(std);
        cv = 100 * std / mean;
      }

      return std::make_pair(std, cv);
    }

    /**
     * @brief 根据直方图计算其积分光密度。
     *
     * 计算公式为：OD = sum(log10(background/Ixy))。
     *
     * @param background 背景值。
     * @param histogram 直方图。
     * @return 积分光密度。
     */
    template <class T>
    double GetOpticalDensity(T background, int *histogram)
    {
      double od = 0;
      static double map[256][256];

      if (background > 0)
      {
        for (int i = 1; i < background; i++)
        {
          if (histogram[i] > 0)
          {
            if (map[background][i] == 0) // 没有初始化。
            {
              map[background][i] = log10((double)background) - log10((double)i);
            }
            od += map[background][i] * histogram[i];
          }
        }
      }

      return od;
    }

    /**
     * @brief 直方图大津阈值分割计算算法。
     *
     * @param histogram 直方图数据。
     * @param length 直方图长度。
     * @param value 将保存直方图的大津阈值。
     */
    template <class T>
    void GetHistogramThreshold(int *histogram, int length, T *value)
    {
      double *PHS = new double[length];
      double A, B, D, P, RH, RM, RMEAN, RI, BMAX;
      double EPS = 1.0e-10;
      int Tab = 0;
      int Line, JTHD = 0;

      for (Line = 0; Line < length; Line++)
      {
        Tab += histogram[Line];
      }
      for (Line = 0; Line < length; Line++)
      {
        PHS[Line] = (double)histogram[Line] / Tab;
      }
      RM = 0.0;
      for (Line = 0; Line < length; Line++)
      {
        RI = Line;
        RH = RI * PHS[Line];
        RM = RM + RH;
      }
      RMEAN = RM;
      BMAX = 0.0;
      P = 0.0;
      A=0.0;
      for (Line = 0; Line < length; Line++)
      {
        RH = PHS[Line];
        P = P + RH;
        A = A + Line * RH;
        B = RM * P - A;
        D = P * (1.0 - P);
        if (D < EPS)
        {
          continue;
        }
        B = B * B / D;
        if (B >= BMAX)
        {
          BMAX=B;
          JTHD=Line;
        }
      }
      *value = JTHD;

      delete [] PHS;
    }

    /**
     * @brief 用阈值方法分割一幅图像。
     *
     * 该函数用大津阈值方法对一幅图像进行分割，目标为较暗的部分（背景较亮），分割后源图像被改变。
     *
     * @param image 必须是灰度索引图像。分割后目标的灰度值为object，背景灰度值为0。
     * @param object 分割目标的灰度值，一般去取1～32。缺省值为1。
     */
    template <class T>
    void SegmentImageWithThreshold(ImageDef<T> *image, T object = 1)
    {
      if (image->Format != IMAGE_FORMAT_INDEX) throw new UnsupportedFormatException();

      T max_T;
      MBL::Utility::GetMaxValue(&max_T);
      int *hist = new int[max_T + 1];
      GetImageHistogram(image, hist);
      T threshold;
      GetHistogramThreshold(hist, max_T + 1, &threshold);
      T *p = image->Pixels;
      for (int i = 0, n = GetUnitsPerRow(image) * image->Height; i < n; i++)
      {
        if (*p <= threshold)
        {
          *p++ = object;
        }
        else
        {
          *p++ = 0;
        }
      }

      delete [] hist;
    }

    /**
     * @brief 对一副图像进行直方图增强处理。
     *
     * @param image 欲处理的图像，目前必须是RGB 8bit格式。
     *
     * @author 钟明亮，陈进，赵宇。
     */
    template <class T>
    void EnhanceHistogram(ImageDef<T> *image)
    {
      const float m_dwHistRate = 0.01;
      //const int nEffectWidth 400;
      //const int nEffectHeight = 300;

      static int	nHistogram[3][256];//直方图数据
      memset(nHistogram, 0, sizeof(nHistogram));

      //统计Histogram------------
      //int dwWStep = MBL::Utility::GetMax(image->Width / nEffectWidth, 1);
      //int dwHStep = MBL::Utility::GetMax(image->Height / nEffectHeight, 1);

      int i;
      int dwtotal = image->Width * image->Height;
      T *ptr = image->Pixels;
      for (i = 0; i < dwtotal; i++)
      {
        nHistogram[0][*ptr++]++;
        nHistogram[1][*ptr++]++;
        nHistogram[2][*ptr++]++;
      }

      //统计Histogram_Low, Histogram_High
      T Histogram_Low = 0, Histogram_High = 255;
      float dwRateB = 0, dwRateG = 0, dwRateR = 0;
      for (i = 0; i < 256; i++)
      {
        dwRateB += ((float)nHistogram[0][i]) / dwtotal;
        dwRateG += ((float)nHistogram[1][i]) / dwtotal;
        dwRateR += ((float)nHistogram[2][i]) / dwtotal;

        //m_dwHistRate,default=0.01
        if (dwRateG > m_dwHistRate || dwRateB > m_dwHistRate || dwRateR > m_dwHistRate)
        {
          Histogram_Low = i;
          break;
        }
      }

      dwRateB = 0, dwRateG = 0, dwRateR = 0;
      for (i = 255; i >= 0; i--)
      {
        dwRateB += ((float)nHistogram[0][i]) / dwtotal;
        dwRateG += ((float)nHistogram[1][i]) / dwtotal;
        dwRateR += ((float)nHistogram[2][i]) / dwtotal;

        //m_dwHistRate,default=0.01
        if (dwRateG > m_dwHistRate || dwRateB > m_dwHistRate || dwRateR > m_dwHistRate)
        {
          Histogram_High = i;
          break;
        }
      }

      static T HistogramTable[256];
      T nCenter = (Histogram_High + Histogram_Low) >> 1;
      //刷新HistogramTable的数据
      for (i = 0; i < Histogram_Low; i++)
      {
        HistogramTable[i]=0;
      }
      for (i = Histogram_Low; i < nCenter; i++)
      {
        HistogramTable[i] = (T)(127.0 * (i - Histogram_Low) / (nCenter-Histogram_Low));
      }
      HistogramTable[nCenter] = 128;
      for (i = nCenter + 1; i < Histogram_High; i++)
      {
        HistogramTable[i] = (T)(127.0 * (i - nCenter) / (Histogram_High - nCenter) + 128);
      }
      for (i = Histogram_High; i < 256; i++)
      {
        HistogramTable[i] = 255;
      }

      ptr = image->Pixels;
      for (i = 0; i < dwtotal; i++)
      {
        *ptr = HistogramTable[*ptr];
        ptr++;
        *ptr = HistogramTable[*ptr];
        ptr++;
        *ptr = HistogramTable[*ptr];
        ptr++;
      }
    }
  }
}

#endif // __HISTOGRAM_H__
