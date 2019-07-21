#include <mods-quickjs/common.hpp>
#include <mods-quickjs/quickjs.hpp>
#include <cstring>
#include <unordered_map>

#include "MyQuerySystem.hpp"

// * common global variable
JSRuntime *js_runtime;
JSContext *js_context;
ScriptApi::QuickJSInterface *ifce;
MinecraftServerScriptEngine *scriptengine;
std::vector<std::function<void(JSValue const &, JSValue const &)>> quickjs_extras;
std::vector<std::function<void(JSValue const &)>> quickjs_preloads;
std::vector<std::function<void(JSValue const &)>> quickjs_proto_extras;
std::vector<std::function<void()>> init_hooks;

// * Core Object
template class std::vector<ScriptApi::ScriptObjectHandle>;
template class std::vector<ScriptApi::EventTracking>;
template class std::unordered_map<std::string, std::vector<ScriptApi::EventTracking>>;

// * BinderComponent
// clang-format off
template std::unique_ptr<ScriptEventDataBinderComponent> std::make_unique<ScriptEventDataBinderComponent, ScriptApi::ScriptObjectHandle>(ScriptApi::ScriptObjectHandle &&);

template std::unique_ptr<ScriptComponentBinderComponent> std::make_unique<ScriptComponentBinderComponent, ScriptApi::ScriptObjectHandle>(ScriptApi::ScriptObjectHandle &&);
template std::unique_ptr<ScriptComponentBinderComponent> std::make_unique<ScriptComponentBinderComponent>();

template std::unique_ptr<ScriptTickingAreaBinderComponent> std::make_unique<ScriptTickingAreaBinderComponent, ScriptApi::ScriptObjectHandle>(ScriptApi::ScriptObjectHandle &&);
template std::unique_ptr<ScriptTickingAreaBinderComponent> std::make_unique<ScriptTickingAreaBinderComponent>();

template std::unique_ptr<ScriptBlockPositionBinderComponent> std::make_unique<ScriptBlockPositionBinderComponent, BlockPos const &>(BlockPos const &);
template std::unique_ptr<ScriptBlockPositionBinderComponent> std::make_unique<ScriptBlockPositionBinderComponent>();
// clang-format on

// ! Path for Queries
MakeSingleInstance(MyQuerySystem);

// * Symbol patch
extern "C" {
bool DO_PATCH(char const *name) {
  return name && name[0] == '_' &&
         (strstr(name, "ScriptObjectHandle") || strstr(name, "ScriptFramework") || strstr(name, "EventTracking") || strstr(name, "BinderComponent"));
}
}