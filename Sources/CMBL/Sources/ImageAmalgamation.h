#ifndef __IMAGEAMALGAMATION_H__
#define __IMAGEAMALGAMATION_H__

#include <map>

/**
 * @file
 *
 * @brief 本文件包含多幅图像融合的算法。
 */

namespace MBL
{
  namespace Image2D
  {
    ///图像融和算子类。
    /**
     * 该类是个Template类，用户输入一组图像，该类图像融和后可得到结果图像。
     *
     * @author 赵宇
     */
    template <class T>
    class Amalgamator
    {
      private:
        ImageDef<T> *Result;

      protected:
        /**
         * 构造函数。
         */
        Amalgamator()
        {
          Result = 0;
        }

      public:
        /**
         * 析构函数。
         */
        virtual ~Amalgamator()
        {
          MBL::Utility::SafeRelease(&Result);
        }

      public:
        /**
         * @brief 添加一幅融合图像。
         *
         * 新添加的图像会被立即融合到结果图像中去。
         *
         * @param image 新添加的图像。所有添加的图像必须是同一格式，但尺寸可以不同。
         */
        void AddImage(ImageDef<T> *image)
        {
          if (Result == 0)
          {
            Result = DuplicateImage(image);
          }
          else if (Result->Format == image->Format)
          {
            ImageDef<T> *temp = 0;

            if (Result->Width < image->Width || Result->Height < image->Height)
            {
              temp = ImageDef<T>::CreateSameFormatInstance(Result,
                                                           MBL::Utility::GetMax(Result->Width, image->Width),
                                                           MBL::Utility::GetMax(Result->Height, image->Height));
              memset(temp->Pixels, 0, GetBytesOfPixelData(temp));
              WriteWindow(temp, 0, 0, Result->Width - 1, Result->Height - 1, Result->Pixels);
            }
            else
            {
              temp = Result;
            }

            T min = 0, max;
            MBL::Utility::GetMaxValue(&max);
            int b = GetUnitsPerPixel(Result);
            T *src1, *src2, buf[8];

            for (int y = 0; y < image->Height; y++)
            {
              for (int x = 0; x < image->Width; x++)
              {
                src2 = &(image->Pixels[(y * image->Width + x) * b]);
                if (x < Result->Width && y < Result->Height)
                {
                  src1 = &(Result->Pixels[(y * Result->Width + x) * b]);
                  AmalgamatePixel(src1, src2, buf, b, min, max);
                  WritePixel(temp, x, y, buf);
                }
                else
                {
                  WritePixel(temp, x, y, src2);
                }
              }
            }

            if (temp != Result)
            {
              MBL::Utility::SafeRelease(&Result);
              Result = temp;
            }

            //由于融合时可能会影响Alpha通道，所以重新填充Alpha通道为最大值。
            int alpha = -1;
            if (Result->Format == IMAGE_FORMAT_ARGB)
            {
              alpha = 0;
            }
            else if (Result->Format == IMAGE_FORMAT_RGBA)
            {
              alpha = 3;
            }
            else if (Result->Format == IMAGE_FORMAT_INDEX_ALPHA)
            {
              alpha = 1;
            }

            if (alpha >= 0)
            {
              for (int i = 0; i < Result->Height; i++)
              {
                for (int j = 0; j < Result->Width; j++)
                {
                  Result->Pixels[(i * Result->Width + j) * b + alpha] = max;
                }
              }
            }
          }
        }
        /**
         * @brief 取得融合的结果图像指针。
         *
         * @return 结果图像指针，该图像保存在融合算子内部，外部不要删除。
         */
        ImageDef<T> * GetResult()
        {
          return Result;
        }

      protected:
        /**
         * @brief 融合两个象素。
         *
         * 该函数为纯虚函数，子类必须继承它。
         *
         * @param src1 源象素1。
         * @param src2 源象素2。
         * @param buf 融合后的数据存放缓冲区。
         * @param len 象素单元长度，也就是融合缓冲区长度。
         * @param min 融合后象素单元的最小值。
         * @param max 融合后象素单元的最大值。
         */
        virtual void AmalgamatePixel(T *src1, T *src2, T *buf, int len, T min, T max) = 0;
    };

    /// 比例融合算子。
    /**
     * 该算子分别按比例取源图像的象素值融合。
     */
    template <class T>
    class ProportionmentAmalgamator : public Amalgamator<T>
    {
      private:
        int proportion1, proportion2;

