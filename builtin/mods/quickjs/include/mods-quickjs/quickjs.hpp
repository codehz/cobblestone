#pragma once
#include <deque>
#include <functional>
#include <memory>
#include <minecraft/json.h>
#include <modloader/hook.hpp>
#include <modloader/utils.hpp>
#include <queue>
#include <string>
#include <vector>

#include <minecraft/script/ScriptBinderComponent.h>
#include <minecraft/script/ScriptCallbackInterface.h>
#include <minecraft/script/ScriptCommandCallbackData.h>
#include <minecraft/script/ScriptEngine.h>
#include <minecraft/script/ScriptEngineWithContext.h>
#include <minecraft/script/ScriptFramework.h>
#include <minecraft/script/ScriptObjectBinder.h>
#include <minecraft/script/ScriptObjectType.h>
#include <minecraft/script/ScriptReport.h>
#include <minecraft/script/ScriptReportItem.h>
#include <minecraft/script/ScriptReportItemType.h>
#include <minecraft/script/ScriptServerContext.h>

#include "common.hpp"

namespace ScriptApi {
class JavaScriptErrorHandler;
class ScriptObjectHandle {
public:
  JSValue value;
  inline ScriptObjectHandle()
      : value(JS_UNDEFINED) {}
  inline ScriptObjectHandle(JSValue &&rhs)
      : value(rhs) {}
  inline ScriptObjectHandle(JSValue const &rhs)
      : value(JS_DupValue(js_context, rhs)) {}
  inline ScriptObjectHandle(ScriptObjectHandle const &rhs)
      : value(rhs.value) {}
  inline void release() const { JS_FreeValue(js_context, value); }
  inline void release() {
    JS_FreeValue(js_context, value);
    value = JS_UNDEFINED;
  }
  inline ScriptObjectHandle &operator=(JSValue &&val) {
    release();
    value = val;
    return *this;
  }
  inline ScriptObjectHandle &operator=(ScriptObjectHandle const &rhs) {
    release();
    value = JS_DupValue(js_context, rhs.value);
    return *this;
  }
  inline operator JSValue &() { return this->value; }
  inline operator JSValue const &() const { return this->value; }
  inline JSValue transfer() {
    auto ret    = this->value;
    this->value = JS_UNDEFINED;
    return ret;
  }
  inline operator bool() const noexcept { return !JS_IsUndefined(value) && !JS_IsNull(value); }
  bool isEmpty() const;
};

class EventTracking : public ScriptObjectHandle {
public:
  inline EventTracking(ScriptObjectHandle &&obj)
      : ScriptObjectHandle(std::move(obj)) {}
  inline EventTracking()
      : ScriptObjectHandle() {}
};

class QuickJSInterface {
  bool inited;
  ScriptCallbackInterface *callbacks;
  friend class ScriptFramework;

public:
  QuickJSInterface();
  virtual ~QuickJSInterface();
  virtual bool initialize(ScriptReport &);
  virtual bool shutdown(ScriptReport &);
  virtual bool initialized();
  virtual bool runScript(std::string const &, std::string const &, ScriptReport &);
  virtual bool createObject(ScriptObjectHandle &, ScriptReport &);
  virtual bool createArray(ScriptObjectHandle &, int const &, ScriptReport &);
  virtual bool cloneObject(ScriptObjectHandle const &, ScriptObjectHandle &, ScriptReport &);
  virtual bool hasMember(ScriptObjectHandle const &, std::string const &, bool &, ScriptReport &);
  virtual bool hasMember(ScriptObjectHandle const &, int const &, bool &, ScriptReport &);
  virtual bool setMember(ScriptObjectHandle const &, std::string const &, ScriptObjectHandle const &, ScriptReport &);
  virtual bool setMember(ScriptObjectHandle const &, int const &, ScriptObjectHandle const &, ScriptReport &);
  virtual bool getMember(ScriptObjectHandle const &, std::string const &, ScriptObjectHandle &, ScriptReport &);
  virtual bool getMember(ScriptObjectHandle const &, int const &, ScriptObjectHandle &, ScriptReport &);
  virtual bool setValue(ScriptObjectHandle &, int, ScriptReport &);
  virtual bool setValue(ScriptObjectHandle &, double, ScriptReport &);
  virtual bool setValue(ScriptObjectHandle &, std::string const &, ScriptReport &);
  virtual bool setValue(ScriptObjectHandle &, bool, ScriptReport &);
  virtual bool getValue(ScriptObjectHandle const &, int &, ScriptReport &);
  virtual bool getValue(ScriptObjectHandle const &, double &, ScriptReport &);
  virtual bool getValue(ScriptObjectHandle const &, std::string &, ScriptReport &);
  virtual bool getValue(ScriptObjectHandle const &, bool &, ScriptReport &);
  virtual bool callObjectFunction(ScriptObjectHandle const &, std::string const &, std::vector<ScriptObjectHandle> const &, ScriptObjectHandle &,
                                  ScriptReport &);
  virtual bool callGlobalFunction(ScriptObjectHandle const &, std::vector<ScriptObjectHandle> const &, ScriptObjectHandle &, ScriptReport &);
  virtual bool getHandleType(ScriptObjectHandle const &, ScriptObjectType &, ScriptReport &);
  virtual bool getMemberNames(ScriptObjectHandle const &, std::vector<std::string> &, ScriptReport &);
  virtual bool getArrayLength(ScriptObjectHandle const &, int &, ScriptReport &);
  virtual bool getGlobalObject(ScriptObjectHandle &, ScriptReport &);
  virtual bool createUndefined(ScriptObjectHandle &, ScriptReport &);
  virtual bool createNull(ScriptObjectHandle &, ScriptReport &);
  virtual bool defineGlobalCallbacks(ScriptObjectHandle const &, ScriptCallbackInterface &, ScriptReport &);
  virtual bool defineSystemSharedCallbacks(ScriptObjectHandle const &, ScriptCallbackInterface &, ScriptReport &);
  virtual bool defineSystemServerCallbacks(ScriptObjectHandle const &, ScriptCallbackInterface &, ScriptReport &);
  virtual bool defineSystemClientCallbacks(ScriptObjectHandle const &, ScriptCallbackInterface &, ScriptReport &);

