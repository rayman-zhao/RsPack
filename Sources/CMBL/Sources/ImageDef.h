#ifndef __IMAGEDEF_H__
#define __IMAGEDEF_H__

/**
 * @file
 *
 * @brief 包含二维图像结构定义。
 */

namespace MBL
{
  namespace Image2D
  {
    /// 图像格式枚举常量。
    /**
     * 在图像处理中基本上有四种图像格式：各个象素的颜色值连续排列（如256色灰度图像），各个象素的颜色值按规律排列（如RGB
     * 格式的真彩色图像），Bayer组合格式，各个象素的颜色值组合排列（如将8位RGB值组合为一个32位整数的Java图像）。这些枚举
     * 常量就分别表示这些格式，并可根据今后的需要进行扩充。
     */
    typedef enum {IMAGE_FORMAT_UNKNOWN,       /**< 未知图像格式，用于表示图像的未初始化状态。 */
                  IMAGE_FORMAT_INDEX,         /**< 索引图像格式，如256色灰度图像，真彩色图像的RGB分量图像。 */
                  IMAGE_FORMAT_RGB,           /**< RGB图像格式，图像数据是按照R、G、B分量的顺序排列的。 */
                  IMAGE_FORMAT_BGR,           /**< BGR图像格式，图像数据是按照B、G、R分量的顺序排列的。 */
                  IMAGE_FORMAT_RGBA,          /**< RGBA图像格式，图像数据是按照R、G、B、Alpha分量的顺序排列的。 */
                  IMAGE_FORMAT_ARGB,          /**< ARGB图像格式，图像数据是按照Alpha、R、G、B分量的顺序排列的。 */
                  IMAGE_FORMAT_INDEX_ALPHA,   /**< 索引＋A图像格式，图像数据是按照索引、Alpha分量的顺序排列的。 */
                  IMAGE_FORMAT_BAYER_GR_BG,   /**< GR开头的Bayer图像格式。 */
                  IMAGE_FORMAT_BAYER_BG_GR,   /**< BG开头的Bayer图像格式。 */
                  IMAGE_FORMAT_BAYER_GB_RG,   /**< GB开头的Bayer图像格式。 */
                  IMAGE_FORMAT_BAYER_RG_GB,   /**< RG开头的Bayer图像格式。 */
                  IMAGE_FORMAT_YUV422_PACKED, /**< YUV4:2:2采样，按YUYV打包排列的图像格式。  */
                  IMAGE_FORMAT_YUV420_PLANAR  /**< YUV4:2:0采样，按Y-U-V平面排列的图像格式。  */
                 } ImageFormat;

    /// 索引颜色的调色板。
    /**
     * 该调色板用于确定索引颜色的实际RGB颜色，与标准位图的结构完全一致。颜色的RGB分量分别用一个字节表示。
     */
    typedef struct {unsigned char Blue;     /// 调色板的蓝色分量。
                    unsigned char Green;    /// 调色板的绿色分量。
                    unsigned char Red;      /// 调色板的红色分量。
                    unsigned char Reserved; /// 保留以后使用。
                   } ImageRGBQUAD;

    /// MBL库处理的二维图像结构。
    /**
     * 在所有MBL库中的二维图像处理中，都用该结构描述图像数据。该结构描述了在内存中的一幅图像数据的相关信息，包括图像数
     * 据格式、图像尺寸等。在这个类中使用了模板指针，可以处理各种数据类型。但开发者要根据具体的图像格式做不同的处理。
     *
     * @author 赵宇 Binger
     * @version 1.01
     */
    template <class T>
    class ImageDef
    {
      public:
        /// 图像数据格式。
        ImageFormat Format;
        /// 图像的宽度（象素）。
        int Width;
        /// 图像的高度（象素）。
        int Height;
        /// 图像中用到的颜色数。
        /**
         * 它表示索引图像中用到的颜色数，即调色板的大小。对于可以预知颜色数的索引图像，该值可为0，如8位数据对应的256色
         * 灰度，或者RGB彩色分量。对于非索引图像，该值没有意义，一般为0。
         */
        int UsedColor;
        /// 图像调色板指针。
        /**
         * 对于可以预知调色板的图像，该值可为0。
         */
        ImageRGBQUAD *Palette;
        /// 实际图像数据指针，为模板类型。
        T *Pixels;

      private:
        //该图像结构是否真正拥有数据缓冲区指针，是的话在对象析构时会自动删除该缓冲区。
        bool OwnDataBuf;

      private:
        /// 缺省构造函数。
        /**
         * 缺省构造函数产生一个空的图像对象，图像格式设为未知，尺寸为0，数据和调色板指针都置0。构造函数设置为私有成员，即禁
         * 止调用，如果想创建ImageDef对象，必须调用静态工厂方法。
         */
        ImageDef()
          : Format(IMAGE_FORMAT_UNKNOWN),
            Width(0),
            Height(0),
            UsedColor(0),
            Palette(0),
            Pixels(0),
            OwnDataBuf(true)
        {
        }

