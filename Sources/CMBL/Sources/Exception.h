#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

/**
 * @file
 *
 * @brief 包含MBL库中所有的异常定义。
 */

namespace MBL
{
  /// 所有MBL库中的异常的基类。
  /**
   * MBL采用了异常机制来处理错误，所有异常子类都要从该类继承，这样可以用catch (MBL::Exception)来捕获所有异常。该类是个空
   * 类，没有任何实际意义。在实际使用MBL库时，当出现这些异常的时候一般都表示出现了逻辑上的错误，而且大都不可恢复。所以不强制
   * 要求MBL库函数对其可抛出的异常进行文档说明。开发者应该尽量使用这些定义好的标准异常。
   *
   * @deprecated 请使用 std::exception 代替。
   */
  class Exception
  {
    public:
      Exception() {}
      virtual ~Exception() {}
  };

  /// 空指针异常。
  /// @deprecated 请使用 std::invalid_argument 代替。
  class NullPointerException : public Exception {};

  /// 无效参数异常。
  /// @deprecated 请使用 std::invalid_argument 代替。
  class IllegalArgumentException : public Exception {};

  /// 下标越界异常。
  /// @deprecated 请使用 std::out_of_range 代替。
  class IndexOutOfBoundsException : public Exception {};

  /// 内存溢出异常。
  /// @deprecated 请使用 std::bad_alloc 代替。
  class OutOfMemoryException : public Exception {};

  /// 文件I/O异常。
  /// @deprecated 请使用 std::runtime_error 代替。
  class FileIOException : public Exception {};

  namespace Image2D
  {
    /// 不能处理该图像格式异常。
    /// @deprecated 请使用 std::invalid_argument 代替。
    class UnsupportedFormatException : public Exception {};

    /// 不能处理图像的子区。
    /// @deprecated 请使用 std::invalid_argument 代替。
    class UnsupportedSubAreaException : public Exception {};

    /// 该图像对象还没有被初始化。
    /// @deprecated 请使用 std::invalid_argument 代替。
    class UninitializedImageException : public Exception {};

    /// 同时处理的两幅图像不匹配。
    /// @deprecated 请使用 std::invalid_argument 代替。
    class UnmatchedImageException : public Exception {};
  }
}

#endif // __EXCEPTION_H__
