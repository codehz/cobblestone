#include <mods-quickjs/quickjs.hpp>

#include <minecraft/script/EventInfo.h>
#include <minecraft/script/ScriptBinderQueryTemplate.h>
#include <minecraft/script/ScriptEventCoordinator.h>
#include <minecraft/script/ScriptQueryBinderComponent.h>
#include <minecraft/script/ScriptVersionInfo.h>

#include "MyQuerySystem.hpp"

namespace ScriptApi {

bool ScriptObjectHandle::isEmpty() const { return JS_IsUndefined(value); }

JSValue QuickJSInterface::processRegisterSystem(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
  auto ret = JS_NewObjectClass(js_context, system_class);
  ifce->systems.emplace_back(ret);
  return ret;
}

JSValue QuickJSInterface::processLog(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
  for (int i = 0; i < argc; i++) {
    autostr str = JS_ToCString(js_context, argv[i]);
    Log::debug("QuickJS/log", "%s", str);
  }
  return JS_UNDEFINED;
}

JSValue QuickJSInterface::processRegisterEventData(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
  if (argc != 2)
    return JS_ThrowTypeError(ctx, "Require 2 parameters");
  ScriptVersionInfo version;
  std::string name;
  if (!scriptengine->getValue(argv[0], name) || !JS_IsObject(argv[1]))
    return JS_ThrowTypeError(ctx, "Require (string, object)");
  if (!scriptengine->registerEventData(version, name, argv[1]))
    return JS_ThrowInternalError(ctx, "Failed to register event");
  return JS_UNDEFINED;
}

JSValue QuickJSInterface::processCreateEventData(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
  if (argc != 1)
    return JS_ThrowTypeError(ctx, "Require 1 parameters");
  ScriptVersionInfo version;
  std::string name;
  if (!scriptengine->getValue(argv[0], name))
    return JS_ThrowTypeError(ctx, "Require (string)");
  ScriptObjectHandle handle;
  if (!scriptengine->createEventData(version, name, handle))
    return JS_ThrowInternalError(ctx, "Failed to create event");
  return handle;
}

JSValue QuickJSInterface::processListenForEvent(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
  if (argc != 2)
    return JS_ThrowTypeError(ctx, "Require 2 parameters");
  EventInfo info;
  if (!scriptengine->getValue(argv[0], info.data) || info.data.empty() || !JS_IsFunction(js_context, argv[1]))
    return JS_ThrowTypeError(ctx, "Require (string(nonempty), function)");
  scriptengine->getScriptEventCoordinator().sendScriptListenForEvent(info.data);
  auto &registry = scriptengine->getEventTrackings();
  registry[info.data].emplace_back(argv[1]);
  // Log::debug("QuickJS", "listen %s", info.data.data());
  return JS_TRUE;
}

JSValue QuickJSInterface::processBroadcastEvent(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
  if (argc != 2)
    return JS_ThrowTypeError(ctx, "Require 2 parameters");
  ScriptVersionInfo version;
  std::string name;
  if (!scriptengine->getValue(argv[0], name) || !JS_IsObject(argv[1]))
    return JS_ThrowTypeError(ctx, "Require (string, object)");
  return scriptengine->onEventReceivedFromScriptEngine(version, name, argv[1]) ? JS_TRUE : JS_FALSE;
}

JSValue QuickJSInterface::processCreateEntity(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
  if (argc != 0 && argc != 2)
    return JS_ThrowTypeError(ctx, "Require 0 or 2 parameters");
  ScriptVersionInfo version;
  if (argc == 0) {
    ScriptObjectHandle ret = JS_NewObject(js_context);
    if (!scriptengine->createEntity(version, ret))
      return JS_ThrowInternalError(ctx, "Failed to create entity");
    return ret;
  } else {
    std::string type, identifier;
    if (!scriptengine->getValue(argv[0], type) || !scriptengine->getValue(argv[1], identifier))
      return JS_ThrowTypeError(ctx, "Require (string, string)");
    ScriptObjectHandle ret = JS_NewObject(js_context);
    if (!scriptengine->createEntity(version, ret, type, identifier))
      return JS_ThrowInternalError(ctx, "Failed to create entity");
    return ret;
  }
}

JSValue QuickJSInterface::processDestroyEntity(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
  if (argc != 1)
    return JS_ThrowTypeError(ctx, "Require only 1 parameter");
  ScriptVersionInfo version;
  if (!JS_IsObject(argv[0]))
    return JS_ThrowTypeError(ctx, "Require (object)");
  if (!scriptengine->destroyEntity(version, argv[0]))
    return JS_ThrowInternalError(ctx, "Failed to destroy entity");
  return JS_TRUE;
}

JSValue QuickJSInterface::processIsValidEntity(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
  if (argc != 1)
    return JS_ThrowTypeError(ctx, "Require only 1 parameter");
  ScriptVersionInfo version;
  if (!JS_IsObject(argv[0]))
    return JS_ThrowTypeError(ctx, "Require (object)");
  bool value;
  if (!scriptengine->isValidEntity(version, argv[0], value))
    return JS_ThrowInternalError(ctx, "Failed to validate entity");
  return JS_NewBool(js_context, value);
}

JSValue QuickJSInterface::processRegisterComponent(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
  if (argc != 2)
    return JS_ThrowTypeError(ctx, "Require 2 parameters");
  ScriptVersionInfo version;
  std::string identifier;
  if (!scriptengine->getValue(argv[0], identifier) || !JS_IsObject(argv[1]))
    return JS_ThrowTypeError(ctx, "Require (string, object)");
  if (!scriptengine->registerComponent(version, identifier, argv[1]))
    return JS_ThrowInternalError(ctx, "Failed to register component");
  return JS_TRUE;
}

JSValue QuickJSInterface::processCreateComponent(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
  if (argc != 2)
    return JS_ThrowTypeError(ctx, "Require 2 parameters");
  ScriptVersionInfo version;
  std::string identifier;
  if (!JS_IsObject(argv[0]) || !scriptengine->getValue(argv[1], identifier))
    return JS_ThrowTypeError(ctx, "Require (object, string)");
  ScriptObjectHandle ret = JS_NewObject(js_context);
  if (!scriptengine->createComponent(version, argv[0], identifier, ret))
    return JS_ThrowInternalError(ctx, "Failed to register component: %s", identifier.data());
  return ret;
}

JSValue QuickJSInterface::processHasComponent(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
  if (argc != 2)
    return JS_ThrowTypeError(ctx, "Require 2 parameters");
  ScriptVersionInfo version;
  std::string identifier;
  if (!JS_IsObject(argv[0]) || !scriptengine->getValue(argv[1], identifier))
    return JS_ThrowTypeError(ctx, "Require (object, string)");
  bool value;
  if (!scriptengine->hasComponent(version, argv[0], identifier, value))
    return JS_ThrowInternalError(ctx, "Failed to detect component: %s", identifier.data());
  return value ? JS_TRUE : JS_FALSE;
}

JSValue QuickJSInterface::processGetComponent(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
  if (argc != 2)
    return JS_ThrowTypeError(ctx, "Require 2 parameters");
  ScriptVersionInfo version;
  std::string identifier;
  if (!JS_IsObject(argv[0]) || !scriptengine->getValue(argv[1], identifier))
    return JS_ThrowTypeError(ctx, "Require (object, string)");
  ScriptObjectHandle ret = JS_NewObject(js_context);
  if (!scriptengine->getComponent(version, argv[0], identifier, ret))
    return JS_ThrowInternalError(ctx, "Failed to retrieve component: %s", identifier.data());
  return ret;
}

JSValue QuickJSInterface::processApplyComponentChanges(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
  if (argc != 2)
    return JS_ThrowTypeError(ctx, "Require 2 parameters");
  ScriptVersionInfo version;
  if (!JS_IsObject(argv[0]) || !JS_IsObject(argv[1]))
    return JS_ThrowTypeError(ctx, "Require (object, object)");
  if (!scriptengine->applyComponentChanges(version, argv[0], argv[1]))
    return JS_ThrowInternalError(ctx, "Failed to apply component changes");
  return JS_TRUE;
}

JSValue QuickJSInterface::processDestroyChanges(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
  if (argc != 2)
    return JS_ThrowTypeError(ctx, "Require 2 parameters");
  ScriptVersionInfo version;
  std::string identifier;
  if (!JS_IsObject(argv[0]) || !scriptengine->getValue(argv[1], identifier))
    return JS_ThrowTypeError(ctx, "Require (object, string)");
  if (!scriptengine->destroyComponent(version, argv[0], identifier))
    return JS_ThrowInternalError(ctx, "Failed to destroy component");
  return JS_TRUE;
}

JSValue QuickJSInterface::processRegisterQuery(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
  if (argc == 0) {
    auto id = Instance<MyQuerySystem>().registerQuery();
    if (auto binder = ScriptBinderQueryTemplate::build(id); binder) {
      ScriptObjectHandle ret = JS_NewObject(js_context);
      if (!binder->serialize(*scriptengine, ret))
        return JS_ThrowInternalError(js_context, "Failed to serialize query object");
      return ret;
    } else
      return JS_ThrowInternalError(js_context, "Failed to create object binder");
  } else if (argc == 4) {
    std::string name, x, y, z;
    if (!scriptengine->getValue(argv[0], name) || !scriptengine->getValue(argv[1], x) || !scriptengine->getValue(argv[2], y) || !scriptengine->getValue(argv[3], z))
      return JS_ThrowTypeError(ctx, "Require (string, string, string, string)");
    auto id = Instance<MyQuerySystem>().registerQuery(name, x, y, z);
    if (auto binder = ScriptBinderQueryTemplate::build(id); binder) {
      ScriptObjectHandle ret = JS_NewObject(js_context);
      if (!binder->serialize(*scriptengine, ret))
        return JS_ThrowInternalError(js_context, "Failed to serialize query object");
      return ret;
    } else
      return JS_ThrowInternalError(js_context, "Failed to create object binder");
  } else {
    return JS_ThrowTypeError(ctx, "Require 0 or 4 parameters");
  }
}

JSValue QuickJSInterface::processAddFilterToQuery(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
  if (argc != 2)
    return JS_ThrowTypeError(ctx, "Require 2 parameters");
  std::string filter;
  if (!JS_IsObject(argv[0]) || !scriptengine->getValue(argv[1], filter))
    return JS_ThrowTypeError(ctx, "Require (object, string)");
  if (auto binder = scriptengine->getScriptBinderTemplateController()->deserialize(*scriptengine, argv[0]); binder) {
    if (auto query_component = binder->getComponent<ScriptQueryBinderComponent>(); query_component) {
      auto identifier = query_component->getIdentifier();
      if (!Instance<MyQuerySystem>().addFilter(identifier, filter))
        return JS_ThrowInternalError(js_context, "Failed to add filter to query");
    } else
      return JS_ThrowInternalError(js_context, "Failed to add filter to query: %s", "cannot get query component");
  } else
    return JS_ThrowInternalError(js_context, "Failed to add filter to query: %s", "cannot get query binder");
  return JS_TRUE;
}

JSValue QuickJSInterface::processGetEntitiesFromQuery(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
  std::vector<ScriptApi::ScriptObjectHandle> ret;
  if (argc == 1) {
    if (!JS_IsObject(argv[0]))
      return JS_ThrowTypeError(ctx, "Require (object)");
    if (auto binder = scriptengine->getScriptBinderTemplateController()->deserialize(*scriptengine, argv[0]); binder) {
      if (auto query_component = binder->getComponent<ScriptQueryBinderComponent>(); query_component) {
        auto identifier = query_component->getIdentifier();
        return Instance<MyQuerySystem>().queryEntities(identifier);
      } else
        return JS_ThrowInternalError(js_context, "Failed to get entities from query: %s", "cannot get query component");
    } else
      return JS_ThrowInternalError(js_context, "Failed to get entities from query: %s", "cannot get query binder");
  } else if (argc == 7) {
    double x1, x2, y1, y2, z1, z2;
    if (!JS_IsObject(argv[0]) || !scriptengine->getValue(argv[1], x1) || !scriptengine->getValue(argv[2], x2) || !scriptengine->getValue(argv[3], y1) || !scriptengine->getValue(argv[4], y2) ||
        !scriptengine->getValue(argv[5], z1) || !scriptengine->getValue(argv[6], z2))
      return JS_ThrowTypeError(ctx, "Require (object, number, number, number, number, number, number)");
    if (auto binder = scriptengine->getScriptBinderTemplateController()->deserialize(*scriptengine, argv[0]); binder) {
      if (auto query_component = binder->getComponent<ScriptQueryBinderComponent>(); query_component) {
        auto identifier = query_component->getIdentifier();
        return Instance<MyQuerySystem>().queryEntities(identifier, x1, x2, y1, y2, z1, z2);
      } else
        return JS_ThrowInternalError(js_context, "Failed to get entities from query: %s", "cannot get query component");
    } else
      return JS_ThrowInternalError(js_context, "Failed to get entities from query: %s", "cannot get query binder");
  } else {
    return JS_ThrowTypeError(ctx, "Require 1 or 7 parameters");
  }
}

JSValue QuickJSInterface::processGetBlock(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
  BlockPos pos;
  ScriptVersionInfo version;
  if (argc == 2) {
    if (!JS_IsObject(argv[0]) || !JS_IsObject(argv[1]))
      return JS_ThrowTypeError(ctx, "Require (object, object)");
    if (!scriptengine->helpGetPosition(argv[1], pos))
      return JS_ThrowTypeError(ctx, "Require postion object");
  } else if (argc == 4) {
    if (!JS_IsObject(argv[0]) || !scriptengine->getValue(argv[1], pos.x) || !scriptengine->getValue(argv[2], pos.y) || !scriptengine->getValue(argv[3], pos.z))
      return JS_ThrowTypeError(ctx, "Require (object, number, number, number)");
  }
  CLEANUP(QJS_FreeHandle) ScriptObjectHandle ret = JS_NewObject(js_context);
  if (!scriptengine->getBlock(version, ret, pos, argv[0]))
    return JS_ThrowInternalError(js_context, "Failed to get block");
  return ret.transfer();
}

JSValue QuickJSInterface::processGetBlocks(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
  BlockPos pos_min, pos_max;
  ScriptVersionInfo version;
  if (argc == 3) {
    if (!JS_IsObject(argv[0]) || !JS_IsObject(argv[1]) || !JS_IsObject(argv[2]))
      return JS_ThrowTypeError(ctx, "Require (object, object, object)");
    if (!scriptengine->helpGetPosition(argv[1], pos_min))
      return JS_ThrowTypeError(ctx, "Require postion object");
    if (!scriptengine->helpGetPosition(argv[2], pos_max))
      return JS_ThrowTypeError(ctx, "Require postion object");
  } else if (argc == 7) {
    if (!JS_IsObject(argv[0]) || !scriptengine->getValue(argv[1], pos_min.x) || !scriptengine->getValue(argv[2], pos_min.y) || !scriptengine->getValue(argv[3], pos_min.z) ||
        !scriptengine->getValue(argv[4], pos_max.x) || !scriptengine->getValue(argv[5], pos_max.y) || !scriptengine->getValue(argv[6], pos_max.z))
      return JS_ThrowTypeError(ctx, "Require (object, number, number, number, number, number, number)");
  }
  if (pos_min.x >= pos_max.x || pos_min.y >= pos_max.y || pos_min.z >= pos_max.z)
    return JS_ThrowRangeError(js_context, "Invalid range");
  CLEANUP(QJS_FreeHandle) ScriptObjectHandle ret = JS_NewArray(js_context);
#define FOR_RANGE(i) for (int i = pos_min.i, idx_##i = 0; i < pos_max.i; i++, idx_##i++)
  FOR_RANGE(x) {
    CLEANUP(QJS_FreeHandle) ScriptObjectHandle x_arr = JS_NewArray(js_context);
    FOR_RANGE(y) {
      CLEANUP(QJS_FreeHandle) ScriptObjectHandle y_arr = JS_NewArray(js_context);
      FOR_RANGE(z) {
        CLEANUP(QJS_FreeHandle) ScriptObjectHandle val = JS_NewObject(js_context);
        if (!scriptengine->getBlock(version, val, {x, y, z}, argv[0]))
          return JS_ThrowInternalError(js_context, "Failed to get block");
        if (!scriptengine->setMember(y_arr, idx_z, val.transfer()))
          return JS_ThrowInternalError(js_context, "Failed to set blocks[3]");
      }
      if (!scriptengine->setMember(x_arr, idx_y, y_arr.transfer()))
        return JS_ThrowInternalError(js_context, "Failed to set blocks[2]");
    }
    if (!scriptengine->setMember(ret, idx_x, x_arr.transfer()))
      return JS_ThrowInternalError(js_context, "Failed to set blocks[1]");
  }
#undef FOR_RANGE
  return ret.transfer();
}

JSValue QuickJSInterface::processExecuteCommand(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
  if (argc != 2)
    return JS_ThrowTypeError(ctx, "Require 2 parameters");
  std::string name;
  if (!scriptengine->getValue(argv[0], name) || !JS_IsFunction(js_context, argv[1]))
    return JS_ThrowTypeError(ctx, "Require (string, function)");
  ifce->commandPendingList.push({name, [self{ScriptObjectHandle{this_val}}, handle{ScriptObjectHandle{argv[1]}}](Json::Value &&value) {
                                   GUARD(release, self.release(); handle.release());
                                   CLEANUP(QJS_FreeHandle) ScriptObjectHandle temp;
                                   scriptengine->serializeJsonToScriptObjectHandle(temp, value);
                                   JS_FreeValue(js_context, JS_Call(js_context, handle, self, 1, (JSValue *)&temp));
                                   handle_exception(js_context, "CommandCallback");
                                 }});
  return JS_UNDEFINED;
}

} // namespace ScriptApi