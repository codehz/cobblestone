#include <mods-quickjs/quickjs.hpp>

#include <minecraft/block/Block.h>
#include <minecraft/block/BlockSource.h>
#include <minecraft/block/BlockTypeRegistry.h>
#include <minecraft/core/WeakPtr.h>
#include <minecraft/script/ScriptBinderComponent.h>
#include <minecraft/script/ScriptVersionInfo.h>
#include <minecraft/script/details/BinderTemplates.h>

static bool getExtraBlock(ScriptApi::ScriptVersionInfo const &version, ScriptApi::ScriptObjectHandle &target, BlockPos const &pos, ScriptApi::ScriptObjectHandle const &ticking_area) {
  if (auto binder = scriptengine->getScriptBinderTemplateController()->deserialize(*scriptengine, ticking_area); binder) {
    auto level_ticking = binder->getComponent<ScriptLevelAreaBinderComponent>();
    auto actor_ticking = binder->getComponent<ScriptActorAreaBinderComponent>();
    if (auto source = scriptengine->_helpGetBlockSourceFromBinder(*scriptengine->getScriptServerContext().level, level_ticking, actor_ticking); source) {
      auto &block = source->getExtraBlock(pos);
      autohandle temp = ticking_area;
      if (auto obj_template = ScriptBinderBlockTemplate::build(block, pos, temp.transfer()); obj_template) {
        if (!obj_template->serialize(*scriptengine, target)) {
          scriptengine->getScriptReportQueue().addWarning("Failed to serialize block");
          return false;
        }
        return true;
      } else {
        scriptengine->getScriptReportQueue().addWarning("Failed to create block template binder");
        return false;
      }

    } else {
      scriptengine->getScriptReportQueue().addWarning("Failed to get block source");
      return false;
    }
  } else {
    scriptengine->getScriptReportQueue().addWarning("Failed to get template binder for ticking area");
    return false;
  }
}
static bool setBlock(ScriptApi::ScriptVersionInfo const &version, Block const &block, BlockPos const &pos, ScriptApi::ScriptObjectHandle const &ticking_area) {
  if (auto binder = scriptengine->getScriptBinderTemplateController()->deserialize(*scriptengine, ticking_area); binder) {
    auto level_ticking = binder->getComponent<ScriptLevelAreaBinderComponent>();
    auto actor_ticking = binder->getComponent<ScriptActorAreaBinderComponent>();
    if (auto source = scriptengine->_helpGetBlockSourceFromBinder(*scriptengine->getScriptServerContext().level, level_ticking, actor_ticking); source) {
      source->setBlockNoUpdate(pos, block);
      return true;
    } else {
      scriptengine->getScriptReportQueue().addWarning("Failed to get block source");
      return false;
    }
  } else {
    scriptengine->getScriptReportQueue().addWarning("Failed to get template binder for ticking area");
    return false;
  }
}
static bool setExtraBlock(ScriptApi::ScriptVersionInfo const &version, Block const &block, BlockPos const &pos, ScriptApi::ScriptObjectHandle const &ticking_area) {
  if (auto binder = scriptengine->getScriptBinderTemplateController()->deserialize(*scriptengine, ticking_area); binder) {
    auto level_ticking = binder->getComponent<ScriptLevelAreaBinderComponent>();
    auto actor_ticking = binder->getComponent<ScriptActorAreaBinderComponent>();
    if (auto source = scriptengine->_helpGetBlockSourceFromBinder(*scriptengine->getScriptServerContext().level, level_ticking, actor_ticking); source) {
      return source->setExtraBlock(pos, block, 0);
    } else {
      scriptengine->getScriptReportQueue().addWarning("Failed to get block source");
      return false;
    }
  } else {
    scriptengine->getScriptReportQueue().addWarning("Failed to get template binder for ticking area");
    return false;
  }
}

