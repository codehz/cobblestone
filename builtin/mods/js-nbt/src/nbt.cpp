#include <modloader/hook.hpp>
#include <modloader/log.hpp>

#include <minecraft/core/NBT.h>

#include <mods-js-nbt/nbt.hpp>
#include <mods-quickjs/quickjs.hpp>

inline namespace nbt ABITAG(js_nbt) {
  JSClassID Base;
  JSClassID Byte;
  JSClassID Short;
  JSClassID Int;
  JSClassID Int64;
  JSClassID Float;
  JSClassID Double;
  JSClassID ByteArray;
  JSClassID IntArray;
  JSClassID String;
  JSClassID List;
  JSClassID Compound;
  JSClassID End;

  LAZY(ClassID, {
    JS_NewClassID(&Base);
    JS_NewClassID(&Byte);
    JS_NewClassID(&Short);
    JS_NewClassID(&Int);
    JS_NewClassID(&Int64);
    JS_NewClassID(&Float);
    JS_NewClassID(&Double);
    JS_NewClassID(&ByteArray);
    JS_NewClassID(&IntArray);
    JS_NewClassID(&String);
    JS_NewClassID(&List);
    JS_NewClassID(&Compound);
    JS_NewClassID(&End);
  });

  namespace def {
  JSClassDef Base = {"Tag"};
#define MakeDef(name) JSClassDef name = {#name "Tag", &general_finalizer<name##Tag, ::name>}
  MakeDef(Byte);
  MakeDef(Short);
  MakeDef(Int);
  MakeDef(Int64);
  MakeDef(Float);
  MakeDef(Double);
  MakeDef(ByteArray);
  MakeDef(IntArray);
  MakeDef(String);
  MakeDef(List);
  MakeDef(Compound);
  MakeDef(End);
#undef MakeDef
  } // namespace def

  namespace function_list {
  JSCFunctionListEntry Base[] = {JS_CFUNC_DEF("toString", 0, general_toString)};
  JSCFunctionListEntry Byte[] = {JS_CGETSET_DEF("value", general_get<ByteTag>, general_set<ByteTag>)};
  JSCFunctionListEntry Short[] = {JS_CGETSET_DEF("value", general_get<ShortTag>, general_set<ShortTag>)};
  JSCFunctionListEntry Int[] = {JS_CGETSET_DEF("value", general_get<IntTag>, general_set<IntTag>)};
  JSCFunctionListEntry Int64[] = {JS_CGETSET_DEF("value", general_get<Int64Tag>, general_set<Int64Tag>)};
  JSCFunctionListEntry Float[] = {JS_CGETSET_DEF("value", general_get<FloatTag>, general_set<FloatTag>)};
  JSCFunctionListEntry Double[] = {JS_CGETSET_DEF("value", general_get<DoubleTag>, general_set<DoubleTag>)};
  JSCFunctionListEntry ByteArray[] = {JS_CGETSET_DEF("value", general_get<ByteArrayTag>, general_set<ByteArrayTag>)};
  JSCFunctionListEntry IntArray[] = {JS_CGETSET_DEF("value", general_get<IntArrayTag>, general_set<IntArrayTag>)};
  JSCFunctionListEntry String[] = {JS_CGETSET_DEF("value", general_get<StringTag>, general_set<StringTag>)};
  JSCFunctionListEntry List[] = {JS_CGETSET_DEF("value", general_get<ListTag>, general_set<ListTag>)};
  JSCFunctionListEntry Compound[] = {JS_CGETSET_DEF("value", general_get<CompoundTag>, general_set<CompoundTag>)};
  JSCFunctionListEntry End[] = {};
  } // namespace function_list

  JSValue general_toString(JSContext * ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    // clang-format off
    if (auto ptr = reinterpret_cast<Tag *>(
          JS_GetOpaque(this_val, Byte)      ?:
          JS_GetOpaque(this_val, Short)     ?:
          JS_GetOpaque(this_val, Int)       ?:
          JS_GetOpaque(this_val, Int64)     ?:
          JS_GetOpaque(this_val, Float)     ?:
          JS_GetOpaque(this_val, Double)    ?:
          JS_GetOpaque(this_val, ByteArray) ?:
          JS_GetOpaque(this_val, IntArray)  ?:
          JS_GetOpaque(this_val, String)    ?:
          JS_GetOpaque(this_val, List)      ?:
          JS_GetOpaque(this_val, Compound)  ?:
          JS_GetOpaque(this_val, End));
        ptr) {
      // clang-format on
      std::string str = ptr->toString();
      return JS_NewString(ctx, str.data());
    }
    return JS_ThrowTypeError(ctx, "Cannot get internal slot");
  }

} // namespace nbtABITAG(js_nbt)

