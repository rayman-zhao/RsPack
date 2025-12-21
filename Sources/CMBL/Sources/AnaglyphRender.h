#ifndef __ANAGLYPHRENDER_H__
#define __ANAGLYPHRENDER_H__

#include <math.h>

/**
 * @file
 *
 * @brief 视见变换矩阵函数、Montage渲染函数、Splash渲染函数、Anaglyph图像生成函数，视见变换矩阵函数在渲染函数中被调用。
 *
 */
namespace MBL
{
  namespace Image3D
  {
    using namespace MBL::Image2D;

    /// 该函数进行视空间变换。
    /**
     * 定义象空间到物空间视变换矩阵，对其进行转置得到物空间到象空间视变换矩阵。
     *
     * @param Mview 为象空间到物空间视变换矩阵。
     * @param Mview1 为物空间到象空间视变换矩阵。
     * @param Sita 为绕y轴视角方向(单位：角度值)。
     * @param Fia 为绕x轴视角方向(单位：角度值)。
     *
     * @author 孙维忠
     */
    inline void _ComputeMview(double Mview[4][4], double Mview1[4][4], double Sita, double Fia)
    {

      const double PI = 3.14159265358979323846;
      double Cos_Sita, Sin_Sita;
      double Cos_Fia, Sin_Fia;
      int i,j;

      Sita = Sita * PI / 180.0;
      Fia = Fia * PI / 180.0;

      Cos_Sita = cos(Sita);  Sin_Sita = sin(Sita);
      Cos_Fia = cos(Fia);    Sin_Fia = sin(Fia);

      Mview[0][0] = Cos_Sita;
      Mview[1][0] = 0;
      Mview[2][0] = Sin_Sita;
      Mview[3][0] = 0.0;

      Mview[0][1] = - Sin_Fia * Sin_Sita;
      Mview[1][1] = Cos_Fia;
      Mview[2][1] = - Sin_Fia * Cos_Sita;
      Mview[3][1] = 0.0;

      Mview[0][2] = - Sin_Sita * Cos_Fia;
      Mview[1][2] = - Sin_Fia;
      Mview[2][2] = Cos_Sita * Cos_Fia;
      Mview[3][2] = 0.0;

      Mview[0][3] = 0.0;
      Mview[1][3] = 0.0;
      Mview[2][3] = 0.0;
      Mview[3][3] = 1.0;

      for (i = 0; i < 4; i++)
      {
        for (j = 0; j < 4; j++)
        {
          Mview1[i][j] = Mview[j][i];
        }
      }
      return;
    }

