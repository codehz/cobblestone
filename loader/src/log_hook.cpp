#include <bitset>
#include <cstring>
#include <modloader/hook.hpp>
#include <modloader/log.hpp>

THook(
    void,
    _ZN10BedrockLog9createLogERKN4Core4PathERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEESB_NS_11LogCategoryESt6bitsetILm3EEbP18LogSettingsUpdaterd) {
}

using LogAreaID = unsigned int;

extern "C" {
const char *_ZN10BedrockLog17_areaFilterStringE9LogAreaID(LogAreaID);
}

class BedrockLog {};

TStaticHook(void, _ZN10BedrockLog6log_vaENS_11LogCategoryESt6bitsetILm3EENS_7LogRuleE9LogAreaIDjPKciS6_P13__va_list_tag, BedrockLog,
            unsigned int cate, std::bitset<3> unk, int rule, unsigned int area, unsigned int level, char const *tag, int tid, char const *format,
            va_list args) {
  Log::LogLevel ourLevel = MODLOADER_LOG_ERROR;
  if (level == 1) ourLevel = MODLOADER_LOG_TRACE;
  if (level == 2) ourLevel = MODLOADER_LOG_INFO;
  if (level == 4) ourLevel = MODLOADER_LOG_WARN;
  if (level == 8) ourLevel = MODLOADER_LOG_ERROR;
  std::string ourTag = _ZN10BedrockLog17_areaFilterStringE9LogAreaID(area);
  ourTag += '/';
  ourTag += tag;
  Log::vlog(ourLevel, ourTag.c_str(), format, args);
}