        ImageDef(const ImageDef &);
        ImageDef & operator = (const ImageDef &);

      public:
        /**
         * @brief 已有数据包裹构造函数。
         *
         * 一般来讲ImageDef只是图像结构的一个很简单的封装，基本还是以C指针的概念来使用，并没有想要把它设计成为一个很完善的C++类，尽量象C结构一样简单。
         * 这个构造函数应该只用于构造一个堆栈上的自动对象，它在析构的时候并不会删除数据指针。
         *
         * @attention 这个函数主要是为了快速把数据打包处理，使用时需要仔细考虑。
         *
         * @param format 需要的图像格式，任何格式都不会分配调色板内存。
         * @param data 图像数据指针。
         * @param width 图像宽度（象素）。
         * @param height 图像高度（象素）。
         *
         * @see CreateWrapperInstance
         */
        ImageDef(ImageFormat format, T *data, int width, int height)
          : Format(format),
            Width(width),
            Height(height),
            UsedColor(0),
            Palette(0),
            Pixels(data),
            OwnDataBuf(false)
        {
        }

        /// 析构函数，释放对象资源。
        /**
         * 析构函数将释放图像结构所包含的内存，包括调色板内存和图像数据内存。
         */
        ~ImageDef()
        {
          if (OwnDataBuf)
          {
            if (Palette != 0) delete [] Palette;
            if (Pixels != 0) delete [] Pixels;
          }
        }

      public:
        /// 创建一个不包含图像数据的对象。
        /**
         * 该静态工厂方法函数将创建一个不包含图像数据的ImageDef实例，并将该实例的指针返回。创建的图像结构格式未知，尺寸为0，
         * 调色板和图像数据均没有分配内存。这个函数主要在调用会产生新图像的处理函数时使用，例如：
         * @code
         * ImageDef<unsigned char> *dest = ImageDef<unsigned char>::CreateEmptyInstance();
         * DuplicateImage(src, dest); //dest的图像数据会在这个函数内部被赋值。
         * delete dest;
         * @endcode
         *
         * @return 如果创建成功则返回对象的指针，否则会抛出异常。
         */
        static ImageDef<T> * CreateEmptyInstance()
        {
          return new ImageDef<T>();
        }
        /// 创建一个图像对象实例，并分配其内存。
        /**
         * 该静态工厂方法函数将创建一个包含数据内存的ImageDef对象。用户要指定需要的图像格式、图像尺寸，该方法会创建图像对象，
         * 分配相应的内存，并填写相应数据成员的内容。该方法并不会在分配的内存中填充任何数据。如果用户需要调色板，必须自行填充。
         *
         * @param format 需要的图像格式，如果是索引图像，则会分配调色板，其它格式则不分配调色板。
         * @param width 图像宽度（象素）。
         * @param height 图像高度（象素）。
         * @param used_color 图像中用到的颜色数，即调色板的尺寸。由于一般处理的图像颜色数都可以推测，所有缺省值是0，即不分配
         *                   调色板内存。
         * @return 如果创建成功则返回对象的指针，否则会抛出异常。
         */
        static ImageDef<T> * CreateInstance(ImageFormat format, int width, int height, int used_color = 0)
        {
          ImageDef<T> *image = new ImageDef<T>();

          // 计算需要分配的调色板内存和图像数据内存。
          int pal = 0, data = 0;
          switch (format)
          {
            case IMAGE_FORMAT_INDEX:
              pal = used_color;
              data = width * height;
              break;
            case IMAGE_FORMAT_RGB:
            case IMAGE_FORMAT_BGR:
              pal = 0;
              data = 3 * width * height;
              break;
            case IMAGE_FORMAT_RGBA:
            case IMAGE_FORMAT_ARGB:
              pal = 0;
              data = 4 * width * height;
              break;
            case IMAGE_FORMAT_INDEX_ALPHA:
              pal = used_color;
              data = 2 * width * height;
              break;
            case IMAGE_FORMAT_YUV422_PACKED:
              pal = 0;
              data = 2 * width * height;
              break;
            case IMAGE_FORMAT_YUV420_PLANAR:
              pal = 0;
              data = width * height + (width + 1) / 2 * height;
              break;
            default:
              delete image;
              throw UnsupportedFormatException();
              break;
          }

          // 填充图像结构数据，因为这时的调色板大小才是真正准确的。
          image->Format = format;
          image->Width = width;
          image->Height = height;
          image->UsedColor = pal;

          if (pal > 0)
          {
            try
            {
              image->Palette = new ImageRGBQUAD[pal];
            }
            catch (const std::bad_alloc &)
            {
              delete image;
              throw;
            }
          }

          if (data > 0)
          {
            try
            {
              image->Pixels = new T[data];
            }
            catch (const std::bad_alloc &)
            {
              delete image; // 删除ImageDef对象时，会删除已经分配的调色板内存。
              throw;
            }
          }

          return image;
        }

