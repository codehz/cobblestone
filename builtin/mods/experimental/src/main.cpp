#include <minecraft/json.h>
#include <modloader/hook.hpp>
#include <modloader/log.hpp>
#include <modloader/refs.hpp>
#include <modloader/utils.hpp>
#include <netdb.h>
#include <string>
#include <sys/socket.h>

struct PropertiesSettings {
  std::string const &getCustomProperty(std::string const &) const;

  static bool parseBoolValue(std::string const &);

  bool getExperimentalMode() const { return parseBoolValue(getCustomProperty("experimental-mode")); }

  bool getOverrideSavedSettings() const { return parseBoolValue(getCustomProperty("override-saved-settings")); }
};

TClasslessInstanceHook(bool, _ZNK9LevelData30hasExperimentalGameplayEnabledEv) {
  if (refs<PropertiesSettings>->getExperimentalMode())
    return true;
  return original(this);
}

struct LevelSettings {
  void setForceExperimentalGameplay(bool val);
};

TInstanceHook(bool, _ZNK13LevelSettings31shouldForceExperimentalGameplayEv, LevelSettings) {
  if (refs<PropertiesSettings>->getExperimentalMode()) {
    setForceExperimentalGameplay(true);
  }
  return original(this);
}

TClasslessInstanceHook(bool, _ZNK13LevelSettings27shouldOverrideSavedSettingsEv) { return refs<PropertiesSettings>->getOverrideSavedSettings(); }

TClasslessInstanceHook(void, _ZN19ResourcePackManager23setExperimentalGameplayEb, bool flag) { original(this, refs<PropertiesSettings>->getExperimentalMode() || flag); }

TClasslessInstanceHook(bool, _ZNK19ResourcePackManager22isExperimentalGameplayEv) {
  if (refs<PropertiesSettings>->getExperimentalMode())
    return true;
  return original(this);
}

TClasslessInstanceHook(bool, _ZNK12PackInstance9isVanillaEv) {
  bool ret = original(this);
  if (refs<PropertiesSettings>->getExperimentalMode())
    return true;
  return ret;
}

TClasslessInstanceHook(bool, _ZNK6Player16forceAllowEatingEv) { return true; }

TClasslessInstanceHook(void *, _ZN8GameRule17setRequiresCheatsEb, bool flag) { return original(this, false); }

TClasslessInstanceHook(void *, _ZN8GameRule17setAllowInCommandEb, bool flag) { return original(this, true); }

TClasslessInstanceHook(bool, _ZNK15CommandRegistry21requiresCheatsEnabledERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE) { return false; }