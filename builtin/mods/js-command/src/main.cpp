#include <mods-js-nbt/nbt.hpp>
#include <mods-quickjs/quickjs.hpp>

#include <minecraft/actor/Player.h>
#include <minecraft/block/Block.h>
#include <minecraft/commands/Command.h>
#include <minecraft/commands/CommandMessage.h>
#include <minecraft/commands/CommandOrigin.h>
#include <minecraft/commands/CommandOutput.h>
#include <minecraft/commands/CommandParameterData.h>
#include <minecraft/commands/CommandPosition.h>
#include <minecraft/commands/CommandRegistry.h>
#include <minecraft/commands/CommandSelector.h>
#include <minecraft/commands/MinecraftCommands.h>
#include <minecraft/core/Minecraft.h>
#include <minecraft/core/typeid.h>

#include <climits>
#include <cstring>

#include "genfunc.h"

using Parser = bool (CommandRegistry::*)(void *, CommandRegistry::ParseToken const &, CommandOrigin const &, int, std::string &, std::vector<std::string> &) const;

struct ParameterDef {
  size_t size;
  std::string name;
  typeid_t<CommandRegistry> type;
  CommandParameterDataType datatype;
  Parser parser;
  void (*init)(void *);
  void (*deinit)(void *);
  JSValue (*fetch)(void *, CommandOrigin const &);
  std::string enumid;
  bool optional;

  void makeOptional() { optional = true; }
};

struct MyCommandVTable {
  std::vector<ParameterDef> defs;
  JSValue exec;
};
template <typename T> static inline void geninit(void *ptr) { new (ptr) T(); }
template <typename T> static inline void gendeinit(void *ptr) { ((T *)ptr)->~T(); }

template <typename T> struct FetchGenerator;

template <typename T> static JSValue genfetch(void *self, CommandOrigin const &orig);

#define IMPL(T) template <> inline JSValue genfetch<T>(void *self, CommandOrigin const &orig)

IMPL(CommandMessage) { return JS_NewString(js_context, ((CommandMessage *)self)->getMessage(orig).c_str()); }
IMPL(std::string) { return JS_NewString(js_context, ((std::string *)self)->data()); }
IMPL(int) { return JS_NewInt32(js_context, *(int *)self); }
IMPL(float) { return JS_NewFloat64(js_context, *(float *)self); }
IMPL(bool) { return JS_NewBool(js_context, *(bool *)self); }
IMPL(Block const *) {
  if (auto block = *((Block const **)self); block) {
    return JS_NewString(js_context, block->getFullName().data());
  }
  return JS_UNDEFINED;
}
IMPL(Json::Value) {
  autohandle ret = JS_NewObject(js_context);
  scriptengine->serializeJsonToScriptObjectHandle(ret, *((Json::Value *)self));
  return ret.transfer();
}
IMPL(CommandSelector<Actor>) {
  autohandle ret = JS_NewArray(js_context);
  auto results = ((CommandSelector<Actor> *)self)->results(orig);
  int idx = 0;
  for (auto it : *results) {
    autohandle item = JS_NewObject(js_context);
    scriptengine->helpDefineActor(*it, item);
    JS_SetPropertyInt64(js_context, ret, idx++, item.transfer());
  }
  return ret.transfer();
}
IMPL(CommandSelector<Player>) {
  autohandle ret = JS_NewArray(js_context);
  auto results = ((CommandSelector<Actor> *)self)->results(orig);
  int idx = 0;
  for (auto it : *results) {
    autohandle item = JS_NewObject(js_context);
    scriptengine->helpDefineActor(*it, item);
    JS_SetPropertyInt64(js_context, ret, idx++, item.transfer());
  }
  return ret.transfer();
}
IMPL(CommandPosition) {
  Vec3 pos = ((CommandPosition *)self)->getPosition(orig);
  autohandle ret = JS_NewObject(js_context);
  scriptengine->helpDefinePosition(pos, ret);
  return ret.transfer();
}

#undef IMPL

template <typename T> static inline ParameterDef commonParameter(std::string const &name) {
  return {
      .size = sizeof(T),
      .name = name,
      .type = type_id<CommandRegistry, T>(),
      .parser = &CommandRegistry::parse<T>,
      .init = &geninit<T>,
      .deinit = &gendeinit<T>,
      .fetch = &genfetch<T>,
  };
};

