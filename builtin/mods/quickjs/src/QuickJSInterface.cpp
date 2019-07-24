#include <mods-quickjs/common.hpp>
#include <mods-quickjs/quickjs.hpp>

namespace ScriptApi {

QuickJSInterface::QuickJSInterface() : inited(false) {}
QuickJSInterface::~QuickJSInterface() {}
bool QuickJSInterface::initialize(ScriptReport &) {
  inited = true;
  Log::debug("QuickJS", "initialize");
  js_runtime = JS_NewRuntime();
  js_context = JS_NewContext(js_runtime);
  JS_NewClassID(&system_class);
  static JSClassDef system_def = {"MinecraftServerSystem"};
  JS_NewClass(js_runtime, system_class, &system_def);
  auto proto = JS_NewObject(js_context);
  JS_SetPropertyFunctionList(js_context, proto, &systemEntries[0], systemEntries.size());
  for (auto addition : quickjs_proto_extras)
    addition(proto);
  JS_SetClassProto(js_context, system_class, proto);
  autoval global = JS_GetGlobalObject(js_context);
  for (auto &preload : quickjs_preloads)
    preload(global);
  JS_SetPropertyStr(js_context, global, "globalThis", global);
  global = JS_UNDEFINED;
  return true;
}
bool QuickJSInterface::shutdown(ScriptReport &) {
  JS_FreeContext(js_context);
  Log::debug("QuickJS", "shutdown");
  inited = false;
  return true;
}
bool QuickJSInterface::initialized() { return inited; }
bool QuickJSInterface::runScript(std::string const &filename, std::string const &data, ScriptReport &report) {
  Log::debug("QuickJS", "executing %s", filename.data());
  ((void **)js_context)[3] = __builtin_frame_address(0); // FIXME
  JS_Eval(js_context, data.c_str(), data.size(), filename.c_str(), JS_EVAL_TYPE_GLOBAL);
  HANDLE_EXCEPTION();
  return true;
}
bool QuickJSInterface::createObject(ScriptObjectHandle &handle, ScriptReport &) {
  handle = JS_NewObject(js_context);
  return true;
}
bool QuickJSInterface::createArray(ScriptObjectHandle &handle, int const &len, ScriptReport &) {
  auto arr = JS_NewArray(js_context);
  JS_SetPropertyStr(js_context, arr, "length", JS_NewInt32(js_context, len));
  handle = arr;
  return true;
}
bool QuickJSInterface::cloneObject(ScriptObjectHandle const &src, ScriptObjectHandle &dst, ScriptReport &report) {
  Log::debug("QuickJS", "clone object");
  if (src)
    dst = JS_DupValue(js_context, src);
  else {
    report.addError("Try to clone null ptr");
    return false;
  }
  return true;
}
bool QuickJSInterface::hasMember(ScriptObjectHandle const &src, std::string const &name, bool &result, ScriptReport &report) {
  if (!src) {
    report.addError("Try to use null ptr");
    return false;
  }
  auto natom = JS_NewAtom(js_context, name.data());
  result = JS_HasProperty(js_context, src, natom);
  JS_FreeAtom(js_context, natom);
  return true;
}
bool QuickJSInterface::hasMember(ScriptObjectHandle const &src, int const &idx, bool &result, ScriptReport &report) {
  if (!src) {
    report.addError("Try to use null ptr");
    return false;
  }
  autoatom natom = JS_NewAtomUInt32(js_context, idx);
  result = JS_HasProperty(js_context, src, natom);
  HANDLE_EXCEPTION();
  return true;
}
bool QuickJSInterface::setMember(ScriptObjectHandle const &src, std::string const &name, ScriptObjectHandle const &value, ScriptReport &report) {
  if (!src) {
    report.addError("Try to use null ptr");
    return false;
  }
  JS_SetPropertyStr(js_context, src, name.data(), value);
  HANDLE_EXCEPTION();
  return true;
}
bool QuickJSInterface::setMember(ScriptObjectHandle const &src, int const &idx, ScriptObjectHandle const &value, ScriptReport &report) {
  JS_SetPropertyInt64(js_context, src, idx, value);
  HANDLE_EXCEPTION();
  return true;
}
bool QuickJSInterface::getMember(ScriptObjectHandle const &src, std::string const &name, ScriptObjectHandle &result, ScriptReport &report) {
  result = JS_GetPropertyStr(js_context, src, name.c_str());
  HANDLE_EXCEPTION();
  return true;
}
bool QuickJSInterface::getMember(ScriptObjectHandle const &src, int const &idx, ScriptObjectHandle &result, ScriptReport &report) {
  result = JS_GetPropertyUint32(js_context, src, idx);
  HANDLE_EXCEPTION();
  return true;
}
bool QuickJSInterface::setValue(ScriptObjectHandle &src, int value, ScriptReport &) {
  src = JS_NewInt64(js_context, value);
  return true;
}
bool QuickJSInterface::setValue(ScriptObjectHandle &src, double value, ScriptReport &) {
  src = JS_NewFloat64(js_context, value);
  return true;
}
bool QuickJSInterface::setValue(ScriptObjectHandle &src, std::string const &value, ScriptReport &) {
  src = JS_NewStringLen(js_context, value.data(), value.size());
  return true;
}
bool QuickJSInterface::setValue(ScriptObjectHandle &src, bool value, ScriptReport &) {
  src = JS_NewBool(js_context, value);
  return true;
}
bool QuickJSInterface::getValue(ScriptObjectHandle const &src, int &result, ScriptReport &report) {
  auto ret = JS_ToInt32(js_context, &result, src);
  if (ret != 0) {
    report.addError("Target value is not integer (" + std::to_string(JS_VALUE_GET_TAG(src.value)) + ")");
  }
  return ret == 0;
}
bool QuickJSInterface::getValue(ScriptObjectHandle const &src, double &result, ScriptReport &report) {
  auto ret = JS_ToFloat64(js_context, &result, src);
  if (ret != 0) {
    report.addError("Target value is not float");
  }
  return ret == 0;
}
bool QuickJSInterface::getValue(ScriptObjectHandle const &src, std::string &result, ScriptReport &report) {
  int len = 0;
  autostr ret = JS_ToCStringLen(js_context, &len, src, false);
  if (ret) {
    result = {ret, (size_t)len};
  } else {
    report.addError("Target value is not string");
  }
  return ret;
}
bool QuickJSInterface::getValue(ScriptObjectHandle const &src, bool &result, ScriptReport &report) {
  auto ret = JS_ToBool(js_context, src);
  if (ret == -1) {
    report.addError("Target value cannot convert to bool");
    return false;
  }
  result = ret;
  return true;
}
bool QuickJSInterface::callObjectFunction(ScriptObjectHandle const &src, std::string const &name, std::vector<ScriptObjectHandle> const &args, ScriptObjectHandle &result, ScriptReport &report) {
  autoval func = JS_GetPropertyStr(js_context, src, name.data());
  result = JS_Call(js_context, func, src, args.size(), (JSValue *)&args[0]);
  HANDLE_EXCEPTION();
  return true;
}
bool QuickJSInterface::callGlobalFunction(ScriptObjectHandle const &src, std::vector<ScriptObjectHandle> const &args, ScriptObjectHandle &result, ScriptReport &report) {
  Log::debug("QuickJS", "call");
  autoval global = JS_GetGlobalObject(js_context);
  result = JS_Call(js_context, src, global, args.size(), (JSValue *)&args[0]);
  HANDLE_EXCEPTION();
  return true;
}
bool QuickJSInterface::getHandleType(ScriptObjectHandle const &src, ScriptObjectType &result, ScriptReport &report) {
  if (JS_IsUndefined(src))
    result = ScriptObjectType::T_UNDEFINED;
  else if (JS_IsNull(src))
    result = ScriptObjectType::T_NULL;
  else if (JS_IsNumber(src))
    result = ScriptObjectType::T_NUMBER;
  else if (JS_IsString(src))
    result = ScriptObjectType::T_STRING;
  else if (JS_IsBool(src))
    result = ScriptObjectType::T_BOOL;
  else if (JS_IsArray(js_context, src))
    result = ScriptObjectType::T_ARRAY;
  else if (JS_IsObject(src))
    result = ScriptObjectType::T_OBJECT;
  else {
    report.addError("Unknown type");
    return false;
  }
  return true;
}
bool QuickJSInterface::getMemberNames(ScriptObjectHandle const &src, std::vector<std::string> &result, ScriptReport &report) {
  autoval global = JS_GetGlobalObject(js_context);
  autoval reflect = JS_GetPropertyStr(js_context, global, "Reflect");
  autoval ownKeys = JS_GetPropertyStr(js_context, reflect, "ownKeys");
  autoval arr = JS_Call(js_context, ownKeys, reflect, 1, (JSValue *)&src);
  HANDLE_EXCEPTION();
  autoval propLen = JS_GetPropertyStr(js_context, arr, "length");
  result.clear();
  for (int32_t i = 0; i < JS_VALUE_GET_INT(propLen); i++) {
    autoval key = JS_GetPropertyUint32(js_context, arr, i);
    if (JS_IsString(key)) {
      int keyLen = 0;
      autostr keyStr = JS_ToCStringLen(js_context, &keyLen, key, false);
      result.emplace_back(keyStr, (size_t)keyLen);
    }
  }
  HANDLE_EXCEPTION();
  return true;
}
bool QuickJSInterface::getArrayLength(ScriptObjectHandle const &src, int &result, ScriptReport &report) {
  ScriptObjectHandle handle;
  if (!getMember(src, "length", handle, report))
    return false;
  return getValue(handle, result, report);
}
bool QuickJSInterface::getGlobalObject(ScriptObjectHandle &result, ScriptReport &) {
  result = JS_GetGlobalObject(js_context);
  return true;
}
bool QuickJSInterface::createUndefined(ScriptObjectHandle &result, ScriptReport &) {
  result = JS_UNDEFINED;
  return true;
}
bool QuickJSInterface::createNull(ScriptObjectHandle &result, ScriptReport &) {
  result = JS_NULL;
  return true;
}
bool QuickJSInterface::defineGlobalCallbacks(ScriptObjectHandle const &, ScriptCallbackInterface &, ScriptReport &) { return false; }
bool QuickJSInterface::defineSystemSharedCallbacks(ScriptObjectHandle const &, ScriptCallbackInterface &, ScriptReport &) { return false; }
bool QuickJSInterface::defineSystemServerCallbacks(ScriptObjectHandle const &, ScriptCallbackInterface &, ScriptReport &) { return false; }
bool QuickJSInterface::defineSystemClientCallbacks(ScriptObjectHandle const &, ScriptCallbackInterface &, ScriptReport &) { return false; }

std::vector<JSCFunctionListEntry> QuickJSInterface::serverEntries = {
    JS_CFUNC_DEF("registerSystem", 2, QuickJSInterface::processRegisterSystem),
    JS_CFUNC_DEF("log", 0, QuickJSInterface::processLog),
};
std::vector<JSCFunctionListEntry> QuickJSInterface::systemEntries = {
    JS_CFUNC_DEF("registerEventData", 2, QuickJSInterface::processRegisterEventData),
    JS_CFUNC_DEF("createEventData", 1, QuickJSInterface::processCreateEventData),
    JS_CFUNC_DEF("listenForEvent", 2, QuickJSInterface::processListenForEvent),
    JS_CFUNC_DEF("broadcastEvent", 2, QuickJSInterface::processBroadcastEvent),

    JS_CFUNC_DEF("createEntity", 0, QuickJSInterface::processCreateEntity),
    JS_CFUNC_DEF("destroyEntity", 1, QuickJSInterface::processDestroyEntity),
    JS_CFUNC_DEF("isValidEntity", 1, QuickJSInterface::processIsValidEntity),

    JS_CFUNC_DEF("registerComponent", 2, QuickJSInterface::processRegisterComponent),
    JS_CFUNC_DEF("createComponent", 2, QuickJSInterface::processCreateComponent),
    JS_CFUNC_DEF("hasComponent", 2, QuickJSInterface::processHasComponent),
    JS_CFUNC_DEF("getComponent", 2, QuickJSInterface::processGetComponent),
    JS_CFUNC_DEF("applyComponentChanges", 2, QuickJSInterface::processApplyComponentChanges),
    JS_CFUNC_DEF("destroyComponent", 2, QuickJSInterface::processDestroyChanges),

    JS_CFUNC_DEF("registerQuery", 0, QuickJSInterface::processRegisterQuery),
    JS_CFUNC_DEF("addFilterToQuery", 2, QuickJSInterface::processAddFilterToQuery),
    JS_CFUNC_DEF("getEntitiesFromQuery", 1, QuickJSInterface::processGetEntitiesFromQuery),

    JS_CFUNC_DEF("getBlock", 2, QuickJSInterface::processGetBlock),
    JS_CFUNC_DEF("getBlocks", 3, QuickJSInterface::processGetBlocks),

    JS_CFUNC_DEF("executeCommand", 2, QuickJSInterface::processExecuteCommand),
};

JSClassID QuickJSInterface::system_class;

bool QuickJSInterface::defineServerCallbacks(ScriptObjectHandle const &obj, ScriptReport &report) {
  JS_SetPropertyFunctionList(js_context, obj, &serverEntries[0], serverEntries.size());
  return true;
}

} // namespace ScriptApi