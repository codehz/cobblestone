#include "common.h"

#include "ScriptExtraDataComponent.h"

LAZY(register, init_hooks.emplace_back([] {
  scriptengine->getScriptTemplateFactory().components.data.emplace(ScriptExtraDataComponent::getName(), std::make_unique<ScriptExtraDataComponent>());
}));