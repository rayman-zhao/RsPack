#ifndef __IMAGERW_H__
#define __IMAGERW_H__

#include <cassert>

/**
 * @file
 *
 * @brief 包含读写图像象素、行、窗口的函数，以及提取图像信息的函数。
 */

namespace MBL
{
  namespace Image2D
  {
    /// 释放图像数据所占用的内存。
    /**
     * 释放一幅图像所占用的内存，并将图像结构中的内存指针设置为0。如果该图像不包含内存，则该函数没有影响。
     *
     * @param image 欲释放内存的图像
     */
    template <class T>
    void FreeImageData(ImageDef<T> *image)
    {
      if (image->Palette != 0)
      {
        delete [] image->Palette;
        image->Palette = 0;
      }
      if (image->Pixels != 0)
      {
        delete [] image->Pixels;
        image->Pixels = 0;
      }
    }

    /// 取得图像中每个象素所占的存储单元数。
    /**
     * 该函数取得图像中每个象素的存储单元数。如索引图像为1，RGB真彩色图像为3。
     *
     * @param image 欲处理的图像，必须是有效的图像。
     * @return 该图像每象素所占的单元数。
     *
     * @author 赵宇
     */
    template <class T>
    int GetUnitsPerPixel(const ImageDef<T> *image)
    {
      int b = 0;

      switch (image->Format)
      {
        case IMAGE_FORMAT_INDEX:
        case IMAGE_FORMAT_BAYER_GR_BG:
        case IMAGE_FORMAT_BAYER_BG_GR:
        case IMAGE_FORMAT_BAYER_GB_RG:
        case IMAGE_FORMAT_BAYER_RG_GB:
          b = 1;
          break;
        case IMAGE_FORMAT_RGB:
        case IMAGE_FORMAT_BGR:
          b = 3;
          break;
        case IMAGE_FORMAT_RGBA:
        case IMAGE_FORMAT_ARGB:
          b = 4;
          break;
        case IMAGE_FORMAT_INDEX_ALPHA:
        case IMAGE_FORMAT_YUV422_PACKED:
          b = 2;
          break;
        case IMAGE_FORMAT_YUV420_PLANAR:
          assert(false); //YUV420的存储应该为1.5unit，但目前没办法表达。
          break;
        default :
          throw UnsupportedFormatException();
          break;
      }

      return b;
    }

    /// 取得序列图像中每个象素所占的存储单元数。
    /**
     * 该函数取得序列图像中每个象素的存储单元数。如索引图像为1，RGB真彩色图像为3。
     *
     * @param image 欲处理的图像，必须是有效的图像。
     * @return 该图像每象素所占的单元数。
     *
     * @author 赵宇
     */
    template <class T>
    int GetUnitsPerPixel(const ImageSequenceDef<T> *image)
    {
      int b = 0;

      switch (image->Format)
      {
        case IMAGE_FORMAT_INDEX:
        case IMAGE_FORMAT_BAYER_GR_BG:
        case IMAGE_FORMAT_BAYER_BG_GR:
        case IMAGE_FORMAT_BAYER_GB_RG:
        case IMAGE_FORMAT_BAYER_RG_GB:
          b = 1;
          break;
        case IMAGE_FORMAT_RGB:
        case IMAGE_FORMAT_BGR:
          b = 3;
          break;
        case IMAGE_FORMAT_RGBA:
        case IMAGE_FORMAT_ARGB:
          b = 4;
          break;
        case IMAGE_FORMAT_INDEX_ALPHA:
        case IMAGE_FORMAT_YUV422_PACKED:
          b = 2;
          break;
        case IMAGE_FORMAT_YUV420_PLANAR:
          assert(false); //YUV420的存储应该为1.5unit，但目前没办法表达。
          break;
        default :
          throw UnsupportedFormatException();
          break;
      }

      return b;
    }

