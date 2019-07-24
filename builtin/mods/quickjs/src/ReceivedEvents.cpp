#include <mods-quickjs/quickjs.hpp>

#include <minecraft/level/Level.h>
#include <minecraft/script/details/ReceivedEvents.h>

bool ScriptServerSpawnParticleAttachedToActor::getEventData(const ScriptApi::ScriptVersionInfo &, ScriptEngine &, ScriptServerContext &, const std::string &, ScriptApi::ScriptObjectHandle &handle) {
  QCHECK(scriptengine->createObject(handle));
  QCHECK(scriptengine->setMember(handle, "effect", ""));
  QCHECK(scriptengine->helpDefineVec3(handle, "offset", {}));
  QCHECK(scriptengine->setMember(handle, "entity", JS_NewObject(js_context)));
  return true;
}

bool ScriptServerSpawnParticleAttachedToActor::receivedEvent(const ScriptApi::ScriptVersionInfo &, ScriptEngine &, ScriptServerContext &, const std::string &,
                                                             const ScriptApi::ScriptObjectHandle &handle) {
  std::string effect;
  Vec3 offset;
  autohandle entity;
  Actor *actor;
  QCHECK(scriptengine->getMember(handle, "effect", effect));
  QCHECK(scriptengine->helpGetVec3(handle, "offset", offset));
  QCHECK(scriptengine->getMember(handle, "entity", entity));
  QCHECK(scriptengine->helpGetActor(entity, &actor));
  if (actor) {
    refs<ScriptServerContext>->level->spawnParticleEffect(effect, *actor, offset);
  } else {
    scriptengine->getScriptReportQueue().addWarning("Cannot found target entity");
  }
  return true;
}
