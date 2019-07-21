#include "MyQuerySystem.hpp"
#include <mods-quickjs/quickjs.hpp>

#include <minecraft/level/Level.h>
#include <minecraft/script/ScriptVersionInfo.h>

ScriptApi::ScriptObjectHandle MyQuerySystem::queryEntities(int identifier) {
  if (identifier >= (int)queries.size()) return JS_ThrowRangeError(js_context, "Not a valid query identifier");
  auto &query                       = queries[identifier];
  auto hasFilter                    = query.filter.empty();
  ScriptApi::ScriptObjectHandle ret = JS_NewArray(js_context);
  ScriptApi::ScriptVersionInfo version;
  int idx = 0;
  refs<ScriptServerContext>->level->forEachActor([&](Dimension &dim, ActorUniqueID id, Actor *actor) {
    CLEANUP(QJS_FreeHandle) ScriptApi::ScriptObjectHandle temp = JS_NewObject(js_context);
    scriptengine->helpDefineActor(*actor, temp);
    if (hasFilter) {
      bool ok = true;
      for (auto const &filter : query.filter) {
        scriptengine->hasComponent(version, temp, filter, ok);
        if (!ok) return true;
      }
    }
    scriptengine->setMember(ret, idx++, temp.transfer());
    return true;
  });
  return ret;
}

#define INRANGE(value, low, high) (low <= value && value <= high)

ScriptApi::ScriptObjectHandle MyQuerySystem::queryEntities(int identifier, double x1, double x2, double y1, double y2, double z1, double z2) {
  if (identifier >= (int)queries.size()) return JS_ThrowRangeError(js_context, "Not a valid query identifier");
  auto &query                       = queries[identifier];
  auto hasFilter                    = query.filter.empty();
  ScriptApi::ScriptObjectHandle ret = JS_NewArray(js_context);
  ScriptApi::ScriptVersionInfo version;
  int idx = 0;
  refs<ScriptServerContext>->level->forEachActor([&](Dimension &dim, ActorUniqueID id, Actor *actor) -> bool {
    CLEANUP(QJS_FreeHandle) ScriptApi::ScriptObjectHandle temp;
    scriptengine->helpDefineActor(*actor, temp);
    bool ok = true;
    if (scriptengine->hasComponent(version, temp, query.name, ok) && ok) {
      double x, y, z;
      CLEANUP(QJS_FreeHandle) ScriptApi::ScriptObjectHandle component;
      if (!scriptengine->getComponent(version, temp, query.name, component)) {
        ret = JS_ThrowInternalError(js_context, "Cannot get component");
        return false;
      }
      CLEANUP(QJS_FreeHandle) ScriptApi::ScriptObjectHandle data;
      if (!scriptengine->getMember(component, "data", data)) {
        ret = JS_ThrowInternalError(js_context, "Cannot get compoent data");
        return false;
      }
      if (!scriptengine->getMember(data, query.x, x)) {
        ret = JS_ThrowInternalError(js_context, "Cannot get compoent data.%s", query.x.data());
        return false;
      }
      if (!scriptengine->getMember(data, query.y, y)) {
        ret = JS_ThrowInternalError(js_context, "Cannot get compoent data.%s", query.y.data());
        return false;
      }
      if (!scriptengine->getMember(data, query.z, z)) {
        ret = JS_ThrowInternalError(js_context, "Cannot get compoent data.%s", query.z.data());
        return false;
      }
      if (!INRANGE(x, x1, x2) || !INRANGE(x, y1, y2) || !INRANGE(x, z1, z2)) return true;
    }
    if (hasFilter) {
      for (auto const &filter : query.filter) {
        scriptengine->hasComponent(version, temp, filter, ok);
        if (!ok) return true;
      }
    }
    scriptengine->setMember(ret, idx++, temp.transfer());
    return true;
  });
  return ret;
}