    /// 取得图像中每行所占的存储单元数。
    /**
     * 该函数取得图像中每行的存储单元数。
     *
     * @param image 欲处理的图像，必须是有效的图像。
     * @return 该图像每行所占的单元数。
     *
     * @author 赵宇
     */
    template <class T>
    int GetUnitsPerRow(const ImageDef<T> *image)
    {
      return image->Width * GetUnitsPerPixel(image);
    }

    /**
     * @brief 取得该图像对象的实际象素数据所占的存储单元数。
     *
     * @param image 欲处理的图像。
     * @return 图像数据所占的存储单元数，不包括调色板的数据。
     */
    template <class T>
    int GetUnitsOfPixelData(const ImageDef<T> *image)
    {
      return image->Height * GetUnitsPerRow(image);
    }

    /**
     * @brief 取得图像中每个象素所占的存储内存大小（字节）。
     *
     * 该值即每个象素所占单元数 × 每个单元的大小。
     *
     * @param image 欲处理的图像，必须是有效的图像。
     * @return 该图像每象素所占的字节数
     *
     * @author 赵宇
     */
    template <class T>
    int GetBytesPerPixel(const ImageDef<T> *image)
    {
      return sizeof(T) * GetUnitsPerPixel(image);
    }

    /**
     * @brief 取得该图像对象每行象素的数据所占的内存大小（字节）。
     *
     * @param image 欲处理的图像。
     * @return 每行象素内存大小。
     */
    template <class T>
    int GetBytesPerRow(const ImageDef<T> *image)
    {
      return image->Width * GetBytesPerPixel(image);
    }

    /**
     * @brief 取得该图像对象的实际象素数据所占的内存大小（字节）。
     *
     * @param image 欲处理的图像。
     * @return 图像数据所占的内存大小，不包括调色板的数据。
     */
    template <class T>
    size_t GetBytesOfPixelData(const ImageDef<T> *image)
    {
      return static_cast<size_t>(image->Height) * GetBytesPerRow(image);
    }

    /// 从图像中读取一个象素到缓冲区。
    /**
     * 用户指定图像和分配好的数据缓冲区，该函数将图像中指定位置的数据读取到数据缓冲区中。
     *
     * @param image 图像结构指针，必须是有效的内存中的图像。
     * @param x 欲读取象素的x坐标（象素），即列数。
     * @param y 欲读取象素的y坐标（象素），即行数。
     * @param buf 读取缓冲区，必须预先分配好足够的内存，数据类型必须与图像结构一致。
     *
     * @author 赵宇
     */
    template <class T>
    void ReadPixel(ImageDef<T> *image, int x, int y, T *buf)
    {
      memcpy(buf, image->Pixels + (x + y * image->Width) * GetUnitsPerPixel(image), GetBytesPerPixel(image));
    }

    /// 将缓冲区的数据复制到图像中的一个象素。
    /**
     ×
     * @param image 图像结构指针，必须是有效的内存中的图像。
     * @param x 欲写入象素的x坐标（象素），即列数。
     * @param y 欲写入象素的y坐标（象素），即行数。
     * @param buf 源数据缓冲区，数据类型必须与图像结构一致。
     *
     * @author 赵宇
     */
    template <class T>
    void WritePixel(ImageDef<T> *image, int x, int y, T *buf)
    {
      memcpy(image->Pixels + (x + y * image->Width) * GetUnitsPerPixel(image), buf, GetBytesPerPixel(image));
    }

    /// 从图像中读取一行数据到缓冲区。
    /**
     * @param image 欲读取的图像。
     * @param start_pixel 起始列（象素），必须在0 ～ Width - 1之间。
     * @param end_pixel 结束列（象素），必须在起始列和Width - 1之间。
     * @param row 行数，必须在0 ～ Height - 1之间。
     * @param buf 已分配好内存的缓冲区。
     */
    template <class T>
    void ReadRow(ImageDef<T> *image, int start_pixel, int end_pixel, int row, T *buf)
    {
      memcpy(buf, image->Pixels + (start_pixel + row * image->Width)* GetUnitsPerPixel(image),
             (end_pixel - start_pixel + 1) * GetBytesPerPixel(image));
    }