static JSValue processSetBlock(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
  BlockPos pos;
  std::string name;
  ScriptApi::ScriptVersionInfo version;
  if (argc == 3) {
    if (!JS_IsObject(argv[0]) || !scriptengine->getValue(argv[1], name) || !JS_IsObject(argv[2]))
      return JS_ThrowTypeError(ctx, "Require (object, string, object)");
    if (!scriptengine->helpGetPosition(argv[2], pos))
      return JS_ThrowTypeError(ctx, "Require postion object");
  } else if (argc == 5) {
    if (!JS_IsObject(argv[0]) || !scriptengine->getValue(argv[1], name) || !scriptengine->getValue(argv[2], pos.x) || !scriptengine->getValue(argv[3], pos.y) ||
        !scriptengine->getValue(argv[4], pos.z))
      return JS_ThrowTypeError(ctx, "Require (object, string, number, number, number)");
  }
  auto ptr = BlockTypeRegistry::lookupByName(name);
  if (ptr) {
    if (setBlock(version, *ptr->getDefaultState(), pos, argv[0])) {
      return JS_UNDEFINED;
    }
    return JS_ThrowInternalError(js_context, "Cannot set block");
  }
  return JS_ThrowInternalError(js_context, "Block not registered");
}

static JSValue processGetExtraBlock(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
  BlockPos pos;
  ScriptApi::ScriptVersionInfo version;
  if (argc == 2) {
    if (!JS_IsObject(argv[0]) || !JS_IsObject(argv[1]))
      return JS_ThrowTypeError(ctx, "Require (object, object)");
    if (!scriptengine->helpGetPosition(argv[1], pos))
      return JS_ThrowTypeError(ctx, "Require postion object");
  } else if (argc == 4) {
    if (!JS_IsObject(argv[0]) || !scriptengine->getValue(argv[1], pos.x) || !scriptengine->getValue(argv[2], pos.y) || !scriptengine->getValue(argv[3], pos.z))
      return JS_ThrowTypeError(ctx, "Require (object, number, number, number)");
  }
  autohandle ret = JS_NewObject(js_context);
  if (!getExtraBlock(version, ret, pos, argv[0]))
    return JS_ThrowInternalError(js_context, "Failed to get block");
  return ret.transfer();
}
static JSValue processSetExtraBlock(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
  BlockPos pos;
  std::string name;
  ScriptApi::ScriptVersionInfo version;
  if (argc == 3) {
    if (!JS_IsObject(argv[0]) || !scriptengine->getValue(argv[1], name) || !JS_IsObject(argv[2]))
      return JS_ThrowTypeError(ctx, "Require (object, string, object)");
    if (!scriptengine->helpGetPosition(argv[2], pos))
      return JS_ThrowTypeError(ctx, "Require postion object");
  } else if (argc == 5) {
    if (!JS_IsObject(argv[0]) || !scriptengine->getValue(argv[1], name) || !scriptengine->getValue(argv[2], pos.x) || !scriptengine->getValue(argv[3], pos.y) ||
        !scriptengine->getValue(argv[4], pos.z))
      return JS_ThrowTypeError(ctx, "Require (object, string, number, number, number)");
  }
  auto ptr = BlockTypeRegistry::lookupByName(name);
  if (ptr) {
    if (setExtraBlock(version, *ptr->getDefaultState(), pos, argv[0])) {
      return JS_UNDEFINED;
    }
    return JS_ThrowInternalError(js_context, "Cannot set block");
  }
  return JS_ThrowInternalError(js_context, "Block not registered");
}

static JSCFunctionListEntry funcs[] = {
    JS_CFUNC_DEF("setBlock", 3, processSetBlock),
    JS_CFUNC_DEF("getExtraBlock", 2, processGetExtraBlock),
    JS_CFUNC_DEF("setExtraBlock", 3, processSetExtraBlock),
};

static void entry(JSValue const &server) { JS_SetPropertyFunctionList(js_context, server, funcs, countof(funcs)); }

LAZY(register, quickjs_proto_extras.emplace_back(entry));

THook(bool, _ZNK11BlockLegacy15canBeExtraBlockEv) { return true; }