#ifndef __AUTOFOCUSOPERATOR_H__
#define __AUTOFOCUSOPERATOR_H__

/**
 * @file
 *
 * @brief 各种评估图像聚焦程度的算子函数。在自动显微镜的应用中，每采集一幅图像便计算其聚焦算子值，函数返回图像聚焦算子值。
 *        在外部调用时再对函数返回结果取最大值得到聚焦图像的层数。
 */

namespace MBL
{
  namespace Image2D
  {
    /// 利用改进拉普拉斯算子进行图像的自动聚焦。
    /**
     * 使用改进拉普拉斯算子，计算单幅图像聚焦算子之和。函数返回图像聚焦算子值。在调用时再对多幅图像的函数返回值
     * 进行比较大小，返回算子值最大的层即为聚焦层面。例如：
     * <PRE>
     * operator_max = 0;
     * for (int i = START; i <= END; i++)
     * {
     *   load image;
     *   focusoperator[i - 1] = LaplacianAutoFocusOperator(image,5,0);//若不想保留算子值，可直接将focusoperator定义为变量。
     *   if(focusoperator[i - 1]> operator_max)
     *   {
     *     operator_max = focusoperator[i - 1];
     *     position = i;
     *   }
     *   delete image;
     * }
     * </PRE>
     *
     * @param image 单幅图像结构指针，必须是内存中的有效图像。
     * @param step 步长，计算某点的拉普拉斯算子时使用的步长大小。可选用经验值5。
     * @param threshold 阈值，大于阈值的Laplacian值才参与汇总，小于阈值的被忽略。一般选用0值。一般对于噪声干扰不大的序列
     *                  图像，步长和阈值可以取小一些；而对噪声和亮度变化干扰比较大的序列图像，步长和阈值可以取得稍大一些。
     * @return 图像聚焦算子值。
     */
    template <class T>
    long int LaplacianAutoFocusOperator(ImageDef<T> *image, int step = 5, int threshold = 0)
    {
      int i, j, sum;
      long int lap_sum;
      T r, g, b, cendata = 0, leftdata = 0, rightdata = 0, updata = 0, downdata = 0, *pdata = 0;
      int nr = image->Height;
      int nc = image->Width;

      if (image->Pixels == 0 ) throw NullPointerException();
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
      switch (image->Format)
      {
        case IMAGE_FORMAT_INDEX:
        case IMAGE_FORMAT_RGB:
        case IMAGE_FORMAT_BGR:
          pdata = image->Pixels;
          break;
        default:
          break;
      }

      lap_sum = 0;
      for(i = step; i < nr-step; i++)
      {
        for(j = step; j < nc-step; j++)
        {
          switch (image->Format)
          {
            case IMAGE_FORMAT_INDEX:
              cendata = *(pdata + i * nc + j);
              leftdata = *(pdata + i * nc + j - step);
              rightdata = *(pdata + i * nc + j + step);
              updata = *(pdata + (i - step) * nc + j);
              downdata = *(pdata + (i + step) * nc + j);

              break;
            case IMAGE_FORMAT_RGB:
            case IMAGE_FORMAT_BGR:
              r = *(pdata + (i * nc + j) * 3 + 0);
              g = *(pdata + (i * nc + j) * 3 + 1);
              b = *(pdata + (i * nc + j) * 3 + 2);
              cendata = (T)(r * 0.3 + g * 0.59 + b * 0.11);

              r = *(pdata + (i * nc + (j - step)) * 3 + 0);
              g = *(pdata + (i * nc + (j - step)) * 3 + 1);
              b = *(pdata + (i * nc + (j - step)) * 3 + 2);
              leftdata = (T)(r * 0.3 + g * 0.59 + b * 0.11);

              r = *(pdata + (i * nc + (j + step)) * 3 + 0);
              g = *(pdata + (i * nc + (j + step)) * 3 + 1);
              b = *(pdata + (i * nc + (j + step)) * 3 + 2);
              rightdata = (T)(r * 0.3 + g * 0.59 + b * 0.11);

              r = *(pdata + ((i - step) * nc + j) * 3 + 0);
              g = *(pdata + ((i - step) * nc + j) * 3 + 1);
              b = *(pdata + ((i - step) * nc + j) * 3 + 2);
              updata = (T)(r * 0.3 + g * 0.59 + b * 0.11);

              r = *(pdata + ((i + step) * nc + j) * 3 + 0);
              g = *(pdata + ((i + step) * nc + j) * 3 + 1);
              b = *(pdata + ((i + step) * nc + j) * 3 + 2);
              downdata = (T)(r * 0.3 + g * 0.59 + b * 0.11);

              break;
            default:
              break;
          }
          sum = abs(2 * cendata - leftdata - rightdata) + abs(2 * cendata - updata - downdata);
          if (sum > threshold)       lap_sum += sum;
        }//end of j
      }//end of i

      return lap_sum;
    }

