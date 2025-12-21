#ifndef __FILEBMP_H__
#define __FILEBMP_H__

/**
 * @file
 *
 * @brief 声明读写BMP文件的函数。
 */

#include <stdio.h>

namespace MBL
{
  namespace Image2D
  {
    ///从磁盘中读取BMP格式的图像文件，将图像数据写入ImageDef图像结构。
    /**
     * 用户指定Bmp图像文件名，该函数将按标准不压缩位图格式读取该文件，若image的调色板和图像数据未分配内存，则分配相应的
     * 内存，并按图像文件填写相应数据成员的内容。若已分配内存，则释放内存后填写相应的内容。
     *
     * @param image 图像结构指针。
     * @param filename 包含全路径名称的字符串。
     *
     * @see SaveImageAsBmp
     *
     * @author binger
     */
    template <class T>
    void LoadImageAsBmp(ImageDef<T> *image, const char *filename)
    {
      if (filename == 0) throw NullPointerException();
      if (image == 0) throw NullPointerException();

      FILE *lpFile;

      // 位图文件头。
      unsigned short bfType;
      unsigned int   bfSize;
      unsigned short bfReserved1;
      unsigned short bfReserved2;
      unsigned int   bfOffBits;

      // 位图信息头。
      unsigned int   biSize;
      unsigned int   biWidth;
      unsigned int   biHeight;
      unsigned short biPlanes;
      unsigned short biBitCount;
      unsigned int   biCompression;
      unsigned int   biSizeImage;
      unsigned int   biXPelsPerMeter;
      unsigned int   biYPelsPerMeter;
      unsigned int   biClrUsed;
      unsigned int   biClrImportant;

      lpFile = fopen(filename, "rb");
      if (lpFile == 0) throw FileIOException();

       //由于可能有不同编译器的字节对齐问题，所以下面的数据都是按字节读取的。
      // Read BitMap File Header.
      fread(&bfType, sizeof(bfType), 1, lpFile);
      fread(&bfSize, sizeof(bfSize), 1, lpFile);
      fread(&bfReserved1, sizeof(bfReserved1), 1, lpFile);
      fread(&bfReserved2, sizeof(bfReserved2), 1, lpFile);
      fread(&bfOffBits, sizeof(bfOffBits), 1, lpFile);

      // Read BitMap Info Header
      fread(&biSize, sizeof(biSize), 1, lpFile);
      fread(&biWidth, sizeof(biWidth), 1, lpFile);
      fread(&biHeight, sizeof(biHeight), 1, lpFile);
      fread(&biPlanes, sizeof(biPlanes), 1, lpFile);
      fread(&biBitCount, sizeof(biBitCount), 1, lpFile);
      fread(&biCompression, sizeof(biCompression), 1, lpFile);
      fread(&biSizeImage, sizeof(biSizeImage), 1, lpFile);      //not credible
      fread(&biXPelsPerMeter, sizeof(biXPelsPerMeter), 1, lpFile);
      fread(&biYPelsPerMeter, sizeof(biYPelsPerMeter), 1, lpFile);
      fread(&biClrUsed, sizeof(biClrUsed), 1, lpFile);
      fread(&biClrImportant, sizeof(biClrImportant), 1, lpFile);

      // Only for Windows and IRIX, Index and RGB bitmap.
      if (bfType != 0x4D42 && bfType != 0x424D) throw UnsupportedFormatException();
      if (biBitCount != 8 && biBitCount != 24) throw UnsupportedFormatException();

      FreeImageData(image);

      image->Format = (biBitCount == 8) ? IMAGE_FORMAT_INDEX : IMAGE_FORMAT_BGR;
      image->Width = biWidth;
      image->Height = biHeight;
      image->UsedColor = 0;

      // Read color palette
      if (biBitCount == 8 && biClrUsed != 0)
      {
        image->Palette = new ImageRGBQUAD[biClrUsed];
        if (image->Palette == 0)
        {
          fclose(lpFile);
          FreeImageData(image);
          throw OutOfMemoryException();
        }
        image->UsedColor = biClrUsed;
        fread(image->Palette, biClrUsed * sizeof(ImageRGBQUAD), 1, lpFile);
      }
      else //对于256色灰度图像可以直接跳过调色板。
      {
        fseek(lpFile, bfOffBits, SEEK_SET);
      }

      // Read image data
      int b = GetBytesPerPixel(image);
      if ((image->Width * b) % 4 != 0)
      {
        biSizeImage = ((image->Width * b * sizeof(T) + 3) / 4 * 4) * image->Height;
      }
      else
      {
        biSizeImage = (image->Width * b * sizeof(T)) * image->Height;
      }
      image->Pixels = new T[biSizeImage];
      if (image->Pixels == 0)
      {
        fclose(lpFile);
        FreeImageData(image);
        throw OutOfMemoryException();
      }

      fread(image->Pixels, biSizeImage, 1, lpFile);
      fclose(lpFile);

      ConvertImage2Nonaligned(image);
      FlipImage(image); // The bmp file containts reversal image

    }