  std::vector<ScriptObjectHandle> systems;
  std::queue<std::pair<std::string, std::function<void(Json::Value &&)>>> commandPendingList;

private:
  bool defineServerCallbacks(ScriptObjectHandle const &, ScriptReport &) ABITAG(quickjs);

  static JSClassID system_class;

  static JSValue processRegisterSystem(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ABITAG(quickjs);
  static JSValue processLog(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ABITAG(quickjs);

  static JSValue processRegisterEventData(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ABITAG(quickjs);
  static JSValue processCreateEventData(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ABITAG(quickjs);
  static JSValue processListenForEvent(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ABITAG(quickjs);
  static JSValue processBroadcastEvent(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ABITAG(quickjs);
  static JSValue processCreateEntity(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ABITAG(quickjs);
  static JSValue processDestroyEntity(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ABITAG(quickjs);
  static JSValue processIsValidEntity(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ABITAG(quickjs);
  static JSValue processRegisterComponent(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ABITAG(quickjs);
  static JSValue processCreateComponent(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ABITAG(quickjs);
  static JSValue processHasComponent(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ABITAG(quickjs);
  static JSValue processGetComponent(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ABITAG(quickjs);
  static JSValue processApplyComponentChanges(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ABITAG(quickjs);
  static JSValue processDestroyChanges(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ABITAG(quickjs);
  static JSValue processRegisterQuery(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ABITAG(quickjs);
  static JSValue processAddFilterToQuery(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ABITAG(quickjs);
  static JSValue processGetEntitiesFromQuery(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ABITAG(quickjs);
  static JSValue processGetBlock(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ABITAG(quickjs);
  static JSValue processGetBlocks(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ABITAG(quickjs);
  static JSValue processExecuteCommand(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) ABITAG(quickjs);

  static std::vector<JSCFunctionListEntry> serverEntries ABITAG(quickjs);
  static std::vector<JSCFunctionListEntry> systemEntries ABITAG(quickjs);
};
}; // namespace ScriptApi

class MinecraftServerScriptEngine : public ScriptEngineWithContext<ScriptServerContext> {
public:
  bool _registerSystemObjects(ScriptApi::ScriptObjectHandle const &) PATCHABLE;
  void setupInterface();
};

static_assert(offsetof(ScriptObjectBinder, components) == 40);

// ~ BinderComponents.cpp

class ScriptEventDataBinderComponent : public ScriptBinderComponent {
  ScriptApi::ScriptObjectHandle handle;
  static std::string TAG;

public:
  ScriptEventDataBinderComponent(ScriptApi::ScriptObjectHandle &&) PATCHABLE;
  virtual ~ScriptEventDataBinderComponent() PATCHABLE;
  virtual bool serialize(ScriptEngine &, ScriptApi::ScriptObjectHandle const &) const override;
  virtual bool deserialize(ScriptEngine &, ScriptApi::ScriptObjectHandle const &) override;
};

class ScriptComponentBinderComponent : public ScriptBinderComponent {
  ScriptApi::ScriptObjectHandle handle;
  static std::string TAG;

public:
  ScriptComponentBinderComponent() PATCHABLE;
  ScriptComponentBinderComponent(ScriptApi::ScriptObjectHandle &&) PATCHABLE;
  virtual ~ScriptComponentBinderComponent() PATCHABLE;
  virtual bool serialize(ScriptEngine &, ScriptApi::ScriptObjectHandle const &) const override;
  virtual bool deserialize(ScriptEngine &, ScriptApi::ScriptObjectHandle const &) override;

  ScriptApi::ScriptObjectHandle const &getData() const;
};

class ScriptTickingAreaBinderComponent : public ScriptBinderComponent {
  ScriptApi::ScriptObjectHandle handle;
  static std::string TAG;

public:
  ScriptTickingAreaBinderComponent() PATCHABLE;
  ScriptTickingAreaBinderComponent(ScriptApi::ScriptObjectHandle &&) PATCHABLE;
  virtual ~ScriptTickingAreaBinderComponent() PATCHABLE;
  virtual bool serialize(ScriptEngine &, ScriptApi::ScriptObjectHandle const &) const override;
  virtual bool deserialize(ScriptEngine &, ScriptApi::ScriptObjectHandle const &) override;

  ScriptApi::ScriptObjectHandle const &getData() const;
};

class ScriptBlockPositionBinderComponent : public ScriptBinderComponent {
  BlockPos pos;
  static std::string TAG;

public:
  ScriptBlockPositionBinderComponent() PATCHABLE;
  ScriptBlockPositionBinderComponent(BlockPos const &) PATCHABLE;
  virtual ~ScriptBlockPositionBinderComponent() PATCHABLE;
  virtual bool serialize(ScriptEngine &, ScriptApi::ScriptObjectHandle const &) const override PATCHABLE;
  virtual bool deserialize(ScriptEngine &, ScriptApi::ScriptObjectHandle const &) override PATCHABLE;

  BlockPos const &getPosition() const;
};

extern ScriptApi::QuickJSInterface *ifce ABITAG(quickjs);
extern MinecraftServerScriptEngine *scriptengine ABITAG(quickjs);
extern std::vector<std::function<void(JSValue const &, JSValue const &)>> quickjs_extras ABITAG(quickjs);
extern std::vector<std::function<void(JSValue const &)>> quickjs_preloads ABITAG(quickjs);
extern std::vector<std::function<void(JSValue const &)>> quickjs_proto_extras ABITAG(quickjs);
extern std::vector<std::function<void()>> init_hooks ABITAG(quickjs);