    /// 利用MontageRendering函数进行某角度的Montage渲染。
    /**
     * 以象空间的Slice轴向为视线方向，对象空间每个像素进行物空间检索（即从Montage图取色，从Dem取高度信息）
     * 利用检索信息对最终图象进行渲染。
     *
     * @param Sita 为绕Line轴视角方向(单位：角度值)。
     * @param Fia 为绕Pixel轴视角方向(单位：角度值)。以荧屏为例其左上角为原点，左上角至右上角方向为Pixel轴向，左上角至左下角方向为Line轴向。
     * @param MontageImage 为ImageDef类图像Montage的指针,该图像允许彩色和灰度格式。
     * @param DEM_Image 为ImageDef类图像Dem的指针,该图像允许灰度格式。
     * @return ImageDef类Montange渲染图像指针，用后删除指针。
     *
     * @author 孙维忠
     */
    template <class T>
    ImageDef<T > *MontageRendering(double Sita, double Fia, ImageDef<T> *MontageImage, ImageDef<T> *DEM_Image)
    {
      const int MatrixSize = 8193;
      int i,k,Slice, Line, Pixel;
      double NewSlice = 1.0, NewLine = 1.0, NewPixel = 1.0;
      int TotalSlice, TotalLine, TotalPixel;
      double MviewLookupTable00[MatrixSize], MviewLookupTable01[MatrixSize], MviewLookupTable02[MatrixSize];
      double MviewLookupTable10[MatrixSize], MviewLookupTable11[MatrixSize], MviewLookupTable12[MatrixSize];
      double MviewLookupTable20[MatrixSize], MviewLookupTable21[MatrixSize], MviewLookupTable22[MatrixSize];
      int point[8][3];
      int StartSlice = 1000, EndSlice = -1000, StartLine = 1000, EndLine = -1000, StartPixel = 1000, EndPixel = -1000;
      int xx, yy, zz, min_xyz, max_xyz;
      double xx1, yy1, zz1;
      double xx0, yy0, zz0;
      double tabx, taby;
      double Mview[4][4], Mview1[4][4];
      int OutImageWidth, OutImageHeight;
      ImageDef<T> *OutImage;

      // 计算4*4 坐标变换矩阵
      _ComputeMview(Mview, Mview1, Sita, Fia);
      TotalSlice = 255;
      TotalLine = MontageImage->Height;
      TotalPixel = MontageImage->Width;

      // about point true postion, please see document
      point[0][0] = 0;          point[0][1] = TotalLine; point[0][2] = 0;
      point[1][0] = TotalPixel; point[1][1] = TotalLine; point[1][2] = 0;
      point[2][0] = TotalPixel; point[2][1] = TotalLine; point[2][2] = TotalSlice;
      point[3][0] = 0;          point[3][1] = TotalLine; point[3][2] = TotalSlice;

      point[4][0] = 0;          point[4][1] = 0; point[4][2] = 0;
      point[5][0] = TotalPixel; point[5][1] = 0; point[5][2] = 0;
      point[6][0] = TotalPixel; point[6][1] = 0; point[6][2] = TotalSlice;
      point[7][0] = 0;          point[7][1] = 0; point[7][2] = TotalSlice;

      for (i = 0; i < 8; i++)
      {
       xx0 = Mview1[0][0] * (double)point[i][0] + Mview1[0][1] * (double)point[i][1] + Mview1[0][2] * (double)point[i][2] ;
       yy0 = Mview1[1][0] * (double)point[i][0] + Mview1[1][1] * (double)point[i][1] + Mview1[1][2] * (double)point[i][2] ;
       zz0 = Mview1[2][0] * (double)point[i][0] + Mview1[2][1] * (double)point[i][1] + Mview1[2][2] * (double)point[i][2] ;

       xx = int(xx0);
       yy = int(yy0);
       zz = int(zz0);

       if (xx < StartPixel) StartPixel = xx;
       else if (xx > EndPixel) EndPixel = xx;

       if (yy < StartLine) StartLine = yy;
       else if (yy > EndLine) EndLine = yy;

       if (zz < StartSlice) StartSlice = zz;
       else if (zz > EndSlice) EndSlice = zz;
      }

      min_xyz = StartPixel;
      if(min_xyz > StartLine) min_xyz = StartLine;
      if(min_xyz > StartSlice) min_xyz = StartSlice;

      max_xyz = EndPixel;
      if(max_xyz < EndLine) max_xyz = EndLine;
      if(max_xyz < EndSlice) max_xyz = EndSlice;

      if((max_xyz - min_xyz + 3) >= MatrixSize)
      {
        printf ("the matrix size defined too small.");
        throw OutOfMemoryException();
      }

      OutImageWidth = EndPixel - StartPixel;
      OutImageHeight = EndLine - StartLine;

      OutImage = ImageDef<T>::CreateInstance(MontageImage->Format, OutImageWidth, OutImageHeight);


      // 初始化坐标变换矩阵 Lookup table
      for (k = 0; k <= max_xyz - min_xyz + 2; k++)
      {
        MviewLookupTable00[k] = Mview[0][0] * (double)(k + min_xyz);
        MviewLookupTable01[k] = Mview[0][1] * (double)(k + min_xyz);
        MviewLookupTable02[k] = Mview[0][2] * (double)(k + min_xyz);

        MviewLookupTable10[k] = Mview[1][0] * (double)(k + min_xyz);
        MviewLookupTable11[k] = Mview[1][1] * (double)(k + min_xyz);
        MviewLookupTable12[k] = Mview[1][2] * (double)(k + min_xyz);

        MviewLookupTable20[k] = Mview[2][0] * (double)(k + min_xyz);
        MviewLookupTable21[k] = Mview[2][1] * (double)(k + min_xyz);
        MviewLookupTable22[k] = Mview[2][2] * (double)(k + min_xyz);
      }

      if (MontageImage->Format != IMAGE_FORMAT_INDEX)
      {
        // from the Slice direction view the volme body
        for (Line = StartLine; Line < EndLine; Line++)
        {
          for (Pixel = StartPixel; Pixel < EndPixel; Pixel++)
          {
            for (Slice = EndSlice; Slice > StartSlice; Slice--)
            {
              xx = Pixel - min_xyz;
              yy = Line - min_xyz;
              zz = Slice - min_xyz;

              NewPixel = MviewLookupTable00[xx] + MviewLookupTable01[yy] + MviewLookupTable02[zz];
              NewLine  = MviewLookupTable10[xx] + MviewLookupTable11[yy] + MviewLookupTable12[zz];
              NewSlice = MviewLookupTable20[xx] + MviewLookupTable21[yy] + MviewLookupTable22[zz];

              xx1 = NewPixel;
              yy1 = NewLine;
              zz1 = NewSlice;

              if ((xx1 < 0.0) || (xx1 >= (double)(TotalPixel - 1))) continue;
              if ((yy1 < 0.0) || (yy1 >= (double)(TotalLine - 1))) continue;
              if ((zz1 < 0.0) || (zz1 >= (double)(TotalSlice - 1)))continue;

              xx = (int)xx1;
              yy = (int)yy1;
              zz = (int)zz1;

              if (zz <= *(DEM_Image->Pixels + yy * TotalPixel + xx))
              {
                tabx = xx1 - xx;
                taby = yy1 - yy;

                for (int i = 0; i <= 2; i++)
                {
                  *(OutImage->Pixels + (Line - StartLine) * OutImage->Width * 3 + (Pixel - StartPixel) * 3 + i)
                  = (unsigned char)((*(MontageImage->Pixels + yy * TotalPixel * 3+ xx * 3 + i) * tabx
                  + *(MontageImage->Pixels + yy * TotalPixel * 3 + (xx + 1) * 3 + i) * (1 - tabx))* taby
                  + (*(MontageImage->Pixels + (yy + 1) * TotalPixel * 3 + xx * 3 + i) * tabx
                  + *(MontageImage->Pixels + (yy + 1) * TotalPixel * 3 + (xx + 1) * 3 + i) * (1 - tabx)) * (1 - taby));
                }
                break;
              }
            }
          }
        }
      }
      else
      {
        // from the Slice direction view the volme body
        for (Line = StartLine; Line < EndLine; Line++)
        {
          for (Pixel = StartPixel; Pixel < EndPixel; Pixel++)
          {
            for (Slice = EndSlice; Slice > StartSlice; Slice--)
            {
              xx = Pixel - min_xyz;
              yy = Line - min_xyz;
              zz = Slice - min_xyz;

              NewPixel = MviewLookupTable00[xx] + MviewLookupTable01[yy] + MviewLookupTable02[zz];
              NewLine  = MviewLookupTable10[xx] + MviewLookupTable11[yy] + MviewLookupTable12[zz];
              NewSlice = MviewLookupTable20[xx] + MviewLookupTable21[yy] + MviewLookupTable22[zz];

              xx1 = NewPixel;
              yy1 = NewLine;
              zz1 = NewSlice;

              if ((xx1 < 0.0) || (xx1 >= (double)(TotalPixel - 1))) continue;
              if ((yy1 < 0.0) || (yy1 >= (double)(TotalLine - 1))) continue;
              if ((zz1 < 0.0) || (zz1 >= (double)(TotalSlice - 1))) continue;

              xx = (int)xx1;
              yy = (int)yy1;
              zz = (int)zz1;

              if (zz <= *(DEM_Image->Pixels + yy * TotalPixel + xx))
              {
                tabx = xx1 - xx;
                taby = yy1 - yy;

                *(OutImage->Pixels + (  Line - StartLine) * OutImage->Width + (Pixel - StartPixel) + i)
                = (unsigned char)((*(MontageImage->Pixels + yy * TotalPixel + xx  + i) * tabx
                + *(MontageImage->Pixels + yy * TotalPixel + (xx + 1) + i) * (1 - tabx))* taby
                + (*(MontageImage->Pixels + (yy + 1) * TotalPixel + xx + i) * tabx
                + *(MontageImage->Pixels + (yy + 1) * TotalPixel + (xx + 1) + i) * (1 - tabx)) * (1 - taby));
                break;
              }
            }
          }
        }
      }
      return OutImage;
    }

