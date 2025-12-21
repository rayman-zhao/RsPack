#ifndef __RASTERPAINT_H__
#define __RASTERPAINT_H__

/**
 * @file
 *
 * @brief 包含在图像中进行光栅绘制的函数。
 */

namespace MBL
{
  namespace Image2D
  {
    /**
     * @brief 在水平方向增长绘制线。
     *
     * 该函数根据种子点的位置，在水平方向增长绘制线，直到图像边界或者非目标颜色区域为止。
     *
     * @param buf 欲释绘制的图像行缓冲区，目前只支持索引颜色。
     * @param xl 种子点左坐标指针，函数返回时存储增长后的坐标。
     * @param xr 种子点右坐标指针，函数返回时存储增长后的坐标。
     * @param obj 目标颜色，与种子点的颜色无关。与目标颜色相同的点才会被绘制。
     * @param color 绘制颜色。
     * @param dx 图像宽度。
     * @return 所绘制到的点数。
     *
     * @author Alan 赵宇
     */
    template <class T>
    int GrowInLine(T *buf, int *xl, int *xr, T obj, T color, int dx)
    {
      int x = 0;

      x = *xl - 1;       /* look to left */
      while ((x >= 0) && (buf[x] == obj))
      {
        buf[x--] = color;
      }
      x++;
      *xl = x;

      x = *xr + 1;       /* look to right */
      while ((x < dx) && (buf[x] == obj))
      {
        buf[x++] = color;
      }
      x--;
      *xr = x;

      return *xr - *xl;
    }
    /**
     * @brief 在水平方向绘制线。
     *
     * 该函数根据种子点的位置，在水平方向绘制线，直到图像边界、遇到边界颜色、或者已经被绘制的区域为止。
     *
     * @param buf 欲释绘制的图像行缓冲区，目前只支持索引颜色。
     * @param xl 种子点左坐标指针，函数返回时存储增长后的坐标。
     * @param xr 种子点右坐标指针，函数返回时存储增长后的坐标。
     * @param edg 边界颜色，绘制遇到边界颜色就会停止。
     * @param color 绘制颜色，绘制遇到已经是绘制颜色的点就会停止。
     * @param dx 图像宽度。
     * @return 所绘制到的点数。
     *
     * @author Alan 赵宇
     */
    template <class T>
    int FloodInLine(T *buf, int *xl, int *xr, T edg, T color, int dx)
    {
      int x = 0;

      x = *xl - 1;       /* look to left */
      while ((x >= 0) && (buf[x] != edg) && (buf[x] != color))
      {
        buf[x--] = color;
      }
      x++;
      *xl = x;

      x = *xr + 1;       /* look to right */
      while ((x < dx) && (buf[x] != edg) && (buf[x] != color))
      {
        buf[x++] = color;
      }
      x--;
      *xr = x;

      return *xr - *xl;
    }
  }
}

#endif // __RASTERPAINT_H__
