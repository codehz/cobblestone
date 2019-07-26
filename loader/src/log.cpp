#include "../include/modloader/log.hpp"

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <set>

const char *modloader_log_level_str(modloader_log_level level) {
  if (level == MODLOADER_LOG_TRACE)
    return "T";
  if (level == MODLOADER_LOG_DEBUG)
    return "D";
  if (level == MODLOADER_LOG_INFO)
    return "I";
  if (level == MODLOADER_LOG_WARN)
    return "W";
  if (level == MODLOADER_LOG_ERROR)
    return "E";
  return "?";
}

static std::set<void (*)(modloader_log_level level, const char *tag, const char *text)> hooks;

void modloader_log_hook(void (*fn)(modloader_log_level level, const char *tag, const char *text)) { hooks.emplace(fn); }

void modloader_vlog(modloader_log_level level, const char *tag, const char *format, va_list args) {
  constexpr size_t buffer_size = 1048576;
  static char *buffer = new char[buffer_size];
  size_t len = vsnprintf(buffer, buffer_size, format, args);
  if (len > buffer_size)
    len = buffer_size;
  while (len > 0 && (buffer[len - 1] == '\r' || buffer[len - 1] == '\n'))
    buffer[--len] = '\0';

  for (auto it : hooks)
    it(level, tag, buffer);

  char tbuf[128];
  tbuf[0] = '\0';

  time_t t = time(nullptr);
  tm tm;
  localtime_r(&t, &tm);
  strftime(tbuf, sizeof(tbuf), "%H:%M:%S", &tm);
  printf("%s %s [%s] %s\n", tbuf, modloader_log_level_str(level), tag, buffer);
}

void modloader_log(modloader_log_level level, const char *tag, const char *format, ...) {
  va_list args;
  va_start(args, format);
  modloader_vlog(level, tag, format, args);
  va_end(args);
}

void Log::log(LogLevel level, const char *tag, const char *format, ...) {
  va_list args;
  va_start(args, format);
  modloader_vlog(level, tag, format, args);
  va_end(args);
}

#define LogFuncDef(name, logLevel)                                                                                                                                                                     \
  void name(const char *tag, const char *format, ...) {                                                                                                                                                \
    va_list args;                                                                                                                                                                                      \
    va_start(args, format);                                                                                                                                                                            \
    modloader_vlog(logLevel, tag, format, args);                                                                                                                                                       \
    va_end(args);                                                                                                                                                                                      \
  }

LogFuncDef(modloader_logv, MODLOADER_LOG_TRACE);
LogFuncDef(modloader_logd, MODLOADER_LOG_DEBUG);
LogFuncDef(modloader_logi, MODLOADER_LOG_INFO);
LogFuncDef(modloader_logw, MODLOADER_LOG_WARN);
LogFuncDef(modloader_loge, MODLOADER_LOG_ERROR);

LogFuncDef(Log::verbose, MODLOADER_LOG_TRACE);
LogFuncDef(Log::debug, MODLOADER_LOG_DEBUG);
LogFuncDef(Log::info, MODLOADER_LOG_INFO);
LogFuncDef(Log::warn, MODLOADER_LOG_WARN);
LogFuncDef(Log::error, MODLOADER_LOG_ERROR)