#pragma once

#include <atomic>
#include <exception>
#include <iostream>
#include <sstream>

namespace apollo {

enum LogLevel {
  kLogTrace,
  kLogDebug,
  kLogInfo,
  kLogCheck,
};

class LogMessage {
 public:
  LogMessage(const char* function, int line)
      : function_(function), line_(line), stream_() {}

  virtual ~LogMessage() {}

  std::ostream& Stream() { return stream_; }

 protected:
  const char* function_;
  int line_;
  std::stringstream stream_;
};

class InfoLogMessage : public LogMessage {
 public:
  InfoLogMessage(const char* function, int line) : LogMessage(function, line) {}

  virtual ~InfoLogMessage() override {
    std::cerr << "[" << function_ << " line " << line_ << "] " << stream_.str()
              << std::endl;
  }
};

class TraceLogMessage : public InfoLogMessage {
 public:
  TraceLogMessage(const char* function, int line)
      : InfoLogMessage(function, line) {}
};

class DebugLogMessage : public InfoLogMessage {
 public:
  DebugLogMessage(const char* function, int line)
      : InfoLogMessage(function, line) {}
};

class CheckLogMessage : public LogMessage {
 public:
  CheckLogMessage(const char* function, int line, const char* cond)
      : LogMessage(function, line), cond_(cond) {}

  virtual ~CheckLogMessage() override {
    std::cerr << "[" << function_ << " line " << line_ << "] check \"" << cond_
              << "\" failed: " << stream_.str() << std::endl;
    ;
#if defined(__has_feature)
#if __has_feature(address_sanitizer)
    // Goofy hack to make asan print out a stack trace when a check fails.
    volatile int* i = nullptr;
    *i = 42;
#endif
#endif
    std::cerr.flush();
    std::abort();
  }

 private:
  const char* cond_;
};

class VoidMessage {
 public:
  VoidMessage() {}

  void operator&(std::ostream&) {}
};

inline std::atomic<LogLevel> kEnabledLog = kLogCheck;
static_assert(std::atomic<LogLevel>::is_always_lock_free);

inline bool LogTraceEnabled() { return kEnabledLog <= kLogTrace; }
inline bool LogDebugEnabled() { return kEnabledLog <= kLogDebug; }
inline bool LogInfoEnabled() { return kEnabledLog <= kLogInfo; }
inline bool LogCheckEnabled() { return true; }
inline void LogEnable(LogLevel level) { kEnabledLog = level; }

}  // namespace apollo

#define TLOG()                    \
  !(::apollo::LogTraceEnabled())  \
      ? (void)0                   \
      : ::apollo::VoidMessage() & \
            ::apollo::TraceLogMessage(__PRETTY_FUNCTION__, __LINE__).Stream()

#define DCHECK(cond)                                                        \
  (!::apollo::LogDebugEnabled() || (cond))                                  \
      ? (void)0                                                             \
      : ::apollo::VoidMessage() &                                           \
            ::apollo::CheckLogMessage(__PRETTY_FUNCTION__, __LINE__, #cond) \
                .Stream()

#define DLOG()                    \
  !(::apollo::LogDebugEnabled())  \
      ? (void)0                   \
      : ::apollo::VoidMessage() & \
            ::apollo::DebugLogMessage(__PRETTY_FUNCTION__, __LINE__).Stream()

#define LOG()                     \
  !(::apollo::LogInfoEnabled())   \
      ? (void)0                   \
      : ::apollo::VoidMessage() & \
            ::apollo::InfoLogMessage(__PRETTY_FUNCTION__, __LINE__).Stream()

#define CHECK(cond)                                                         \
  (!::apollo::LogCheckEnabled() || (cond))                                  \
      ? (void)0                                                             \
      : ::apollo::VoidMessage() &                                           \
            ::apollo::CheckLogMessage(__PRETTY_FUNCTION__, __LINE__, #cond) \
                .Stream()
