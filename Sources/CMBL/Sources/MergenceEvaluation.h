#ifndef __MERGENCEEVALUATION_H__
#define __MERGENCEEVALUATION_H__

/**
 * @file
 *
 * @brief 序列图像融合效果评价函数。
 */

namespace MBL
{
  namespace Image2D
  {
    /// 均方根误差RMSE。
    /**
     * 该函数计算融合图像和标准参考图像之间的均方根误差，以评价图像融合效果。
     *
     * @param sourceimage 标准参考图像结构指针，必须是有效的内存中的图像。
     * @param fusionimage 融合图像结构指针，必须是有效的内存中的图像。
     * @return 两图像的均方根误差，均方根误差越小，说明融合效果越好。
     *
     * @author binger
     */
    template <class T>
    double RMSEEvaluation(ImageDef<T> *sourceimage, ImageDef<T> *fusionimage)
    {
      int i, j, setof;
      double difference = 0;
      int nr = sourceimage->Height;
      int nc = sourceimage->Width;
      T *psourcedata, *pfusiondata, outdatas,outdataf;
      psourcedata = sourceimage->Pixels;
      pfusiondata = fusionimage->Pixels;
      if ((fusionimage->Height != nr) || (fusionimage->Width != nc) || (fusionimage->Format != sourceimage->Format))
        throw IllegalArgumentException();


      for (i = 0; i < nr; i++)
      {
		  for(j = 0; j < nc; j++)
		  {
			  switch (sourceimage->Format)
			  {
			  case IMAGE_FORMAT_INDEX:

				  setof = i  * nc + j;
				  outdatas = *(psourcedata + setof);
				  outdataf = *(pfusiondata + setof);
				  difference +=  (outdatas - outdataf) * (outdatas - outdataf);
				  break;
			  case IMAGE_FORMAT_RGB:
			  case IMAGE_FORMAT_BGR:
				  setof = (i  * nc + j) * 3;
				  outdatas = (T)(*(psourcedata + setof) * 0.3 + *(psourcedata + setof + 1) * 0.59 + *(psourcedata + setof + 2) * 0.11);
				  outdataf = (T)(*(pfusiondata + setof) * 0.3 + *(pfusiondata + setof + 1) * 0.59 + *(pfusiondata + setof + 2) * 0.11);
				  difference +=  (outdatas - outdataf) * (outdatas - outdataf);
				  break;
			  default:
				  break;
			  }
		  }
      }
      difference /= nr * nc;
      difference = sqrt(difference);
      return difference;
    }

    /// 峰值信噪比PSNR。
    /**
     * 该函数计算融合图像和标准参考图像之间的峰值信噪比，以评价图像融合效果。
     *
     * @param rmse 两图像的均方根误差。
     * @return 两图像的峰值信噪比PSNR，峰值信噪比越大，说明融合效果越好。
     *
     * @author binger
     */
    inline double PSNREvaluation(double rmse)
    {
       double psnr = 0;
       psnr = 10 * log10(255 * 255 / (rmse * rmse));
       return psnr;
    }

