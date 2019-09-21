#include <modloader/hook.hpp>
#include <modloader/log.hpp>
#include <modloader/refs.hpp>
#include <modloader/utils.hpp>
#include <mutex>
#include <sstream>
#include <unordered_set>
#include <vector>

#include <minecraft/actor/Player.h>

std::vector<std::string> split(std::string str, char delimiter) {
  std::vector<std::string> internal;
  std::stringstream ss(str);
  std::string tok;

  while (getline(ss, tok, delimiter)) { internal.push_back(tok); }

  return internal;
}

struct PropertiesSettings {
  std::string const &getCustomProperty(std::string const &) const;

  std::vector<std::string> getDisabledCommands() const { return split(getCustomProperty("disabled-builtin-commands"), ','); }
};

inline namespace MOD_BUILTIN_COMMAND {
std::unordered_set<std::string> set;
std::once_flag oce;
struct initializer {
  void operator()() {
    for (auto &x : refs<PropertiesSettings>->getDisabledCommands()) { set.insert(x); }
  }
} initializer;
} // namespace MOD_BUILTIN_COMMAND

THook(void, _ZN11TellCommand5setupER15CommandRegistry, void *registry) {
  std::call_once(oce, initializer);
  if (!set.contains("tell")) original(registry);
}
THook(void, _ZN14TellRawCommand5setupER15CommandRegistry, void *registry) {
  std::call_once(oce, initializer);
  if (!set.contains("tellraw")) original(registry);
}
THook(void, _ZN10SayCommand5setupER15CommandRegistry, void *registry) {
  std::call_once(oce, initializer);
  if (!set.contains("say")) original(registry);
}
THook(void, _ZN9MeCommand5setupER15CommandRegistry, void *registry) {
  std::call_once(oce, initializer);
  if (!set.contains("me")) original(registry);
}

// TInstanceHook(bool, _ZNK3Mob14isFrostWalkingEv, Mob) {
//   if (this->hasCategory(ActorCategory::Player)) {
//     auto player = (Player *)this;
//     if (player->getPlayerPermissionLevel() == 0)
//       return false;
//   }
//   return original(this);
// }

// TInstanceHook(void, _ZN3Mob9frostWalkEv, Mob) {
//   if (this->hasCategory(ActorCategory::Player)) {
//     auto player = (Player *)this;
//     if (player->getPlayerPermissionLevel() == 0)
//       return;
//   }
//   original(this);
// }