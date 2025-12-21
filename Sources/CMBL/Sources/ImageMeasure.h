#ifndef __IMAGEMEASURE_H__
#define __IMAGEMEASURE_H__

/**
 * @file
 *
 * @brief 包含测量已分割图像目标参数的函数。
 */

#include <vector>

namespace MBL
{
  namespace Image2D
  {
    /// 目标点结构。
    typedef struct
    {
      int x; /**< 目标点在图中的Y坐标（象素）。 */
      int y; /**< 目标点在图中的Y坐标（象素）。 */
    } ObjectPoint;

    /// 目标基本几何象素属性。
    typedef struct
    {
      int gravity_center_line;           /**< 目标重心在图中的Y坐标（象素）。 */
      int gravity_center_pixel;          /**< 目标重心在图中的X坐标（象素）。 */
      int area;                          /**< 目标象素面积。 */
      double perimeter;                  /**< 目标象素周长。相邻象素距离为1，对角线为Sqrt(2)。 */
      int start_line;                    /**< 目标起始位置在图中的Y坐标（象素）。 */
      int start_pixel;                   /**< 目标起始位置在图中的X坐标（象素）。 */
      int end_line;                      /**< 目标结束位置在图中的Y坐标（象素）。 */
      int end_pixel;                     /**< 目标结束位置在图中的X坐标（象素）。 */
      std::vector<ObjectPoint> boundary; /**< 目标边界在图中的坐标（象素）。 */
    } ObjectProperty;

    /// 在图中跟踪一个目标的边界。
    /**
     * 该函数使用边界跟踪的方法找出一个目标的边界，目标必须是实心的，种子点必须是边界点。
     *
     * @param image 源图像，必须是索引图像，不能是彩色图像。
     * @param Line 起始种子点Y坐标（象素）。
     * @param Pixel 起始种子点X坐标（象素）。
     * @param Object 目标颜色。
     * @param Color 跟踪完毕后边界填充的颜色。
     * @param length double类型指针，函数返回后将保存边界的长度值。相邻象素距离为1，对角线为Sqrt(2)。
     * @param boundary 保存所有边界点坐标的Vector，为0时表示不取得边界点。
     *
     * @author 姜志国 赵宇
     *
     * @deprecated 建议使用FollowBoundary2函数。
     */
    template <class T>
    void FollowBoundary(ImageDef<T> *image, int Line, int Pixel, T Object, T Color, double *length, std::vector<ObjectPoint> *boundary)
    {
    	if (image == 0 || image->Format != IMAGE_FORMAT_INDEX) throw UnsupportedFormatException();

    	T Buf[3][3];
      T pixel[8];
      int Line0,Pixel0;
      int Counter1,Counter=0;
      int LC=0, NumR;
      double gen2=1.414;
      int i;
      int Tab;
    	int L_Start, L_End, R_End, R_Start;

    	L_Start = 0;
    	L_End = image->Height - 1;
    	R_Start = 0;
    	R_End = image->Width - 1;

      *length=0.0;
      Line0=Line;
      Pixel0=Pixel;
      NumR=R_End-R_Start;

      do
    	{
    		ReadRow(image, Pixel-1, Pixel+1, Line-1, Buf[0]);
        ReadRow(image, Pixel-1, Pixel+1, Line,   Buf[1]);
        ReadRow(image, Pixel-1, Pixel+1, Line+1, Buf[2]);

        pixel[0]=Buf[1][2];
        pixel[1]=Buf[0][2];
        pixel[2]=Buf[0][1];
        pixel[3]=Buf[0][0];
        pixel[4]=Buf[1][0];
        pixel[5]=Buf[2][0];
        pixel[6]=Buf[2][1];
        pixel[7]=Buf[2][2];

        Tab=0;
        for(i=LC+7;i>=LC;i--)
    		{
          if(i<8)
            Counter=i;
          else
            Counter=i-8;
          Counter1=i-1;
          if(Counter1>=8)
            Counter1-=8;
          if((pixel[Counter1]!=Object)&&(pixel[Counter1]!=Color) && (pixel[Counter]==Object))
    			{
    				Tab=1;
            break;
          }
          if((pixel[Counter1]!=Object)&&(pixel[Counter1]!=Color) && (pixel[Counter]==Color))
    			{
    				*length = 0;
    				return;
    			}
    		}
        if(Tab==0)
    		{
          if(Pixel==1) Counter=6;
          if(Pixel==NumR) Counter=2;
          if(Line==L_Start) Counter=4;
          if(Line==L_End) Counter=0;
        }
    		switch(Counter)
    		{
    			case 0:
    				 LC=4;
    				 Pixel++;
    				 *length=*length + 1;
    				 break;
    			case 1:
    				 LC=5;
    				 Line--;
    				 Pixel++;
    				 *length=*length+gen2;
    				 break;
    			case 2:
    				 LC=6;
    				 Line--;
    				 *length=*length+1;
    				 break;
    			case 3:
    				 LC=7;
    				 Line--;
    				 Pixel--;
    				 *length=*length+gen2;
    				 break;
    			case 4:
    				 LC=0;
    				 Pixel--;
    				 *length=*length+1;
    				 break;
    			case 5:
    				 LC=1;
    				 Line++;
    				 Pixel--;
    				 *length=*length+gen2;
    				 break;
    			case 6:
    				 LC=2;
    				 Line++;
    				 *length=*length+1;
    				 break;
    			case 7:
    				 LC=3;
    				 Line++;
    				 Pixel++;
    				 *length=*length+gen2;
    				 break;
    			default:
    				 break;
    		}
         WriteRow(image, Pixel, Pixel, Line, &Color);
         if (boundary != 0)
         {
           ObjectPoint op = {Pixel, Line};
           boundary->push_back(op);
         }
      } while(Line!=Line0 || Pixel!=Pixel0);
    }