static void preload(JSValue const &global) {
  JSValue base_proto, proto;
#define REG(name)                                                                                                                                                                                      \
  JS_NewClass(js_runtime, name, &def::name);                                                                                                                                                           \
  proto = JS_NewObjectProtoClass(js_context, base_proto, Base);                                                                                                                                        \
  JS_SetPropertyFunctionList(js_context, proto, function_list::name, countof(function_list::name));                                                                                                    \
  JS_SetClassProto(js_context, name, proto)

  JS_NewClass(js_runtime, Base, &def::Base);
  base_proto = JS_NewObject(js_context);
  JS_SetPropertyFunctionList(js_context, base_proto, function_list::Base, countof(function_list::Base));
  JS_SetClassProto(js_context, Base, base_proto);

  REG(Byte);
  REG(Short);
  REG(Int);
  REG(Int64);
  REG(Float);
  REG(Double);
  REG(ByteArray);
  REG(IntArray);
  REG(String);
  REG(List);
  REG(Compound);
#undef REG
}

template <typename T, JSClassID &id> JSValue create(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
  autoval proto = JS_GetClassProto(ctx, id);
  auto ret = JS_NewObjectProtoClass(js_context, proto, id);
  JS_SetOpaque(ret, new T);
  if (argc > 0)
    JS_SetPropertyStr(js_context, ret, "value", JS_DupValue(js_context, argv[0]));
  handle_exception(js_context, "create tag");
  return ret;
}

JSValue nbt::create_tag(std::unique_ptr<Tag> tag) {
  Tag *temp = tag.get();
#define CASE(name)                                                                                                                                                                                     \
  if (auto ptr = dynamic_cast<name##Tag *>(temp); ptr) {                                                                                                                                               \
    autoval proto = JS_GetClassProto(js_context, name);                                                                                                                                                \
    auto ret = JS_NewObjectProtoClass(js_context, proto, name);                                                                                                                                        \
    JS_SetOpaque(ret, tag.release());                                                                                                                                                                  \
    return ret;                                                                                                                                                                                        \
  }

  CASE(Byte);
  CASE(Short);
  CASE(Int);
  CASE(Int64);
  CASE(Float);
  CASE(Double);
  CASE(ByteArray);
  CASE(IntArray);
  CASE(String);
  CASE(List);
  CASE(Compound);
  CASE(End);
#undef CASE
  return JS_UNDEFINED;
}

std::unique_ptr<Tag> nbt::from_tag(JSValue val) {
#define CASE(name)                                                                                                                                                                                     \
  if (auto ptr = reinterpret_cast<name##Tag *>(JS_GetOpaque(val, name)); ptr)                                                                                                                          \
    return ptr->copy();

  CASE(Byte);
  CASE(Short);
  CASE(Int);
  CASE(Int64);
  CASE(Float);
  CASE(Double);
  CASE(ByteArray);
  CASE(IntArray);
  CASE(String);
  CASE(List);
  CASE(Compound);
  CASE(End);

#undef CASE
  return nullptr;
}

static void entry(JSValue const &global, JSValue const &server) {
#define REG(name) JS_SetPropertyStr(js_context, global, #name "Tag", JS_NewCFunction2(js_context, create<name##Tag, name>, #name "Tag", 0, JS_CFUNC_constructor, 0))

  REG(Byte);
  REG(Short);
  REG(Int);
  REG(Int64);
  REG(Float);
  REG(Double);
  REG(ByteArray);
  REG(IntArray);
  REG(String);
  REG(List);
  REG(Compound);
  REG(End);

#undef REG
}

LAZY(nbt, {
  quickjs_preloads.emplace_back(preload);
  quickjs_extras.emplace_back(entry);
});