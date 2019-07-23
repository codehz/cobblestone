#include <fcntl.h>
#include <modloader/hook.hpp>
#include <modloader/refs.hpp>
#include <string>
#include <unistd.h>

namespace Core {
struct FilePathManager {};
} // namespace Core

struct PropertiesSettings;
MakeStaticReference(PropertiesSettings);

THook(void, _ZN19InPackagePackSource12PackMetaDataC2ERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEEb12PackCategory, void *self,
      std::string const &path, bool flag, int mask) {
  original(self, "game/" + path, flag, mask);
}

TInstanceHook(std::string, _ZNK4Core15FilePathManager15getUserDataPathB5cxx11Ev, Core::FilePathManager) { return "data/"; }

TInstanceHook(std::string, _ZNK4Core15FilePathManager13getWorldsPathB5cxx11Ev, Core::FilePathManager) { return "data/worlds"; }

THook(std::string, _ZN11AppPlatform15getSettingsPathB5cxx11Ev, void *) { return "data/"; }

THook(std::string, _ZNK11AppPlatform14getLoggingPathB5cxx11Ev, void *) { return "data/"; }

THook(std::string const &, _ZNK17AppPlatform_linux19getPlatformTempPathB5cxx11Ev, void *) {
  static std::string temp = "data/";
  return temp;
}

THook(std::string, _ZN22ResourcePackRepository18getPremiumPackPathB5cxx11Ev, void *) { return "data/premium_cache"; }

THook(std::string, _ZNK22ResourcePackRepository21getTreatmentPacksPathB5cxx11Ev, void *) { return "data/treatments"; }

THook(std::string, _ZN22ResourcePackRepository26getCachedBehaviorPacksPathB5cxx11Ev, void *) { return "data/packcache/behavior"; }

THook(std::string, _ZN22ResourcePackRepository26getCachedResourcePacksPathB5cxx11Ev, void *) { return "data/packcache/resource"; }

THook(std::string, _ZN22ResourcePackRepository26getGlobalResourcePacksPathB5cxx11Ev, void *self) { return "data/" + original(self); }

THook(std::string, _ZNK22ResourcePackRepository17getKnownPacksPathB5cxx11E13KnownPackType, void *self, int type) {
  return "data/" + original(self, type);
}

static void touch(char const *name) {
  auto fd = open(name, O_RDWR | O_CREAT, 0644);
  close(fd);
}

THook(void, _ZN18PropertiesSettingsC2ERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE, PropertiesSettings *self, std::string const &) {
  touch("data/server.properties");
  refs<PropertiesSettings> = self;
  original(self, "data/server.properties");
}

THook(void, _ZN13WhitelistFileC2ERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE, void *self, std::string const &) {
  touch("data/whitelist.json");
  original(self, "data/whitelist.json");
}

THook(void, _ZN15PermissionsFileC2ERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE, void *self, std::string const &) {
  touch("data/permissions.json");
  original(self, "data/permissions.json");
}

THook(void, _ZN17AppPlatform_linuxC2Ev, void *self) {
  original(self);
  *reinterpret_cast<std::string *>(reinterpret_cast<char *>(self) + 624) = "game/";
  *reinterpret_cast<std::string *>(reinterpret_cast<char *>(self) + 656) = "data/";
}