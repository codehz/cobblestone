#include "common.h"

#include "ScriptDimensionComponent.h"

#include <minecraft/actor/Actor.h>

std::string ScriptDimensionComponent::getName() { return "stone:dimension"; }

ScriptDimensionComponent::ScriptDimensionComponent() {}
ScriptDimensionComponent::~ScriptDimensionComponent() {}

bool ScriptDimensionComponent::applyComponentTo(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &, Actor &actor, ScriptApi::ScriptObjectHandle const &source) const {
  int dim;
  QCHECK(scriptengine->getValue(source, dim) && dim >= 0);
  actor.changeDimension({dim}, false);
  return true;
}

bool ScriptDimensionComponent::retrieveComponentFrom(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &, Actor &actor, ScriptApi::ScriptObjectHandle &target) const {
  target = JS_NewInt32(js_context, actor.getDimensionId());
  return true;
}
bool ScriptDimensionComponent::hasComponent(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &, Actor &, bool &result) const {
  result = true;
  return true;
}