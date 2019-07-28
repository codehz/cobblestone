#include <modloader/hook.hpp>
#include <modloader/log.hpp>
#include <modloader/utils.hpp>

#include <minecraft/actor/Actor.h>
#include <minecraft/actor/Player.h>

TInstanceHook(bool, _ZNK3Mob14isFrostWalkingEv, Actor) {
  if (auto player = dynamic_cast<Player *>(this); player) {
    if (player->getPlayerPermissionLevel() == 0)
      return false;
  }
  return original(this);
}

TInstanceHook(void, _ZN3Mob9frostWalkEv, Actor) {
  if (auto player = dynamic_cast<Player *>(this); player) {
    if (player->getPlayerPermissionLevel() == 0)
      return;
  }
  original(this);
}