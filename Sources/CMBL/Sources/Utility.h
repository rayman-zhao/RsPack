#ifndef __UTILITY_H__
#define __UTILITY_H__

/**
 * @file
 *
 * @brief 本文件包含一些辅助工具杂类函数。
 */

#include <vector>
#include <algorithm>
#include <string>
#include <memory>

#pragma warning(disable : 4996)

namespace MBL
{
  namespace Utility
  {
    /// 释放一个对象的指针。
    /**
     * 该函数在确认该指针有效后删除该指针所指的对象，然后将该指针设置为0。
     *
     * @param p 指向一个有效对象的指针的指针。
     *
     * @author 赵宇
     */
    template <class T>
    void SafeRelease(T **p)
    {
      if (*p != 0)
      {
        delete *p;
        *p = 0;
      }
    }

    /// 释放一个数组的指针。
    /**
     * 该函数在确认该指针有效后删除该指针所指的数组，然后将该指针设置为0。
     *
     * @param p 指向一个有效数组的指针的指针。
     *
     * @author 陈伟卿
     */
    template <class T>
    void SafeReleaseArray(T **p)
    {
      if (*p != 0)
      {
        delete [] *p;
        *p = 0;
      }
    }

    /// 从向量中删除一个元素。
    /**
     * 该函数从一个向量中删除一个元素，如果该元素不在向量中，则对向量没有影响。
     *
     * @param v 向量指针，必须是有效的。
     * @param e 欲删除的元素。
     * @return true表示该元素在向量中，并被删除。false表示该元素不在向量中。
     */
    template <class T>
    bool EraseVectorElement(std::vector<T> *v, T e)
    {
      bool ret = false;
      typename std::vector<T>::iterator itr = std::find(v->begin(), v->end(), e);
      if (itr != v->end())
      {
        v->erase(itr);
        ret = true;
      }

      return ret;
    }

    /// 从向量中删除一个元素。
    /**
     * 该函数从一个向量中删除一个指定下标的元素，如果该下标不在有效范围内，则对向量没有影响。
     *
     * @param v 向量指针，必须是有效的。
     * @param i 欲删除的下标。
     * @return true表示该下标的元素在向量中，并被删除。false表示该下标的元素不在向量中。
     */
    template <class T>
    bool EraseVectorElement(std::vector<T> *v, int i)
    {
      bool ret = false;

      if (i >= 0)
      {
        typename std::vector<T>::iterator itr = v->begin() + i;
        if (itr < v->end())
        {
          v->erase(itr);
          ret = true;
        }
      }

      return ret;
    }

    /**
     *  @brief 删除一个向量中的所有指针元素。
     *
     *  删除向量中的所有指针元素，并清除向量。
     *
     *  @param v 向量指针，该向量的所有元素都是指针，即可以被delete运算符删除。
     *
     *  @author 赵宇
     */
    template <class T>
    void ErasePointerVector(std::vector<T> *v)
    {
      for (typename std::vector<T>::size_type i = 0, n = v->size(); i < n; ++i)
      {
        delete (*v)[i];
      }
      v->clear();
    }

    /**
     * @brief 交换两个变量。
     *
     * @param t1 变量指针1。
     * @param t2 变量指针2。
     *
     * @deprecated 请使用 std::swap 代替。
     */
    template <class T>
    inline void Swap(T *t1, T *t2)
    {
      T t;
      t = *t1;
      *t1 = *t2;
      *t2 = t;
    }

    /// 将一个值剪裁到一个范围中。
    /**
     * @param v 欲剪裁的值。
     * @param min 区域最小值。
     * @param max 区域最大值。
     * @return 剪裁后的值。
     */
    template <class T1, class T2>
    inline T1 Clamp(T1 v, T2 min, T2 max)
    {
      if (max < min) Swap(&min, &max);

      if (v < min)
        v = static_cast<T1>(min);
      else if (v > max)
        v = static_cast<T1>(max);

      return v;
    }

    /**
     * @brief 取得一个数值的符号。
     *
     * @param x 一个数值。
     * @return -1表示该值为负，+1表示该值为正或零。
     */
    template <class T>
    inline int Sign(T x)
    {
      return (x >= 0) ? 1 : -1;
    }

    /**
     * @brief 取得一个数据类型的最大可能值。
     *
     * 目前的方法是将该数据类型的内存单元全部填写FF后取得的值，对无符号的整数是合适的。
     *
     * @param v 一种数据类型的变量指针。
     *
     * @deprecated 请使用ImageDefTraits代替。
     */
    template <class T>
    void GetMaxValue(T *v)
    {
      memset(v, 0xff, sizeof(T));
    }

    /**
     * @brief 取最小值。
     *
     * @param v1 变量1。
     * @param v2 变量2。
     *
     * @deprecated 请使用 std::min 代替。
     */
    template <class T>
    inline T GetMin(T v1, T v2)
    {
      return v1 < v2 ? v1 : v2;
    }

    /**
     * @brief 取最大值。
     *
     * @param v1 变量1。
     * @param v2 变量2。
     *
     * @deprecated 请使用 std::max 代替。
     */
    template <class T>
    inline T GetMax(T v1, T v2)
    {
      return v1 > v2 ? v1 : v2;
    }

    /// 更快地将一个值剪裁到一个范围中。
    /**
     * @param v 欲剪裁的值。
     * @param min 区域最小值。
     * @param max 区域最大值。
     * @return 剪裁后的值。
     */
    template <class T1, class T2>
    inline T1 ClampFast(T1 v, T2 min, T2 max)
    {
      return GetMin(GetMax(v, min), max);
    }

    /**
     * @brief 将一个wchar_t类型的字符串对象转换为char类型的字符串对象。
     *
     * @param w wchar_t类型的字符串对象。
     * @return char类型的字符串对象。
     */
    inline std::string ConvertString(const std::wstring &w)
    {
      std::string s;
      if (w.size() > 0)
      {
        size_t sz = w.size() * sizeof(wchar_t) + 1; //最大可能转换的长度，包括结尾的0。
        std::unique_ptr<char> p(new char[sz]);
        size_t ct = wcstombs(p.get(), w.c_str(), w.size());
        s.assign(p.get(), ct);
      }

      return s;
    }
    /**
     * @brief 将一个char类型的字符串对象转换为wchar_t类型的字符串对象。
     *
     * @param s char类型的字符串对象。
     * @return wchar_t类型的字符串对象。
     */
    inline std::wstring ConvertString(const std::string &s)
    {
      std::wstring w;
      if (s.size() > 0)
      {
        size_t sz = s.size() + 1; //可以容纳结尾的0。
        std::unique_ptr<wchar_t> p(new wchar_t[sz]);
        size_t ct = mbstowcs(p.get(), s.c_str(), s.size());
        w.assign(p.get(), ct);
      }

      return w;
    }
  }
}

#endif //__UTILITY_H__
