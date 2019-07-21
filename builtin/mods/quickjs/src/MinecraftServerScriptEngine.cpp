#include <mods-quickjs/quickjs.hpp>

TClasslessInstanceHook(void, _ZN27MinecraftServerScriptEngine12_handleErrorERKN9ScriptApi16ScriptReportItemE, ScriptApi::ScriptReportItem &item) {
  original(this, item);
  Log::error("QuickJS/report", "%s", item.getMessage().data());
}

TClasslessInstanceHook(void, _ZN27MinecraftServerScriptEngine14_handleWarningERKN9ScriptApi16ScriptReportItemE, ScriptApi::ScriptReportItem &item) {
  original(this, item);
  Log::warn("QuickJS/report", "%s", item.getMessage().data());
}

bool MinecraftServerScriptEngine::_registerSystemObjects(ScriptApi::ScriptObjectHandle const &src) {
  ScriptApi::ScriptObjectHandle level;
  QCHECK(createObject(level));
  QCHECK(helpDefineLevel(level));
  return setMember(src, "level", level);
}