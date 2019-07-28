#include <mods-js-nbt/nbt.hpp>
#include <mods-quickjs/quickjs.hpp>

#include <minecraft/core/NBT.h>
#include <minecraft/level/Level.h>
#include <minecraft/structure/StructureBlockPalette.h>
#include <minecraft/structure/StructureSettings.h>
#include <minecraft/structure/StructureTemplate.h>

std::unique_ptr<CompoundTag> getStructure(BlockSource &source, BlockPos src, BlockPos size) {
  StructureTemplate temp;
  StructureSettings settings;
  settings.offset = {};
  settings.size = size;
  temp.fillFromWorld(source, src, settings);
  if (!temp.isLoaded())
    return nullptr;
  return temp.save();
}
bool setStructure(BlockSource &source, BlockPos const &src, CompoundTag const &data) {
  StructureTemplate temp{};
  StructureSettings settings{};
  settings.offset = {};
  auto palette = scriptengine->getScriptServerContext().level->getGlobalBlockPalette();
  QCHECK(temp.load(data));
  BlockSource *psource = &source;
  BlockPos const *psrc = &src;
  StructureTemplate *ptemp = &temp;
  StructureSettings *psettings = &settings;
  asm(R"asm(
    mov %0, %%r8
    mov %1, %%rcx
    mov %2, %%rdx
    mov %3, %%rsi
    mov %4, %%rdi
    call _ZNK17StructureTemplate12placeInWorldER11BlockSourceRK12BlockPalette8BlockPosRK17StructureSettings@plt
  )asm"
      :
      : "m"(psettings), "m"(psrc), "m"(palette), "m"(psource), "r"(ptemp)
      : "rax", "rdi", "rsi", "rdx", "rcx", "r8");
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
  if (auto temp = getStructure(*source, pos, size); temp)
    return create_tag(std::move(temp));
  return JS_ThrowInternalError(js_context, "failed to save structure");
}

static JSValue processSetStructure(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
  if (argc != 3)
    return JS_ThrowTypeError(js_context, "Require 3 argument");
  BlockSource *source = getBlockSource(argv[0]);
  BlockPos pos;
  if (!source || !scriptengine->helpGetPosition(argv[1], pos) || !JS_IsObject(argv[2]))
    return JS_ThrowTypeError(js_context, "Require (ticking_area, pos, data)");
  auto data = from_tag(argv[2]);
  if (!data)
    return JS_ThrowTypeError(js_context, "Failed to decode structure data");
  if (auto comp = dynamic_cast<CompoundTag *>(&*data)) {
    if (!setStructure(*source, pos, *comp))
      return JS_ThrowInternalError(js_context, "failed to load structure");
  } else {
    return JS_ThrowTypeError(js_context, "Failed to decode structure data: require CompoundTag");
  }
  return JS_UNDEFINED;
}

static JSCFunctionListEntry funcs[] = {
    JS_CFUNC_DEF("getStructure", 3, &processGetStructure),
    JS_CFUNC_DEF("setStructure", 3, &processSetStructure),
};

static void entry(JSValue const &server) { JS_SetPropertyFunctionList(js_context, server, funcs, countof(funcs)); }

LAZY(register, quickjs_proto_extras.emplace_back(entry));

THook(bool, _ZNK14FeatureToggles9isEnabledE15FeatureOptionID) { return true; }