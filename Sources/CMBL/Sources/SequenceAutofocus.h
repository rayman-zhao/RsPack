#ifndef __SEQUENCEAUTOFOCUS_H__
#define __SEQUENCEAUTOFOCUS_H__

/**
 * @file
 *
 * @brief 拉普拉斯算子、Tenengrad函数(Sobel算子)、灰度差分绝对值之和算子(SMD)、灰度方差算子、Robert算子进行图象
 *        自动聚焦的函数，返回聚焦层面的层号。
 */

#include "AutofocusOperator.h"
#include <math.h>

namespace MBL
{
  namespace Image2D
  {
    /// 利用改进拉普拉斯算子进行图象的自动聚焦。
    /**
     * 使用改进拉普拉斯算子，找到序列中聚焦算子之和最大值所在的层，即为聚焦层面。函数返回该层的层号。
     *
     * @param image 序列图像结构指针，必须是有效的内存中的序列图像。
     * @param step 步长，计算某点的拉普拉斯算子时使用的步长大小。可选用经验值5。
     * @param threshold 阈值，大于阈值的Laplacian值才参与汇总，小于阈值的被忽略。一般选用0值。一般对于噪声干扰不大的序列
     *                  图像，步长和阈值可以取小一些；而对噪声和亮度变化干扰比较大的序列图像，步长和阈值可以取得稍大一些。
     * @return 聚焦层面的层号。
     */
    template <class T>
    int LaplacianAutoFocus(ImageSequenceDef<T> *image, int step = 5, int threshold = 0)
    {
      int k, position = 0;
      int num = image->SequenceNumber;
      int nr = image->Height;
      int nc = image->Width;
      long int focusoperator, operator_max;
      double mp, d1, d2, d3;
      long int *outarray = new long int [num];
      FILE *fp;

      if (num < 1 || image->Pixels == 0 ) throw NullPointerException();
      switch (image->Format)
      {
      case IMAGE_FORMAT_INDEX:
      case IMAGE_FORMAT_RGB:
      case IMAGE_FORMAT_BGR:
        break;
      default:
        throw UnsupportedFormatException();
        break;
      }

      //Find the focus layer from source images
      operator_max = 0;
      for(k = 0; k < num; k++)
      {
        ImageDef<T> *singleimage = 0;
        singleimage = ImageDef<T>::CreateWrapperInstance(image->Format, image->Pixels[k], image->Width, image->Height, image->UsedColor);
        focusoperator = LaplacianAutoFocusOperator(singleimage,step,threshold);
        outarray[k] = focusoperator;
        if(focusoperator> operator_max)
        {
          operator_max = focusoperator;
          position = k;
        }
      }//end of k

      //爬山法插值结果mp。
      /*
      d1 = outarray[position] - outarray[position - 1];
      d2 = outarray[position] - outarray[position + 1];
      mp = position + 1.5 * (double)(d1 - d2)/(double)(d1 + d2);
      */
      //高斯插值法结果mp。
      d1 = (log(outarray[position]) - log(outarray[position + 1])) * (2 * position - 1);
      d2 = (log(outarray[position]) - log(outarray[position - 1])) * (- 2 * position - 1);
      d3 = 2 * (2 * log(outarray[position]) - log(outarray[position + 1]) -log(outarray[position - 1]));
      mp = d1/d3 - d2/d3;
      printf("%f",mp);

      //将结果输出到lap.dat文件中，方便用tecplot对数据画图。
      if ((fp = fopen ("lap.dat","w")) != 0)
      {
         for(k=0;k<num;k++)
         {
          fprintf(fp,"%d\t",k);
          fprintf(fp,"%g\n",double(outarray[k])/(nr * nc));
         }
         fclose(fp);
      }
      delete outarray;
      return position;
    }

    /// 利用Tenengrad函数进行序列图象自动聚焦。
    /**
     * 使用Sobel算子计算每一点的梯度并对大于阈值的梯度取和汇总，找到序列中梯度平方和最大值所在的层,即为聚焦层面。
     * 函数返回该层的层号。
     *
     * @param image 序列图像结构指针，必须是有效的内存中的序列图像。
     * @param threshold 阈值。梯度值大于阈值的才参与汇总，小于阈值的被忽略。一般选用0值。
     * @return 聚焦层面的层号。
     */
    template <class T>
    int TenengradAutoFocus(ImageSequenceDef<T> *image, int threshold = 0)
    {
      int k, position;
      int num = image->SequenceNumber;
      long int focusoperator, operator_max;

      if (num < 1 || image->Pixels == 0 ) throw NullPointerException();
      switch (image->Format)
      {
        case IMAGE_FORMAT_INDEX:
        case IMAGE_FORMAT_RGB:
        case IMAGE_FORMAT_BGR:
          break;
        default:
          throw UnsupportedFormatException();
          break;
      }

      //Find the focus layer from source images
      operator_max = 0;
      for(k = 0; k < num; k++)
      {
        ImageDef<T> *singleimage = 0;
        singleimage = ImageDef<T>::CreateWrapperInstance(image->Format, image->Pixels[k], image->Width, image->Height, image->UsedColor);
        focusoperator = TenengradAutoFocusOperator(singleimage,threshold);
        if(focusoperator> operator_max)
        {
          operator_max = focusoperator;
          position = k;
        }
      }//end of k

      return position;
    }