        /// 创建一个已有图像数据的包裹对象。
        /**
         * 该静态工厂方法函数将创建一个包裹已有图像数据的ImageDef对象。一般用户用其它方法得到了一块图像数据后，可以用该函数将
         * 其包裹为一个对象，以使用MBL库函数进行处理。用户需要指定的图像格式、图像尺寸，该方法会创建图像对象，创建调色板，并
         * 将图像指针指向用户已经分配的内存。该方法并不会在分配的调色板中填充任何数据。如果用户需要，必须自行填充。
         *
         * @attention 由于图像数据内存并非由该工厂方法创建，所以在销毁产生的对象时必须注意图像指针的删除。如果用户使用new来分
         *            配图像数据，则可以直接删除ImageDef对象，连带的图像数据也会被删除。如果用户使用其它方法分配图像内存，或者
         *            在销毁ImageDef对象时仍然想保留图像数据内存，则必须在销毁前将Pixels指针置为0。
         *
         * @param format 需要的图像格式，如果是索引图像，则会分配调色板，其它格式则不分配调色板。
         * @param data 图像数据指针。
         * @param width 图像宽度（象素）。
         * @param height 图像高度（象素）。
         * @param used_color 图像中用到的颜色数，即调色板的尺寸。由于一般处理的图像颜色数都可以推测，所有缺省值是0，即不分配
         *                   调色板内存。
         * @return 如果创建成功则返回对象的指针，否则会抛出异常。
         */
        static ImageDef<T> * CreateWrapperInstance(ImageFormat format, T *data, int width, int height, int used_color = 0)
        {
          ImageDef<T> *image = new ImageDef<T>();

          // 计算需要分配的调色板内存和图像数据内存。
          int pal = 0;
          switch (format)
          {
            case IMAGE_FORMAT_INDEX:
              pal = used_color;
              break;
            case IMAGE_FORMAT_RGB:
            case IMAGE_FORMAT_BGR:
            case IMAGE_FORMAT_RGBA:
            case IMAGE_FORMAT_ARGB:
            case IMAGE_FORMAT_BAYER_GR_BG:
            case IMAGE_FORMAT_BAYER_BG_GR:
            case IMAGE_FORMAT_BAYER_GB_RG:
            case IMAGE_FORMAT_BAYER_RG_GB:
            case IMAGE_FORMAT_YUV422_PACKED:
            case IMAGE_FORMAT_YUV420_PLANAR:
              pal = 0;
              break;
            case IMAGE_FORMAT_INDEX_ALPHA:
              pal = used_color;
              break;
            default:
              delete image;
              throw UnsupportedFormatException();
              break;
          }

          // 填充图像结构数据，因为这时的调色板大小才是真正准确的。
          image->Format = format;
          image->Width = width;
          image->Height = height;
          image->UsedColor = pal;

          if (pal > 0)
          {
            try
            {
              image->Palette = new ImageRGBQUAD[pal];
            }
            catch (const std::bad_alloc &)
            {
              delete image;
              throw;
            }
          }

          // 将图像指针指向用户已经分配的内存。
          image->Pixels = data;

          return image;
        }

        /// 创建一个与源图像同样格式的图像。
        /**
         * 源图像的调色板会被复制，但图像数据不会被复制。
         *
         * @param image 源图像，必须是有效的内存图像。
         * @param width 新图像的宽度（象素）。
         * @param height 新图像的高度（象素）。
         * @return 与源图像同样格式的图像，如果创建失败则会抛出异常。
         */
        static ImageDef<T> * CreateSameFormatInstance(ImageDef<T> *image, int width, int height)
        {
          ImageDef<T> *ret = ImageDef<T>::CreateInstance(image->Format, width, height, image->UsedColor);
          if (image->UsedColor != 0) memcpy(ret->Palette, image->Palette, image->UsedColor * sizeof(ImageRGBQUAD));

          return ret;
        }
    };

    /**
     * @brief 简化声明用的8位数据类型定义。
     */
    typedef ImageDef<unsigned char> ImageDef8b;

    /**
     * @brief 图像特征。
     */
    template <class T>
    struct ImageDefTraits
    {
      static const T MinValue = 0;
      static const T MidValueRoundDown = 0;
      static const T MidValueRoundUp = 0;
      static const T MaxValue = 0;
      static const int LengthOfLUT = -1;
    };
    /**
     * @brief 8位图像特征。
     */
    template <>
    struct ImageDefTraits<unsigned char>
    {
      static const unsigned char MinValue = 0;
      static const unsigned char MidValueRoundDown = 127;
      static const unsigned char MidValueRoundUp = 128;
      static const unsigned char MaxValue = 255;
      static const int LengthOfLUT = 256;
    };
    /**
     * @brief 16位图像特征。
     */
    template <>
    struct ImageDefTraits<unsigned short>
    {
      static const unsigned short MinValue = 0;
      static const unsigned short MidValueRoundDown = 32767;
      static const unsigned short MidValueRoundUp = 32768;
      static const unsigned short MaxValue = 65535;
      static const int LengthOfLUT = 65536;
    };
  } // Image2D namespace
} // MBL namespace

#endif // __IMAGEDEF_H__
