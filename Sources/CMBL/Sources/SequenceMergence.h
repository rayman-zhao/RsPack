#ifndef __SEQUENCEMERGENCE_H__
#define __SEQUENCEMERGENCE_H__

/**
 * @file
 *
 * @brief 最大协方差方法、拉普拉斯算子进行序列图像融合的函数，生成DEM高度索引图像及聚焦图像。
 */

namespace MBL
{
  namespace Image2D
  {

    /// 序列图像深度聚焦的一种方法(简单的点处理的方法)——最小灰度法。
    /**
     * 该函数找到序列中各点的灰度最小值，生成DEM高度索引图并返回。
     *
     * @param image 序列图像结构指针，必须是有效的内存中的序列图像。
     * @return 高度索引图的指针，使用完毕后必须将其释放。
     */
    template <class T>
    ImageDef<T> * MinMergeSequenceIntoDEM(ImageSequenceDef<T> *image)
    {
      int i, j, k, kmark;
      int setof;
      T outdata = 0, *pdata, *pDEM, min;
      int nr = image->Height;
      int nc = image->Width;
      int num = image->SequenceNumber;

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

      //High recording image(DEM) allocation and initialization
      ImageDef<T> *DEM = ImageDef<T>::CreateInstance(IMAGE_FORMAT_INDEX, nc, nr);
      pDEM = DEM->Pixels;
      setof = nr * nc;
      memset(pDEM, 0, setof * sizeof(T));

      //Create the DEM image from source images using maximum convariance method
      for (i = 0; i < nr; i++)
      {
        for (j = 0; j < nc; j++)
        {
          min = 255;
          kmark = 0;

          for (k = 0; k < num; k++)
          {
            switch (image->Format)
            {
              case IMAGE_FORMAT_INDEX:
                pdata = image->Pixels[k];
                setof = i * nc + j;
                outdata = *(pdata + setof);
                break;
              case IMAGE_FORMAT_RGB:
              case IMAGE_FORMAT_BGR:
                pdata = image->Pixels[k];
                setof = (i  * nc + j) * 3;
                outdata = (T)(*(pdata + setof) * 0.3 + *(pdata + setof + 1) * 0.59 + *(pdata + setof + 2) * 0.11);
                break;
              default:
                break;
            }

            if(outdata < min)
            {
              min = outdata;
              kmark = k;
            }
          }

          if (kmark > 255)
            kmark = 255;
          else if (kmark < 0)
            kmark = 0;

          *(pDEM + i * nc + j) = (T)kmark;
        }
      }

      return DEM;
    }

    /// 序列图像深度聚焦的一种方法(简单的点处理的方法)——最大灰度法。
    /**
     * 该函数找到序列中各点的灰度最大值，生成DEM高度索引图并返回。
     *
     * @param image 序列图像结构指针，必须是有效的内存中的序列图像。
     * @return 高度索引图的指针，使用完毕后必须将其释放。
     */
    template <class T>
    ImageDef<T> * MaxMergeSequenceIntoDEM(ImageSequenceDef<T> *image)
    {
      int i, j, k, kmark;
      int setof;
      T outdata = 0, *pdata, *pDEM, max;
      int nr = image->Height;
      int nc = image->Width;
      int num = image->SequenceNumber;

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

      //High recording image(DEM) allocation and initialization
      ImageDef<T> *DEM = ImageDef<T>::CreateInstance(IMAGE_FORMAT_INDEX, nc, nr);
      pDEM = DEM->Pixels;
      setof = nr * nc;
      memset(pDEM, 0, setof * sizeof(T));

      //Create the DEM image from source images using maximum convariance method
      for (i = 0; i < nr; i++)
      {
        for (j = 0; j < nc; j++)
        {
          max = 0;
          kmark = 0;

          for (k = 0; k < num; k++)
          {
            switch (image->Format)
            {
              case IMAGE_FORMAT_INDEX:
                pdata = image->Pixels[k];
                setof = i * nc + j;
                outdata = *(pdata + setof);
                break;
              case IMAGE_FORMAT_RGB:
              case IMAGE_FORMAT_BGR:
                pdata = image->Pixels[k];
                setof = (i  * nc + j) * 3;
                outdata = (T)(*(pdata + setof) * 0.3 + *(pdata + setof + 1) * 0.59 + *(pdata + setof + 2) * 0.11);
                break;
              default:
                break;
            }

            if(outdata > max)
            {
                max = outdata;
                kmark = k;
            }

         }
         if (kmark > 255)
           kmark = 255;
         else if (kmark < 0)
           kmark = 0;

         *(pDEM + i * nc + j) = (T)kmark;
        }
      }

      return DEM;
    }


