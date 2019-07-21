#pragma once

#include <cstdarg>

extern "C" {

typedef enum { MODLOADER_LOG_TRACE, MODLOADER_LOG_DEBUG, MODLOADER_LOG_INFO, MODLOADER_LOG_WARN, MODLOADER_LOG_ERROR } modloader_log_level;

void modloader_log(modloader_log_level level, const char *tag, const char *format, ...) __attribute__((format(printf, 3, 4)));
void modloader_vlog(modloader_log_level level, const char *tag, const char *format, va_list args);

const char *modloader_log_level_str(modloader_log_level level);

void modloader_logv(const char *tag, const char *format, ...) __attribute__((format(printf, 2, 3)));
void modloader_logd(const char *tag, const char *format, ...) __attribute__((format(printf, 2, 3)));
void modloader_logi(const char *tag, const char *format, ...) __attribute__((format(printf, 2, 3)));
void modloader_logw(const char *tag, const char *format, ...) __attribute__((format(printf, 2, 3)));
void modloader_loge(const char *tag, const char *format, ...) __attribute__((format(printf, 2, 3)));
}

class Log {

public:
  using LogLevel = modloader_log_level;

  static const char *getLogLevelString(LogLevel level) { return modloader_log_level_str(level); }

  static void vlog(LogLevel level, const char *tag, const char *text, va_list args) { modloader_vlog(level, tag, text, args); }

  static void log(LogLevel level, const char *tag, const char *text, ...) __attribute__((format(printf, 3, 4)));

  static void verbose(const char *tag, const char *text, ...) __attribute__((format(printf, 2, 3)));
  static void debug(const char *tag, const char *text, ...) __attribute__((format(printf, 2, 3)));
  static void info(const char *tag, const char *text, ...) __attribute__((format(printf, 2, 3)));
  static void warn(const char *tag, const char *text, ...) __attribute__((format(printf, 2, 3)));
  static void error(const char *tag, const char *text, ...) __attribute__((format(printf, 2, 3)));
};