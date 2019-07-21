#include <mods-quickjs/quickjs.hpp>

#include <minecraft/actor/Actor.h>
#include <minecraft/actor/AttributeInstance.h>
#include <minecraft/actor/BaseAttributeMap.h>
#include <minecraft/actor/EntityContext.h>
#include <minecraft/actor/Player.h>
#include <minecraft/actor/PlayerInventoryProxy.h>
#include <minecraft/actor/SharedAttributes.h>
#include <minecraft/component/ContainerComponent.h>
#include <minecraft/component/TickWorldComponent.h>
#include <minecraft/container/SimpleContainer.h>
#include <minecraft/item/ItemInstance.h>
#include <minecraft/level/Level.h>
#include <minecraft/script/details/Components.h>

#define IMPL_RETRIEVE(name)                                                                                                                          \
  bool name::retrieveComponentFrom(ScriptApi::ScriptVersionInfo const &version, ScriptEngine &engine, ScriptServerContext &ctx, Actor &actor,        \
                                   ScriptApi::ScriptObjectHandle &target) const
#define IMPL_APPLY(name)                                                                                                                             \
  bool name::applyComponentTo(ScriptApi::ScriptVersionInfo const &version, ScriptEngine &engine, ScriptServerContext &ctx, Actor &actor,             \
                              ScriptApi::ScriptObjectHandle const &target) const

auto helpDefineItemSlots(std::vector<const ItemStack *> const &slots, ScriptApi::ScriptObjectHandle &target) {
  target  = JS_NewArray(js_context);
  int idx = 0;
  for (auto &slot : slots) {
    ItemInstance instance{ *slot };
    CLEANUP(QJS_FreeHandle) ScriptApi::ScriptObjectHandle temp = JS_NewObject(js_context);
    QCHECK(scriptengine->helpDefineItemStack({ *slot }, temp));
    QCHECK(scriptengine->setMember(target, idx++, temp.transfer()));
  }
  return true;
}

/* ------------------------------- begin impl ------------------------------- */

IMPL_RETRIEVE(ScriptArmorContainerComponent) {
  auto slots = actor.getArmorContainer().getSlots();
  return helpDefineItemSlots(slots, target);
}

IMPL_RETRIEVE(ScriptAttackComponent) {
  if (!actor.getActorDefinitionDescriptor()) {
    scriptengine->getScriptReportQueue().addError("cannot get attack component");
    return false;
  }
  auto &damage_attr = actor.getAttribute(SharedAttributes::ATTACK_DAMAGE);
  if (!damage_attr.isValid()) {
    scriptengine->getScriptReportQueue().addError("cannot get attack damage");
    return false;
  }
  CLEANUP(QJS_FreeHandle) ScriptApi::ScriptObjectHandle temp = JS_NewObject(js_context);
  QCHECK(scriptengine->setMember(temp, "range_min", damage_attr.getMinValue()));
  QCHECK(scriptengine->setMember(temp, "range_max", damage_attr.getMaxValue()));
  QCHECK(scriptengine->createObject(target));
  QCHECK(scriptengine->setMember(target, "damage", temp.transfer()));
  return true;
}

IMPL_APPLY(ScriptAttackComponent) {
  auto attrs = actor.getAttributes();
  if (!attrs) {
    scriptengine->getScriptReportQueue().addError("cannot get attack damage");
    return false;
  }
  CLEANUP(QJS_FreeHandle) ScriptApi::ScriptObjectHandle temp;
  QCHECK(scriptengine->getMember(target, "damage", temp));
  double min, max;
  QCHECK(scriptengine->getMember(temp, "range_min", min));
  QCHECK(scriptengine->getMember(temp, "range_max", max));
  attrs->registerAttribute(SharedAttributes::ATTACK_DAMAGE).setRange(min, min, max);
  attrs->registerAttribute(SharedAttributes::ATTACK_DAMAGE).resetToDefaultValue();
  return true;
}

IMPL_RETRIEVE(ScriptHandContainerComponent) {
  auto slots = actor.getHandContainer().getSlots();
  return helpDefineItemSlots(slots, target);
}

IMPL_RETRIEVE(ScriptHotbarContainerComponent) {
  if (!actor.hasCategory(ActorCategory::Player)) {
    scriptengine->getScriptReportQueue().addError("Hotbar container is only valid on players");
    return false;
  }
  auto slots = ((Player &)actor).getSupplies().getSlots();
  return helpDefineItemSlots(slots, target);
}

IMPL_RETRIEVE(ScriptInventoryContainerComponent) {
  if (actor.hasCategory(ActorCategory::Player)) {
    auto slots = ((Player &)actor).getSupplies().getSlots();
    return helpDefineItemSlots(slots, target);
  } else {
    if (auto component = actor.tryGetComponent<ContainerComponent>(); component) {
      auto slots = component->getSlots();
      return helpDefineItemSlots(slots, target);
    } else {
      scriptengine->getScriptReportQueue().addError("Target entity don't have container component");
      return false;
    }
  }
}

IMPL_RETRIEVE(ScriptTickWorldComponent) {
  CLEANUP(QJS_FreeHandle) ScriptApi::ScriptObjectHandle temp = JS_NewObject(js_context);
  double distance_to_players                                 = 0.0;
  int radius;
  bool never_despawn = false;
  if (auto component = actor.tryGetComponent<TickWorldComponent>(); component) {
    auto ticking_area = component->getTickingArea();
    if (!ticking_area) {
      scriptengine->getScriptReportQueue().addError("cannot get tick_area");
      return false;
    }
    distance_to_players = component->getMaxDistToPlayers();
    radius              = component->getChunkRadius();
    never_despawn       = component->isAlwaysActive();
    if (!scriptengine->helpDefineTickingArea(version, temp, *ticking_area)) {
      scriptengine->getScriptReportQueue().addError("cannot define ticking area");
      return false;
    }
  } else if (actor.hasCategory(ActorCategory::Player)) {
    radius = ctx.level->getChunkTickRange();
    if (!scriptengine->helpDefineTickingArea(version, temp, actor.getUniqueID())) {
      scriptengine->getScriptReportQueue().addError("cannot define ticking area");
      return false;
    }
  } else {
    scriptengine->getScriptReportQueue().addError("cannot get tick_area component and target is not player");
    return false;
  }
  QCHECK(scriptengine->createObject(target));
  QCHECK(scriptengine->setMember(target, "distance_to_players", distance_to_players));
  QCHECK(scriptengine->setMember(target, "radius", radius));
  QCHECK(scriptengine->setMember(target, "never_despawn", never_despawn));
  QCHECK(scriptengine->setMember(target, "ticking_area", temp.transfer()));
  return true;
}