#include <mods-quickjs/quickjs.hpp>

#include <minecraft/commands/CommandContext.h>
#include <minecraft/commands/CommandOrigin.h>
#include <minecraft/commands/CommandVersion.h>
#include <minecraft/commands/MinecraftCommands.h>
#include <minecraft/core/Minecraft.h>
#include <minecraft/script/EventInfo.h>
#include <minecraft/script/ScriptBinderEventDataTemplate.h>

#include "MyScriptCommandOrigin.hpp"

THook(void, _ZN9__gnu_cxx13new_allocatorIN9ScriptApi18ScriptObjectHandleEE7destroyIS2_EEvPT_, ScriptApi::ScriptObjectHandle *self) { self->release(); }
THook(void, _ZNSt12_Destroy_auxILb1EE9__destroyIPN9ScriptApi18ScriptObjectHandleEEEvT_S5_, ScriptApi::ScriptObjectHandle *self) { self->release(); }
TClasslessInstanceHook(bool, _ZN12ScriptEngine18isScriptingEnabledEv) { return true; }
THook(ScriptApi::QuickJSInterface *, _ZSt11make_uniqueIN9ScriptApi20EmptyScriptInterfaceEJEENSt9_MakeUniqIT_E15__single_objectEDpOT0_) { return ifce = new ScriptApi::QuickJSInterface(); }
THook(void, _ZNSt10unique_ptrIN9ScriptApi23ScriptLanguageInterfaceESt14default_deleteIS1_EE5resetEPS1_) { delete ifce; }
THook(void, _ZNSt10unique_ptrIN9ScriptApi23ScriptLanguageInterfaceESt14default_deleteIS1_EED2Ev) {}
THook(void *, _ZNKSt10unique_ptrIN9ScriptApi23ScriptLanguageInterfaceESt14default_deleteIS1_EEptEv) { return ifce; }

THook(bool, _ZN12ScriptEngine15_getVersionInfoERKN9ScriptApi18ScriptObjectHandleERNS0_17ScriptVersionInfoE) { return true; }

INLINE void call_lifetime_hook(char const *name) {
  for (auto &system : ifce->systems) {
    autoval func = JS_GetPropertyStr(js_context, system, name);
    if (JS_IsFunction(js_context, func)) {
      JS_FreeValue(js_context, JS_Call(js_context, func, system, 0, nullptr));
      handle_exception(js_context, name);
    }
  }
}

TInstanceHook(void, _ZN12ScriptEngine24_processSystemInitializeEv, ScriptEngine) {
  for (auto &system : ifce->systems) {
    autoval func = JS_GetPropertyStr(js_context, system, "initialize");
    if (JS_IsFunction(js_context, func)) {
      JS_FreeValue(js_context, JS_Call(js_context, func, system, 0, nullptr));
      handle_exception(js_context, "initialize");
      autoatom f = JS_NewAtom(js_context, "initialize");
      JS_DeleteProperty(js_context, system, f, 0);
    }
  }
}
TInstanceHook(void, _ZN12ScriptEngine20_processSystemUpdateEv, ScriptEngine) { call_lifetime_hook("update"); }
TInstanceHook(void, _ZN12ScriptEngine22_processSystemShutdownEv, ScriptEngine) { call_lifetime_hook("shutdown"); }

TInstanceHook(void, _ZN12ScriptEngine22_callAllScriptCallbackERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEERKSt6vectorIN9ScriptApi18ScriptObjectHandleESaISA_EERSA_, ScriptEngine) {}

