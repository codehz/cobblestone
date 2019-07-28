#include <mods-js-nbt/nbt.hpp>
#include <mods-quickjs/quickjs.hpp>

#include <minecraft/core/NBT.h>
#include <minecraft/level/Level.h>
#include <minecraft/structure/StructureSettings.h>
#include <minecraft/structure/StructureTemplate.h>

std::unique_ptr<CompoundTag> getStructure(BlockSource &source, BlockPos src, BlockPos size) {
  StructureTemplate temp;
  StructureSettings settings;
  settings.offset = {};
  settings.size = size;
  temp.fillFromWorld(source, src, settings);
  return temp.save();
}
bool setStructure(BlockSource &source, BlockPos src, std::unique_ptr<CompoundTag> const &data) {
  StructureTemplate temp;
  StructureSettings settings;
  settings.offset = {};
  auto palette = scriptengine->getScriptServerContext().level->getGlobalBlockPalette();
  QCHECK(temp.load(data));
  temp.placeInWorld(source, *palette, src, settings);
  return true;
}

BlockSource *getBlockSource(JSValue ticking_area) {
  if (auto binder = scriptengine->getScriptBinderTemplateController()->deserialize(*scriptengine, ticking_area); binder) {
    auto level_ticking = binder->getComponent<ScriptLevelAreaBinderComponent>();
    auto actor_ticking = binder->getComponent<ScriptActorAreaBinderComponent>();
    return scriptengine->_helpGetBlockSourceFromBinder(*scriptengine->getScriptServerContext().level, level_ticking, actor_ticking);
  }
  return nullptr;
}

static JSValue processGetStructure(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
  if (argc != 3)
    return JS_ThrowTypeError(js_context, "Require 3 argument");
  BlockSource *source = getBlockSource(argv[0]);
  BlockPos pos, size;
  if (!source || !scriptengine->helpGetPosition(argv[1], pos) || !scriptengine->helpGetPosition(argv[2], size))
    return JS_ThrowTypeError(js_context, "Require (ticking_area, pos, size)");
  return create_tag(getStructure(*source, pos, size));
}

static JSValue processSetStructure(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
  if (argc != 4)
    return JS_ThrowTypeError(js_context, "Require 3 argument");
  BlockSource *source = getBlockSource(argv[0]);
  BlockPos pos;
  if (!source || !scriptengine->helpGetPosition(argv[1], pos) || !JS_IsObject(argv[2]))
    return JS_ThrowTypeError(js_context, "Require (ticking_area, pos, data)");
  auto data = from_tag(argv[4]);
  if (!data)
    return JS_ThrowTypeError(js_context, "Failed to decode structure data");
  if (!setStructure(*source, pos, (std::unique_ptr<CompoundTag> const &)data))
    return JS_ThrowInternalError(js_context, "failed to load structure");
  return JS_UNDEFINED;
}

static JSCFunctionListEntry funcs[] = {
  JS_CFUNC_DEF("getStructure", 3, &processGetStructure),
  JS_CFUNC_DEF("setStructure", 4, &processSetStructure),
};

static void entry(JSValue const &server) { JS_SetPropertyFunctionList(js_context, server, funcs, countof(funcs)); }

LAZY(register, quickjs_proto_extras.emplace_back(entry));