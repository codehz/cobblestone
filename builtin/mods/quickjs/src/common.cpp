#include <mods-quickjs/common.hpp>
#include <mods-quickjs/quickjs.hpp>

void QJS_FreeCString(char const **ptr) { JS_FreeCString(js_context, *ptr); }
void QJS_FreeAtom(JSAtom *ptr) { JS_FreeAtom(js_context, *ptr); }
void QJS_FreeHandle(ScriptApi::ScriptObjectHandle *ptr) { ptr->release(); }
void QJS_FreeValue(JSValue *ptr) { JS_FreeValue(js_context, *ptr); }