    /// 在图中跟踪一个目标的边界。
    /**
     * 该函数使用边界跟踪的方法找出一个目标的边界，目标必须是实心的，种子点必须是边界点。该函数比
     * FollowBoundary函数更加稳定，而且适应性更强，建议使用。
     *
     * @param image 源图像，必须是索引图像，不能是彩色图像。
     * @param line 起始种子点Y坐标（象素）。
     * @param pixel 起始种子点X坐标（象素）。
     * @param object 目标颜色。
     * @param color 跟踪完毕后边界填充的颜色。
     * @param length double类型指针，函数返回后将保存边界的长度值。相邻象素距离为1，对角线为Sqrt(2)。
     * @param boundary 保存所有边界点坐标的Vector，为0时表示不取得边界点。
     *
     * @attention 当boundary参数为0时，不会填充边界的颜色。
     *
     * @author 陈进 赵宇
     */
    template <class T>
    void FollowBoundary2(ImageDef<T> *image, int line, int pixel, T object, T color, double *length, std::vector<ObjectPoint> *boundary)
    {
    	if (image == 0 || image->Format != IMAGE_FORMAT_INDEX) throw UnsupportedFormatException();

      #define fy(t) (t+2)/4-(t-1)/4-2*((t+2)/8-(t-1)/8)
      #define fx(t) (t+4)/4-(t+1)/4-2*((t+4)/8-(t+1)/8)

      int xstart = pixel, ystart = line, xnew = xstart, ynew = ystart, xold, yold;
      int flag = 0;
      bool ScanMode = false;
      int m, n, b = 0;
      int xsize = image->Width, ysize = image->Height;
      int li;
      T ivalue;
      T t = object;

      ObjectPoint op;

      while(!((xnew==xstart)&&(ynew==ystart)&&(flag))) {
        flag=1;
        xold=xnew; yold=ynew;
        for(n=1;n<=8;n++) {
          m=n+b;
          m=m-8*((m-1)/8);
          if (ScanMode) { // vertical scan
            if ( (yold+fy(m)) > (xsize-1) ) continue ;
            if ( (yold+fy(m)) < 1 ) continue ;
            if ( (xold+fx(m)) > (ysize-1) ) continue ;
            if ( (xold+fx(m)) < 1 ) continue ;

            li = (int)yold+(int)fy(m)+((int)xold+(int)fx(m))*(int)xsize ;
          }
          else { // horizontal scan
            if ( (yold+fy(m)) > (ysize-1) ) continue ;
            if ( (yold+fy(m)) < 1 ) continue ;
            if ( (xold+fx(m)) > (xsize-1) ) continue ;
            if ( (xold+fx(m)) < 1 ) continue ;

            li = ((int)yold+(int)fy(m))*(int)xsize+(int)xold+(int)fx(m) ;
          }

          ivalue = image->Pixels[li] ;
          if (ivalue == t) {
            xnew=xold+fx(m);
            ynew=yold+fy(m);
            b=m+4-8*((m-1)/4);
            break;
          }
        }

        if (boundary != 0)
        {
          if (ScanMode) { // vertical scan
            op.x=ynew;
            op.y=xnew;
          }
          else { // horizontal scan
            op.x=xnew;
            op.y=ynew;
          }
          boundary->push_back(op);
        }
      }  // end of while

      if (boundary != 0 && boundary->size() > 0)
      {
        WritePixel(image, (*boundary)[0].x, (*boundary)[0].y, &color);
        for (std::vector<ObjectPoint>::size_type i = 1; i < boundary->size(); i++)
        {
          const ObjectPoint &p1 = (*boundary)[i - 1];
          const ObjectPoint &p2 = (*boundary)[i];

          if (p1.x != p2.x && p1.y != p2.y)
          {
            *length += 1.414;
          }
          else
          {
            *length += 1;
          }

          WritePixel(image, p2.x, p2.y, &color);
        }
      }
    }

