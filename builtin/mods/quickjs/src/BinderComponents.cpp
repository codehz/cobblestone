#include <mods-quickjs/quickjs.hpp>

// * ScriptComponentBinderComponent
ScriptComponentBinderComponent::ScriptComponentBinderComponent() {}
ScriptComponentBinderComponent::ScriptComponentBinderComponent(ScriptApi::ScriptObjectHandle &&handle) : handle(handle) {}
ScriptComponentBinderComponent::~ScriptComponentBinderComponent() {}

// * ScriptEventDataBinderComponent
ScriptEventDataBinderComponent::ScriptEventDataBinderComponent(ScriptApi::ScriptObjectHandle &&handle) : handle(handle) {}
ScriptEventDataBinderComponent::~ScriptEventDataBinderComponent() {}

// * ScriptTickingAreaBinderComponent
ScriptTickingAreaBinderComponent::ScriptTickingAreaBinderComponent() {}
ScriptTickingAreaBinderComponent::ScriptTickingAreaBinderComponent(ScriptApi::ScriptObjectHandle &&handle) : handle(handle) {}
ScriptTickingAreaBinderComponent::~ScriptTickingAreaBinderComponent() {}

// * ScriptBlockPositionBinderComponent
ScriptBlockPositionBinderComponent::ScriptBlockPositionBinderComponent() {}
ScriptBlockPositionBinderComponent::ScriptBlockPositionBinderComponent(BlockPos const &pos) : pos(pos) {}
ScriptBlockPositionBinderComponent::~ScriptBlockPositionBinderComponent() {}
bool ScriptBlockPositionBinderComponent::serialize(ScriptEngine &, ScriptApi::ScriptObjectHandle const &target) const {
  autohandle temp = JS_NewObject(js_context);
  QCHECK(scriptengine->helpSetPosition(pos, temp));
  QCHECK(scriptengine->setMember(target, TAG, temp.transfer()));
  return true;
}
bool ScriptBlockPositionBinderComponent::deserialize(ScriptEngine &, ScriptApi::ScriptObjectHandle const &source) {
  autohandle temp;
  QCHECK(scriptengine->getMember(source, TAG, temp));
  QCHECK(scriptengine->helpGetPosition(temp, pos));
  return true;
}