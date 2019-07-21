#include <mods-js-policy/policy.h>

MakeSingleInstance(PolicyManager);

static JSValue processCheckPolicy(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
  if (argc != 3) return JS_ThrowTypeError(ctx, "Require 3 arguments");
  std::string name;
  bool def = false;
  if (!scriptengine->getValue(argv[0], name) || !JS_IsObject(argv[1]) || !scriptengine->getValue(argv[2], def))
    return JS_ThrowTypeError(ctx, "Require (string, object, bool)");
  ScriptPolicy policy{ name, argv[1], def };
  Instance<PolicyManager>().dispatch(policy);
  return policy.result ? JS_TRUE : JS_FALSE;
}

static JSValue processHandlePolicy(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
  if (argc != 2) return JS_ThrowTypeError(ctx, "Require 2 arguments");
  std::string name;
  if (!scriptengine->getValue(argv[0], name) || !JS_IsFunction(js_context, argv[1])) return JS_ThrowTypeError(ctx, "Require (string, function)");
  Instance<PolicyManager>().addListener(name, argv[1]);
  return JS_UNDEFINED;
}

static JSValue processHasPolicy(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
  if (argc != 1) return JS_ThrowTypeError(ctx, "Require 1 argument");
  std::string name;
  if (!scriptengine->getValue(argv[0], name)) return JS_ThrowTypeError(ctx, "Require (string)");
  return Instance<PolicyManager>().hasPolicy(name) ? JS_TRUE : JS_FALSE;
}

static JSValue processRegisterPolicy(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
  if (argc != 1) return JS_ThrowTypeError(ctx, "Require 1 argument");
  std::string name;
  if (!scriptengine->getValue(argv[0], name)) return JS_ThrowTypeError(ctx, "Require (string)");
  Instance<PolicyManager>().registerPolicy(name);
  return JS_UNDEFINED;
}

static JSCFunctionListEntry funcs[] = {
  JS_CFUNC_DEF("checkPolicy", 3, processCheckPolicy),
  JS_CFUNC_DEF("handlePolicy", 2, processHandlePolicy),
  JS_CFUNC_DEF("hasPolicy", 1, processHasPolicy),
  JS_CFUNC_DEF("registerPolicy", 1, processRegisterPolicy),
};

static void entry(JSValue const &server) { JS_SetPropertyFunctionList(js_context, server, funcs, countof(funcs)); }

LAZY(register, quickjs_proto_extras.emplace_back(entry));