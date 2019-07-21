#include <minecraft/actor/Player.h>
#include <minecraft/dimension/Dimension.h>
#include <minecraft/level/Level.h>
#include <modloader/utils.hpp>

std::vector<std::unique_ptr<Actor>> &Level::getActorVector() const { return fetch<std::vector<std::unique_ptr<Actor>>, 6784>(this); }

void Level::forEachActor(std::function<bool(Dimension &, ActorUniqueID, Actor *)> fn) {
  forEachDimension([&](Dimension &dim) {
    for (auto &[id, actor] : dim.getEntityIdMap())
      if (!fn(dim, id, actor)) return false;
    bool cont = true;
    dim.forEachPlayer([&](Player &target) -> bool {
      if (!fn(dim, target.getUniqueID(), &target)) return cont = false;
      return true;
    });
    return cont;
  });
}