    /// 将缓冲区的数据写到图像中。
    /**
     * @param image 欲写入的图像。
     * @param start_pixel 起始列（象素），必须在0 ～ Width - 1之间。
     * @param end_pixel 结束列（象素），必须在起始列和Width - 1之间。
     * @param row 行数，必须在0 ～ Height - 1之间。
     * @param buf 数据缓冲区。
     */
    template <class T>
    void WriteRow(ImageDef<T> *image, int start_pixel, int end_pixel, int row, T *buf)
    {
      memcpy(image->Pixels + (start_pixel + row * image->Width)* GetUnitsPerPixel(image), buf,
             (end_pixel - start_pixel + 1) * GetBytesPerPixel(image));
    }

    /// 从图像中读取一个窗口的数据到缓冲区。
    /**
     * @param image 欲读取的图像。
     * @param start_pixel 起始列（象素），必须在0 ～ Width - 1之间。
     * @param end_pixel 结束列（象素），必须在起始列和Width - 1之间。
     * @param start_row 起始行数，必须在0 ～ Height - 1之间。
     * @param end_row 结束行数，必须在起始行和Height - 1之间。
     * @param buf 已分配好内存的缓冲区。
     */
    template <class T>
    void ReadWindow(ImageDef<T> *image, int start_pixel, int start_row, int end_pixel, int end_row, T *buf)
    {
      int n = (end_pixel - start_pixel + 1) * GetBytesPerPixel(image),
          l = (end_pixel - start_pixel + 1) * GetUnitsPerPixel(image),
          m = GetUnitsPerRow(image);
      T *p = image->Pixels + (start_pixel + start_row * image->Width) * GetUnitsPerPixel(image);
      for (int i = start_row; i <= end_row; i++)
      {
        memcpy(buf, p, n);
        buf += l;
        p += m;
      }
    }

    /// 将缓冲区的数据写到图像中的一个窗口。
    /**
     * @param image 欲写入的图像。
     * @param start_pixel 起始列（象素），必须在0 ～ Width - 1之间。
     * @param end_pixel 结束列（象素），必须在起始列和Width - 1之间。
     * @param start_row 起始行数，必须在0 ～ Height - 1之间。
     * @param end_row 结束行数，必须在起始行和Height - 1之间。
     * @param buf 数据缓冲区。
     */
    template <class T>
    void WriteWindow(ImageDef<T> *image, int start_pixel, int start_row, int end_pixel, int end_row, T *buf)
    {
      int n = (end_pixel - start_pixel + 1) * GetBytesPerPixel(image),
          l = (end_pixel - start_pixel + 1) * GetUnitsPerPixel(image),
          m = GetUnitsPerRow(image);
      T *p = image->Pixels + (start_pixel + start_row * image->Width) * GetUnitsPerPixel(image);
      for (int i = start_row; i <= end_row; i++)
      {
        memcpy(p, buf, n);
        buf += l;
        p += m;
      }
    }

    /**
     * @brief 从图像中剪裁出一个子图像。
     *
     * 子图像的位置在源图像坐标系中描述。
     *
     * @param image 源图像。
     * @param left 子图像的左边（象素）。
     * @param top 子图像的顶边（象素）。
     * @param width 子图像的宽度（象素）。
     * @param height 子图像的高度（象素）。
     * @return 剪裁出的子图像。该图像在堆上分配，使用完毕后请用delete删除。
     *
     * @author 赵宇
     */
    template <class T>
    ImageDef<T> * CutImage(ImageDef<T> *image, int left, int top, int width, int height)
    {
      ImageDef<T> *ret = ImageDef<T>::CreateSameFormatInstance(image, width, height);
      ReadWindow(image, left, top, left + width - 1, top + height - 1, ret->Pixels);

      return ret;
    }