    /// 序列图像深度聚焦的一种方法——最大离差法。
    /**
     * 该函数首先计算序列中像素灰度的平均值，然后选择与该平均值相差最大的像素作为融合图像的像素值，生成DEM高度索引图并返回。
     *
     * @param image 序列图像结构指针，必须是有效的内存中的序列图像。
     * @return 高度索引图的指针，使用完毕后必须将其释放。
     */
    template <class T>
    ImageDef<T> * ContrastMergeSequenceIntoDEM(ImageSequenceDef<T> *image)
    {
      int i, j, k, kmark,kmark1,kmark2;
      double average, diff_max, diff;
      int setof;
      T outdata = 0, *pdata, *pDEM, min, max, sum;
      int nr = image->Height;
      int nc = image->Width;
      int num = image->SequenceNumber;

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

      //High recording image(DEM) allocation and initialization
      ImageDef<T> *DEM = ImageDef<T>::CreateInstance(IMAGE_FORMAT_INDEX, nc, nr);
      pDEM = DEM->Pixels;
      setof = nr * nc;
      memset(pDEM, 0, setof * sizeof(T));

      //Create the DEM image from source images using maximum convariance method
      for (i = 0; i < nr; i++)
      {
        for (j = 0; j < nc; j++)
        {
          max = 0;
          min = 255;
          sum = 0;
          kmark = 0;
          average = 0.0;
          kmark1 = 0;
          kmark2 = 0;

          for (k = 0; k < num; k++)
          {
            switch (image->Format)
            {
              case IMAGE_FORMAT_INDEX:
                pdata = image->Pixels[k];
                setof = i * nc + j;
                outdata = *(pdata + setof);
                break;
              case IMAGE_FORMAT_RGB:
              case IMAGE_FORMAT_BGR:
                pdata = image->Pixels[k];
                setof = (i  * nc + j) * 3;
                outdata = (T)(*(pdata + setof) * 0.3 + *(pdata + setof + 1) * 0.59 + *(pdata + setof + 2) * 0.11);
                break;
              default:
                break;
             }
             sum +=outdata;
             average += outdata;

             if(outdata < min)
             {
               min = outdata;
               kmark1 = k;
             }

             if(outdata > max)
             {
                max = outdata;
                kmark2 = k;
             }
          }
          average /= num;

          diff = fabs(max - average) - fabs(min - average);

          if (diff > 0)
          {
            diff_max = max;
            kmark = kmark2;
          }
          else
          {
            diff_max = min;
            kmark = kmark1;
          }

          if (kmark > 255)
            kmark = 255;
          else if (kmark < 0)
            kmark = 0;

          *(pDEM + i * nc + j) = (T)kmark;
        }
      }

      return DEM;
    }