    /**
     * 取得一个已分割目标的基本几何参数。
     *
     * 目标必须是实心的，种子点必须是边界点。
     *
     * @param image 源图像，必须是索引图像，不能是彩色图像。
     * @param StartSeedLine 起始种子点Y坐标（象素）。
     * @param StartSeedPixel 起始种子点X坐标（象素）。
     * @param B_Color 目标颜色。
     * @param L_Color 测量完毕后目标的填充颜色。
     * @param F_Color 测量完毕后目标边界的填充颜色。
     * @param getBoundary 是否得到分割目标边界所有点的坐标。缺省值为false，即不取得。
     *
     * @author 姜志国 赵宇
     *
     * @bug 输入图像宽度不能超过2048。
     * @bug 如果不跟踪边界，并将边界填充新的颜色，则有时会导致死循环。
     */
    template <class T>
    ObjectProperty GetSegmentedObjectProperty(ImageDef<T> *image, int StartSeedLine, int StartSeedPixel, T B_Color, T L_Color, T F_Color, bool getBoundary = false)
    {
      if (image == 0 || image->Format != IMAGE_FORMAT_INDEX) throw UnsupportedFormatException();

    	int Area;
    	double Long;
    	int Zh_L,Zh_P;
    	int L_S,P_S,L_E,P_E;
    	int Pop_Up,Pop_Down;
    	static int Up_Seed[2048][2],Down_Seed[2048][2];
    	int ImageSizeX = image->Width, ImageSizeY = image->Height;

    	int Seed_Line,Seed_Pixel;
      int S_Raw;
      int Hang,vab;
      int detect_i,detect_j;
      int Right,Left;
    	static T Up_Buf[2048], Down_Buf[2048];

    	Zh_L=0;
    	Zh_P=0;
    	Area=0;
    	Long=0;
    	L_S=StartSeedLine;
    	L_E=StartSeedLine;
    	P_S=StartSeedPixel;
    	P_E=StartSeedPixel;

    	Pop_Up=-1;
    	Pop_Down=0;   /*  start to detect down   */
    	Down_Seed[0][0]=StartSeedLine;
    	Down_Seed[0][1]=StartSeedPixel;
    	do
    	{
    	  do
    	  {
    	    if(Pop_Down<0) break;
    			Seed_Line=Down_Seed[Pop_Down][0];
    			Seed_Pixel=Down_Seed[Pop_Down][1];
          //Down_Detect(Seed_Line,Seed_Pixel,Object_Color,Detect_Color1);
          //start of DOWNDETECT function  2001/04/05
          Hang=Seed_Line;
          Left=Seed_Pixel;
          Right=Seed_Pixel;
          S_Raw=Seed_Pixel;
          Pop_Down=Pop_Down-1;
          for(detect_i=0;detect_i<ImageSizeX;detect_i++)
            Up_Buf[detect_i]=L_Color; /* to distinguish object color */

          ReadRow(image,0,ImageSizeX-1,Hang-1,Up_Buf);
          ReadRow(image,0,ImageSizeX-1,Hang,Down_Buf);
          do
          {
            vab=0;
            for(detect_i=S_Raw;detect_i<ImageSizeX-1;detect_i++)
            {
              if(Down_Buf[detect_i]!=B_Color)
              {
                if(P_E<detect_i-1) P_E=detect_i-1;

                if(detect_i<Right)
                {
                  for(detect_j=detect_i;detect_j<Right;detect_j++)
                  {
                    if((Down_Buf[detect_j]!=B_Color)&&(Down_Buf[detect_j+1]==B_Color))
                    {
                      if(Pop_Down<2048-1)
                      {
                        Pop_Down=Pop_Down+1;
                        Down_Seed[Pop_Down][0]=Hang;
                        Down_Seed[Pop_Down][1]=detect_j+1;
                      }
                    }
                  }
                }
                Right=detect_i;
                break;
              }
              Down_Buf[detect_i]=L_Color;

              Area=Area+1;
              Zh_L=Zh_L+Hang;
              Zh_P=Zh_P+detect_i;

              if(Right<=detect_i) Right=detect_i+1;
              if(detect_i==ImageSizeX-1) Right=ImageSizeX-1;
            }
            for(detect_i=S_Raw-1;detect_i>0;detect_i--)
            {
              if(Down_Buf[detect_i]!=B_Color)
              {
                if(P_S>detect_i+1)  P_S=detect_i+1;

                if(detect_i>Left)
                {
                  for(detect_j=detect_i;detect_j>Left;detect_j--)
                  {
                    if((Down_Buf[detect_j]!=B_Color)&&(Down_Buf[detect_j-1]==B_Color))
                    {
                      if(Pop_Down<2048-1)
                      {
                        Pop_Down=Pop_Down+1;
                        Down_Seed[Pop_Down][0]=Hang;
                        Down_Seed[Pop_Down][1]=detect_j-1;
                      }
                    }
                  }
                }

                Left=detect_i;
                break;
              }

              Area=Area+1;
              Zh_L=Zh_L+Hang;
              Zh_P=Zh_P+detect_i;

              Down_Buf[detect_i]=L_Color;
              if(Left>=detect_i) Left=detect_i-1;
              if(detect_i==0) Left=0;
            }

            WriteRow(image,0,ImageSizeX-1,Hang,Down_Buf);
            for(detect_i=Left;detect_i<=Right;detect_i++)
            {
              if(((Up_Buf[detect_i]==B_Color)&&(Up_Buf[detect_i+1]!=B_Color))
                 ||((Up_Buf[Right]==B_Color)&&(Up_Buf[Right+1]==B_Color)))
              {
                if((Up_Buf[detect_i]==B_Color)&&(Up_Buf[detect_i+1]!=B_Color))
                {
                  if(Pop_Up<2048-1)
                  {
                    Pop_Up=Pop_Up+1;
                    Up_Seed[Pop_Up][0]=Hang;
                    Up_Seed[Pop_Up][1]=detect_i;
                  }
                }
              }
            }

            for(detect_i=0;detect_i<ImageSizeX;detect_i++)
              Up_Buf[detect_i]=Down_Buf[detect_i];

            if(L_E<Hang) L_E=Hang;
            Hang=Hang+1;
            ReadRow(image, 0,ImageSizeX-1,Hang,Down_Buf);
            for(detect_i=Left;detect_i<=Right;detect_i++)
            {
              if(Down_Buf[detect_i]==B_Color)
              {
                vab=1;
                S_Raw=detect_i;
                break;
              }
            }
            if(Hang>ImageSizeY-1) vab=0;
          } while(vab!=0);
          // end of DOWNDETECT function  2001/04/05
        } while(Pop_Down>-1 && Pop_Down<2048);
    	  do
    	  {
    	    if(Pop_Up<0) break;
    		  Seed_Line=Up_Seed[Pop_Up][0];
    		  Seed_Pixel=Up_Seed[Pop_Up][1];
          //Up_Detect(Seed_Line,Seed_Pixel,Object_Color,Detect_Color1);
          //start of UPDETECT function   2001/04/05
          Hang=Seed_Line;
          Left=Seed_Pixel;
          Right=Seed_Pixel;
          S_Raw=Seed_Pixel;
          Pop_Up=Pop_Up-1;
          for(detect_i=0;detect_i<ImageSizeX;detect_i++)
            Down_Buf[detect_i]=L_Color; /* to distinguish object color */

          ReadRow(image,0,ImageSizeX-1,Hang+1,Down_Buf);
          ReadRow(image,0,ImageSizeX-1,Hang,Up_Buf);

          do
          {
            vab=0;
            for(detect_i=S_Raw;detect_i<ImageSizeX-1;detect_i++)
            {
              if(Up_Buf[detect_i]!=B_Color)
              {
                if(P_E<detect_i-1) P_E=detect_i-1;

                if(detect_i<Right)
                {
                  for(detect_j=detect_i;detect_j<Right;detect_j++)
                  {
                    if((Up_Buf[detect_j]!=B_Color)&&(Up_Buf[detect_j+1]==B_Color))
                    {
                      if(Pop_Up<2048-1)
                      {
                        Pop_Up=Pop_Up+1;
                        Up_Seed[Pop_Up][0]=Hang;
                        Up_Seed[Pop_Up][1]=detect_j+1;
                      }
                    }
                  }
                }
                Right=detect_i;
                break;
              }
              Up_Buf[detect_i]=L_Color;

              Area=Area+1;
              Zh_L=Zh_L+Hang;
              Zh_P=Zh_P+detect_i;
              if(Right<=detect_i) Right=detect_i+1;
              if(detect_i==ImageSizeX-1) Right=ImageSizeX-1;
            }
            for(detect_i=S_Raw-1;detect_i>0;detect_i--)
            {
              if(Up_Buf[detect_i]!=B_Color)
              {
                if(P_S>detect_i+1) P_S=detect_i+1;

                if(detect_i>Left)
                {
                  for(detect_j=detect_i;detect_j>Left;detect_j--)
                  {
                    if((Up_Buf[detect_j]!=B_Color)&&(Up_Buf[detect_j-1]==B_Color))
                    {
                      if(Pop_Up<2048-1)
                      {
                        Pop_Up=Pop_Up+1;
                        Up_Seed[Pop_Up][0]=Hang;
                        Up_Seed[Pop_Up][1]=detect_j+1;
                      }
                    }
                  }
                }

                Left=detect_i;
                break;
              }
              Up_Buf[detect_i]=L_Color;

              Area=Area+1;
              Zh_L=Zh_L+Hang;
              Zh_P=Zh_P+detect_i;

              if(Left>=detect_i) Left=detect_i-1;
              if(detect_i==0) Left=0;
            }
            WriteRow(image,0,ImageSizeX-1,Hang,Up_Buf);
            for(detect_i=Left;detect_i<=Right;detect_i++)
            {
              if(((Down_Buf[detect_i]==B_Color)&&(Down_Buf[detect_i+1]!=B_Color))
                 ||((Down_Buf[Right]==B_Color)&&(Down_Buf[Right+1]==B_Color)))
              {
                if((Down_Buf[detect_i]==B_Color)&&(Down_Buf[detect_i+1]!=B_Color))
                {
                  if(Pop_Down<2048-1)
                  {
                    Pop_Down=Pop_Down+1;
                    Down_Seed[Pop_Down][0]=Hang;
                    Down_Seed[Pop_Down][1]=detect_i;
                  }
                }
              }
            }
            for(detect_i=0;detect_i<ImageSizeX;detect_i++)
              Down_Buf[detect_i]=Up_Buf[detect_i];

            if(L_S>Hang) L_S=Hang;
            Hang=Hang-1;
            ReadRow(image, 0,ImageSizeX-1,Hang,Up_Buf);
            for(detect_i=Left;detect_i<=Right;detect_i++)
            {
              if(Up_Buf[detect_i]==B_Color)
              {
                vab=1;
                S_Raw=detect_i;
                break;
              }
            }
            if(Hang<0) vab=0;
          } while(vab!=0);
          // end of UPDETECT function    2001/04/05
        } while(Pop_Up>-1 && Pop_Up < 2048);
      } while((Pop_Up>-1 && Pop_Up < 2048) && (Pop_Down>-1 && Pop_Down < 2048));

    	ObjectProperty DetectPara;
    	DetectPara.area=Area;
    	DetectPara.end_line=L_E;
    	DetectPara.start_line=L_S;
    	DetectPara.end_pixel=P_E;
    	DetectPara.start_pixel=P_S;
    	DetectPara.gravity_center_line=Zh_L;
    	DetectPara.gravity_center_pixel=Zh_P;

    	if (getBoundary == true)
    	{
    	  FollowBoundary2(image, StartSeedLine, StartSeedPixel, L_Color, F_Color, &DetectPara.perimeter, &DetectPara.boundary);
    	}
    	else
    	{
    	  FollowBoundary2(image, StartSeedLine, StartSeedPixel, L_Color, F_Color, &DetectPara.perimeter, 0);
    	}

    	return DetectPara;
    }