    /// 利用Tenengrad函数进行序列图像自动聚焦。
    /**
     * 使用Sobel算子计算每一点的梯度并对大于阈值的梯度取和汇总，计算梯度平方和作为图像聚焦算子。函数返回图像聚焦算子值。
     *
     * @param image 单幅图像结构指针，必须是内存中的有效图像。
     * @param threshold 阈值。梯度值大于阈值的才参与汇总，小于阈值的被忽略。一般选用0值。
     * @return 图像聚焦算子值。
     */
    template <class T>
    long int TenengradAutoFocusOperator(ImageDef<T> *image, int threshold = 0)
    {
      int i, j, soblex, sobley;
      long int ten_sum, grad = 0;
      T r, g, b, leftupdata = 0, leftcendata = 0, leftdowndata = 0, *pdata;
      T rightupdata = 0, rightcendata = 0, rightdowndata = 0, cenupdata = 0, cendowndata = 0;
      int nr = image->Height;
      int nc = image->Width;

      if (image->Pixels == 0 ) throw NullPointerException();
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
      switch (image->Format)
      {
        case IMAGE_FORMAT_INDEX:
        case IMAGE_FORMAT_RGB:
        case IMAGE_FORMAT_BGR:
          pdata = image->Pixels;
          break;
        default:
          break;
      }

      ten_sum = 0;
      for(i = 1; i < nr - 1; i++)
      {
        for(j = 1; j < nc - 1; j++)
        {
          switch (image->Format)
          {
            case IMAGE_FORMAT_INDEX:
              leftupdata = *(pdata + (i - 1 )* nc + j - 1);
              leftcendata = *(pdata + (i - 1 )* nc + j);
              leftdowndata = *(pdata + (i - 1 )* nc + j + 1);
              rightupdata = *(pdata + (i + 1 )* nc + j - 1);
              rightcendata = *(pdata + (i + 1 )* nc + j);
              rightdowndata = *(pdata + (i + 1 )* nc + j + 1);
              cenupdata = *(pdata + i * nc + j - 1);
              cendowndata = *(pdata + i * nc + j + 1);

              break;
            case IMAGE_FORMAT_RGB:
            case IMAGE_FORMAT_BGR:
              r = *(pdata + ((i - 1) * nc + j - 1) * 3 + 0);
              g = *(pdata + ((i - 1) * nc + j - 1) * 3 + 1);
              b = *(pdata + ((i - 1) * nc + j - 1) * 3 + 2);
              leftupdata = (T)(r * 0.3 + g * 0.59 + b * 0.11);

              r = *(pdata + ((i - 1 )* nc + j) * 3 + 0);
              g = *(pdata + ((i - 1 )* nc + j) * 3 + 1);
              b = *(pdata + ((i - 1 )* nc + j) * 3 + 2);
              leftcendata = (T)(r * 0.3 + g * 0.59 + b * 0.11);

              r = *(pdata + ((i - 1) * nc + j + 1) * 3 + 0);
              g = *(pdata + ((i - 1) * nc + j + 1) * 3 + 1);
              b = *(pdata + ((i - 1) * nc + j + 1) * 3 + 2);
              leftdowndata = (T)(r * 0.3 + g * 0.59 + b * 0.11);

              r = *(pdata + ((i + 1 )* nc + j - 1) * 3 + 0);
              g = *(pdata + ((i + 1 )* nc + j - 1) * 3 + 1);
              b = *(pdata + ((i + 1 )* nc + j - 1) * 3 + 2);
              rightupdata = (T)(r * 0.3 + g * 0.59 + b * 0.11);

              r = *(pdata + ((i + 1 )* nc + j) * 3 + 0);
              g = *(pdata + ((i + 1 )* nc + j) * 3 + 1);
              b = *(pdata + ((i + 1 )* nc + j) * 3 + 2);
              rightcendata = (T)(r * 0.3 + g * 0.59 + b * 0.11);

              r = *(pdata + ((i + 1) * nc + j + 1) * 3 + 0);
              g = *(pdata + ((i + 1) * nc + j + 1) * 3 + 1);
              b = *(pdata + ((i + 1) * nc + j + 1) * 3 + 2);
              rightdowndata = (T)(r * 0.3 + g * 0.59 + b * 0.11);

              r = *(pdata + (i * nc + j - 1) * 3 + 0);
              g = *(pdata + (i * nc + j - 1) * 3 + 1);
              b = *(pdata + (i * nc + j - 1) * 3 + 2);
              cenupdata = (T)(r * 0.3 + g * 0.59 + b * 0.11);

              r = *(pdata + (i * nc + j + 1) * 3 + 0);
              g = *(pdata + (i * nc + j + 1) * 3 + 1);
              b = *(pdata + (i * nc + j + 1) * 3 + 2);
              cendowndata = (T)(r * 0.3 + g * 0.59 + b * 0.11);

              break;
            default:
              break;
          }
          soblex = rightupdata + 2 * rightcendata + rightdowndata - leftupdata - 2 * leftcendata - leftdowndata;
          sobley = leftupdata + 2 * cenupdata + rightupdata - leftdowndata - 2 * cendowndata - rightdowndata;
          grad = soblex * soblex + sobley * sobley;
          if (grad > threshold)     ten_sum += grad;
        }//end of j
      }//end of i

      return ten_sum;
     }

