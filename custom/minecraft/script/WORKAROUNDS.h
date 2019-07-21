#pragma once

#include "../core/types.h"

namespace entt {
template <typename> class Registry;
}

class Level;

namespace ScriptApi {
class WORKAROUNDS {
public:
  static void cacheActors(entt::Registry<unsigned int> &, Level &);

  struct tempActorComponent {
    ActorUniqueID id;
  };
};
} // namespace ScriptApi