    /// 图像视线中心位置模式枚举常量。
    /**
     * 在SplashRendering函数设定三种图像视线中心位置模式：以三维体的中心为视线中心，以三维体的底面中心为视线中心，以三维
     * 体中心和底面中心的中点为视线中心。通过设定三维体图像的视线中心，人为调整观察者的视线焦点中心位置，以达到最佳视觉效
     * 果，推荐使用VOLUME_CENTRAL模式。这些模式可根据今后的需要进行扩充。
     */
    typedef enum {VOLUME_CENTRAL,    /**< 图像视线中心位置模式，表示以三维体的中心为视线中心。*/
                  BOTTOM_CENTRAL,    /**< 图像视线中心位置模式，表示以三维体的底面中心为视线中心。*/
                  HALFSHIFT_CENTRAL, /**< 图像视线中心位置模式，表示以三维体的中心和底面中心的中点为视线中心。 */
                 } CentralMode;

    /// 利用SplashRendering函数进行某角度的Montage渲染。
    /**
     * 以象空间的Slice轴向为视线方向，对象空间每个像素进行物空间检索（即从Montage图取色，从Dem取高度信息），
     * 利用检索信息对最终图象进行渲染。该算法是MontageRendering的快速算法，超过一定的视线角度图像质量下降。
     *
     * @param Sita 为绕Line轴视角方向(单位：角度值)。
     * @param Fia 为绕Pixel轴视角方向(单位：角度值)。以荧屏为例其左上角为原点，左上角至右上角方向为Pixel轴向，左上角至左下角方向为Line轴向。
     * @param MontageImage 为ImageDef类图像Montage的指针,该图像允许彩色和灰度格式。
     * @param DEM_Image 为ImageDef类图像Dem的指针,该图像允许灰度格式。
     * @param CtMode 为Montange渲染图像视线的中心模式,推荐使用VolumeCentral模式。
     * @return ImageDef类Montange渲染图像指针，用后删除指针。
     *
     * @author 孙维忠
     */
    template <class T>
    ImageDef<T > * SplashRendering(double Sita, double Fia, ImageDef<T> *MontageImage, ImageDef<T> *DEM_Image, CentralMode CtMode)
    {
      const int MatrixSize = 8193;
      int i, Slice, Line, Pixel;
      int TotalSlice, TotalLine, TotalPixel;
      int point[8][3];
      int StartSlice = 1000, EndSlice = -1000, StartLine = 1000, EndLine = -1000, StartPixel = 1000, EndPixel = -1000;
      int xx, yy, zz, min_xyz, max_xyz;
      double xx0, yy0, zz0;
      double Mview[4][4], Mview1[4][4];
      int OutImageWidth, OutImageHeight,Flag = 0;

      int BtCentralPixel,BtCentralLine;
      int VlCentralPixel,VlCentralLine;

      ImageDef<T> *OutImage;

      // 计算4*4 坐标变换矩阵
      _ComputeMview(Mview, Mview1, Sita, Fia);
      TotalSlice = 255;
      TotalLine = MontageImage->Height;
      TotalPixel = MontageImage->Width;

      // about point true postion, please see document
      point[0][0] = 0;          point[0][1] = TotalLine; point[0][2] = 0;
      point[1][0] = TotalPixel; point[1][1] = TotalLine; point[1][2] = 0;
      point[2][0] = TotalPixel; point[2][1] = TotalLine; point[2][2] = TotalSlice;
      point[3][0] = 0;          point[3][1] = TotalLine; point[3][2] = TotalSlice;

      point[4][0] = 0;          point[4][1] = 0; point[4][2] = 0;
      point[5][0] = TotalPixel; point[5][1] = 0; point[5][2] = 0;
      point[6][0] = TotalPixel; point[6][1] = 0; point[6][2] = TotalSlice;
      point[7][0] = 0;          point[7][1] = 0; point[7][2] = TotalSlice;

      for (i = 0; i < 8; i++)
      {
       xx0 = Mview1[0][0] * (double)point[i][0] + Mview1[0][1] * (double)point[i][1] + Mview1[0][2] * (double)point[i][2] ;
       yy0 = Mview1[1][0] * (double)point[i][0] + Mview1[1][1] * (double)point[i][1] + Mview1[1][2] * (double)point[i][2] ;
       zz0 = Mview1[2][0] * (double)point[i][0] + Mview1[2][1] * (double)point[i][1] + Mview1[2][2] * (double)point[i][2] ;

       xx = int(xx0);
       yy = int(yy0);
       zz = int(zz0);

       if (xx < StartPixel) StartPixel = xx;
       else if (xx > EndPixel) EndPixel = xx;

       if (yy < StartLine) StartLine = yy;
       else if (yy > EndLine) EndLine = yy;

       if (zz < StartSlice) StartSlice = zz;
       else if (zz > EndSlice) EndSlice = zz;
      }

      min_xyz = StartPixel;
      if(min_xyz > StartLine) min_xyz = StartLine;
      if(min_xyz > StartSlice) min_xyz = StartSlice;

      max_xyz = EndPixel;
      if(max_xyz < EndLine) max_xyz = EndLine;
      if(max_xyz < EndSlice) max_xyz = EndSlice;

      if((max_xyz - min_xyz + 3) >= MatrixSize)
      {
        printf ("the matrix size defined too small.");
        throw OutOfMemoryException();
      }

      OutImageWidth = EndPixel - StartPixel;
      OutImageHeight = EndLine - StartLine;

      BtCentralPixel = (int)(Mview1[0][0] * (double)(TotalPixel / 2) + Mview1[0][1] * (double)(TotalLine) + Mview1[0][2] * 0.0);
      BtCentralLine = (int)(Mview1[1][0] * 0.0 + Mview1[1][1] * (double)(TotalLine / 2) + Mview1[1][2] * 0.0);

      VlCentralPixel = (int)(Mview1[0][0] * (double)(TotalPixel / 2) + Mview1[0][1] * (double)(TotalLine / 2) + Mview1[0][2] * (double)(TotalSlice / 2));
      VlCentralLine = (int)(Mview1[1][0] * (double)(TotalPixel / 2) + Mview1[1][1] * (double)(TotalLine / 2) + Mview1[1][2] * (double)(TotalSlice / 2));

      switch (CtMode)
      {
        case VOLUME_CENTRAL:
          VlCentralPixel = 0;
          VlCentralLine =0;
          break;

        case BOTTOM_CENTRAL:
          VlCentralPixel = (BtCentralPixel - VlCentralPixel);
          VlCentralLine = (BtCentralLine - VlCentralLine);
          break;

        case HALFSHIFT_CENTRAL:
          VlCentralPixel = (BtCentralPixel - VlCentralPixel) / 2;
          VlCentralLine = (BtCentralLine - VlCentralLine) / 2;
          break;
      }

      OutImage = ImageDef<T>::CreateInstance(MontageImage->Format, OutImageWidth, OutImageHeight);

      if ( OutImage->Format != IMAGE_FORMAT_INDEX)
      {
        for (int i = 0; i < 3 * OutImageWidth * OutImageHeight; i++)
        {
          *(OutImage->Pixels + i) = 255;
        }

        for (int i = 0; i < 3*TotalLine * TotalPixel; i++)
        {
          if ((*(MontageImage->Pixels + i) == 255) && (*(MontageImage->Pixels + ++i) == 255) && (*(MontageImage->Pixels + ++i)))
          {
            *(MontageImage->Pixels + i - 2) = 254;
            *(MontageImage->Pixels + i - 1) = 254;
            *(MontageImage->Pixels + i) = 254;
          }
        }
      }
      else
      {
        for (int i = 0; i < OutImageWidth * OutImageHeight; i++)
        {
          *(OutImage->Pixels + i) = 255;
        }
        for (int i = 0; i < TotalLine * TotalPixel; i++)
        {
          if (*(MontageImage->Pixels + i) == 255)
          {
            *(MontageImage->Pixels + i) = 254;
          }
        }
      }

      // 初始化坐标变换矩阵 Lookup table

      if ( MontageImage->Format != IMAGE_FORMAT_INDEX)
      {
        for (Line = 0; Line < TotalLine - 1; Line++)
        {
          for (Pixel = 0; Pixel < TotalPixel - 1; Pixel++)
          {
            Slice = *(DEM_Image->Pixels + Line * TotalPixel + Pixel);

            xx0 = Mview1[0][0] * (double)Pixel + Mview1[0][1] * (double)Line + Mview1[0][2] * (double)Slice;
            yy0 = Mview1[1][0] * (double)Pixel + Mview1[1][1] * (double)Line + Mview1[1][2] * (double)Slice;

            if ((xx0 - VlCentralPixel) < StartPixel + 1 || (xx0 - VlCentralPixel) > EndPixel - 1) continue;
            if ((yy0 - VlCentralLine) < StartLine + 1 || (yy0 - VlCentralLine) > EndLine - 1) continue;

            xx = (int)xx0;
            yy = (int)yy0;

            for (int i = 0; i <= 2; i++)
            {
              *(OutImage->Pixels + (yy - VlCentralLine - StartLine) * OutImage->Width * 3 + (xx - VlCentralPixel - StartPixel) * 3 + i)
              = (unsigned char)(*(MontageImage->Pixels + Line * TotalPixel * 3+ Pixel * 3 + i));

              *(OutImage->Pixels + (yy- VlCentralLine - 1 - StartLine) * OutImage->Width * 3 + (xx - VlCentralPixel- StartPixel) * 3 + i)
              = (unsigned char)(*(MontageImage->Pixels + Line * TotalPixel * 3+ Pixel * 3 + i));

              *(OutImage->Pixels + (yy - VlCentralLine + 1 - StartLine) * OutImage->Width * 3 + (xx - VlCentralPixel - StartPixel) * 3 + i)
              = (unsigned char)(*(MontageImage->Pixels + Line * TotalPixel * 3+ Pixel * 3 + i));

              *(OutImage->Pixels + (yy - VlCentralLine - StartLine) * OutImage->Width * 3 + (xx - VlCentralPixel - 1 - StartPixel) * 3 + i)
              = (unsigned char)(*(MontageImage->Pixels + Line * TotalPixel * 3+ Pixel * 3 + i));

              *(OutImage->Pixels + (yy - VlCentralLine - StartLine) * OutImage->Width * 3 + (xx - VlCentralPixel + 1 - StartPixel) * 3 + i)
              = (unsigned char)(*(MontageImage->Pixels + Line * TotalPixel * 3+ Pixel * 3 + i));

              *(OutImage->Pixels + (yy - VlCentralLine -1- StartLine) * OutImage->Width * 3 + (xx- VlCentralPixel - 1 - StartPixel) * 3 + i)
              = (unsigned char)(*(MontageImage->Pixels + Line * TotalPixel * 3+ Pixel * 3 + i));

              *(OutImage->Pixels + (yy- VlCentralLine + 1 - StartLine) * OutImage->Width * 3 + (xx - VlCentralPixel + 1 - StartPixel) * 3 + i)
              = (unsigned char)(*(MontageImage->Pixels + Line * TotalPixel * 3+ Pixel * 3 + i));

              *(OutImage->Pixels + (yy- VlCentralLine - 1 - StartLine) * OutImage->Width * 3 + (xx - VlCentralPixel + 1 - StartPixel) * 3 + i)
              = (unsigned char)(*(MontageImage->Pixels + Line * TotalPixel * 3+ Pixel * 3 + i));

              *(OutImage->Pixels + (yy+ VlCentralLine + 1 - StartLine) * OutImage->Width * 3 + (xx - VlCentralPixel - 1 - StartPixel) * 3 + i)
              = (unsigned char)(*(MontageImage->Pixels + Line * TotalPixel * 3+ Pixel * 3 + i));
            }
          }
        }
        if(Sita < 0)
        {
          for (int i = 0; i < OutImageHeight; i++)
          {
            Flag = 0;
            for (int j= 0; j < 3 * OutImageWidth; j++)
            {
              if (Flag == 0 && (*(OutImage->Pixels + i * 3 * OutImageWidth + j) == 255) && (*(OutImage->Pixels + i * 3 * OutImageWidth + j +1) == 255) && (*(OutImage->Pixels + i * 3 * OutImageWidth + j +2) ==255))
              {
                j++;
                j++;
                continue;
              }
              else if (Flag == 0 && ((*(OutImage->Pixels + i * 3 * OutImageWidth + j) != 255) || (*(OutImage->Pixels + i * 3 * OutImageWidth + j +1) != 255) || (*(OutImage->Pixels + i * 3 * OutImageWidth + j +2) !=255)))
              {
                j++;
                j++;
                Flag = 1;
                continue;
              }
              else if (Flag == 1 && (*(OutImage->Pixels + i * 3 * OutImageWidth + j) == 255) && (*(OutImage->Pixels + i * 3 * OutImageWidth + j + 1) == 255) && (*(OutImage->Pixels + i*3*OutImageWidth + j +2) ==255))
              {
                *(OutImage->Pixels +i * 3 * OutImageWidth + j) = *(OutImage->Pixels + 3 * i * OutImageWidth + j - 3);
                *(OutImage->Pixels + i * 3 * OutImageWidth + j + 1) = *(OutImage->Pixels + 3 * i * OutImageWidth + j - 2);
                *(OutImage->Pixels + i * 3 * OutImageWidth + j + 2) = *(OutImage->Pixels + 3 * i * OutImageWidth + j - 1);
                j++;
                j++;
                continue;
              }
              else
              {
                j++;
                j++;
                continue;
              }
            }
          }
        }
        else if(Sita > 0)
        {
          for (int i = 0; i < OutImageHeight; i++)
          {
            Flag = 0;
            for (int j= 0; j < 3 * OutImageWidth; j++)
            {
              if (Flag == 0 && (*(OutImage->Pixels + i * 3 * OutImageWidth + j) == 255) && (*(OutImage->Pixels + i * 3 * OutImageWidth + j +1) == 255) && (*(OutImage->Pixels + i * 3 * OutImageWidth + j + 2) == 255))
              {
                j++;
                j++;
                continue;
              }
              else if (Flag == 0 && ((*(OutImage->Pixels + i * 3 * OutImageWidth + j) != 255) || (*(OutImage->Pixels + i * 3 * OutImageWidth + j +1) != 255) || (*(OutImage->Pixels + i * 3 * OutImageWidth + j + 2) != 255)))
              {
                j++;
                j++;
                Flag = 1;
                continue;
              }
              else if ((Flag == 1) &&(*(OutImage->Pixels + i * 3 * OutImageWidth + j) == 255) && (*(OutImage->Pixels + i * 3 * OutImageWidth + j +1) == 255) && (*(OutImage->Pixels + i * 3 * OutImageWidth + j + 2) == 255))
              {
                *(OutImage->Pixels +i * 3 * OutImageWidth + j) = *(OutImage->Pixels + 3 * i * OutImageWidth + j - 3);
                *(OutImage->Pixels + i * 3 * OutImageWidth + j + 1) = *(OutImage->Pixels + 3 * i * OutImageWidth + j - 2);
                *(OutImage->Pixels + i * 3 * OutImageWidth + j + 2) = *(OutImage->Pixels + 3 * i * OutImageWidth + j - 1);
                j++;
                j++;
                continue;
              }
              else
              {
                j++;
                j++;
                continue;
              }
            }
          }
        }
      }
      else
      {
        for (Line = 0; Line < TotalLine-1; Line++)
        {
          for (Pixel = 0; Pixel < TotalPixel-1; Pixel++)
          {
            Slice = *(DEM_Image->Pixels + Line * TotalPixel + Pixel);

            xx0 = Mview1[0][0] * (double)Pixel + Mview1[0][1] * (double)Line + Mview1[0][2] * (double)Slice;
            yy0 = Mview1[1][0] * (double)Pixel + Mview1[1][1] * (double)Line + Mview1[1][2] * (double)Slice;

            if ((xx0 - VlCentralPixel) < StartPixel + 1 || (xx0 - VlCentralPixel) > EndPixel - 1) continue;
            if ((yy0 - VlCentralLine) < StartLine + 1 || (yy0 - VlCentralLine) > EndLine - 1) continue;

            xx = (int)xx0;
            yy = (int)yy0;

            for (int i = 0; i <= 2; i++)
            {
              *(OutImage->Pixels + (yy - VlCentralLine- StartLine) * OutImage->Width + (xx - VlCentralPixel- StartPixel))
              = (unsigned char)(*(MontageImage->Pixels + Line * TotalPixel + Pixel));

              *(OutImage->Pixels + (yy - VlCentralLine- 1 - StartLine) * OutImage->Width + (xx - VlCentralPixel- StartPixel))
              = (unsigned char)(*(MontageImage->Pixels + Line * TotalPixel + Pixel));

              *(OutImage->Pixels + (yy - VlCentralLine+ 1 - StartLine) * OutImage->Width + (xx - VlCentralPixel- StartPixel))
              = (unsigned char)(*(MontageImage->Pixels + Line * TotalPixel + Pixel));

              *(OutImage->Pixels + (yy - VlCentralLine- StartLine) * OutImage->Width + (xx - VlCentralPixel- 1 - StartPixel))
              = (unsigned char)(*(MontageImage->Pixels + Line * TotalPixel + Pixel));

              *(OutImage->Pixels + (yy - VlCentralLine- StartLine) * OutImage->Width + (xx - VlCentralPixel + 1 - StartPixel))
              = (unsigned char)(*(MontageImage->Pixels + Line * TotalPixel + Pixel));

              *(OutImage->Pixels + (yy - VlCentralLine- 1 - StartLine) * OutImage->Width + (xx - VlCentralPixel - 1 - StartPixel))
              = (unsigned char)(*(MontageImage->Pixels + Line * TotalPixel + Pixel));

              *(OutImage->Pixels + (yy - VlCentralLine+ 1 - StartLine) * OutImage->Width + (xx - VlCentralPixel -1- StartPixel))
              = (unsigned char)(*(MontageImage->Pixels + Line * TotalPixel + Pixel));

              *(OutImage->Pixels + (yy - VlCentralLine- 1 - StartLine) * OutImage->Width + (xx - VlCentralPixel + 1 - StartPixel))
              = (unsigned char)(*(MontageImage->Pixels + Line * TotalPixel + Pixel));

              *(OutImage->Pixels + (yy - VlCentralLine+ 1 - StartLine) * OutImage->Width + (xx - VlCentralPixel + 1 - StartPixel))
              = (unsigned char)(*(MontageImage->Pixels + Line * TotalPixel + Pixel));
            }
          }
        }
        if(Sita < 0)
        {
          for (int i = 0; i < OutImageHeight; i++)
          {
            Flag = 0;
            for (int j = 0; j < OutImageWidth; j++)
            {
              if (Flag == 0 && *(OutImage->Pixels + i*OutImageWidth + j) == 255 )
              {
                continue;
              }
              else if (Flag == 0 && *(OutImage->Pixels + i*OutImageWidth + j) != 255)
              {
                Flag = 1;
                continue;
              }
              else if (Flag == 1 && *(OutImage->Pixels + i*OutImageWidth + j) == 255)
              {
                *(OutImage->Pixels + i * OutImageWidth + j) = *(OutImage->Pixels + i*OutImageWidth + j - 1);
                continue;
              }
              else
              {
                continue;
              }
            }
          }
        }
        else if(Sita > 0)
        {
          for (int i = 0; i < OutImageHeight; i++)
          {
            Flag = 0;
            for (int j = 0; j < 3 * OutImageWidth; j++)
            {
              if (Flag == 0 && *(OutImage->Pixels + i*OutImageWidth + j) == 255)
              {
                continue;
              }
              else if (Flag == 0 && *(OutImage->Pixels + i*OutImageWidth + j) != 255)
              {
                Flag = 1;
                continue;
              }
              else if (Flag == 1 && *(OutImage->Pixels + i*OutImageWidth + j) == 255)
              {
                *(OutImage->Pixels + i * OutImageWidth + j) = *(OutImage->Pixels + i * OutImageWidth + j - 1);
                continue;
              }
              else
              {
                continue;
              }
            }
          }
        }
      }
      return OutImage;
    }