    /**
     * @brief 从图像中取 ROI 图像。
     *
     * ROI图像 的位置在源图像坐标系中描述。
     *
     * @param image 源图像。
     * @param top 子图像的顶边（象素）,从0开始。
     * @param left 子图像的左边（象素）,从0开始。
     * @param bottom 子图像的底边（象素）,从0开始。
     * @param right 子图像的右边（象素）,从0开始。
     * @return 剪裁出的 ROI图像。该图像在堆上分配，使用完毕后请用delete删除。
     *
     * @author 陈伟卿
     */
    template <class T>
    ImageDef<T> *GetROI(ImageDef<T> *image, int top, int left, int bottom, int right)
    {
      ImageDef<T> *ret = ImageDef<T>::CreateSameFormatInstance(image, right - left + 1, bottom - top + 1);
      ReadWindow(image, left, top, right, bottom, ret->Pixels);

      return ret;
    }

    /**
     * @brief 从图像中剪裁出一个任意形状的子图像。
     *
     * @param image 源图像，目前只支持RGB格式的图像。
     * @param sub_area 子区对象。
     * @return 剪裁出的子图像。该图像在堆上分配，使用完毕后请用delete删除。剪裁后的图像会多出一个Alpha通道，如RGB图像会返回RGBA格式。
     */
    template <class T>
    ImageDef<T> * CutImage(ImageDef<T> *image, ImageSubArea *sub_area)
    {
      ImageDef<T> *ret = 0;
      switch (image->Format)
      {
        case IMAGE_FORMAT_RGB:
          ret = ImageDef<T>::CreateInstance(IMAGE_FORMAT_RGBA, sub_area->Width, sub_area->Height, 0);
          break;
        default:
          throw UnsupportedFormatException();
      }

      T buf[8];
      memset(ret->Pixels, 0, GetBytesOfPixelData(ret));
      for (int y = 0, y1 = sub_area->Top; y < ret->Height; y++, y1++)
      {
        for (int x = 0, x1 = sub_area->Left; x < ret->Width; x++, x1++)
        {
          if (sub_area->IsFill(x1, y1) == true)
          {
            ReadPixel(image, x1, y1, buf);
            buf[3] = 255;
            WritePixel(ret, x, y, buf);
          }
        }
      }

      return ret;
    }

    /**
     * @brief 将一幅图像填充到另一幅图像中。
     *
     * 该函数将检查被填充的位置，范围外的图像不会被复制。
     *
     * @param dest 被填充图像。
     * @param src 欲填充图像。两者必须格式相同。
     * @param left 在被填充图像中的起始位置X（象素）。
     * @param top 在被填充图像中的起始位置Y（象素）。
     */
    template <class T>
    void PutImage(ImageDef<T> *dest, ImageDef<T> *src, int left, int top)
    {
      if (dest->Format != src->Format) throw UnmatchedImageException();

      int w = Utility::GetMin(src->Width, dest->Width - left);
      int h = Utility::GetMin(src->Height, dest->Height - top);
      int wb = w * GetBytesPerPixel(src);
      int src_step = GetUnitsPerRow(src);
      int dest_step = GetUnitsPerRow(dest);
      T *buf1 = src->Pixels;
      T *buf2 = dest->Pixels + top * dest_step + left * GetUnitsPerPixel(dest);
      
      for (int y = 0; y < h; ++y)
      {
        memcpy(buf2, buf1, wb);
        buf1 += src_step;
        buf2 += dest_step;
      }

      //int x1, y1;
      //T buf[8];
      //for (int y = 0; y < src->Height; y++)
      //{
      //  y1 = top + y;
      //  if (y1 >= 0 && y1 < dest->Height)
      //  {
      //    for (int x = 0; x < src->Width; x++)
      //    {
      //      x1 = left + x;
      //      if (x1 >= 0 && x1 < dest->Width)
      //      {
      //        ReadPixel(src, x, y, buf);
      //        WritePixel(dest, x1, y1, buf);
      //      }
      //    }
      //  }
      //}
    }

