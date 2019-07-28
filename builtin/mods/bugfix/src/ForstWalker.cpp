#include <modloader/hook.hpp>
#include <modloader/log.hpp>
#include <modloader/utils.hpp>

#include <minecraft/actor/Player.h>

TInstanceHook(bool, _ZNK3Mob14isFrostWalkingEv, Mob) {
  if (this->hasCategory(ActorCategory::Player)) {
    auto player = (Player *)this;
    if (player->getPlayerPermissionLevel() == 0)
      return false;
  }
  return original(this);
}

TInstanceHook(void, _ZN3Mob9frostWalkEv, Mob) {
  if (this->hasCategory(ActorCategory::Player)) {
    auto player = (Player *)this;
    if (player->getPlayerPermissionLevel() == 0)
      return;
  }
  original(this);
}