      public:
        /**
         * @brief 构造函数，记录融合参数。
         *
         * @param p1 源图像1的融合比例，必须为0～100的整数（对应0～100％）。
         * @param p2 源图像2的融合比例，必须为0～100的整数（对应0～100％）。
         */
        ProportionmentAmalgamator(int p1, int p2)
        {
          proportion1 = p1;
          proportion2 = p2;
        }
        /**
         * 析构函数。
         */
        virtual ~ProportionmentAmalgamator() {}

      protected:
        virtual void AmalgamatePixel(T *src1, T *src2, T *buf, int len, T min, T max)
        {
          for (int i = 0; i < len; i++)
          {
            buf[i] = MBL::Utility::Clamp(src1[i] * proportion1 / 100 + src2[i] * proportion2 / 100, min, max);
          }
        }
    };

    /// 相加融合算子。
    /**
     * 该算子将源图像的象素值相加后融合。
     */
    template <class T>
    class AddAmalgamator : public Amalgamator<T>
    {
      public:
        /**
         * @brief 构造函数。
         */
        AddAmalgamator() {}
        /**
         * 析构函数。
         */
        virtual ~AddAmalgamator() {}

      protected:
        virtual void AmalgamatePixel(T *src1, T *src2, T *buf, int len, T min, T max)
        {
          for (int i = 0; i < len; i++)
          {
            buf[i] = MBL::Utility::Clamp(src1[i] * 1 + src2[i] * 1, min, max);
          }
        }
    };

    /// 相减融合算子。
    /**
     * 该算子将源图像的象素值相减后融合。
     */
    template <class T>
    class SubtractAmalgamator : public Amalgamator<T>
    {
      private:
        int Tolerance, Enhance;

      public:
        /**
         * @brief 构造函数。
         *
         * @param t 融合时两个象素减的容差，如果象素差超过容差，则会补偿一个值。
         * @param e 融合时容差超过范围后的补偿值。
         */
        SubtractAmalgamator(int t, int e)
        {
          Tolerance = t;
          Enhance = e;
        }
        /**
         * 析构函数。
         */
        virtual ~SubtractAmalgamator() {}

      protected:
        virtual void AmalgamatePixel(T *src1, T *src2, T *buf, int len, T min, T max)
        {
          double rlt;

          for (int i = 0; i < len; i++)
          {
            rlt = src1[i] * 1 - src2[i] * 1;
            if (rlt > Tolerance) rlt += Enhance;
            buf[i] = (T)(MBL::Utility::Clamp(rlt, min, max));
          }
        }
    };

    /// 逻辑与融合算子。
    /**
     * 该算子将源图像的象素值逻辑与后融合。
     */
    template <class T>
    class AndAmalgamator : public Amalgamator<T>
    {
      public:
        /**
         * @brief 构造函数。
         */
        AndAmalgamator() {}
        /**
         * 析构函数。
         */
        virtual ~AndAmalgamator() {}

      protected:
        virtual void AmalgamatePixel(T *src1, T *src2, T *buf, int len, T /*min*/, T /*max*/)
        {
          for (int i = 0; i < len; i++)
          {
            buf[i] = src1[i] & src2[i];
          }
        }
    };

    /// 逻辑或融合算子。
    /**
     * 该算子将源图像的象素值逻辑或后融合。
     */
    template <class T>
    class OrAmalgamator : public Amalgamator<T>
    {
      public:
        /**
         * @brief 构造函数。
         */
        OrAmalgamator() {}
        /**
         * 析构函数。
         */
        virtual ~OrAmalgamator() {}

      protected:
        virtual void AmalgamatePixel(T *src1, T *src2, T *buf, int len, T /*min*/, T /*max*/)
        {
          for (int i = 0; i < len; i++)
          {
            buf[i] = src1[i] | src2[i];
          }
        }
    };

    /// 差额融合算子。
    /**
     * 该算子将源图像的象素值取差额（相减绝对值）后融合。
     */
    template <class T>
    class DifferenceAmalgamator : public Amalgamator<T>
    {
      private:
        int Tolerance, Enhance;

      public:
        /**
         * @brief 构造函数。
         *
         * @param t 融合时两个象素差额的容差，如果象素差额超过容差，则会补偿一个值。
         * @param e 融合时容差超过范围后的补偿值。
         */
        DifferenceAmalgamator(int t, int e)
        {
          Tolerance = t;
          Enhance = e;
        }
        /**
         * 析构函数。
         */
        virtual ~DifferenceAmalgamator() {}