    /**
     * @brief 创建一个复本图像。
     *
     * @param image 源图像。
     * @return 复本图像。该图像在堆上创建，使用完毕后请用delete删除。
     */
    template <class T>
    ImageDef<T> * DuplicateImage(ImageDef<T> *image)
    {
      return CutImage(image, 0, 0, image->Width, image->Height);
    }

    /**
     * @brief 复制一幅图像。
     *
     * 将一幅图像的内容复制到另外一幅图像。两幅图像的属性必须一致。
     *
     * @param dest 目的图像，必须是已分配好内存的图像。
     * @param src 源图像，必须与目的图像匹配。
     */
    template <class T>
    void CopyImage(ImageDef<T> *dest, ImageDef<T> *src)
    {
      if (dest->Format != src->Format || dest->Width != src->Width || dest->Height != src->Height
          || dest->UsedColor != src->UsedColor)
      {
        throw UnmatchedImageException();
      }

      memcpy(dest->Pixels, src->Pixels, GetBytesOfPixelData(src));
      if (src->UsedColor != 0) memcpy(dest->Palette, src->Palette, src->UsedColor * sizeof(ImageRGBQUAD));
    }

    /// 用一个像素数据填充到整个图像。
    /**
     * 该函数将一个像素的数据填充到整个图像中。一般用于设置图像背景。
     *
     * @param image 欲处理的图像。
     * @param buf 一个像素的图像数据缓冲区。
     *
     * @author 赵宇
     */
    template <class T>
    void FillImage(ImageDef<T> *image, T *buf)
    {
      unsigned char *p = image->Pixels;
      size_t n = GetBytesOfPixelData(image);
      int b = GetBytesPerPixel(image);

      if (b == 1)
      {
        memset(p, buf[0], n);
      }
      else
      {
        unsigned char *e = p + n;
        while (p != e)
        {
          memcpy(p, buf, b);
          p += b;
        }
      }
    }

    /// 从图像中提取一个通道的数据。
    /**
     * 该函数从图像对象中提取一个通道的数据，放在缓冲区中。
     *
     * @param image 欲处理的图像。
     * @param band 提取的通道号。0为第一个通道，1为第二个通道...。如果图像中不包含该通道，则抛出异常。
     * @param buf 提取数据存放的缓冲区指针的地址。如果*buf为0，则函数内部使用new来分配新的缓冲区，否则则认为*buf是有效地、足够大小的地址。
     *
     * @author 赵宇
     */
    template <class T>
    void ExtractBand(ImageDef<T> *image, int band, T **buf)
    {
      int b = GetUnitsPerPixel(image);
      if (band >= b) throw IndexOutOfBoundsException();

      if (*buf == 0)
      {
        *buf = new T[image->Width * image->Height];
        if (*buf == 0) throw OutOfMemoryException();
      }
      T *p = *buf;

      for (int i = 0; i < image->Height; i++)
      {
        for (int j = 0; j < image->Width; j++)
        {
          *p++ = image->Pixels[(i * image->Width + j) * b + band];
        }
      }
    }

    /// 将一个通道的数据填充到图像中。
    /**
     * 该函数将一个通道的数据填充回图像中。
     *
     * @param image 欲处理的图像。
     * @param band 填充的通道号。0为第一个通道，1为第二个通道...。如果图像中不包含该通道，则抛出异常。
     * @param buf 单一通道图像数据存放的缓冲区。
     *
     * @see ExtractBand
     * @author 赵宇
     */
    template <class T>
    void FillBand(ImageDef<T> *image, int band, T *buf)
    {
      int b = GetUnitsPerPixel(image);
      if (band >= b) throw IndexOutOfBoundsException();

      for (int i = 0; i < image->Height; i++)
      {
        for (int j = 0; j < image->Width; j++)
        {
          image->Pixels[(i * image->Width + j) * b + band] = *buf++;
        }
      }
    }

