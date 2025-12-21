#ifndef __IMAGESEQUENCEDEF_H__
#define __IMAGESEQUENCEDEF_H__

/**
 * @file
 *
 * @brief 包含二维序列图像结构定义。
 */

namespace MBL
{
  namespace Image2D
  {
    /// MBL库处理的二维序列图像结构。
    /**
    * 在所有MBL库中的二维序列图像处理中，都用该结构描述图像数据。该结构描述了在内存中的SequenceNumber幅图像数据的相关信
    * 息，包括图像数据格式、图像尺寸等(注：此时序列图像中每一幅的图像格式、大小均相同)。在这个类中使用了模板指针，可以
    * 处理各种数据类型。但开发者要根据具体的图像格式做不同的处理。
    *
    * @author Binger
    * @version 1.0
    */
    template <class T>
    class ImageSequenceDef
    {
      public:
        /// 图像数据格式(取ImageDef定义的枚举类型的图像格式)。
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
        /// 图像调色板指针(取ImageDef定义的枚举类型的索引颜色的调色板)。
        /**
         * 对于可以预知调色板的图像，该值可为NULL。
         */
        ImageRGBQUAD *Palette;
        ///序列图像的幅数。
        int SequenceNumber;
        /// 实际图像数据指针数组，为模板类型。
        T **Pixels;

      private:
        /// 缺省构造函数。
        ImageSequenceDef();

      public:
        /// 析构函数，释放对象资源。
        ~ImageSequenceDef();

      public:
        /// 创建一个不包含图像数据的对象。
        static ImageSequenceDef<T> * CreateEmptyInstance();
        /// 创建一个图像对象实例，并分配其内存。
        static ImageSequenceDef<T> * CreateInstance(ImageFormat format, int width, int height, int used_color = 0, int seq_number = 1);
        /// 创建一个已有图像数据的包裹对象。
        static ImageSequenceDef<T> * CreateWrapperInstance(ImageFormat format, T **data, int width, int height, int used_color = 0,
                                                           int seq_number = 1);

        /// 添加一帧数据到序列图像。
        void AppendFrame(T *data);
    };

    /**
     * 缺省构造函数产生一个空的序列图像对象，图像格式设为未知，尺寸为0，数据和调色板指针都置0, 幅数为0。构造函数设置为
     * 私有成员，即禁止调用，如果想创建ImageSequenceDef对象，必须调用静态工厂方法。
     */
    template <class T>
    ImageSequenceDef<T>::ImageSequenceDef()
    {
      Format = IMAGE_FORMAT_UNKNOWN;
      Width = 0;
      Height = 0;
      UsedColor = 0;
      Palette = 0;
      SequenceNumber = 0;
      Pixels = 0;
    }

    /**
     * 析构函数将释放图像结构所包含的内存，包括调色板内存和图像数据内存。
     */
    template <class T>
    ImageSequenceDef<T>::~ImageSequenceDef()
    {
      if (Palette != 0) delete [] Palette;
      if (Pixels != 0)
      {
        for(int i = 0; i < SequenceNumber; i++)
        {
          if (Pixels[i] != 0) delete Pixels[i];
        }
        delete [] Pixels;
      }

    }

    /**
     * 该静态工厂方法函数将创建一个不包含图像数据的ImageSequenceDef实例，并将该实例的指针返回。创建的图像结构格式未知，
     * 尺寸为0，调色板和图像数据均没有分配内存。这个函数主要在调用会产生新序列图像的处理函数时使用，例如：
     * <PRE>
     * ImageSequenceDef<unsigned char> *dest = ImageSequenceDef<unsigned char>::CreateInstance();
     * DuplicateSequenceImage(src, dest); //dest的图像数据会在这个函数内部被赋值。
     * delete dest;
     * </PRE>
     *
     * @return 如果创建成功则返回对象的指针，否则会抛出异常。
     */
    template <class T>
    ImageSequenceDef<T> * ImageSequenceDef<T>::CreateEmptyInstance()
    {
      ImageSequenceDef<T> *image = new ImageSequenceDef<T>();
      if (image == 0) throw OutOfMemoryException();

      return image;
    }

    /**
     * 该静态工厂方法函数将创建一个包含数据内存的ImageSequenceDef对象。用户要指定需要的图像格式、图像尺寸，该方法会创建
     * 图像对象，分配相应的内存，并填写相应数据成员的内容。该方法并不会在分配的内存中填充任何数据。如果用户需要调色板，
     * 必须自行填充。
     *
     * @param format 需要的图像格式，如果是索引图像，则会分配调色板，其它格式则不分配调色板。
     * @param width 图像宽度（象素）。
     * @param height 图像高度（象素）。
     * @param used_color 图像中用到的颜色数，即调色板的尺寸。由于一般处理的图像颜色数都可以推测，所有缺省值是0，即不分配
     *                   调色板内存。
     * @param seq_number 序列图像的幅数。如果为0，则该序列图像不包含任何数据，Pixels成员为0。
     * @return 如果创建成功则返回对象的指针，否则会抛出异常。
     */
    template <class T>
    ImageSequenceDef<T> * ImageSequenceDef<T>::CreateInstance(ImageFormat format, int width, int height, int used_color, int seq_number)
    {
      ImageSequenceDef<T> *image = new ImageSequenceDef<T>();
      if (image == 0) throw OutOfMemoryException();

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
          pal = 0;
          data = 4 * width * height;
          break;
        case IMAGE_FORMAT_INDEX_ALPHA:
          pal = used_color;
          data = 2 * width * height;
          break;
        default:
          throw UnsupportedFormatException();
          break;
      }