      protected:
        virtual void AmalgamatePixel(T *src1, T *src2, T *buf, int len, T min, T max)
        {
          double rlt;

          for (int i = 0; i < len; i++)
          {
            rlt = fabs(src1[i] * 1 - src2[i] * 1);
            if (rlt > Tolerance) rlt += Enhance;
            buf[i] = (T)(MBL::Utility::Clamp(rlt, min, max));
          }
        }
    };

    /// 相乘融合算子。
    /**
     * 该算子将源图像的象素值相乘后融合。
     */
    template <class T>
    class MultiplyAmalgamator : public Amalgamator<T>
    {
      public:
        /**
         * @brief 构造函数。
         */
        MultiplyAmalgamator() {}
        /**
         * 析构函数。
         */
        virtual ~MultiplyAmalgamator() {}

      protected:
        virtual void AmalgamatePixel(T *src1, T *src2, T *buf, int len, T min, T max)
        {
          for (int i = 0; i < len; i++)
          {
            buf[i] = MBL::Utility::Clamp(1 * src1[i] * src2[i], min, max);
          }
        }
    };

    /// 取最暗融合算子。
    /**
     * 该算子将取源图像最暗的象素值融合。
     */
    template <class T>
    class DarkestAmalgamator : public Amalgamator<T>
    {
      public:
        /**
         * @brief 构造函数。
         */
        DarkestAmalgamator() {}
        /**
         * 析构函数。
         */
        virtual ~DarkestAmalgamator() {}

      protected:
        virtual void AmalgamatePixel(T *src1, T *src2, T *buf, int len, T /*min*/, T /*max*/)
        {
          for (int i = 0; i < len; i++)
          {
            buf[i] = MBL::Utility::GetMin(src1[i], src2[i]);
          }
        }
    };

    /// 取最亮融合算子。
    /**
     * 该算子将取源图像最亮的象素值融合。
     */
    template <class T>
    class LightestAmalgamator : public Amalgamator<T>
    {
      public:
        /**
         * @brief 构造函数。
         */
        LightestAmalgamator() {}
        /**
         * 析构函数。
         */
        virtual ~LightestAmalgamator() {}

      protected:
        virtual void AmalgamatePixel(T *src1, T *src2, T *buf, int len, T /*min*/, T /*max*/)
        {
          for (int i = 0; i < len; i++)
          {
            buf[i] = MBL::Utility::GetMax(src1[i], src2[i]);
          }
        }
    };

    /**
     * @brief 对图像进行背景平衡处理。
     *
     * 背景平衡算法的原理是用待处理图像减去背景校准图像。所以调用该函数时必须准备好一幅同样尺寸的
     * 背景图像。该函数调用示例如下：
     *
     * @code
     * ImageDef<unsigned char> *bk = ...;
     * ImageDef<unsigned char> *rgb = ...;
     * BalanceBackground(image, bk, this); //初始化背景校准图像，同时平衡该图像。
     * BalanceBackground(image, 0, this);  //背景校准图像初始化后即可连续平衡后续图像。
     * ......
     * BalanceBackground(new_image, new_bk, this); //改变背景校准图像尺寸后必须重新校准。
     * ......
     * BalanceBackground(0, 0, this); //最后调用该函数清除内部资源。
     * @endcode
     *
     * @param image 欲处理图像，必须是RGB 8bit图像，且必须与背景校准图像尺寸一致。
     * @param bk 背景图像，必须是RGB 8bit图像。image和bk均为0时，表示清除内部资源。
		 * @param id 标识符，用来唯一标识函数调用者，这样就可以同时处理多个图像，而不互相干扰。
     *
     * @author 钟明亮，赵宇，陈伟卿。
     */
		template <class T>
    void BalanceBackground(ImageDef<T> *image, ImageDef<T> *bk, void *id = 0)
    {
			static std::map<void *, ImageDef<short> *> pool;

			if (image == 0 && bk == 0 && pool.count(id))
      {
        MBL::Utility::SafeRelease(&pool[id]);
				pool.erase(id);
        return;
      }

      ImageDef<short> *background = 0;
    	int count = pool.count(id);
			if(count == 1) background = pool[id];

      int i, n;
      T *ptr = 0;
      short *bkptr = 0;

      if (bk != 0 )
      {
        if (count == 0 || bk->Width != background->Width || bk->Height != background->Height)
        {
          MBL::Utility::SafeRelease(&background);
          background = ImageDef<short>::CreateInstance(IMAGE_FORMAT_RGB, bk->Width, bk->Height);
					pool[id] = background;
        }

        //计算全幅图象的平均R,G,B值作为参考光的R,G,B值。
        int R = 0, G = 0, B = 0;
        n = bk->Width * bk->Height;
        ptr = bk->Pixels;
        for (i = 0; i < n; i++)
        {
          R += *ptr++;
          G += *ptr++;
          B += *ptr++;
        }
        R /= n;
        G /= n;
        B /= n;

        bkptr = background->Pixels;
        ptr = bk->Pixels;
        for (i = 0; i < n; i++)
        {
          *bkptr++ = R - *ptr++;
          *bkptr++ = G - *ptr++;
          *bkptr++ = B - *ptr++;
        }
      }

      if (image != 0 && background != 0
          && image->Width == background->Width && image->Height == background->Height)
      {
        ptr = image->Pixels;
        bkptr = background->Pixels;
        n = image->Width * image->Height * 3;
        for (i = 0; i < n; i++)
        {
          *ptr = MBL::Utility::Clamp(*ptr + *bkptr++, 0, 255);
          ptr++;
        }
      }
    }

