#include "common.h"

#include "ScriptLoreComponent.h"
#include <minecraft/actor/Actor.h>
#include <minecraft/actor/ItemActor.h>
#include <minecraft/item/ItemStack.h>

std::string ScriptLoreComponent::getName() { return "stone:lore"; }

ScriptLoreComponent::ScriptLoreComponent() {}
ScriptLoreComponent::~ScriptLoreComponent() {}

bool ScriptLoreComponent::applyComponentTo(ScriptApi::ScriptVersionInfo const &version, ScriptEngine &, ScriptServerContext &ctx, Actor &actor, ScriptApi::ScriptObjectHandle const &obj) const {
  if (!JS_IsArray(js_context, obj)) {
    scriptengine->getScriptReportQueue().addError("stone:lore requires array");
    return false;
  }
  autoval lengthProp = JS_GetPropertyStr(js_context, obj, "length");
  int32_t length;
  JS_ToInt32(js_context, &length, lengthProp);
  std::vector<std::string> lores;
  for (int32_t i = 0; i < length; i++) {
    autoval item = JS_GetPropertyUint32(js_context, obj, (uint32_t)i);
    autostr lore = JS_ToCString(js_context, item);
    lores.emplace_back(lore);
  }
  if (auto itemactor = dynamic_cast<ItemActor *>(&actor); itemactor) {
    itemactor->getItemStack().setCustomLore(lores);
    return true;
  }
  return false;
}

bool ScriptLoreComponent::hasComponent(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &, Actor &actor, bool &result) const {
  result = dynamic_cast<ItemActor *>(&actor);
  return true;
}

bool ScriptLoreComponent::retrieveComponentFrom(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &ctx, Actor &actor, ScriptApi::ScriptObjectHandle &target) const {
  if (auto itemactor = dynamic_cast<ItemActor *>(&actor); itemactor) {
    auto lores = itemactor->getItemStack().getCustomLore();
    target = JS_NewArray(js_context);
    uint32_t idx = 0;
    for (auto &lore : lores) {
      JS_SetPropertyUint32(js_context, target, idx++, JS_NewString(js_context, lore.c_str()));
    }
    return true;
  }
  return false;
}

bool ScriptLoreComponent::applyComponentTo(ScriptApi::ScriptVersionInfo const &version, ScriptEngine &, ScriptServerContext &ctx, ItemInstance &item, ScriptApi::ScriptObjectHandle const &obj) const {
  if (!JS_IsArray(js_context, obj)) {
    scriptengine->getScriptReportQueue().addError("stone:lore requires array");
    return false;
  }
  autoval lengthProp = JS_GetPropertyStr(js_context, obj, "length");
  int32_t length;
  JS_ToInt32(js_context, &length, lengthProp);
  std::vector<std::string> lores;
  for (int32_t i = 0; i < length; i++) {
    autoval item = JS_GetPropertyUint32(js_context, obj, (uint32_t)i);
    autostr lore = JS_ToCString(js_context, item);
    lores.emplace_back(lore);
  }
  item.setCustomLore(lores);
  return true;
}

bool ScriptLoreComponent::hasComponent(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &, ItemInstance &, bool &result) const {
  result = true;
  return true;
}

bool ScriptLoreComponent::retrieveComponentFrom(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &ctx, ItemInstance &item, ScriptApi::ScriptObjectHandle &target) const {
  auto lores = item.getCustomLore();
  target = JS_NewArray(js_context);
  uint32_t idx = 0;
  for (auto &lore : lores) {
    JS_SetPropertyUint32(js_context, target, idx++, JS_NewString(js_context, lore.c_str()));
  }
  return true;
}