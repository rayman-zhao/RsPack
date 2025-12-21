#ifndef __IMAGESUBAREA_H__
#define __IMAGESUBAREA_H__

#include <cstring>

/**
 * @file
 *
 * @brief 包含二维图像处理子区定义。
 */

namespace MBL
{
  namespace Image2D
  {
    /// 表示图像处理子区的类。
    /**
     * 处理子区表示一幅图像的部分区域，一般作为原始图像后的第二个参数传递给图像处理函数。原始图像的左上角的坐标为(0, 0)，
     * 整个图像的坐标范围为(0 ~ Width - 1, 0 ~ Height - 1)。坐标用整数表示，单位为象素。该类可表示图像内部的矩形区域和
     * 任意封闭区域。任意封闭区域用一个与原始图像同样大小的8bit灰度图像表示，其中的灰度值为0的象素表示背景，非0的象素表示
     * 欲处理的子区。处理子区包括该对象指定的边界。如果图像处理函数不能处理图像的子区，则可以抛出UnsupportedSubAreaException。
     *
     * @author 赵宇
     * @version 1.0
     */
    class ImageSubArea
    {
      public:
        /// 矩形处理子区（或任意封闭子区的外接矩形）的左边坐标（象素）。
        int Left;
        /// 矩形处理子区（或任意封闭子区的外接矩形）的上边坐标（象素）。
        int Top;
        /// 矩形处理子区（或任意封闭子区的外接矩形）的宽度（象素）。
        int Width;
        /// 矩形处理子区（或任意封闭子区的外接矩形）的高度（象素）。
        int Height;
        /// 该子区所对应图像的宽度（象素）。
        int ImageWidth;
        /// 该子区所对应图像的高度（象素）。
        int ImageHeight;
        /// 表示任意封闭子区的图像数据指针。如果为0，则表示该处理子区类表示的是矩形子区。
        unsigned char *Pixels;

      private:
        /// 私有缺省构造函数。
        /**
         * 缺省构造函数设置所有数据成员为0，不分配任何内存。该函数为私有，禁止外部调用，只能用静态工厂方法创建处理子区对象。
         */
        ImageSubArea()
        {
          Left = 0;
          Top = 0;
          Width = 0;
          Height = 0;
          ImageWidth = 0;
          ImageHeight = 0;
          Pixels = 0;
        }

      public:
        /// 析构函数，释放对象资源。
        /**
         * 析构函数将释放图像处理子区的数据指针。
         */
        ~ImageSubArea()
        {
          if (Pixels != 0) delete [] Pixels;
        }

      public:
        /// 静态工厂方法，创建一个空的子区实例。
        /**
         * 创建一个空的处理子区对象，其各个坐标为0，没有分配数据内存。
         *
         * @return 如果创建成功则返回对象指针，否则会抛出异常。
         */
        static ImageSubArea * CreateInstance()
        {
          ImageSubArea *area = new ImageSubArea();
          if (area == 0) throw OutOfMemoryException();

          return area;
        }
        /// 静态工厂方法，创建一个矩形处理子区。
        /**
         * 输入的参数确定矩形子区的坐标，函数会创建子区对象，并给相应成员赋值，但不分配图像数据内存。
         *
         * @param left 处理子区的左边（象素）。
         * @param top 处理子区的上边（象素）。
         * @param width 处理子区的宽度（象素）。
         * @param height 处理子区的高度（象素）。
         * @return 如果创建成功则返回对象指针，否则会抛出异常。
         */
        static ImageSubArea * CreateInstance(int left, int top, int width, int height)
        {
          ImageSubArea *area = new ImageSubArea();
          if (area == 0) throw OutOfMemoryException();

          area->Left = left;
          area->Top = top;
          area->Width = width;
          area->Height = height;

          return area;
        }
        /// 静态工厂方法，创建一个任意子区。
        /**
         * 输入的参数确定任意子区的外接矩形，并指定原始图像的尺寸，函数会创建子区对象，给相应数据成员赋值，并分配数据内存，
         * 以待用户填充来表示任意子区。
         *
         * @param left 任意子区外接矩形的左边（象素）。
         * @param top 任意子区外接矩形的上边（象素）。
         * @param width 任意子区外接矩形的宽度（象素）。
         * @param height 任意子区外接矩形的高度（象素）。
         * @param image_width 任意子区所表示的原始图像宽度。
         * @param image_height 任意子区所表示的原始图像高度。
         * @param fill 是否填充该子区，即设置该子区全部为非0。缺省为填充该子区。
         * @return 如果创建成功则返回对象指针，否则会抛出异常。
         */
        static ImageSubArea * CreateInstance(int left, int top, int width, int height, int image_width, int image_height, bool fill = true)
        {
          ImageSubArea *area = CreateInstance(left, top, width, height);

          area->Pixels = new unsigned char[image_width * image_height];
          if (area->Pixels == 0)
          {
            delete area;
            throw OutOfMemoryException();
          }
          area->ImageWidth = image_width;
          area->ImageHeight = image_height;
          if (fill)
          {
            for (int y = top; y < top + height; y++)
            {
              for (int x = left; x < left + width; x++)
              {
                area->Pixels[x + y * image_width] = 1;
              }
            }
          }

          return area;
        }

      public:
        /**
         * @brief 判断一个象素点是否在子区范围内。
         *
         * @return 返回true表示该点在子区内，否则返回false。
         */
        bool IsFill(int x, int y)
        {
          if (Pixels != 0)
          {
            return (Pixels[x + y * ImageWidth] > 0);
          }
          else if (Left <= x && x <= Left + Width && Top <= y && y <= Top + Height)
          {
            return true;
          }
          else
          {
            return false;
          }
        }

        /**
         * @brief 清除子区内容，设置子区数据全部为0。
         */
        void Clear()
        {
          if (Pixels != 0)
          {
            memset(Pixels, 0, ImageWidth * ImageHeight);
            Left = 0;
            Top = 0;
            Width = 0;
            Height = 0;
          }
        }
    };
  } // Image2D namespace
} // MBL namespace

#endif // __IMAGESUBAREA_H__