struct CustomCommand : Command {
  MyCommandVTable *vt;
  CustomCommand(MyCommandVTable *vt) : Command(), vt(vt) {
    size_t offset = 0;
    for (auto &def : vt->defs) {
      if (def.init)
        def.init((void *)((size_t)this + sizeof(CustomCommand) + offset));
      offset += def.size;
    }
  }

  ~CustomCommand() {
    size_t offset = 0;
    for (auto def : vt->defs) {
      if (def.deinit)
        def.deinit((void *)((size_t)this + sizeof(CustomCommand) + offset));
      offset += def.size;
    }
  }

  virtual void execute(CommandOrigin const &orig, CommandOutput &outp) override {
    auto size = vt->defs.size();
    size_t offset = 0;

    autohandle arr = JS_NewArray(js_context);
    for (size_t i = 0; i < size; i++) {
      auto &def = vt->defs[i];
      JSValue obj = def.fetch((void *)((size_t)this + sizeof(CustomCommand) + offset), orig);
      JS_SetPropertyInt64(js_context, arr, i, obj);
      offset += def.size;
    }

    autohandle orig_obj = JS_NewObject(js_context);
    scriptengine->setMember(orig_obj, "name", orig.getName());
    {
      autohandle temp;
      scriptengine->helpDefinePosition(orig.getWorldPosition(), temp);
      scriptengine->setMember(orig_obj, "world_pos", temp.transfer());
    }
    if (auto entity = orig.getEntity(); entity) {
      autohandle temp = JS_NewObject(js_context);
      scriptengine->helpDefineActor(*entity, temp);
      scriptengine->setMember(orig_obj, "entity", temp.transfer());
    }
    auto ret = JS_Call(js_context, vt->exec, orig_obj, 1, (JSValue *)&arr);
    if (autohandle ex = JS_GetException(js_context); !JS_IsNull(ex)) {
      autostr txt = JS_ToCString(js_context, ex);
      if (txt)
        outp.error(txt);
      else
        outp.error("Unknown error");
    } else {
      if (JS_IsUndefined(ret)) {
        outp.success();
        return;
      }
      autostr txt = JS_ToCString(js_context, ret);
      if (txt)
        outp.success(txt);
      else
        outp.success();
      if (JS_IsObject(ret)) {
        autoval global = JS_GetGlobalObject(js_context);
        autoval reflect = JS_GetPropertyStr(js_context, global, "Reflect");
        autoval ownKeys = JS_GetPropertyStr(js_context, reflect, "ownKeys");
        autoval arr = JS_Call(js_context, ownKeys, reflect, 1, (JSValue *)&ret);
        autoval propLen = JS_GetPropertyStr(js_context, arr, "length");
        for (int32_t i = 0; i < JS_VALUE_GET_INT(propLen); i++) {
          autoval key = JS_GetPropertyUint32(js_context, arr, i);
          autostr keyStr = JS_ToCString(js_context, key);
          autoval val = JS_GetPropertyStr(js_context, ret, keyStr);
          if (JS_IsInteger(val)) {
            int value;
            JS_ToInt32(js_context, &value, val);
            outp.set(keyStr, value);
          } else if (JS_IsNumber(val)) {
            double value;
            JS_ToFloat64(js_context, &value, val);
            outp.set<float>(keyStr, value);
          } else if (JS_IsBool(val)) {
            outp.set<bool>(keyStr, JS_ToBool(js_context, val));
          } else if (JS_IsString(val)) {
            autostr valStr = JS_ToCString(js_context, val);
            outp.set<std::string>(keyStr, valStr);
          } else if (JS_IsObject(val)) {
            if (double x, y, z; scriptengine->getMember(val, "x", x) && scriptengine->getMember(val, "y", y) && scriptengine->getMember(val, "z", z)) {
              Vec3 vec;
              vec.x = x;
              vec.y = y;
              vec.z = z;
              outp.set<Vec3>(keyStr, vec);
            }
          }
        }
      }
    }
  }
  static auto create(MyCommandVTable *vt) {
    size_t size = 0;
    for (auto def : vt->defs)
      size += def.size;
    auto src = calloc(1, sizeof(CustomCommand) + size);
    auto ptr = new (src) CustomCommand(vt);
    return std::unique_ptr<Command>(ptr);
  }
};