    /// 将图像中的两个通道数据交换。
    /**
     * 该函数将一幅图像中的两个通道数据交换对调。
     *
     * @param image 欲处理的图像。
     * @param band1 第一个通道号。0为第一个通道，1为第二个通道...。如果图像中不包含该通道，则抛出异常。
     * @param band2 第二个通道号。0为第一个通道，1为第二个通道...。如果图像中不包含该通道，则抛出异常。
     *
     * author 赵宇
     */
    template <class T>
    void ExchangeBand(ImageDef<T> *image, int band1, int band2)
    {
      if (band1 == band2) return;

      int b = GetUnitsPerPixel(image);
      if (band1 >= b || band2 >= b) throw IndexOutOfBoundsException();

      T tmp;
      T *p1 = image->Pixels + band1;
      T *p2 = image->Pixels + band2;
      for (int i = 0, n = image->Width * image->Height; i < n; ++i, p1 += b, p2 += b)
      {
        tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
      }
    }

    /**
     * @brief 创建一个新的添加了Alpha通道的图像。
     *
     * @param image 源图像，必须是RGB格式。
     * @param fmt 新图像格式，RGBA或者ARGB。
     * @param v 新添加的Alpha通道被填充的数值。
     * @return 添加了Alpha通道的图像。
     */
    template <class T>
    ImageDef<T> * AddAlphaBand(ImageDef<T> *image, ImageFormat fmt, T v)
    {
      if (image == 0) throw NullPointerException();
      if (image->Format != IMAGE_FORMAT_RGB) throw UnsupportedFormatException();

      ImageDef<T> *image1 = ImageDef<T>::CreateInstance(fmt, image->Width, image->Height);
      int len = image->Width * image->Height;
      T *buf = new T[len];
      for (int i = 0; i < len; i++)
      {
        buf[i] = v;
      }

      if (fmt == IMAGE_FORMAT_RGBA)
      {
        FillBand(image1, 3, buf);

        ExtractBand(image, 0, &buf);
        FillBand(image1, 0, buf);
        ExtractBand(image, 1, &buf);
        FillBand(image1, 1, buf);
        ExtractBand(image, 2, &buf);
        FillBand(image1, 2, buf);
      }
      else if (fmt == IMAGE_FORMAT_ARGB)
      {
        FillBand(image1, 0, buf);

        ExtractBand(image, 0, &buf);
        FillBand(image1, 1, buf);
        ExtractBand(image, 1, &buf);
        FillBand(image1, 2, buf);
        ExtractBand(image, 2, &buf);
        FillBand(image1, 3, buf);
      }

      delete [] buf;
      return image1;
    }

    /**
     * @brief 创建从一幅图像中删除其Alpha通道的图像。
     *
     * @param image ARGB或RGBA格式的图像。
     * @return 格式为RGB的图像。调用者应该负责删除该图像。
     */
    template <class T>
    ImageDef<T> * RemoveAlphaBand(ImageDef<T> *image)
    {
      if (image == 0) throw NullPointerException();
      if (image->Format != IMAGE_FORMAT_RGBA && image->Format != IMAGE_FORMAT_ARGB)
        throw UnsupportedFormatException();

      ImageDef<T> *image1 = ImageDef<T>::CreateInstance(IMAGE_FORMAT_RGB, image->Width, image->Height);
      T *buf = new T[image->Width * image->Height];

      if (image->Format == IMAGE_FORMAT_RGBA)
      {
        ExtractBand(image, 0, &buf);
        FillBand(image1, 0, buf);
        ExtractBand(image, 1, &buf);
        FillBand(image1, 1, buf);
        ExtractBand(image, 2, &buf);
        FillBand(image1, 2, buf);
      }
      else if (image->Format == IMAGE_FORMAT_ARGB)
      {
        ExtractBand(image, 1, &buf);
        FillBand(image1, 0, buf);
        ExtractBand(image, 2, &buf);
        FillBand(image1, 1, buf);
        ExtractBand(image, 3, &buf);
        FillBand(image1, 2, buf);
      }

      delete [] buf;
      return image1;
    }
  }
}

#endif // __IMAGERW_H__
