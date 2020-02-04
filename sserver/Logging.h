#ifndef SSERVER_LOGGING_H
#define SSERVER_LOGGING_H
#include "LogStream.h"
#include "Timestamp.h"

namespace sserver
{
//日志输出
class TimeZone; //前置申明

class Logger
{
public:
  enum LogLevel
  {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    NUM_LOG_LEVELS, //级别个数
  };

  // compile time calculation of basename of source file
  class SourceFile //帮助获得文件名
  {
  public:
    template <int N>
    inline SourceFile(const char (&arr)[N])
        : data_(arr),
          size_(N - 1)
    {
      const char *slash = strrchr(data_, '/'); // builtin function，查到最后一个“/”并返回位置
      if (slash)
      {
        data_ = slash + 1;
        size_ -= static_cast<int>(data_ - arr);
      }
    }

    explicit SourceFile(const char *filename)
        : data_(filename)
    {
      const char *slash = strrchr(filename, '/');
      if (slash)
      {
        data_ = slash + 1;
      }
      size_ = static_cast<int>(strlen(data_));
    }

    const char *data_;
    int size_;
  };

  Logger(SourceFile file, int line);
  Logger(SourceFile file, int line, LogLevel level);
  Logger(SourceFile file, int line, LogLevel level, const char *func);
  Logger(SourceFile file, int line, bool toAbort);
  ~Logger();

  LogStream &stream() { return impl_.stream_; } //LogStream重载了<<运算符，因此可以直接使用

  static LogLevel logLevel();
  static void setLogLevel(LogLevel level);

  typedef void (*OutputFunc)(const char *msg, int len); //函数指针，下同
  typedef void (*FlushFunc)();
  static void setOutput(OutputFunc); //设置输出函数
  static void setFlush(FlushFunc);   //清空缓冲
  static void setTimeZone(const TimeZone &tz);

private:
  //Loger类的内部嵌套类
  //Impl类主要是负责日志的格式化
  class Impl
  {
  public:
    typedef Logger::LogLevel LogLevel;
    Impl(LogLevel level, int old_errno, const SourceFile &file, int line);
    void formatTime(); //格式化事件
    void finish();     //将日志写道缓冲区

    Timestamp time_;   //Timestamp时间戳
    LogStream stream_; //LogStream类对象成员
    LogLevel level_;   //日志级别
    int line_;         //行号
    SourceFile basename_;
  };

  Impl impl_; //Impl类对象成员
  //类SouceFile用来确定日志文件的名字，而Impl是真正实现日志输出的地方。
  //在Logger类中可以设置Logger日志级别，以及设置缓存和清空缓存函数。
};

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::logLevel() //当前级别返回的是logLevel
{
  return g_logLevel;
}

//
// CAUTION: do not write:
//
// if (good)
//   LOG_INFO << "Good news";
// else
//   LOG_WARN << "Bad news";
//
// this expends to
//
// if (good)
//   if (logging_INFO)
//     logInfoStream << "Good news";
//   else
//     logWarnStream << "Bad news";
//
//使用宏来定义匿名对象，LogStream重载了<<，因此可以使用 LOG_REACE<<"日志"<<
//日志输出宏，输出在哪个文件? 哪一行? 哪个函数? 哪种级别?
//无名对象所在语句执行后就立即被析构，然后调用析构函数将缓冲区的内容分输出
#define LOG_TRACE                                            \
  if (sserver::Logger::logLevel() <= sserver::Logger::TRACE) \
  sserver::Logger(__FILE__, __LINE__, sserver::Logger::TRACE, __func__).stream()
#define LOG_DEBUG                                            \
  if (sserver::Logger::logLevel() <= sserver::Logger::DEBUG) \
  sserver::Logger(__FILE__, __LINE__, sserver::Logger::DEBUG, __func__).stream()
#define LOG_INFO                                            \
  if (sserver::Logger::logLevel() <= sserver::Logger::INFO) \
  sserver::Logger(__FILE__, __LINE__).stream()
#define LOG_WARN sserver::Logger(__FILE__, __LINE__, sserver::Logger::WARN).stream()
#define LOG_ERROR sserver::Logger(__FILE__, __LINE__, sserver::Logger::ERROR).stream()
#define LOG_FATAL sserver::Logger(__FILE__, __LINE__, sserver::Logger::FATAL).stream()
#define LOG_SYSERR sserver::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL sserver::Logger(__FILE__, __LINE__, true).stream()
//构造了一个Logger对象，重载了输入运算符，以流的机制运行
//构造了一个匿名对象，所以用完之后，就没有存在价值了，接着调用析构函数

const char *strerror_tl(int savedErrno);

// Taken from glog/logging.h
//
// Check that the input is non NULL.  This very useful in constructor
// initializer lists.

#define CHECK_NOTNULL(val) \
  sserver::CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

// A small helper for CHECK_NOTNULL().
template <typename T>
T *CheckNotNull(Logger::SourceFile file, int line, const char *names, T *ptr)
{
  if (ptr == NULL)
  {
    Logger(file, line, Logger::FATAL).stream() << names;
  }
  return ptr;
}

} // namespace sserver
//使用宏定义定了了LOG_*为Logger的匿名对象返回的LogStream对象，最后使用匿名对象来输出日志，
//匿名对象只存在构造这个匿名对象的那行代码，离开这行代码马上析构，在析构时会清空缓存，输出日志。
#endif // SSERVER_LOGGING_H