    /// 序列图像深度聚焦的一种方法——最大协方差法。
    /**
     * 该函数使用大小为5×5的小窗口，找到序列中各点的协方差的最大值，生成DEM高度索引图并返回。
     *
     * @param image 序列图像结构指针，必须是有效的内存中的序列图像。
     * @return 高度索引图的指针，使用完毕后必须将其释放。
     */
    template <class T>
    ImageDef<T> * CovarianceMergeSequenceIntoDEM(ImageSequenceDef<T> *image)
    {
      int i, j, k, m, n, kmark;
      double average, cova_max, cova, cova_sum;
      int setof;
      T outdata = 0, *pdata, *pDEM;
      int nr = image->Height;
      int nc = image->Width;
      int num = image->SequenceNumber;

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

      //High recording image(DEM) allocation and initialization
      ImageDef<T> *DEM = ImageDef<T>::CreateInstance(IMAGE_FORMAT_INDEX, nc, nr);
      pDEM = DEM->Pixels;
      setof = nr * nc;
      memset(pDEM, 0, setof * sizeof(T));

      //Create the DEM image from source images using maximum convariance method
      for (i = 2; i < nr - 2; i++)
      {
        for (j = 2; j < nc - 2; j++)
        {
          cova_max = 0.0;
          kmark = 0;

          for (k = 0; k < num; k++)
          {
            average = 0.0;
            for (m = -2; m < 3; m++)
            {
              for(n = -2; n < 3; n++)
              {
                switch (image->Format)
                {
                  case IMAGE_FORMAT_INDEX:
                    pdata = image->Pixels[k];
                    setof = (i + m) * nc + (j + n);
                    outdata = *(pdata + setof);
                    break;
                  case IMAGE_FORMAT_RGB:
                  case IMAGE_FORMAT_BGR:
                    pdata = image->Pixels[k];
                    setof = ((i + m) * nc + (j + n) ) * 3;
                    outdata = (T)(*(pdata + setof) * 0.3 + *(pdata + setof + 1) * 0.59 + *(pdata + setof + 2) * 0.11);
                    break;
                  default:
                    break;
                }
                average += outdata;
              }
            }
            average /= 25.0;

            cova_sum = 0.0;
            for (m = -2; m < 3; m++)
            {
              for (n = -2; n < 3; n++)
              {
                switch (image->Format)
                {
                  case IMAGE_FORMAT_INDEX:
                    pdata = image->Pixels[k];
                    setof = (i + m) * nc + (j + n);
                    outdata = *(pdata + setof);
                    break;
                  case IMAGE_FORMAT_RGB:
                  case IMAGE_FORMAT_BGR:
                    pdata = image->Pixels[k];
                    setof = ((i + m) * nc + (j + n) ) * 3;
                    outdata = (T)(*(pdata + setof) * 0.3 + *(pdata + setof + 1) * 0.59 + *(pdata + setof + 2) * 0.11);
                    break;
                  default:
                    break;
                }
                cova = (average - outdata) * (average - outdata);
                cova_sum += cova;
              }
            }
            cova = cova_sum / (average * average);
            if (cova>cova_max)
            {
              cova_max = cova;
              kmark = k;
            }
          }
          if (kmark > 255)
            kmark = 255;
          else if (kmark < 0)
            kmark = 0;

          *(pDEM + i * nc + j) = (T)kmark;
        }
      }

      return DEM;
    }