    /// 利用灰度差分绝对值之和算子Sum-Modulus-Difference(SMD)进行序列图象自动聚焦。
    /**
     * 对图像中每一点及其左方和上方最邻近点的灰度值做差分运算并汇总，找到序列中图像灰度差分绝对值之和算子最大值
     * 所在的层,即为聚焦层面。函数返回该层的层号。
     *
     * @param image 序列图像结构指针，必须是有效的内存中的序列图像。
     * @return 聚焦层面的层号。
     */
     template <class T>
     int SMDAutoFocus(ImageSequenceDef<T> *image)
     {
       int k, position;
       int num = image->SequenceNumber;
       long int focusoperator, operator_max;

       if (num < 1 || image->Pixels == 0 ) throw NullPointerException();
       switch (image->Format)
       {
         case IMAGE_FORMAT_INDEX:
         case IMAGE_FORMAT_RGB:
         case IMAGE_FORMAT_BGR:
           break;
         default:
           throw UnsupportedFormatException();
           break;
       }

       //Find the focus layer from source images
       operator_max = 0;
       for(k = 0; k < num; k++)
       {
         ImageDef<T> *singleimage = 0;
         singleimage = ImageDef<T>::CreateWrapperInstance(image->Format, image->Pixels[k], image->Width, image->Height, image->UsedColor);
         focusoperator = SMDAutoFocusOperator(singleimage);
         if(focusoperator> operator_max)
         {
           operator_max = focusoperator;
           position = k;
         }
       }//end of k

       return position;
    }

    /// 利用灰度方差算子Gradation Variance进行序列图象自动聚焦。
    /**
     * 求图象的灰度变化的均方差并进行比较，均方差之和最大的层面即为聚焦层面。函数返回该层的层号。
     *
     * @param image 序列图像结构指针，必须是有效的内存中的序列图像。
     * @return 聚焦层面的层号。
     */
     template <class T>
     int VarianceAutoFocus(ImageSequenceDef<T> *image)
     {
       int k, position;
       int num = image->SequenceNumber;
       long int focusoperator, operator_max;

       if (num < 1 || image->Pixels == 0 ) throw NullPointerException();
       switch (image->Format)
       {
         case IMAGE_FORMAT_INDEX:
         case IMAGE_FORMAT_RGB:
         case IMAGE_FORMAT_BGR:
           break;
         default:
           throw UnsupportedFormatException();
           break;
       }

       //Find the focus layer from source images
       operator_max = 0;
       for(k = 0; k < num; k++)
       {
         ImageDef<T> *singleimage = 0;
         singleimage = ImageDef<T>::CreateWrapperInstance(image->Format, image->Pixels[k], image->Width, image->Height, image->UsedColor);
         focusoperator = VarianceAutoFocusOperator(singleimage);
         if(focusoperator> operator_max)
         {
           operator_max = focusoperator;
           position = k;
         }
       }//end of k

       return position;
    }

    /// 利用Robert算子进行序列图象自动聚焦。
    /**
     * 对图像中每一点及其邻近点的灰度值做差分运算并汇总，找到Robert梯度算子最大值所在的层，即为聚焦层面。
     * 函数返回该层的层号。
     *
     * @param image 序列图像结构指针，必须是有效的内存中的序列图像。
     * @return 聚焦层面的层号。
     */
     template <class T>
     int RobertAutoFocus(ImageSequenceDef<T> *image)
     {
      int k, position;
      int num = image->SequenceNumber;
      long int focusoperator, operator_max;

      if (num < 1 || image->Pixels == 0 ) throw NullPointerException();
      switch (image->Format)
      {
        case IMAGE_FORMAT_INDEX:
        case IMAGE_FORMAT_RGB:
        case IMAGE_FORMAT_BGR:
          break;
        default:
          throw UnsupportedFormatException();
          break;
      }

      //Find the focus layer from source images
      operator_max = 0;
      for(k = 0; k < num; k++)
      {
        ImageDef<T> *singleimage = 0;
        singleimage = ImageDef<T>::CreateWrapperInstance(image->Format, image->Pixels[k], image->Width, image->Height, image->UsedColor);
        focusoperator = RobertAutoFocusOperator(singleimage);
        if(focusoperator> operator_max)
        {
          operator_max = focusoperator;
          position = k;
        }
      }//end of k

      return position;
    }

  }// Image2D namespace
}// MBL namespace

#endif // __SEQUENCEAUTOFOCUS_H__