    /**
     * @brief 找出图像中最大面积的黑色目标。
     *
     * 该函数先将该图像转换为灰度图像，分割出黑色目标，然后找出面积最大的目标，返回其几何参数。一般用于显微图像
     * 校准园的搜索。如果源图像为灰度图像，则该函数会改变源图像，请预先做好备份。
     *
     * @param image 源图像。
     * @return 最大面积的目标属性。
     */
    template <class T>
    ObjectProperty FindObjectWithMaxArea(ImageDef<T> *image)
    {
      ImageDef<T> *gray_image = 0;
      if (image->Format != IMAGE_FORMAT_INDEX)
      {
        gray_image = CreateGrayImage(image);
        image = gray_image;
      }

      // 阈值分割。
      SegmentImageWithThreshold(image, (T)200);

      // 搜索全图。
      int start_line = 0, start_pixel = 0, end_line = image->Height - 1, end_pixel = image->Width - 1;
      //int image_size_x = image->Width, image_size_y = image->Height;
      T *buf = new T[image->Width];
      int line, pixel;
      ObjectProperty obj, max_obj;
      max_obj.area = 0;

      //必须清除出一个2个象素宽的边框，否则由于GetSegmentedObjectProperty有时会向上搜索会出错。
      memset(buf, 0, image->Width * sizeof(T));
      WriteRow(image, start_pixel, end_pixel, start_line, buf);
      WriteRow(image, start_pixel, end_pixel, start_line + 1, buf);
      WriteRow(image, start_pixel, end_pixel, end_line, buf);
      WriteRow(image, start_pixel, end_pixel, end_line - 1, buf);
      for (int i = start_line; i <= end_line; i++)
      {
        WriteRow(image, start_pixel, start_pixel + 1, i, buf);
        WriteRow(image, end_pixel - 1, end_pixel, i, buf);
      }

      for (line = start_line; line < end_line; line++)
      {
        ReadRow(image, start_pixel, end_pixel, line, buf);
        for (pixel = start_pixel; pixel < end_pixel; pixel++)
        {
          if (buf[pixel] == 200)
          {
            obj = GetSegmentedObjectProperty(image, line, pixel, (T)200, (T)64, (T)128, true);
            ReadRow(image, start_pixel, end_pixel, line, buf);

            if (obj.area > max_obj.area) max_obj = obj;
          }
        }
      }

      delete [] buf;
      if (gray_image != 0)
      {
        delete gray_image;
      }

      return max_obj;
    }