      // 填充图像结构数据，因为这时的调色板大小才是真正准确的。
      image->Format = format;
      image->Width = width;
      image->Height = height;
      image->UsedColor = pal;
      image->SequenceNumber = seq_number;

      if (pal > 0)
      {
        image->Palette = new ImageRGBQUAD[pal];
        if (image->Palette == 0)
        {
          delete image;
          throw OutOfMemoryException();
        }
      }

      if (data > 0 && image->SequenceNumber > 0)
      {
        image->Pixels = new T* [image->SequenceNumber];
        if (image->Pixels == 0)
        {
          delete image; // 删除ImageSequenceDef对象时，会删除已经分配的调色板内存。
          throw OutOfMemoryException();
        }
        for(int i = 0; i < image->SequenceNumber; i++)
        {
          image->Pixels[i] = new T [data];
          if (image->Pixels[i] == 0)
          {
            delete image;
            throw OutOfMemoryException();
          }
        }
      }

      return image;
    }

    /**
     * 该静态工厂方法函数将创建一个包裹已有图像数据的ImageSequenceDef对象。一般用户用其它方法得到了一序列图像数据后，可以
     * 用该函数将其包裹为一个对象，以使用MBL库函数进行处理。用户需要指定的图像格式、图像尺寸，该方法会创建图像对象，创建
     * 调色板，并将图像指针指向用户已经分配的内存。该方法并不会在分配的调色板中填充任何数据。如果用户需要，必须自行填充。
     *
     * @attention 由于图像数据内存并非由该工厂方法创建，所以在销毁产生的对象时必须注意图像指针的删除。如果用户使用new来分
     *            配图像数据，则可以直接删除ImageSequenceDef对象，连带的图像数据也会被删除。如果用户使用其它方法分配图像
     *            内存，或者在销毁ImageSequenceDef对象时仍然想保留图像数据内存，则必须在销毁前将Pixels指针置为0。
     *
     * @param format 需要的图像格式，如果是索引图像，则会分配调色板，其它格式则不分配调色板。
     * @param data 图像数据指针。
     * @param width 图像宽度（象素）。
     * @param height 图像高度（象素）。
     * @param used_color 图像中用到的颜色数，即调色板的尺寸。由于一般处理的图像颜色数都可以推测，所有缺省值是0，即不分配
     *                   调色板内存。
     * @param seq_number 序列图像的幅数。
     * @return 如果创建成功则返回对象的指针，否则会抛出异常。
     */
    template <class T>
    ImageSequenceDef<T> * ImageSequenceDef<T>::CreateWrapperInstance(ImageFormat format, T **data, int width, int height,
                                                                     int used_color, int seq_number)
    {
      ImageSequenceDef<T> *image = new ImageSequenceDef<T>();
      if (image == 0) throw OutOfMemoryException();

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
          pal = 0;
          break;
        case IMAGE_FORMAT_INDEX_ALPHA:
          pal = used_color;
          break;
        default:
          throw UnsupportedFormatException();
          break;
      }

      // 填充图像结构数据，因为这时的调色板大小才是真正准确的。
      image->Format = format;
      image->Width = width;
      image->Height = height;
      image->UsedColor = pal;
      image->SequenceNumber = seq_number;

      if (pal > 0)
      {
        image->Palette = new ImageRGBQUAD[pal];
        if (image->Palette == 0)
        {
          delete image;
          throw OutOfMemoryException();
        }
      }

      // 将图像指针指向用户已经分配的内存。
      image->Pixels = data;

      return image;
    }

    /**
     * 该函数在序列图像中添加一帧，并将参数指针所指的数据复制到序列图像对象内部。这要求该数据格式和尺寸必须与序列图像一致。
     *
     * @param data 欲添加的图像数据，其格式和尺寸必须与序列图像一致。
     */
    template <class T>
    void ImageSequenceDef<T>::AppendFrame(T *data)
    {
      if (Format == IMAGE_FORMAT_UNKNOWN) throw UninitializedImageException();

      T **p = Pixels;
      Pixels = new T* [SequenceNumber + 1];
      if (Pixels == 0) throw OutOfMemoryException();
      if (p != 0) memcpy(Pixels, p, sizeof(T*) * SequenceNumber);
      delete [] p;

      int len = 0;
      switch (Format)
      {
        case IMAGE_FORMAT_INDEX:
          len = Width * Height;
          break;
        case IMAGE_FORMAT_RGB:
        case IMAGE_FORMAT_BGR:
          len = 3 * Width * Height;
          break;
        case IMAGE_FORMAT_RGBA:
          len = 4 * Width * Height;
          break;
        case IMAGE_FORMAT_INDEX_ALPHA:
          len = 2 * Width * Height;
          break;
        default:
          throw UnsupportedFormatException();
          break;
      }
      Pixels[SequenceNumber] = new T [len];
      if (Pixels[SequenceNumber] == 0) throw OutOfMemoryException();
      memcpy(Pixels[SequenceNumber], data, len * sizeof(T));
      SequenceNumber++;
    }
  } // Image2D namespace
} // MBL namespace

#endif // __IMAGESEQUENCEDEF_H__