    ///计算高度索引图中的最大灰度值。
    /**
     * @param step 电机走每一步的Z轴实际距离。
     * @param totalslice 采集图像序列所包含的图像幅数。
     * @param proport 尺度变量。
     * return 高度索引图中的最大灰度值。
     */
    inline int Maxgradation (float step, int totalslice, float proport)
    {
      float totalheight;
      int heightscale;
      totalheight = step *totalslice;
      heightscale = int(totalheight / proport);
      return heightscale;
    }
    /// 序列图像深度聚焦的一种方法——改进拉普拉斯算法。
    /**
     * 使用改进拉普拉斯算子，找到序列中各点聚焦算子最大值所在的层。函数生成DEM高度索引图结构并返回。
     *
     * @param image 序列图像结构指针，必须是有效的内存中的序列图像。
     * @param DEM 高度索引图像结构指针，若为有效的内存中的图像并在外部已分配好图像数据内存，其格式必须是灰度索引图，尺寸
     *            必须与序列图像一致。若不是有效的内存中的图像，则分配相应的内存，并按上述格式与大小填写相应数据成员的内容。
     *            函数结束时将被填充为实际高度索引图数据。
     * @param GAUSS 高度索引图像结构指针，若为有效的内存中的图像并在外部已分配好图像数据内存，其格式必须是灰度索引图，尺寸
     *              必须与序列图像一致。若不是有效的内存中的图像，则分配相应的内存，并按上述格式与大小填写相应数据成员的内容。
     *              函数结束时将被填充为GAUSS拟合后再经线性拉伸形成的高度索引图数据。若为0，则不计算GAUSS拟合结果。
     * @param window_size 窗口的大小。计算拉普拉斯值的总和时使用的窗口。如window_size = 1时，窗口大小为
     *                    2 × window_size + 1 = 3， 即3×3窗口。一般选择经验值5。
     * @param step 步长。计算某点的拉普拉斯算子时使用的步长大小，若取为1，即取该点的四邻域。一般选择经验值5。
     * @param threshold 阈值。拉普拉斯算子大于阈值的才参与汇总，小于阈值的被忽略。一般选择经验值0。
     * @param heightscale 高度索引图中的最大灰度值。
     */
    template <class T>
    void  LaplacianMergeSequenceIntoDEM(ImageSequenceDef<T> *image, ImageDef<T> *DEM, ImageDef<T> *GAUSS, int window_size, int step, int threshold, int heightscale = 255)
    {
      int i, j, k, m, n, ii, jj, kmark, sum;
      int lap_max, lap_sum;
      T cendata = 0, leftdata = 0, rightdata = 0, updata = 0, downdata = 0, *pdata, *pDEM, *pGuss;
      int nr = image->Height;
      int nc = image->Width;
      int num = image->SequenceNumber;
      int margin = window_size + step;
      int window_length = window_size * 2 + 1;
      int array_length = window_length * window_length;
      double mp, d1, d2, d3;

      if(heightscale < 1 || heightscale > 255) throw IllegalArgumentException();

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

      //High recording image allocation and initialization
      if ((DEM->Height != nr) || (DEM->Width != nc) || (DEM->Format != IMAGE_FORMAT_INDEX))
      {
        FreeImageData(DEM);
        DEM->Format = IMAGE_FORMAT_INDEX;
        DEM->Width = nc;
        DEM->Height = nr;
        DEM->Pixels = new T[nc * nr];
      }
      pDEM = DEM->Pixels;

      if (GAUSS != 0)
      {
        if ((GAUSS->Height != nr) || (GAUSS->Width != nc) || (GAUSS->Format != IMAGE_FORMAT_INDEX))
        {
          FreeImageData(GAUSS);
          GAUSS->Format = IMAGE_FORMAT_INDEX;
          GAUSS->Width = nc;
          GAUSS->Height = nr;
          GAUSS->Pixels = new T[nc * nr];
        }
        pGuss = GAUSS->Pixels;
      }
      else
      {
        pGuss = 0;
      }

      int **Warray = new int *[num];
      for (i = 0; i < num; i++)
      {
        Warray[i] = new int[array_length];
      }

      //Create the lapcacion image from source images
      for (i = margin; i < nr - margin; i++)
      {
        for (k = 0; k < num; k++)
        {
          for (m = -window_size; m <= window_size; m++)
          {
            for (n = -window_size; n <= window_size; n++)
            {
              pdata = image->Pixels[k];
              switch (image->Format)
              {
                case IMAGE_FORMAT_INDEX:
                  cendata = *(pdata + (i + m) * nc + (margin + n));
                  leftdata = *(pdata + (i + m) * nc + (margin + n) - step);
                  rightdata = *(pdata + (i + m) * nc + (margin + n) + step);
                  updata = *(pdata + ((i + m) - step) * nc + (margin + n));
                  downdata = *(pdata + ((i + m) + step) * nc + (margin + n));

                  break;

                case IMAGE_FORMAT_RGB:
                case IMAGE_FORMAT_BGR:
                  cendata = *(pdata + ((i + m) * nc + (margin + n)) * 3 + 1);
                  leftdata = *(pdata + ((i + m) * nc + (margin + n - step)) * 3 + 1);
                  rightdata = *(pdata + ((i + m) * nc + (margin + n + step)) * 3 + 1);
                  updata = *(pdata + ((i + m - step) * nc + (margin + n)) * 3 + 1);
                  downdata = *(pdata + ((i + m + step) * nc + (margin + n)) * 3 + 1);

                  break;

                default:
                  break;
              }
              sum = abs(2 * cendata - leftdata - rightdata) + abs(2 * cendata - updata - downdata);
              Warray[k][(n + window_size) * window_length + (m + window_size)] = sum;
            }
          }
        }

        for (j = margin; j < nc - margin; j++)
        {
          lap_max = 0;
          kmark = 0;
          int *f = new int [num];

          for (k = 0; k < num; k++)
          {
            lap_sum = 0;

            if (j != margin)
            {
              for (jj = 0; jj < array_length - window_length; jj++)
              {
                Warray[k][jj] = Warray[k][jj + window_length];
              }

              for (m = -window_size; m <= window_size; m++)
              {
                pdata = image->Pixels[k];
                switch (image->Format)
                {
                  case IMAGE_FORMAT_INDEX:
                    cendata = *(pdata + (i + m) * nc + (j + window_size));
                    leftdata = *(pdata + (i + m) * nc + (j + window_size) - step);
                    rightdata = *(pdata + (i + m) * nc + (j + window_size) + step);
                    updata = *(pdata + ((i + m) - step) * nc + (j + window_size));
                    downdata = *(pdata + ((i + m) + step) * nc + (j + window_size));

                    break;

                  case IMAGE_FORMAT_RGB:
                  case IMAGE_FORMAT_BGR:
                    cendata = *(pdata + ((i + m) * nc + (j + window_size)) * 3 + 1);

                    jj = j + window_size - step;
                    leftdata = *(pdata + ((i + m) * nc + jj) * 3 + 1);

                    jj = j + window_size + step;
                    rightdata = *(pdata + ((i + m) * nc + jj) * 3 + 1);

                    jj = i + m - step;
                    updata = *(pdata + (jj * nc + (j + window_size)) * 3 + 1);

                    jj = i + m + step;
                    downdata = *(pdata + (jj * nc + (j + window_size)) * 3 + 1);

                    break;

                  default:
                    break;
                }
                sum = abs(2 * cendata - leftdata - rightdata) + abs(2 * cendata - updata - downdata);
                Warray[k][m + window_size + array_length - window_length] = sum;
              }
            }

            for (ii = 0; ii < array_length; ii++)
            {
              if (Warray[k][ii] > threshold)
              {
                lap_sum += Warray[k][ii];
              }
            }

            f[k] = lap_sum;

            if (lap_sum > lap_max)
            {
              lap_max = lap_sum;
              kmark = k;
            }
          }//end of k

          if (kmark > 255)
            kmark = 255;
          else if (kmark < 0)
            kmark = 0;

          *(pDEM + i * nc + j) = (T)kmark;

          if (pGuss != 0)
          {
            if (kmark != 0 && kmark != num - 1)
            {
              d1 = (log((double)f[kmark]) - log((double)f[kmark + 1])) * (2 * kmark - 1);
              d2 = (log((double)f[kmark]) - log((double)f[kmark - 1])) * (- 2 * kmark - 1);
              d3 = 2 * (2 * log((double)f[kmark]) - log((double)f[kmark + 1]) -log((double)f[kmark - 1]));
              mp = d1/d3 - d2/d3;
            }
            else mp = kmark;
            delete f;
            *(pGuss + i * nc + j) = (T)(heightscale - (mp * heightscale / (num - 1) + 0.5));
          }
        }
      }

      //边缘处理
      for (i = 0; i < margin; i++)
      {
        for (j = 0; j < nc; j++)
        {
          if (j < margin)
          {
            *(pDEM + i * nc + j) = *(pDEM + margin * nc + margin);
          }
          else
          {
            if (j >= (nc - margin))
              *(pDEM + i * nc + j) = *(pDEM +  margin * nc + (nc - margin - 1));
            else
              *(pDEM + i * nc + j) = *(pDEM +  margin * nc + j);
          }
        }
      }

      for (i = nr - margin; i < nr; i++)
      {
        for (j = 0; j < nc; j++)
        {
          if (j < margin)
          {
            *(pDEM + i * nc + j) = *(pDEM + (nr - margin - 1) * nc + margin);
          }
          else
          {
            if (j >= (nc - margin))
              *(pDEM + i * nc + j) = *(pDEM +  (nr - margin - 1) * nc + (nc - margin - 1));
            else
              *(pDEM + i * nc + j) = *(pDEM +  (nr - margin - 1) * nc + j);
          }
        }
      }

      for (i = margin; i < nr - margin; i++)
      {
        for (j = 0; j < margin; j++)
        {
          *(pDEM + i * nc + j) = *(pDEM +  i * nc + margin);
        }

        for (j = nc - margin; j < nc; j++)
        {
          *(pDEM + i * nc + j) = *(pDEM +  i * nc + (nc - margin - 1));
        }
      }
      //边缘处理
      if (pGuss != 0)
      {
        for (i = 0; i < margin; i++)
        {
          for (j = 0; j < nc; j++)
          {
            if (j < margin)
            {
              *(pGuss + i * nc + j) = *(pGuss + margin * nc + margin);
            }
            else
            {
              if (j >= (nc - margin))
                *(pGuss + i * nc + j) = *(pGuss +  margin * nc + (nc - margin - 1));
              else
                *(pGuss + i * nc + j) = *(pGuss +  margin * nc + j);
            }
          }
        }

        for (i = nr - margin; i < nr; i++)
        {
          for (j = 0; j < nc; j++)
          {
            if (j < margin)
            {
              *(pGuss + i * nc + j) = *(pGuss + (nr - margin - 1) * nc + margin);
            }
            else
            {
              if (j >= (nc - margin))
                *(pGuss + i * nc + j) = *(pGuss +  (nr - margin - 1) * nc + (nc - margin - 1));
              else
                *(pGuss + i * nc + j) = *(pGuss +  (nr - margin - 1) * nc + j);
            }
          }
        }

        for (i = margin; i < nr - margin; i++)
        {
          for (j = 0; j < margin; j++)
          {
            *(pGuss + i * nc + j) = *(pGuss +  i * nc + margin);
          }
          for (j = nc - margin; j < nc; j++)
          {
            *(pGuss + i * nc + j) = *(pGuss +  i * nc + (nc - margin - 1));
          }
        }
      }

      for (i = 0; i < num; i++)
      {
        delete [] Warray[i];
      }
      delete [] Warray;
    }

