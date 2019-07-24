#pragma once

#include <modloader/hook.hpp>
#include <modloader/log.hpp>
#include <modloader/refs.hpp>
#include <modloader/utils.hpp>

extern "C" {
#include <quickjs.h>
}

#define QCHECK(expr)                                                                                                                                                                                   \
  if (!(expr))                                                                                                                                                                                         \
  return false

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define HANDLE_EXCEPTION()                                                                                                                                                                             \
  if (!handle_exception(js_context, __FILE__ ":" TOSTRING(__LINE__))) {                                                                                                                                \
    print_stacktrace();                                                                                                                                                                                \
    return false;                                                                                                                                                                                      \
  }

#define HANDLE_EXCEPTIONX()                                                                                                                                                                            \
  if (!handle_exception(js_context, __FILE__ ":" TOSTRING(__LINE__)))                                                                                                                                  \
  asm("int3")

extern JSRuntime *js_runtime ABITAG(quickjs);
extern JSContext *js_context ABITAG(quickjs);

namespace ScriptApi {
class ScriptObjectHandle;
};

void QJS_FreeCString(char const **ptr) ABITAG(quickjs);
void QJS_FreeAtom(JSAtom *ptr) ABITAG(quickjs);
void QJS_FreeValue(JSValue *ptr) ABITAG(quickjs);
void QJS_FreeHandle(ScriptApi::ScriptObjectHandle *ptr) ABITAG(quickjs);

#define autostr CLEANUP(QJS_FreeCString) char const *
#define autoval CLEANUP(QJS_FreeValue) JSValue
#define autoatom CLEANUP(QJS_FreeAtom) JSAtom
#define autohandle CLEANUP(QJS_FreeHandle) ScriptApi::ScriptObjectHandle

INLINE bool handle_exception(JSContext *ctx, char const *name) {
  autoval exception = JS_GetException(ctx);
  if (!JS_IsNull(exception)) {
    autostr ret = JS_ToCString(ctx, exception);
    if (ret)
      Log::error("QuickJS/lifetime", "(%s) %s", name, ret);
    else
      Log::error("QuickJS/lifetime", "(%s) Unknown exception", name);
    if (JS_IsError(ctx, exception)) {
      autoval val = JS_GetPropertyStr(ctx, exception, "stack");
      if (!JS_IsUndefined(val)) {
        autostr stack = JS_ToCString(ctx, val);
        Log::error("QuickJS/lifetime", "%s", stack);
      }
    }
    return false;
  }
  return true;
}