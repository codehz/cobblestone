#include "common.h"

#include "ScriptExtraDataComponent.h"
#include "ScriptLoreComponent.h"

LAZY(register, init_hooks.emplace_back([] {
  scriptengine->getScriptTemplateFactory().components.data.emplace(ScriptExtraDataComponent::getName(), std::make_unique<ScriptExtraDataComponent>());
  scriptengine->getScriptTemplateFactory().components.data.emplace(ScriptLoreComponent::getName(), std::make_unique<ScriptLoreComponent>());
}));