    /// 利用灰度差分绝对值之和算子Sum-Modulus-Difference(SMD)进行序列图像自动聚焦。
    /**
     * 对图像中每一点及其左方和上方最邻近点的灰度值做差分运算并汇总，计算图像灰度差分绝对值之和算子。
     * 函数返回图像聚焦算子值。
     *
     * @param image 单幅图像结构指针，必须是内存中的有效图像。
     * @return 图像聚焦算子值。
     */
    template <class T>
    long int SMDAutoFocusOperator(ImageDef<T> *image)
    {
     int i, j, smd = 0;
     long int smd_sum;
     T r, g, b, *pdata, cendata = 0, rightcendata = 0, cenupdata = 0;
     int nr = image->Height;
     int nc = image->Width;

     if (image->Pixels == 0 ) throw NullPointerException();
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

     switch (image->Format)
     {
       case IMAGE_FORMAT_INDEX:
       case IMAGE_FORMAT_RGB:
       case IMAGE_FORMAT_BGR:
         pdata = image->Pixels;
         break;
       default:
         break;
     }

     smd_sum = 0;
     for(i = 1; i < nr - 1; i++)
     {
       for(j = 1; j < nc - 1; j++)
       {
         switch (image->Format)
         {
           case IMAGE_FORMAT_INDEX:
             cendata = *(pdata + i * nc + j);
             rightcendata  = *(pdata + (i + 1 )* nc + j);
             cenupdata   = *(pdata + i * nc + j - 1);

             break;
           case IMAGE_FORMAT_RGB:
           case IMAGE_FORMAT_BGR:
             r = *(pdata + (i * nc + j) * 3 + 0);
             g = *(pdata + (i * nc + j) * 3 + 1);
             b = *(pdata + (i * nc + j) * 3 + 2);
             cendata = (T)(r * 0.3 + g * 0.59 + b * 0.11);

             r = *(pdata + ((i + 1 )* nc + j) * 3 + 0);
             g = *(pdata + ((i + 1 )* nc + j) * 3 + 1);
             b = *(pdata + ((i + 1 )* nc + j) * 3 + 2);
             rightcendata = (T)(r * 0.3 + g * 0.59 + b * 0.11);

             r = *(pdata + (i * nc + j - 1) * 3 + 0);
             g = *(pdata + (i * nc + j - 1) * 3 + 1);
             b = *(pdata + (i * nc + j - 1) * 3 + 2);
             cenupdata = (T)(r * 0.3 + g * 0.59 + b * 0.11);

             break;
           default:
             break;
         }
         smd = abs(cendata - cenupdata) + abs(cendata - rightcendata);
         smd_sum += smd;
       }//end of j
     }//end of i

     return smd_sum;
    }