		/**
		 * @brief 由背景图像取得背景平衡查找表。
		 *
		 * @param bg 背景图像。
		 * @return 查找表，按RGB通道排列，每个通道包含256×256=65536个元素。
		 */
		template <class T>
		const T * GetBGBalanceLUT(ImageDef<T> *bg)
		{
		  static T lut[ImageDefTraits<T>::LengthOfLUT * ImageDefTraits<T>::LengthOfLUT * 3];
		  T *lut_r = lut;
		  T *lut_g = lut + ImageDefTraits<T>::LengthOfLUT * ImageDefTraits<T>::LengthOfLUT;
		  T *lut_b = lut + ImageDefTraits<T>::LengthOfLUT * ImageDefTraits<T>::LengthOfLUT * 2;

		  //计算平均值
		  T *p = bg->Pixels;
		  int m_Rav = 0, m_Gav = 0, m_Bav = 0;
		  int n = bg->Width * bg->Height;
		  for (int i = 0; i < n; ++i)
		  {
		    m_Rav += *p++;
		    m_Gav += *p++;
		    m_Bav += *p++;
		  }
		  m_Rav /= n;
		  m_Gav /= n;
		  m_Bav /= n;

		  //填充查找表
		  for (int i = 0; i < ImageDefTraits<T>::LengthOfLUT; ++i)
		  {
		    for (int j = 0; j < ImageDefTraits<T>::LengthOfLUT; ++j)
		    {
		      int i1 = (i == 0) ? 1 : i;
		      int j1 = (j == 0) ? 1 : j;

		      lut_r[i * ImageDefTraits<T>::LengthOfLUT + j] = Utility::Clamp((i1 * m_Rav) / j1, ImageDefTraits<T>::MinValue, ImageDefTraits<T>::MaxValue);
		      lut_g[i * ImageDefTraits<T>::LengthOfLUT + j] = Utility::Clamp((i1 * m_Gav) / j1, ImageDefTraits<T>::MinValue, ImageDefTraits<T>::MaxValue);
		      lut_b[i * ImageDefTraits<T>::LengthOfLUT + j] = Utility::Clamp((i1 * m_Bav) / j1, ImageDefTraits<T>::MinValue, ImageDefTraits<T>::MaxValue);
		    }
		  }
		  return lut;
		}

		/**
		 * @brief 对图像进行背景平衡处理。
		 *
		 * @param image 处理的图像。
		 * @param bg 背景图像，必须与处理图像具有同样的尺寸和格式。
		 * @param lut 由背景图像计算得到的查找表。
		 * @see GetBGBalanceLUT
		 */
		template <class T>
		void ApplyBGBalanceLUT(ImageDef<T> *image, ImageDef<T> *bg, const T *lut)
		{
		  assert(image->Width == bg->Width && image->Height == bg->Height && image->Format == bg->Format);

		  T *p = image->Pixels;
		  T *pbg = bg->Pixels;
		  const T *lut_r = lut;
		  const T *lut_g = lut + ImageDefTraits<T>::LengthOfLUT * ImageDefTraits<T>::LengthOfLUT;
		  const T *lut_b = lut + ImageDefTraits<T>::LengthOfLUT * ImageDefTraits<T>::LengthOfLUT * 2;
		  for (int i = 0, n = image->Width * image->Height; i < n; ++i)
		  {
		    *p = lut_r[(*p) * ImageDefTraits<T>::LengthOfLUT + (*pbg)]; ++p; ++pbg;
		    *p = lut_g[(*p) * ImageDefTraits<T>::LengthOfLUT + (*pbg)]; ++p; ++pbg;
		    *p = lut_b[(*p) * ImageDefTraits<T>::LengthOfLUT + (*pbg)]; ++p; ++pbg;
		  }
		}