auto registerCustomCommand(std::string name, std::string desc, int lvl) {
  auto registry = scriptengine->getScriptServerContext().minecraft->getCommands()->getRegistry();
  registry->registerCommand(name, desc.c_str(), (CommandPermissionLevel)lvl, (CommandFlag)0, (CommandFlag)0);
  return [=](MyCommandVTable &rvt) {
    registry->registerCustomOverload(name, {0, INT32_MAX}, gen_function(new MyCommandVTable(rvt), CustomCommand::create), [&](CommandRegistry::Overload &overload) {
      Log::info("CustomCommand", "register /%s", name.data());
      size_t offset = sizeof(CustomCommand);
      for (auto &p : rvt.defs) {
        overload.params.emplace_back(CommandParameterData{
            p.type,
            p.parser,
            p.name.c_str(),
            p.datatype,
            nullptr,
            (int)offset,
            p.optional,
            -1,
        });
        offset += p.size;
      }
    });
  };
}

static JSValue registerCommand(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
  if (argc != 2)
    return JS_ThrowTypeError(ctx, "Require 2 arguments");
  std::string name, desc;
  int level;
  if (!scriptengine->getValue(argv[0], name) || !JS_IsObject(argv[1]))
    return JS_ThrowTypeError(ctx, "Require (string, object)");
  if (!scriptengine->getMember(argv[1], "description", desc) || !scriptengine->getMember(argv[1], "permission", level))
    return JS_ThrowTypeError(ctx, "Require (string, { description: string, permission: 0 | 1 | 2 | 3 | 4, ...})");
  autoval defs = JS_GetPropertyStr(js_context, argv[1], "overloads");
  autoval length = JS_GetPropertyStr(js_context, defs, "length");
  uint32_t len;
  JS_ToUint32(js_context, &len, length);
  auto registerOverload = registerCustomCommand(name, desc, level);
  for (uint32_t i = 0; i < len; i++) {
    autoval val = JS_GetPropertyUint32(js_context, defs, i);
    autoval params = JS_GetPropertyStr(js_context, val, "parameters");
    autoval handler = JS_GetPropertyStr(js_context, val, "handler");
    autoval param_length = JS_GetPropertyStr(js_context, params, "length");
    if (!JS_IsArray(js_context, params) || !JS_IsFunction(js_context, handler))
      return JS_ThrowTypeError(js_context, "Overload require { parameters: object, handler: function }");
    uint32_t param_len;
    if (JS_ToUint32(js_context, &param_len, param_length) == -1)
      return JS_ThrowTypeError(ctx, "The parameters of the command overload definition require array");
    MyCommandVTable mvt;
    for (uint32_t j = 0; j < param_len; j++) {
      autoval param = JS_GetPropertyUint32(js_context, params, j);
      std::string pname, ptype;
      bool poptional = JS_ToBool(js_context, JS_GetPropertyStr(js_context, param, "optional")) == 1;
      if (!scriptengine->getMember(param, "name", pname) || !scriptengine->getMember(param, "type", ptype))
        return JS_ThrowTypeError(ctx, "Parameter require { name: string, type: string, optional: bool }");
      if (ptype == "message")
        mvt.defs.emplace_back(commonParameter<CommandMessage>(pname));
      else if (ptype == "string")
        mvt.defs.emplace_back(commonParameter<std::string>(pname));
      else if (ptype == "int")
        mvt.defs.emplace_back(commonParameter<int>(pname));
      else if (ptype == "float")
        mvt.defs.push_back(commonParameter<float>(pname));
      else if (ptype == "bool")
        mvt.defs.push_back(commonParameter<bool>(pname));
      else if (ptype == "block")
        mvt.defs.push_back(commonParameter<Block const *>(pname));
      else if (ptype == "json")
        mvt.defs.push_back(commonParameter<Json::Value>(pname));
      else if (ptype == "entity")
        mvt.defs.push_back(commonParameter<CommandSelector<Actor>>(pname));
      else if (ptype == "player")
        mvt.defs.push_back(commonParameter<CommandSelector<Player>>(pname));
      else if (ptype == "position")
        mvt.defs.push_back(commonParameter<CommandPosition>(pname));
      else
        return JS_ThrowRangeError(js_context, "Parameter type %s hs not been supported", ptype.data());
      if (poptional)
        mvt.defs.rbegin()->makeOptional();
    }
    mvt.exec = JS_DupValue(js_context, handler);
    registerOverload(mvt);
  }
  return JS_UNDEFINED;
}

static JSCFunctionListEntry list[] = {
    JS_CFUNC_DEF("registerCommand", 2, &registerCommand),
};

static void entry(JSValue const &system) { JS_SetPropertyFunctionList(js_context, system, list, countof(list)); }

LAZY(register, quickjs_proto_extras.emplace_back(entry));