    ///将内存中的图像结构保存为BMP格式的文件。
    /**
     * 用户指定内存图像以及保存的文件名，该函数将图像保存为不压缩的标准位图格式。
     *
     * @param image 内存中的有效图像。
     * @param filename 包含全路径名称的字符串。
     *
     * @see LoadImageAsBmp
     */
    template <class T>
    void SaveImageAsBmp(ImageDef<T> *image, char *filename)
    {
      if (image->Pixels == 0 || filename == 0) throw NullPointerException();

      FILE *lpFile;
      int nPal = 0; //整个调色板所占的字节数。

      // 位图文件头。
      unsigned short bfType = 0x4D42;
      unsigned int   bfSize;
      unsigned short bfReserved1 = 0;
      unsigned short bfReserved2 = 0;
      unsigned int   bfOffBits;

      // 位图信息头。
      unsigned int   biSize = 40;
      unsigned int   biWidth = image->Width;
      unsigned int   biHeight = image->Height;
      unsigned short biPlanes = 1;
      unsigned short biBitCount;
      unsigned int   biCompression = 0;
      unsigned int   biSizeImage;
      unsigned int   biXPelsPerMeter = 0;
      unsigned int   biYPelsPerMeter = 0;
      unsigned int   biClrUsed;
      unsigned int   biClrImportant = 0;


      switch (image->Format)
      {
        case IMAGE_FORMAT_RGB:
        case IMAGE_FORMAT_BGR:
          biBitCount = 24;
          biClrUsed = 0;
          nPal = 0;
          break;
        case IMAGE_FORMAT_RGBA:
          biBitCount = 32;
          biClrUsed = 0;
          nPal = 0;
          break;
        case IMAGE_FORMAT_INDEX_ALPHA:
          biBitCount = 16;
          biClrUsed = 0;
          nPal = 0;
          break;
        case IMAGE_FORMAT_INDEX:
          biBitCount = 8;
          if (image->UsedColor > 0 && image->UsedColor != 256)
          {
            biClrUsed = image->UsedColor;
            nPal = image->UsedColor * sizeof(ImageRGBQUAD);
          }
          else
          {
            biClrUsed = 0;
            nPal = 256 * sizeof(ImageRGBQUAD);
          }
          break;
        default:
          throw UnsupportedFormatException();
          break;
      }

      int b = GetBytesPerPixel(image);
      if ((image->Width * b) % 4 != 0)
      {
        biSizeImage = ((image->Width * b * sizeof(T) + 3) / 4 * 4) * image->Height;
      }
      else
      {
        biSizeImage = (image->Width * b * sizeof(T)) * image->Height;
      }
      bfSize = 14 + 40 + nPal + biSizeImage;
      bfOffBits = 14 + 40 + nPal;

      lpFile = fopen(filename, "wb");
      if (lpFile != NULL) //由于可能有不同编译器的字节对齐问题，所以下面的数据都是按字节写出的。
      {
        // Write BitMap File Header.
        fwrite(&bfType, sizeof(bfType), 1, lpFile);
        fwrite(&bfSize, sizeof(bfSize), 1, lpFile);
        fwrite(&bfReserved1, sizeof(bfReserved1), 1, lpFile);
        fwrite(&bfReserved2, sizeof(bfReserved2), 1, lpFile);
        fwrite(&bfOffBits, sizeof(bfOffBits), 1, lpFile);

        // Write BitMap Info Header
        fwrite(&biSize, sizeof(biSize), 1, lpFile);
        fwrite(&biWidth, sizeof(biWidth), 1, lpFile);
        fwrite(&biHeight, sizeof(biHeight), 1, lpFile);
        fwrite(&biPlanes, sizeof(biPlanes), 1, lpFile);
        fwrite(&biBitCount, sizeof(biBitCount), 1, lpFile);
        fwrite(&biCompression, sizeof(biCompression), 1, lpFile);
        fwrite(&biSizeImage, sizeof(biSizeImage), 1, lpFile);
        fwrite(&biXPelsPerMeter, sizeof(biXPelsPerMeter), 1, lpFile);
        fwrite(&biYPelsPerMeter, sizeof(biYPelsPerMeter), 1, lpFile);
        fwrite(&biClrUsed, sizeof(biClrUsed), 1, lpFile);
        fwrite(&biClrImportant, sizeof(biClrImportant), 1, lpFile);

        // Write color palette
        if (nPal != 0)
        {
          if (image->Palette != 0)
          {
            fwrite(image->Palette, nPal, 1, lpFile);
          }
          else
          {
            int pal_size = nPal / sizeof(ImageRGBQUAD);
            ImageRGBQUAD *temp_pal = new ImageRGBQUAD[pal_size];
            if (temp_pal == 0) throw OutOfMemoryException();

            for (int i = 0; i < pal_size; i++)
            {
              temp_pal[i].Blue = temp_pal[i].Green = temp_pal[i].Red = (unsigned char)i;
              temp_pal[i].Reserved = 0;
            }

            fwrite(temp_pal, nPal, 1, lpFile);
            delete [] temp_pal;
          }
        }

        // Write image data
        if (image->Format == IMAGE_FORMAT_RGB) ExchangeBand(image, 0, 2);
        FlipImage(image);
        ConvertImage2Aligned(image);
        fwrite(image->Pixels, biSizeImage, 1, lpFile);
        ConvertImage2Nonaligned(image);
        FlipImage(image);
        if (image->Format == IMAGE_FORMAT_RGB) ExchangeBand(image, 0, 2);

        fclose(lpFile);
      }
      else
      {
        throw FileIOException();
      }
    }
  } // Image2D namespace
} // MBL namespace

#endif // __FILEBMP_H__