    /// 把两幅图像合成一个ANAGLYPH效果图。
    /**
     * 对于给定的两幅输入图像要求长，宽相等，属于同一类型，比如都是彩色图或灰度图，然后合成一个彩色的ANAGLYPH图像。
     * 函数返回生成的彩色ANAGLYPH图像。
     *
     * 利用Montage渲染函数，和Anaglyph浮雕创建函数生成浮雕图像的过程如下：
     *
     * 使用Montage渲染函数生成左右视野的Montage图像, 然后调用Anaglyph浮雕创建函数生成浮雕图像。
     * Fia设定为0, Sita 左视野设置范围为[6,0), 右视野设置范围为(0,-6]，左右视角要对称，例如：
     * <PRE>
     * if(i == 0)
     * { //left image
     *   Image1 = MontageRendering(6, 0, MontageImage, DEM_Image);
     *   //Image1 = SplashRendering(6, 0, MontageImage, DEM_Image);
     * }
     * else
     * { //right image
     *   Image2 = MontageRendering(-6, 0, MontageImage, DEM_Image);
     *   //Image2 = SplashRendering(-6, 0, MontageImage, DEM_Image);
     * }
     * AnaglyphImage = CreateAnaglyphDb(imageL, imageR);
     * //AnaglyphImage = CreateAnaglyphTri(imageL, imageM,imageR);
     *
     * SaveImageAsBmp(AnaglyphImage, "Anaglyph.bmp");
     * </PRE>
     *
     * @param imageL 输入的第一幅图像,该图像允许彩色和灰度格式。
     * @param imageR 输入的第二幅图像,该图像允许彩色和灰度格式。
     * @return 生成的彩色ANAGLYPH图像，用后删除指针。
     *
     * @author 刘莉
     */
    template <class T>
    ImageDef<T>* CreateAnaglyphDb(ImageDef<T> *imageL, ImageDef<T> *imageR)
    {
      ImageDef<T> *AnaglyphImage;
      if (imageL->Height != imageR->Height)
        throw UnsupportedFormatException();

      if (imageL->Format != imageR->Format)
        throw UnsupportedFormatException();

      ImageFormat format = imageL->Format;

      int width,width1,width2,height;
      width1 = imageL->Width;
      width2 = imageR->Width;
      height = imageL->Height;

      width = (width1 > width2 ? width1 : width2);

      int usedColor = imageL->UsedColor;

      T *pImgData1 = NULL;
      T *pImgData2 = NULL;

      AnaglyphImage = ImageDef<T>::CreateInstance(IMAGE_FORMAT_RGB,width,height,usedColor);
      AnaglyphImage->Palette = NULL;
      T *pImgData = AnaglyphImage->Pixels;

      int i,j,j2,k,k2;

      //两副图合成AnaglyphImage
      switch (format)
      {
        case IMAGE_FORMAT_INDEX:
        {
          pImgData1 = imageL->Pixels;
          pImgData2 = imageR->Pixels;

          for (i = 0; i < height; i++,pImgData += 3 * width,pImgData1 += width1)
          {
            for (j = 0,j2 = 0; j < width1; j++,j2++)
            {
              k=3*j;
              pImgData[k+2] = pImgData1[j2];
            }
          }

          //复位
          pImgData = AnaglyphImage->Pixels;

          for (i = 0; i < height; i++, pImgData += 3 * width, pImgData2 += width2)
          {
            for (j = 0,j2 = 0; j < width2; j++,j2++)
            {
              k=3*j;
              pImgData[k] = pImgData2[j2];
              pImgData[k+1] = pImgData2[j2];
            }
          }
          break;
        }
        case IMAGE_FORMAT_RGB:
        case IMAGE_FORMAT_BGR:
        {
          pImgData1 = imageL->Pixels;
          pImgData2 = imageR->Pixels;

          for (i = 0; i < height; i++, pImgData += 3 * width, pImgData1 += 3 * width1)
          {
            for (j = 0, j2=0; j < width1; j++, j2++)
            {
              k=3 * j;
              k2=3 * j2;
              pImgData[k + 2] = pImgData1[k2 + 2];
            }
          }

          //复位
          pImgData = AnaglyphImage->Pixels;

          for (i = 0; i < height; i++, pImgData += 3 * width, pImgData2 += 3 * width2)
          {
            for (j = 0, j2 = 0; j < width2; j++, j2++)
            {
              k=3 * j;
              k2=3 * j2;
              pImgData[k + 0] = pImgData2[k2 + 0];
              pImgData[k + 1] = pImgData2[k2 + 1];
            }
          }
          break;
        }
        default:
          throw UnsupportedFormatException();
          break;
      }
      return AnaglyphImage;
    }