    /// 利用灰度方差算子Gradation Variance进行序列图像自动聚焦。
    /**
     * 计算图像的灰度变化的均方差之和。函数返回图像聚焦算子值。
     *
     * @param image 单幅图像结构指针，必须是内存中的有效图像。
     * @return 图像聚焦算子值。
     */
    template <class T>
    long int VarianceAutoFocusOperator(ImageDef<T> *image)
    {
     int i, j, var = 0;
     long int var_sum;
     T r, g, b, *pdata, cendata = 0;
     int nr = image->Height;
     int nc = image->Width;

     if (image->Pixels == 0 ) throw NullPointerException();
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
     switch (image->Format)
     {
       case IMAGE_FORMAT_INDEX:
       case IMAGE_FORMAT_RGB:
       case IMAGE_FORMAT_BGR:
         pdata = image->Pixels;
         break;
       default:
         break;
     }

     var_sum = 0;
     for(i = 0; i < nr; i++)
     {
       for(j = 0; j < nc; j++)
       {
         switch (image->Format)
         {
           case IMAGE_FORMAT_INDEX:
             cendata = *(pdata + i * nc + j);
             break;
           case IMAGE_FORMAT_RGB:
           case IMAGE_FORMAT_BGR:
             r = *(pdata + (i * nc + j) * 3 + 0);
             g = *(pdata + (i * nc + j) * 3 + 1);
             b = *(pdata + (i * nc + j) * 3 + 2);
             cendata = T(r * 0.3 + g * 0.59 + b * 0.11);
             break;
           default:
             break;
         }
         var += cendata;
       }//end of j
     }//end of i
     var /= nr * nc;

     for (i = 0; i < nr; i++)
     {
       for (j = 0; j < nc; j++)
       {
         switch (image->Format)
         {
           case IMAGE_FORMAT_INDEX:
             cendata = *(pdata + i * nc + j);
             break;
           case IMAGE_FORMAT_RGB:
           case IMAGE_FORMAT_BGR:
             r = *(pdata + (i * nc + j) * 3 + 0);
             g = *(pdata + (i * nc + j) * 3 + 1);
             b = *(pdata + (i * nc + j) * 3 + 2);
             cendata = T(r * 0.3 + g * 0.59 + b * 0.11);
             break;
           default:
             break;
         }
         var_sum += (cendata - var) * (cendata - var);
       }
     }
     var_sum /= nr * nc;

     return var_sum;
    }

