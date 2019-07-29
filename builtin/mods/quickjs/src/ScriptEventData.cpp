#include <mods-quickjs/quickjs.hpp>

#include <minecraft/item/ItemAcquisitionMethodMap.h>
#include <minecraft/item/ItemUseMethodMap.h>
#include <minecraft/script/details/ScriptEventData.h>

#define IMPL(name) bool name::_serialize(ScriptEngine &, ScriptApi::ScriptObjectHandle &handle) const
#define PREPARE_OBJECT(obj) QCHECK(scriptengine->createObject(obj))
#define NEW_OBJECT(name)                                                                                                                                                                               \
  autohandle name;                                                                                                                                                                                     \
  PREPARE_OBJECT(name)

#define FOR_ENTITY(name, var)                                                                                                                                                                          \
  ({                                                                                                                                                                                                   \
    NEW_OBJECT(temp);                                                                                                                                                                                  \
    if (scriptengine->helpDefineActor(var, temp)) {                                                                                                                                                    \
      QCHECK(scriptengine->setMember(handle, name, temp.transfer()));                                                                                                                                  \
    } else {                                                                                                                                                                                           \
      QCHECK(scriptengine->setMember(handle, name, JS_NULL));                                                                                                                                          \
    }                                                                                                                                                                                                  \
  })

#define FOR_ITEM(name, var)                                                                                                                                                                            \
  ({                                                                                                                                                                                                   \
    NEW_OBJECT(temp);                                                                                                                                                                                  \
    QCHECK(scriptengine->helpDefineItemStack(var, temp));                                                                                                                                              \
    QCHECK(scriptengine->setMember(handle, name, temp.transfer()));                                                                                                                                    \
  })

#define FOR_POS(name, var)                                                                                                                                                                             \
  ({                                                                                                                                                                                                   \
    NEW_OBJECT(temp);                                                                                                                                                                                  \
    QCHECK(scriptengine->helpDefinePosition(var, temp));                                                                                                                                               \
    QCHECK(scriptengine->setMember(handle, name, temp.transfer()));                                                                                                                                    \
  })

IMPL(ScriptServerActorAcquiredItemEvent) {
  PREPARE_OBJECT(handle);
  FOR_ENTITY("entity", entity);
  FOR_ITEM("item_stack", item);
  auto methodname = ItemAcquisitionMethodMap::getItemAcquisitionMethodName(method);
  QCHECK(scriptengine->setMember(handle, "acquisition_method", methodname));
  QCHECK(scriptengine->setMember(handle, "acquisition_amount", (int)amount));
  FOR_ENTITY("secondary_entity", secondary_entity);
  return true;
}

IMPL(ScriptServerActorCarriedItemChangedEvent) {
  PREPARE_OBJECT(handle);
  FOR_ENTITY("entity", entity);
  FOR_ITEM("previous_carried_item", previous_carried_item);
  FOR_ITEM("carried_item", carried_item);
  return true;
}

IMPL(ScriptServerActorCreatedEvent) {
  PREPARE_OBJECT(handle);
  FOR_ENTITY("entity", entity);
  return true;
}

IMPL(ScriptServerActorDeathEvent) {
  PREPARE_OBJECT(handle);
  FOR_ENTITY("entity", entity);
  return true;
}

IMPL(ScriptServerActorDroppedItemEvent) {
  PREPARE_OBJECT(handle);
  FOR_ENTITY("entity", entity);
  FOR_ITEM("item_stack", item);
  return true;
}

IMPL(ScriptServerActorEquippedArmorEvent) {
  PREPARE_OBJECT(handle);
  FOR_ENTITY("entity", entity);
  FOR_ITEM("item_stack", item);
  return true;
}

IMPL(ScriptServerActorStartRidingEvent) {
  PREPARE_OBJECT(handle);
  FOR_ENTITY("entity", entity);
  FOR_ENTITY("ride", ride);
  return true;
}

IMPL(ScriptServerActorStopRidingEvent) {
  PREPARE_OBJECT(handle);
  FOR_ENTITY("entity", entity);
  QCHECK(scriptengine->setMember(handle, "exit_from_rider", exit_from_rider));
  QCHECK(scriptengine->setMember(handle, "entity_is_being_destroyed", entity_is_being_destroyed));
  QCHECK(scriptengine->setMember(handle, "switching_rides", switching_rides));
  return true;
}

IMPL(ScriptServerActorTickEvent) {
  PREPARE_OBJECT(handle);
  FOR_ENTITY("entity", entity);
  return true;
}

IMPL(ScriptServerActorUseItemEvent) {
  PREPARE_OBJECT(handle);
  FOR_ENTITY("entity", entity);
  FOR_ITEM("item_stack", item);
  auto methodname = ItemUseMethodMap::getItemUseMethodName(use_method);
  QCHECK(scriptengine->setMember(handle, "use_member", methodname));
  return true;
}

IMPL(ScriptServerBlockDestructionStartedEvent) {
  PREPARE_OBJECT(handle);
  FOR_ENTITY("player", entity);
  FOR_POS("block_position", pos);
  return true;
}

IMPL(ScriptServerBlockDestructionStoppedEvent) {
  PREPARE_OBJECT(handle);
  FOR_ENTITY("player", entity);
  FOR_POS("block_position", pos);
  QCHECK(scriptengine->setMember(handle, "destruction_progress", progress));
  return true;
}

IMPL(ScriptServerBlockInteractedWithEvent) {
  PREPARE_OBJECT(handle);
  FOR_ENTITY("player", entity);
  FOR_POS("block_position", pos);
  return true;
}

IMPL(ScriptServerBlockMovedByPistonEvent) {
  PREPARE_OBJECT(handle);
  FOR_POS("piston_position", piston);
  FOR_POS("block_position", block);
  QCHECK(scriptengine->setMember(handle, "piston_action", action));
  return true;
}

IMPL(ScriptServerPlayerDestroyedBlockEvent) {
  PREPARE_OBJECT(handle);
  FOR_ENTITY("player", entity);
  FOR_POS("block_position", block);
  QCHECK(scriptengine->setMember(handle, "block_identifier", identifier));
  return true;
}

IMPL(ScriptServerPlayerPlacedBlockEvent) {
  PREPARE_OBJECT(handle);
  FOR_ENTITY("player", entity);
  FOR_POS("block_position", block);
  return true;
}

IMPL(ScriptPlayerAttackedActorEvent) {
  PREPARE_OBJECT(handle);
  FOR_ENTITY("player", player);
  FOR_ENTITY("attacked_entity", entity);
  return true;
}