    /// 序列图像深度聚焦的一种方法——tenengrad算法。
    /**
     * 使用Sobel算子，找到序列中各点聚焦算子最大值所在的层。函数生成DEM高度索引图结构并返回。
     *
     * @param image 序列图像结构指针，必须是有效的内存中的序列图像。
     * @param window_size 窗口的大小。计算拉普拉斯值的总和时使用的窗口。如window_size = 1时，窗口大小为
     *                    2 × window_size + 1 = 3， 即3×3窗口。一般选择经验值5。
     * @param threshold 阈值。拉普拉斯算子大于阈值的才参与汇总，小于阈值的被忽略。一般选择经验值0。
     * @return 高度索引图的指针，使用完毕后必须将其释放。
     */
    template <class T>
    ImageDef<T> * TenengradMergeSequenceIntoDEM(ImageSequenceDef<T> *image, int window_size, int threshold)
    {
      int i, j, k, m, n, kmark;
      int ten_max, ten_sum, soblex, sobley, grad;
      int setof;
      T r, g, b, *pdata, *pDEM, leftupdata = 0, leftcendata = 0, leftdowndata = 0, rightupdata = 0;
      T rightcendata = 0, rightdowndata = 0, cenupdata = 0, cendowndata = 0;
      int nr = image->Height;
      int nc = image->Width;
      int num = image->SequenceNumber;
      int margin = window_size + 1;

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

      //High recording image allocation and initialization
      ImageDef<T> *DEM = ImageDef<T>::CreateInstance(IMAGE_FORMAT_INDEX, nc, nr);
      pDEM = DEM->Pixels;
      setof = nr * nc;
      memset(pDEM, 0, setof * sizeof(T));

      //Create the Tenengrad image from source images
      for(i=margin;i<nr-margin;i++)
      {
        for(j=margin;j<nc-margin;j++)
        {
          ten_max = 0;
          kmark = 0;
          for(k=0;k<num;k++)
          {
            ten_sum = 0;
            pdata = image->Pixels[k];
            for(m=-window_size;m<=window_size;m++)
            {
              for(n=-window_size;n<=window_size;n++)
              {
                switch (image->Format)
                {
                  case IMAGE_FORMAT_INDEX:
                    leftupdata    = *(pdata + (i - 1 + m ) * nc + j - 1 + n);
                    leftcendata   = *(pdata + (i - 1 + m ) * nc + j + n);
                    leftdowndata  = *(pdata + (i - 1 + m ) * nc + j + 1 + n);
                    rightupdata   = *(pdata + (i + 1 + m ) * nc + j - 1 + n);
                    rightcendata  = *(pdata + (i + 1 + m ) * nc + j + n);
                    rightdowndata = *(pdata + (i + 1 + m ) * nc + j + 1 + n);
                    cenupdata     = *(pdata + (i + m) * nc + j - 1 + n);
                    cendowndata   = *(pdata + (i + m) * nc + j + 1 + n);
                    break;
                  case IMAGE_FORMAT_RGB:
                  case IMAGE_FORMAT_BGR:
                    r = *(pdata + ((i - 1 + m) * nc + j - 1 + n) * 3 + 0);
                    g = *(pdata + ((i - 1 + m) * nc + j - 1 + n) * 3 + 1);
                    b = *(pdata + ((i - 1 + m) * nc + j - 1 + n) * 3 + 2);
                    leftupdata = (T)(r * 0.3 + g * 0.59 + b * 0.11);

                    r = *(pdata + ((i - 1 + m )* nc + j + n) * 3 + 0);
                    g = *(pdata + ((i - 1 + m )* nc + j + n) * 3 + 1);
                    b = *(pdata + ((i - 1 + m )* nc + j + n) * 3 + 2);
                    leftcendata = (T)(r * 0.3 + g * 0.59 + b * 0.11);

                    r = *(pdata + ((i - 1 + m) * nc + j + 1 + n) * 3 + 0);
                    g = *(pdata + ((i - 1 + m) * nc + j + 1 + n) * 3 + 1);
                    b = *(pdata + ((i - 1 + m) * nc + j + 1 + n) * 3 + 2);
                    leftdowndata = (T)(r * 0.3 + g * 0.59 + b * 0.11);

                    r = *(pdata + ((i + 1 + m )* nc + j - 1 + n) * 3 + 0);
                    g = *(pdata + ((i + 1 + m )* nc + j - 1 + n) * 3 + 1);
                    b = *(pdata + ((i + 1 + m )* nc + j - 1 + n) * 3 + 2);
                    rightupdata = (T)(r * 0.3 + g * 0.59 + b * 0.11);

                    r = *(pdata + ((i + 1 + m )* nc + j + n) * 3 + 0);
                    g = *(pdata + ((i + 1 + m )* nc + j + n) * 3 + 1);
                    b = *(pdata + ((i + 1 + m )* nc + j + n) * 3 + 2);
                    rightcendata = (T)(r * 0.3 + g * 0.59 + b * 0.11);

                    r = *(pdata + ((i + 1 + m) * nc + j + 1 + n) * 3 + 0);
                    g = *(pdata + ((i + 1 + m) * nc + j + 1 + n) * 3 + 1);
                    b = *(pdata + ((i + 1 + m) * nc + j + 1 + n) * 3 + 2);
                    rightdowndata = (T)(r * 0.3 + g * 0.59 + b * 0.11);

                    r = *(pdata + ((i + m) * nc + j - 1 + n) * 3 + 0);
                    g = *(pdata + ((i + m) * nc + j - 1 + n) * 3 + 1);
                    b = *(pdata + ((i + m) * nc + j - 1 + n) * 3 + 2);
                    cenupdata = (T)(r * 0.3 + g * 0.59 + b * 0.11);

                    r = *(pdata + ((i + m) * nc + j + 1 + n) * 3 + 0);
                    g = *(pdata + ((i + m) * nc + j + 1 + n) * 3 + 1);
                    b = *(pdata + ((i + m) * nc + j + 1 + n) * 3 + 2);
                    cendowndata = (T)(r * 0.3 + g * 0.59 + b * 0.11);

                    break;
                  default:
                    break;
                }
                soblex = rightupdata + 2 * rightcendata + rightdowndata - leftupdata - 2 * leftcendata - leftdowndata;
                sobley = leftupdata + 2 * cenupdata + rightupdata - leftdowndata - 2 * cendowndata - rightdowndata;
                grad = soblex * soblex + sobley * sobley;
                ten_sum += grad;
              }
            }

            if(ten_sum > ten_max)
            {
              ten_max = ten_sum;
              kmark = k;
            }
          }
          if (kmark > 255)
            kmark = 255;
          else if (kmark < 0)
            kmark = 0;
          *(pDEM+i*nc+j) =  (T)kmark;
        }
      }
      //边缘处理
      for (i = 0; i < margin; i++)
      {
        for (j = 0; j < nc; j++)
        {
          if (j < margin)
            *(pDEM + i * nc + j) = *(pDEM + margin * nc + margin);
          else
            if (j >= (nc-margin))
              *(pDEM + i * nc + j) = *(pDEM +  margin * nc + (nc - margin - 1));
          else
              *(pDEM + i * nc + j) = *(pDEM +  margin * nc + j);

        }
      }

      for (i = nr - margin; i < nr; i++)
      {
        for (j = 0; j < nc; j++)
        {
          if (j < margin)
            *(pDEM + i * nc + j) = *(pDEM + (nr - margin - 1) * nc + margin);
          else
            if (j >= (nc - margin))
              *(pDEM + i * nc + j) = *(pDEM +  (nr - margin - 1) * nc + (nc - margin - 1));
          else
              *(pDEM + i * nc + j) = *(pDEM +  (nr - margin - 1) * nc + j);
        }
      }

      for (i = margin; i < nr - margin; i++)
      {
        for (j = 0;j < margin; j++)
        {
          *(pDEM + i * nc + j) = *(pDEM +  i * nc + margin);
        }

        for (j = nc - margin ;j < nc; j++)
        {
          *(pDEM + i * nc + j) = *(pDEM +  i * nc + (nc - margin - 1));
        }
      }

      return DEM;
    }