    /// 把三幅图像合成一个ANAGLYPH效果图。
    /**
     * 对于给定的三幅输入图像要求属于同一类型，比如都是彩色图或灰度图，然后合成一个彩色的ANAGLYPH图像。
     * 函数返回生成的彩色ANAGLYPH图像。
     *
     * @param imageL 输入的左图像,三幅图像之间各偏移一个角度,该图像允许彩色和灰度格式。
     * @param imageM 输入的中间图像,该图像允许彩色和灰度格式。
     * @param imageR 输入的右图像,该图像允许彩色和灰度格式。
     * @return 生成的彩色ANAGLYPH图像,其长和宽和中间图象保持一致。
     */
    template <class T>
    ImageDef<T>* CreateAnaglyphTri(ImageDef<T> *imageL, ImageDef<T> *imageM,ImageDef<T> *imageR)
    {
      if (imageL->Format != imageM->Format || imageM->Format != imageR->Format || imageL->Format != imageR->Format)
      throw UnsupportedFormatException();

      ImageFormat format = imageL->Format;

      int width1,width2,width3,width,height;
      width1 = imageL->Width;
      width2 = imageM->Width;
      width3 = imageR->Width;
      width = width2;
      height = imageM->Height;

      int usedColor = imageL->UsedColor;

      T *pImgData1 = NULL;
      T *pImgData2 = NULL;
      T *pImgData3 = NULL;

      pImgData1 =  imageL->Pixels;
      pImgData2 =  imageM->Pixels;
      pImgData3 =  imageR->Pixels;

      ImageDef<T> *AnaglyphImage = ImageDef<T>::CreateInstance(IMAGE_FORMAT_BGR,width,height,usedColor);
      AnaglyphImage->Palette = NULL;
      T *pImgData = AnaglyphImage->Pixels;

      int i,j,j2,k,k2;

      //三副图合成Anaglyph

      switch (format)
      {
        case IMAGE_FORMAT_INDEX:
        {
          for (i = 0; i < height; i++,pImgData+=3*width,pImgData1+=width1)
          {
            for (j=0,j2=(width1-width)/2; j < width; j++,j2++)
            {
              k=3*j;
              pImgData[k+2] = pImgData1[j2];
            }
          }

          pImgData = AnaglyphImage->Pixels;//复位

          for (i = 0; i < height; i++,pImgData+=3*width,pImgData2+=width2)
          {
            for (j = 0,j2 = 0; j < width; j++,j2++)
            {
              k=3*j;
              pImgData[k+1] = pImgData2[j2];
            }
          }

          pImgData = AnaglyphImage->Pixels;//复位

          for (i = 0; i < height; i++,pImgData+=3*width,pImgData3+=width3)
          {
            for (j=0,j2=(width3-width)/2; j < width; j++,j2++)
            {
              k=3*j;
              pImgData[k] = pImgData3[j2];
            }
          }
          break;
        }
        case IMAGE_FORMAT_RGB:
        case IMAGE_FORMAT_BGR:
        {
          for (i = 0; i < height; i++,pImgData+=3*width,pImgData1+=3*width1)
          {
            for (j=0,j2=(width1-width)/2; j < width; j++,j2++)
            {
              k=3*j;
              k2=3*j2;
              pImgData[k+2] = pImgData1[k2+2];
            }
          }

          pImgData = AnaglyphImage->Pixels;//复位

          for (i = 0; i < height; i++,pImgData+=3*width,pImgData2+=3*width2)
          {
            for (j = 0, j2 = 0; j < width; j++,j2++)
            {
              k=3*j;
              k2=3*j2;
              pImgData[k+1] = pImgData2[k2+1];
            }
          }

          pImgData = AnaglyphImage->Pixels;//复位

          for (i = 0; i < height; i++,pImgData+=3*width,pImgData3+=3*width3)
          {
            for (j=0,j2=(width3-width)/2; j < width; j++,j2++)
            {
              k=3*j;
              k2=3*j2;
              pImgData[k] = pImgData3[k2];
            }
          }
          break;
        }
        default:
          throw UnsupportedFormatException();
          break;
      }
      return AnaglyphImage;
    }
  }// Image3D namespace
}// MBL namespace

#endif //__ANAGLYPHRENDER_H__