    /**
     * @brief 找出图像中所有的目标。
     *
     * 该函数先将该图像转换为灰度图像，然后分割出所有黑色目标，并得到每个目标的边界点。
     * 如果源图像为灰度图像，则该函数会改变源图像，请预先做好备份。
     *
     * @param image 源图像。
     * @param sub_area 子区对象，如果为0则表示处理全图。
     * @param v Vector对象指针。所有分割目标将存储在其中。
     */
    template <class T>
    void SegmentAllObject(ImageDef<T> *image, ImageSubArea *sub_area, std::vector<ObjectProperty> *v)
    {
      ImageDef<T> *gray_image = 0;
      if (image->Format != IMAGE_FORMAT_INDEX)
      {
        gray_image = CreateGrayImage(image);
        image = gray_image;
      }

      // 阈值分割。
      SegmentImageWithThreshold(image, (T)200);

      // 搜索全图。
      int start_line = 0, start_pixel = 0, end_line = image->Height - 1, end_pixel = image->Width - 1;
      //int image_size_x = image->Width, image_size_y = image->Height;
      T *buf = new T[image->Width];
      int line, pixel;
      ObjectProperty obj;

      //必须清除出一个2个象素宽的边框，否则由于GetSegmentedObjectProperty有时会向上搜索会出错。
      memset(buf, 0, image->Width * sizeof(T));
      WriteRow(image, start_pixel, end_pixel, start_line, buf);
      WriteRow(image, start_pixel, end_pixel, start_line + 1, buf);
      WriteRow(image, start_pixel, end_pixel, end_line, buf);
      WriteRow(image, start_pixel, end_pixel, end_line - 1, buf);
      for (int i = start_line; i <= end_line; i++)
      {
        WriteRow(image, start_pixel, start_pixel + 1, i, buf);
        WriteRow(image, end_pixel - 1, end_pixel, i, buf);
      }

      // 子区外的图像也都清零。
      if (sub_area != 0)
      {
        T zero = 0;

        for (int y = 2; y < image->Height - 2; y++)
        {
          for (int x = 2; x < image->Width - 2; x++)
          {
            if (sub_area->IsFill(x, y) == false)
            {
              WritePixel(image, x, y, &zero);
            }
          }
        }
      }

      for (line = start_line; line < end_line; line++)
      {
        ReadRow(image, start_pixel, end_pixel, line, buf);
        for (pixel = start_pixel; pixel < end_pixel; pixel++)
        {
          if (buf[pixel] == 200)
          {
            obj = GetSegmentedObjectProperty(image, line, pixel, (T)200, (T)64, (T)128, true);

            ReadRow(image, start_pixel, end_pixel, line, buf);

            v->push_back(obj);
          }
        }
      }

      delete [] buf;
      if (gray_image != 0)
      {
        delete gray_image;
      }
    }
  }
}

#endif // __IMAGEMEASURE_H__
