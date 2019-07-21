#include <modloader/hook.hpp>
#include <modloader/log.hpp>
#include <modloader/refs.hpp>
#include <netdb.h>
#include <string>
#include <sys/socket.h>

struct PropertiesSettings {
  std::string const &getCustomProperty(std::string const &) const;

  static bool parseBoolValue(std::string const &);

  bool getExperimentalMode() const { return parseBoolValue(getCustomProperty("experimental-mode")); }
};

TClasslessInstanceHook(bool, _ZNK9LevelData30hasExperimentalGameplayEnabledEv) {
  if (refs<PropertiesSettings>->getExperimentalMode()) return true;
  return original(this);
}

TClasslessInstanceHook(bool, _ZNK13LevelSettings31shouldForceExperimentalGameplayEv) {
  if (refs<PropertiesSettings>->getExperimentalMode()) return true;
  return original(this);
}