    /// 图像的熵Entropy。
    /**
     * 该函数计算图像的熵，以评价图像融合效果。
     *
     * @param sourceimage 标准参考图像结构指针，必须是有效的内存中的图像。
     * @return 图像的熵entropy，融合图像的熵越大，说明融合后的图像上的信息量增加的越多，融合图像所含的信息越丰富，融合质量越好。
     *
     * @author binger
     */
	template <class T>
    double EntropyEvaluation(ImageDef<T> *sourceimage)
    {
      int i, j, k, setof;
      double entropy = 0, p[256];
      int nr = sourceimage->Height;
      int nc = sourceimage->Width;
      T *psourcedata, outdata;
      psourcedata = sourceimage->Pixels;

	  for(k = 0; k < 256; k++)
	  {
		  p[k] = 0;
	  }


      for(k = 0; k < 256; k++)
	  {
		  for (i = 0; i < nr; i++)
		  {
			  for(j = 0; j < nc; j++)
			  {
				  switch (sourceimage->Format)
				  {
				  case IMAGE_FORMAT_INDEX:

					  setof = i  * nc + j;
					  outdata = *(psourcedata + setof);
					  if((int)outdata == k)
						  p[k]++;
					  break;
				  case IMAGE_FORMAT_RGB:
				  case IMAGE_FORMAT_BGR:
					  setof = (i  * nc + j) * 3;
					  outdata = (T)(*(psourcedata + setof) * 0.3 + *(psourcedata + setof + 1) * 0.59 + *(psourcedata + setof + 2) * 0.11);
					  if((int)outdata == k)
						  p[k]++;
					  break;
				  default:
					  break;
				  }
			  }
		  }

		  if(p[k] != 0)
		  {
			  p[k] /= nr * nc;
			  entropy += -p[k] * log10(p[k]) / log10(2);
		  }
	  }
      return entropy;
    }
    /// 图像的交叉熵cross Entropy。
    /**
     * 该函数计算图像的交叉熵，以评价图像融合效果。
     *
     * @param sourceimage 标准参考图像结构指针，必须是有效的内存中的图像。
     * @param fusionimage 融合图像结构指针，必须是有效的内存中的图像。
     * @return 图像的交叉熵cross entropy，融合图像的交叉熵越小，说明融合后的图像上的信息量增加的越多，融合图像所含的信息越丰富，融合质量越好。
     *
     * @author binger
     */

    template <class T>
    double CERFEvaluation(ImageDef<T> *sourceimage, ImageDef<T> *fusionimage)
    {
      int i, j, k, setof;
      double crossentropy = 0, ps[256], pf[256];
      int nr = sourceimage->Height;
      int nc = sourceimage->Width;
      T *psourcedata, *pfusiondata, outdatas, outdataf;
      psourcedata = sourceimage->Pixels;
      pfusiondata = fusionimage->Pixels;
      if ((fusionimage->Height != nr) || (fusionimage->Width != nc) || (fusionimage->Format != sourceimage->Format))
        throw IllegalArgumentException();

	  for(k = 0; k < 256; k++)
	  {
		  ps[k] = 0;
		  pf[k] = 0;
	  }


      for(k = 0; k < 256; k++)
	  {
		  for (i = 0; i < nr; i++)
		  {
			  for(j = 0; j < nc; j++)
			  {
				  switch (sourceimage->Format)
				  {
				  case IMAGE_FORMAT_INDEX:
					  setof = i  * nc + j;
					  outdatas = *(psourcedata + setof);
					  outdataf = *(pfusiondata + setof);
					  if((int)outdatas == k)
						  ps[k]++;
					  if((int)outdataf == k)
					      pf[k]++;
					  break;
				  case IMAGE_FORMAT_RGB:
				  case IMAGE_FORMAT_BGR:
					  setof = (i  * nc + j) * 3;
					  outdatas = (T)(*(psourcedata + setof) * 0.3 + *(psourcedata + setof + 1) * 0.59 + *(psourcedata + setof + 2) * 0.11);
					  outdataf = (T)(*(pfusiondata + setof) * 0.3 + *(pfusiondata + setof + 1) * 0.59 + *(pfusiondata + setof + 2) * 0.11);
					  if((int)outdatas == k)
						  ps[k]++;
 					  if((int)outdataf == k)
					      pf[k]++;

					  break;
				  default:
					  break;
				  }
			  }
		  }

		  if((ps[k] != 0) && (pf[k] != 0))
		  {
			  ps[k] /= nr * nc;
			  pf[k] /= nr * nc;

			  crossentropy += ps[k] * (log10(ps[k] / pf[k]) / log10(2));
		  }
	  }

      return crossentropy;
    }
  }
}

#endif //__MERGENCEEVALUATION_H__
