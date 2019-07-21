#pragma once
#include <mods-quickjs/quickjs.hpp>

#include <minecraft/core/NBT.h>

inline namespace nbt ABITAG(js_nbt) {

  extern JSClassID Base;
  extern JSClassID Byte;
  extern JSClassID Short;
  extern JSClassID Int;
  extern JSClassID Int64;
  extern JSClassID Float;
  extern JSClassID Double;
  extern JSClassID ByteArray;
  extern JSClassID IntArray;
  extern JSClassID String;
  extern JSClassID List;
  extern JSClassID Compound;
  extern JSClassID End;

  template <typename T, JSClassID &id> static void general_finalizer(JSRuntime * rt, JSValue val) {
    delete reinterpret_cast<T *>(JS_GetOpaque(val, id));
  };

  namespace def {
  extern JSClassDef Base;
#define MakeDef(name) extern JSClassDef name;
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

  extern JSValue general_toString(JSContext * ctx, JSValueConst this_val, int argc, JSValueConst *argv);

  template <typename T> JSValue general_get(JSContext * ctx, JSValueConst this_val);
  template <typename T> JSValue general_set(JSContext * ctx, JSValueConst this_val, JSValueConst val);

  extern JSValue create_tag(std::unique_ptr<Tag> tag);
  extern std::unique_ptr<Tag> from_tag(JSValue);

} // namespace nbtABITAG(js_nbt)