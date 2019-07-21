#include <mods-js-nbt/nbt.hpp>

#include <cstring>

inline namespace nbt ABITAG(js_nbt) {
  // * ByteTag
  template <> JSValue general_get<ByteTag>(JSContext * ctx, JSValueConst this_val) {
    if (auto ptr = reinterpret_cast<ByteTag *>(JS_GetOpaque(this_val, Byte)); ptr) return JS_NewInt32(js_context, ptr->value);
    return JS_ThrowTypeError(ctx, "Cannot get internal slot");
  }

  template <> JSValue general_set<ByteTag>(JSContext * ctx, JSValueConst this_val, JSValueConst val) {
    if (!JS_IsNumber(val)) return JS_ThrowTypeError(js_context, "Tag type mismatch");
    if (auto ptr = reinterpret_cast<ByteTag *>(JS_GetOpaque(this_val, Byte)); ptr) {
      int32_t temp;
      JS_ToInt32(js_context, &temp, val);
      ptr->value = temp;
      return JS_UNDEFINED;
    }
    return JS_ThrowTypeError(ctx, "Cannot get internal slot");
  }

  // * ShortTag
  template <> JSValue general_get<ShortTag>(JSContext * ctx, JSValueConst this_val) {
    if (auto ptr = reinterpret_cast<ShortTag *>(JS_GetOpaque(this_val, Short)); ptr) return JS_NewInt32(js_context, ptr->value);
    return JS_ThrowTypeError(ctx, "Cannot get internal slot");
  }

  template <> JSValue general_set<ShortTag>(JSContext * ctx, JSValueConst this_val, JSValueConst val) {
    if (!JS_IsNumber(val)) return JS_ThrowTypeError(js_context, "Tag type mismatch");
    if (auto ptr = reinterpret_cast<ShortTag *>(JS_GetOpaque(this_val, Short)); ptr) {
      int32_t temp;
      JS_ToInt32(js_context, &temp, val);
      ptr->value = temp;
      return JS_UNDEFINED;
    }
    return JS_ThrowTypeError(ctx, "Cannot get internal slot");
  }

  // * IntTag
  template <> JSValue general_get<IntTag>(JSContext * ctx, JSValueConst this_val) {
    if (auto ptr = reinterpret_cast<IntTag *>(JS_GetOpaque(this_val, Int)); ptr) return JS_NewInt32(js_context, ptr->value);
    return JS_ThrowTypeError(ctx, "Cannot get internal slot");
  }

  template <> JSValue general_set<IntTag>(JSContext * ctx, JSValueConst this_val, JSValueConst val) {
    if (!JS_IsNumber(val)) return JS_ThrowTypeError(js_context, "Tag type mismatch");
    if (auto ptr = reinterpret_cast<IntTag *>(JS_GetOpaque(this_val, Int)); ptr) {
      int32_t temp;
      JS_ToInt32(js_context, &temp, val);
      ptr->value = temp;
      return JS_UNDEFINED;
    }
    return JS_ThrowTypeError(ctx, "Cannot get internal slot");
  }

  // * Int64Tag
  template <> JSValue general_get<Int64Tag>(JSContext * ctx, JSValueConst this_val) {
    if (auto ptr = reinterpret_cast<Int64Tag *>(JS_GetOpaque(this_val, Int64)); ptr) return JS_NewInt64(js_context, ptr->value);
    return JS_ThrowTypeError(ctx, "Cannot get internal slot");
  }

  template <> JSValue general_set<Int64Tag>(JSContext * ctx, JSValueConst this_val, JSValueConst val) {
    if (!JS_IsNumber(val)) return JS_ThrowTypeError(js_context, "Tag type mismatch");
    if (auto ptr = reinterpret_cast<Int64Tag *>(JS_GetOpaque(this_val, Int64)); ptr) {
      int64_t temp;
      JS_ToInt64(js_context, &temp, val);
      ptr->value = temp;
      return JS_UNDEFINED;
    }
    return JS_ThrowTypeError(ctx, "Cannot get internal slot");
  }

  // * FloatTag
  template <> JSValue general_get<FloatTag>(JSContext * ctx, JSValueConst this_val) {
    if (auto ptr = reinterpret_cast<FloatTag *>(JS_GetOpaque(this_val, Float)); ptr) return JS_NewFloat64(js_context, ptr->value);
    return JS_ThrowTypeError(ctx, "Cannot get internal slot");
  }

  template <> JSValue general_set<FloatTag>(JSContext * ctx, JSValueConst this_val, JSValueConst val) {
    if (!JS_IsNumber(val)) return JS_ThrowTypeError(js_context, "Tag type mismatch");
    if (auto ptr = reinterpret_cast<FloatTag *>(JS_GetOpaque(this_val, Float)); ptr) {
      double temp;
      JS_ToFloat64(js_context, &temp, val);
      ptr->value = temp;
      return JS_UNDEFINED;
    }
    return JS_ThrowTypeError(ctx, "Cannot get internal slot");
  }

  // * DoubleTag
  template <> JSValue general_get<DoubleTag>(JSContext * ctx, JSValueConst this_val) {
    if (auto ptr = reinterpret_cast<DoubleTag *>(JS_GetOpaque(this_val, Double)); ptr) return JS_NewFloat64(js_context, ptr->value);
    return JS_ThrowTypeError(ctx, "Cannot get internal slot");
  }

  template <> JSValue general_set<DoubleTag>(JSContext * ctx, JSValueConst this_val, JSValueConst val) {
    if (!JS_IsNumber(val)) return JS_ThrowTypeError(js_context, "Tag type mismatch");
    if (auto ptr = reinterpret_cast<DoubleTag *>(JS_GetOpaque(this_val, Double)); ptr) {
      double temp;
      JS_ToFloat64(js_context, &temp, val);
      ptr->value = temp;
      return JS_UNDEFINED;
    }
    return JS_ThrowTypeError(ctx, "Cannot get internal slot");
  }

  // * ByteArrayTag
  template <> JSValue general_get<ByteArrayTag>(JSContext * ctx, JSValueConst this_val) {
    if (auto ptr = reinterpret_cast<ByteArrayTag *>(JS_GetOpaque(this_val, ByteArray)); ptr)
      return JS_NewArrayBufferCopy(js_context, ptr->value.data(), ptr->value.size());
    return JS_ThrowTypeError(ctx, "Cannot get internal slot");
  }

  template <> JSValue general_set<ByteArrayTag>(JSContext * ctx, JSValueConst this_val, JSValueConst val) {
    if (!JS_IsObject(val)) return JS_ThrowTypeError(js_context, "Tag type mismatch");
    if (auto ptr = reinterpret_cast<ByteArrayTag *>(JS_GetOpaque(this_val, ByteArray)); ptr) {
      size_t len;
      auto data = JS_GetArrayBuffer(js_context, &len, val);
      if (!data) return JS_ThrowTypeError(ctx, "Invalid arraybuffer");
      ptr->value.~TagMemoryChunk();
      new (&ptr->value) TagMemoryChunk();
      ptr->value.alloc<signed char>(len);
      mempcpy(ptr->value.data(), data, len);
      return JS_UNDEFINED;
    }
    return JS_ThrowTypeError(ctx, "Cannot get internal slot");
  }

  // * IntArrayTag
  template <> JSValue general_get<IntArrayTag>(JSContext * ctx, JSValueConst this_val) {
    if (auto ptr = reinterpret_cast<IntArrayTag *>(JS_GetOpaque(this_val, IntArray)); ptr)
      return JS_NewArrayBufferCopy(js_context, ptr->value.data(), ptr->value.size());
    return JS_ThrowTypeError(ctx, "Cannot get internal slot");
  }

  template <> JSValue general_set<IntArrayTag>(JSContext * ctx, JSValueConst this_val, JSValueConst val) {
    if (!JS_IsObject(val)) return JS_ThrowTypeError(js_context, "Tag type mismatch");
    if (auto ptr = reinterpret_cast<IntArrayTag *>(JS_GetOpaque(this_val, IntArray)); ptr) {
      size_t len;
      auto data = JS_GetArrayBuffer(js_context, &len, val);
      if (!data) return JS_ThrowTypeError(ctx, "Invalid arraybuffer");
      ptr->value.~TagMemoryChunk();
      new (&ptr->value) TagMemoryChunk();
      ptr->value.alloc<signed char>(len);
      mempcpy(ptr->value.data(), data, len);
      return JS_UNDEFINED;
    }
    return JS_ThrowTypeError(ctx, "Cannot get internal slot");
  }

  // * StringTag
  template <> JSValue general_get<StringTag>(JSContext * ctx, JSValueConst this_val) {
    if (auto ptr = reinterpret_cast<StringTag *>(JS_GetOpaque(this_val, String)); ptr)
      return JS_NewStringLen(js_context, ptr->value.data(), ptr->value.size());
    return JS_ThrowTypeError(ctx, "Cannot get internal slot");
  }

  template <> JSValue general_set<StringTag>(JSContext * ctx, JSValueConst this_val, JSValueConst val) {
    if (!JS_IsString(val)) return JS_ThrowTypeError(js_context, "Tag type mismatch");
    if (auto ptr = reinterpret_cast<StringTag *>(JS_GetOpaque(this_val, String)); ptr) {
      int len;
      auto data  = JS_ToCStringLen(js_context, &len, val, false);
      ptr->value = { data, (size_t)len };
      return JS_UNDEFINED;
    }
    return JS_ThrowTypeError(ctx, "Cannot get internal slot");
  }

  // * ListTag
  template <> JSValue general_get<ListTag>(JSContext * ctx, JSValueConst this_val) {
    if (auto ptr = reinterpret_cast<ListTag *>(JS_GetOpaque(this_val, List)); ptr) {
      auto ret     = JS_NewArray(js_context);
      uint32_t idx = 0;
      for (auto &item : ptr->value) JS_SetPropertyUint32(js_context, ret, idx++, create_tag(item->copy()));
      return ret;
    }
    return JS_ThrowTypeError(ctx, "Cannot get internal slot");
  }

  template <> JSValue general_set<ListTag>(JSContext * ctx, JSValueConst this_val, JSValueConst val) {
    if (!JS_IsArray(js_context, val)) return JS_ThrowTypeError(js_context, "Tag type mismatch");
    if (auto ptr = reinterpret_cast<ListTag *>(JS_GetOpaque(this_val, List)); ptr) {
      ptr->deleteChildren();
      auto len_ = JS_GetPropertyStr(js_context, val, "length");
      uint32_t len;
      JS_ToUint32(js_context, &len, len_);
      for (uint32_t i = 0; i < len; i++) {
        auto temp = from_tag(JS_GetPropertyUint32(js_context, val, i));
        if (!temp) return JS_ThrowTypeError(ctx, "Wrong type");
        ptr->add(std::move(temp));
      }
      return JS_UNDEFINED;
    }
    return JS_ThrowTypeError(ctx, "Cannot get internal slot");
  }

  // * CompoundTag
  template <> JSValue general_get<CompoundTag>(JSContext * ctx, JSValueConst this_val) {
    if (auto ptr = reinterpret_cast<CompoundTag *>(JS_GetOpaque(this_val, Compound)); ptr) {
      auto ret = JS_NewObjectProto(js_context, JS_NULL);
      for (auto &[key, value] : ptr->value) JS_SetPropertyStr(js_context, ret, key.data(), create_tag(value->copy()));
      return ret;
    }
    return JS_ThrowTypeError(ctx, "Cannot get internal slot");
  }

  template <> JSValue general_set<CompoundTag>(JSContext * ctx, JSValueConst this_val, JSValueConst val) {
    if (!JS_IsObject(val)) return JS_ThrowTypeError(js_context, "Tag type mismatch");
    if (auto ptr = reinterpret_cast<CompoundTag *>(JS_GetOpaque(this_val, Compound)); ptr) {
      ptr->deleteChildren();
      CLEANUP(QJS_FreeValue) auto global  = JS_GetGlobalObject(js_context);
      CLEANUP(QJS_FreeValue) auto reflect = JS_GetPropertyStr(js_context, global, "Reflect");
      CLEANUP(QJS_FreeValue) auto ownKeys = JS_GetPropertyStr(js_context, reflect, "ownKeys");
      CLEANUP(QJS_FreeValue) auto arr     = JS_Call(js_context, ownKeys, reflect, 1, (JSValue *)&val);
      CLEANUP(QJS_FreeValue) auto propLen = JS_GetPropertyStr(js_context, arr, "length");
      for (int32_t i = 0; i < JS_VALUE_GET_INT(propLen); i++) {
        CLEANUP(QJS_FreeValue) auto key      = JS_GetPropertyUint32(js_context, arr, i);
        CLEANUP(QJS_FreeCString) auto keyStr = JS_ToCString(js_context, key);
        CLEANUP(QJS_FreeValue) auto value    = JS_GetPropertyStr(js_context, val, keyStr);
        if (auto temp = from_tag(value); temp)
          ptr->value.emplace(std::string{ keyStr }, std::move(temp));
        else
          return JS_ThrowTypeError(js_context, "Require tag object");
      }
      return JS_UNDEFINED;
    }
    return JS_ThrowTypeError(ctx, "Cannot get internal slot");
  }
} // namespace nbtABITAG(js_nbt)