TClasslessInstanceHook(void, _ZN23ScriptEngineWithContextI19ScriptServerContextE20_processCommandQueueEv) {
  while (!ifce->commandPendingList.empty()) {
    auto &thunk = ifce->commandPendingList.front();
    auto &level = *(ServerLevel *)refs<ScriptServerContext>->level;
    auto origin = std::make_unique<MyScriptCommandOrigin>(level, *scriptengine, thunk.second);
    auto commands = refs<ScriptServerContext>->minecraft->getCommands();
    auto context = std::make_shared<CommandContext>(thunk.first, std::move(origin), CommandVersion::CurrentVersion);
    commands->executeCommand(context, false);
    ifce->commandPendingList.pop();
  }
}
TClasslessInstanceHook(void, _ZN23ScriptEngineWithContextI19ScriptServerContextE28_processCommandCallbackQueueEv) {
  JSContext *ctx;
  auto err = JS_ExecutePendingJob(js_runtime, &ctx);
  if (err < 0)
    handle_exception(ctx, "PendingJob");
}

bool ScriptEngine::initialize() {
  ScriptApi::ScriptObjectHandle handle;
  scriptengine = (MinecraftServerScriptEngine *)this;
  QCHECK(ScriptApi::ScriptFramework::initialize());
  QCHECK(registerGlobalAPI("server", interface, handle));
  QCHECK(((MinecraftServerScriptEngine *)this)->_registerSystemObjects(handle));
  scriptengine->setupInterface();
  for (auto &hook : init_hooks)
    hook();
  return false;
}

bool ScriptEngine::helpDefineVec3(ScriptApi::ScriptObjectHandle &target, const std::string &key, const Vec3 &value) {
  ScriptApi::ScriptObjectHandle temp = JS_NewArray(js_context);
  for (int i = 0; i < 3; i++) {
    JS_SetPropertyUint32(js_context, temp, i, JS_NewFloat64(js_context, value[i]));
  }
  return JS_SetPropertyStr(js_context, target, key.data(), temp);
}

bool ScriptEngine::helpGetVec3(const ScriptApi::ScriptObjectHandle &source, const std::string &key, Vec3 &value) {
  autoval temp = JS_GetPropertyStr(js_context, source, key.data());
  if (JS_IsArray(js_context, temp)) {
    for (int i = 0; i < 3; i++) {
      autoval val = JS_GetPropertyUint32(js_context, temp, i);
      if (!JS_IsNumber(val)) {
        getScriptReportQueue().addWarning("Wrong vec3");
        return false;
      }
      double raw;
      JS_ToFloat64(js_context, &raw, val);
      value[i] = raw;
    }
    return true;
  }
  getScriptReportQueue().addWarning("No vec3");
  return false;
}

bool ScriptEngine::serializeJsonToScriptObjectHandle(ScriptApi::ScriptObjectHandle &handle, Json::Value const &json) {
  handle.release();
  switch (json.type()) {
  case Json::nullValue:
    handle = JS_NULL;
    break;
  case Json::intValue:
    [[fallthrough]];
  case Json::uintValue:
    handle = JS_NewInt64(js_context, json.asInt64(-1));
    break;
  case Json::realValue:
    handle = JS_NewFloat64(js_context, json.asDouble(0.0));
    break;
  case Json::booleanValue:
    handle = JS_NewBool(js_context, json.asBool(false));
    break;
  case Json::stringValue: {
    auto temp = json.asString("");
    handle = JS_NewStringLen(js_context, temp.data(), temp.length());
    break;
  }
  case Json::objectValue: {
    handle = JS_NewObject(js_context);
    auto end = json.end();
    for (auto it = json.begin(); it != end; ++it) {
      ScriptApi::ScriptObjectHandle value;
      serializeJsonToScriptObjectHandle(value, *it);
      JS_SetPropertyStr(js_context, handle, it.key().asCString(), value);
    }
    break;
  }
  case Json::arrayValue: {
    handle = JS_NewArray(js_context);
    JS_SetPropertyStr(js_context, handle, "length", JS_NewInt32(js_context, json.size()));
    int64_t idx = 0;
    for (auto &it : json) {
      ScriptApi::ScriptObjectHandle value;
      serializeJsonToScriptObjectHandle(value, it);
      JS_SetPropertyInt64(js_context, handle, idx++, value);
    }
    break;
  }
  }
  return true;
}