    /// 利用Robert算子进行序列图像自动聚焦。
    /**
     * 对图像中每一点及其邻近点的灰度值做差分运算并汇总，计算Robert梯度算子。
     * 函数返回图像聚焦算子值。
     *
     * @param image 单幅图像结构指针，必须是内存中的有效图像。
     * @return 图像聚焦算子值。
     */
    template <class T>
    long int RobertAutoFocusOperator(ImageDef<T> *image)
    {
      int i, j, robert = 0;
      long int robert_sum;
      T r, g, b, *pdata, cendata = 0, rightcendata = 0, cendowndata = 0, rightdowndata = 0;
      int nr = image->Height;
      int nc = image->Width;

      if (image->Pixels == 0 ) throw NullPointerException();
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
      switch (image->Format)
      {
        case IMAGE_FORMAT_INDEX:
        case IMAGE_FORMAT_RGB:
        case IMAGE_FORMAT_BGR:
          pdata = image->Pixels;
          break;
        default:
          break;
      }

      robert_sum = 0;
      for(i = 1; i < nr - 1; i++)
      {
        for(j = 1; j < nc - 1; j++)
        {
          switch (image->Format)
          {
            case IMAGE_FORMAT_INDEX:
              cendata = *(pdata + i * nc + j);
              rightcendata  = *(pdata + (i + 1 )* nc + j);
              rightdowndata = *(pdata + (i + 1 )* nc + j + 1);
              cendowndata = *(pdata + i * nc + j + 1);

              break;
            case IMAGE_FORMAT_RGB:
            case IMAGE_FORMAT_BGR:
              r = *(pdata + (i * nc + j) * 3 + 0);
              g = *(pdata + (i * nc + j) * 3 + 1);
              b = *(pdata + (i * nc + j) * 3 + 2);
              cendata = (T)(r * 0.3 + g * 0.59 + b * 0.11);

              r = *(pdata + ((i + 1 )* nc + j) * 3 + 0);
              g = *(pdata + ((i + 1 )* nc + j) * 3 + 1);
              b = *(pdata + ((i + 1 )* nc + j) * 3 + 2);
              rightcendata = (T)(r * 0.3 + g * 0.59 + b * 0.11);

              r = *(pdata + ((i + 1 )* nc + j + 1) * 3 + 0);
              g = *(pdata + ((i + 1 )* nc + j + 1) * 3 + 1);
              b = *(pdata + ((i + 1 )* nc + j + 1) * 3 + 2);
              rightdowndata = (T)(r * 0.3 + g * 0.59 + b * 0.11);

              r = *(pdata + (i * nc + j + 1) * 3 + 0);
              g = *(pdata + (i * nc + j + 1) * 3 + 1);
              b = *(pdata + (i * nc + j + 1) * 3 + 2);
              cendowndata = (T)(r * 0.3 + g * 0.59 + b * 0.11);

              break;
            default:
              break;
          }
          robert = abs(cendata - rightdowndata) + abs(rightcendata - cendowndata);
          robert_sum += robert;
        }//end of j
      }//end of i

      return robert_sum;
    }

    /**
     * @brief 另一种Robert聚焦算子。
     *
     * 实际测试效果比较好。
     *
     * @param image 单幅图像结构指针，必须是内存中的有效图像。
     * @return 图像聚焦算子值。
     *
     * @author 陈进 赵宇
     */
    template <class T>
    double Robert2AutoFocusOperator(ImageDef<T> *image)
    {
      //归一化标准灰度。得到的聚焦因子值按此进行归一化，以避免图像亮度引起结果的差异。
      const double refGray = ImageDefTraits<T>::MidValueRoundUp;  //对于字节类型的数据，取128。

      double res = 0;
      int cnt = 0;
      int avgGray = 0;//灰度

      int hb = GetUnitsPerPixel(image);
      int vb = GetUnitsPerRow(image);
      int row = image->Width - hb;

      T *p = image->Pixels, *p_end = image->Pixels + GetUnitsOfPixelData(image) - vb - hb;
      T *p_right = p + hb;
      T *p_bellow = p + vb;
      T *p_right_bellow = p + hb + vb;

      int g = 0, gr = 0, gb = 0, grb = 0;
      double v;

      while (p < p_end)
      {
        for (int x = 0; x < row; x++)
        {
          switch (image->Format)
          {
          case IMAGE_FORMAT_INDEX:
            g = *p++;
            gr = *p_right++;
            gb = *p_bellow++;
            grb = *p_right_bellow++;
            break;
          case IMAGE_FORMAT_RGB:
          case IMAGE_FORMAT_BGR:
            g = *p++;
            gr = *p_right++;
            gb = *p_bellow++;
            grb = *p_right_bellow++;

            g += *p++;
            gr += *p_right++;
            gb += *p_bellow++;
            grb += *p_right_bellow++;

            g += *p++;
            gr += *p_right++;
            gb += *p_bellow++;
            grb += *p_right_bellow++;
            break;
          default:
            break;
          }

          v = abs(g - grb) + abs(gb - gr);
          v /= 6;
          v *= v;
          res += v;
          cnt++;
          avgGray += g; //最后再计算平均灰度。
        }
        //跳过最后一个像素。
        p += hb;
        p_right += hb;
        p_bellow += hb;
        p_right_bellow += hb;
      }

      if (cnt)
      {
        double avg_gray = avgGray / (double)hb;
        avg_gray /= cnt;
        res /= cnt;
        res *= (refGray / avg_gray);
      }
      else
      {
        res = 0;
      }
      return res;
    }
  }// Image2D namespace
}// MBL namespace

#endif // __AUTOFOCUSOPERATOR_H__