    /// 根据最大协方差及拉普拉斯方法得到的DEM图，以及原有的序列图像生成聚焦图像。
    /**
     * 用户指定序列图像和该序列的高度索引图像，该函数找到高度索引所对应的各点数据，生成一幅聚焦图像并返回。
     *
     * @param image 序列图像结构指针，必须是有效的内存中的序列图像。
     * @param DEM DEM图像结构指针，必须是对应于序列图像的灰度图像。
     * @return 生成的聚焦图像指针，使用完毕后必须将其释放。
     */
    template <class T>
    ImageDef<T> * MontageSequenceDEM(ImageSequenceDef<T> *image, ImageDef<T> *DEM)
    {
      int i, j, kmark;
      int setof;
      T *pdata, *pout, *pDEM;
      int nr = image->Height;
      int nc = image->Width;
      int num = image->SequenceNumber;

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

      //montage image allocation and initialization
      ImageDef<T>* imgout = ImageDef<T>::CreateInstance(image->Format, nc, nr, image->UsedColor);
      pout = imgout->Pixels;

      pDEM = DEM->Pixels;

      //Create the montage image from source images and DEM
      for (i = 0; i < nr; i++)
      {
        for (j = 0; j < nc; j++)
        {
          kmark = *(pDEM + i * nc +j);

          switch (image->Format)
          {
            case IMAGE_FORMAT_INDEX:
              pdata = image->Pixels[kmark];
              setof = i * nc +j;
              *(pout + setof) = *(pdata + setof);
              break;
            case IMAGE_FORMAT_RGB:
            case IMAGE_FORMAT_BGR:
              pdata = image->Pixels[kmark];
              setof = (i * nc +j) * 3;
              *(pout + setof + 0) = *(pdata + setof + 0);
              *(pout + setof + 1) = *(pdata + setof + 1);
              *(pout + setof + 2) = *(pdata + setof + 2);
              break;
            default:
              break;
          }
        }
      }

      return imgout;
    }

  }// Image2D namespace
}// MBL namespace

#endif // __SEQUENCEMERGENCE_H__