bool ScriptEngine::deserializeScriptObjectHandleToJson(ScriptApi::ScriptObjectHandle const &handle, Json::Value &json) {
  ScriptApi::ScriptObjectType type;
  QCHECK(getHandleType(handle, type));
  switch (type) {
  case ScriptApi::ScriptObjectType::T_UNDEFINED:
    [[fallthrough]];
  case ScriptApi::ScriptObjectType::T_NULL:
    json = Json::Value::null;
    break;
  case ScriptApi::ScriptObjectType::T_BOOL:
    json = Json::Value(JS_ToBool(js_context, handle));
    break;
  case ScriptApi::ScriptObjectType::T_NUMBER: {
    if (JS_IsInteger(handle)) {
      int64_t temp = 0;
      JS_ToInt64(js_context, &temp, handle);
      if (temp >= INT32_MIN && temp <= INT32_MAX)
        json = Json::Value((int32_t)temp);
      else
        json = Json::Value((double)temp);
    } else {
      double temp = 0.0;
      JS_ToFloat64(js_context, &temp, handle);
      json = Json::Value(temp);
    }
    break;
  }
  case ScriptApi::ScriptObjectType::T_STRING: {
    int len;
    autostr str = JS_ToCStringLen(js_context, &len, handle, false);
    json = Json::Value(std::string{str, (size_t)len});
    break;
  }
  case ScriptApi::ScriptObjectType::T_OBJECT: {
    autoval global = JS_GetGlobalObject(js_context);
    autoval reflect = JS_GetPropertyStr(js_context, global, "Reflect");
    autoval ownKeys = JS_GetPropertyStr(js_context, reflect, "ownKeys");
    autoval arr = JS_Call(js_context, ownKeys, reflect, 1, (JSValue *)&handle);
    HANDLE_EXCEPTION();
    autoval propLen = JS_GetPropertyStr(js_context, arr, "length");
    json = Json::Value(Json::objectValue);
    for (int32_t i = 0; i < JS_VALUE_GET_INT(propLen); i++) {
      Json::Value temp;
      autoval key = JS_GetPropertyUint32(js_context, arr, i);
      if (JS_IsString(key)) {
        int keyLen = 0;
        autostr keyStr = JS_ToCStringLen(js_context, &keyLen, key, false);
        autoval value = JS_GetPropertyStr(js_context, handle, keyStr);
        Json::Value val_temp;
        deserializeScriptObjectHandleToJson(value, val_temp);
        json[keyStr] = val_temp;
      }
    }
    break;
  }
  case ScriptApi::ScriptObjectType::T_ARRAY: {
    autoval propLen = JS_GetPropertyStr(js_context, handle, "length");
    json = Json::Value(Json::arrayValue);
    for (int32_t i = 0; i < JS_VALUE_GET_INT(propLen); i++) {
      autoval value = JS_GetPropertyUint32(js_context, handle, i);
      Json::Value val_temp;
      deserializeScriptObjectHandleToJson(value, val_temp);
      json[i] = val_temp;
    }
    break;
  }
  }
  return true;
}

std::unordered_map<std::string, EventInfo> &ScriptEngine::getEventInfoMap() { return *union_cast<std::unordered_map<std::string, EventInfo> *>(union_cast<int>(this) + 344); }

std::unordered_map<std::string, std::vector<ScriptApi::EventTracking>> &ScriptEngine::getEventTrackings() {
  return *union_cast<std::unordered_map<std::string, std::vector<ScriptApi::EventTracking>> *>(union_cast<int>(this) + 56);
}

bool ScriptEngine::fireEventToScript(EventInfo const &info, ScriptApi::ScriptObjectHandle &&obj) {
  autohandle handle = JS_NewObject(js_context);
  ScriptBinderEventDataTemplate::build(info.data, obj.value)->serialize(*scriptengine, handle);
  auto it = getEventTrackings().find(info.data);
  if (it != getEventTrackings().end()) {
    for (auto &item : it->second) {
      JS_FreeValue(js_context, JS_Call(js_context, item, JS_UNDEFINED, 1, &handle.value));
      handle_exception(js_context, "fireEventToScript");
    }
  }
  return true;
}