    /**
     * @brief 去除噪声算法内部使用的缓冲区对象。
     *
     * 由于在CodeWarrior中编译时，发现若将本类声明在RemoveImageNoise函数内部则无法编译，所以只能声明在全局范围，并以下划线开头命名，客户不要使用它。
     * 此外CodeWarrior无法编译map中的值为模版类型，所以只能声明为模版类的指针，并在构造和析构函数中进行内存的处理。
     *
     * @see RemoveImageNoise
     */
    template <class T>
	  class _RemoveNoiseBuffer
	  {
	    public:
        /// 新到达的图像在缓冲区中保存的索引。
  	    int index;
        /// 最多5个图像的缓冲区。
        ImageDef<T> *recent[5];

        /// 构造函数，初始化内存。
        _RemoveNoiseBuffer()
        {
          index = 0;
          memset(recent, 0, sizeof(recent));
        }
        /// 析构函数，销毁内存资源。
        ~_RemoveNoiseBuffer()
        {
          index = 0;
          for (int i = 0; i < 5; i++)
          {
            MBL::Utility::SafeRelease(&recent[i]);
          }
        }
	  };

    /**
     * @brief 去除图像中的噪声。
     *
     * 该算法对多幅图像进行平均处理。该函数调用示例代码如下：
     *
     * @code
     * ImageDef<unsigned char> *rgb = ...;
     * RemoveImageNoise(rgb, 5, this); //对5幅图像进行平均处理。
     * ......
     * RemoveImageNoise(rgb, 5, this); //函数内部会保留最近的5幅图像。
     * RemoveImageNoise(rgb, 0, this); // 处理完毕，清除函数内部资源。
     * @endcode
     *
     * @param image 欲处理的图像。目前必须是RGB格式。
     * @param level 针对多少幅图像进行去噪处理，必须为2～5的整数。为0时表示清除内部资源。
		 * @param id 标识符，用来唯一标识函数调用者，这样就可以同时处理多个图像，而不互相干扰。
     *
     * @author 钟明亮，赵宇，陈伟卿。
     */
	  template <class T>
    void RemoveImageNoise(ImageDef<T> *image, int level, void *id = 0)
    {
			static std::map<void *, _RemoveNoiseBuffer<T> *> pool;

			int count = pool.count(id);
			if (level == 0)
      {
        if (count > 0)
        {
          MBL::Utility::SafeRelease(&pool[id]);
          pool.erase(id);
        }
        return;
      }

      if (level < 2 || level > 5 || image == 0) return;

			if (count == 0)
			{
			  pool[id] = new _RemoveNoiseBuffer<T>();
			}
			ImageDef<T> **recent = pool[id]->recent;
      int &index = pool[id]->index;

      // 复制图像。index是静态变量，保留了上一次函数调用的值
      if (recent[index] == 0 || recent[index]->Width != image->Width || recent[index]->Height != image->Height)
      {
        MBL::Utility::SafeRelease(&recent[index]);
        recent[index] = DuplicateImage(image);
      }
      else
      {
        CopyImage(recent[index], image);
      }

      index++;
      if (index >= level)
      {
        index = 0;
      }

      int i = 0, n = 0;
      for (i = 0; i < level; i++)//检测 recent[i] 有几个可用
      {
        if (recent[i] == 0)
        {
          break;
        }
        else if (recent[i]->Width != image->Width || recent[i]->Height != image->Height)
        {
          return;
        }
        else
        {
          n++;
        }
      }

      if (n > 1 && n <= level)
      {
        int m = GetUnitsOfPixelData(image);
        int t = 0;
        int j = 0;
        for (i = 0; i < m; i++)
        {
          t = 0;
          for (j = 0; j < n; j++)
          {
            t += recent[j]->Pixels[i];
          }
          t /= n;
          image->Pixels[i] = MBL::Utility::Clamp(t, 0, 255);
        }
      }
    }
  } // Image2D namespace
} // MBL namespace

#endif // __IMAGEAMALGAMATION_H__
