#include "loader.hpp"

#include "elf.hpp"
#include <cstring>
#include <dirent.h>
#include <dlfcn.h>
#include <filesystem>
#include <modloader/hook.hpp>
#include <modloader/loader.hpp>
#include <modloader/log.hpp>
#include <set>
#include <unistd.h>

namespace fs = std::filesystem;

#ifndef MODLOADER_VERSION
#define MODLOADER_VERSION "unknown"
#endif

static ModLoaderImpl loaderImpl;

extern "C" {

const char *modloader_version() { return ModLoader::getVersion(); }

void modloader_add_lib_search_path(const char *path) { loaderImpl.addLibSearchDir(path); }

void *modloader_load_mod(const char *path) { return loaderImpl.loadMod(path); }

void modloader_load_mods_from_directory(const char *path) { loaderImpl.loadModsFromDirectory(path); }

void modloader_iterate_mods(modloader_foreach_fn cb, void *userdata) {
  for (void *v : loaderImpl.mods) cb(v, userdata);
}
}

const char *ModLoader::getVersion() { return MODLOADER_VERSION; }

void ModLoader::addLibSearchDir(std::string const &path) { loaderImpl.addLibSearchDir(path); }

void *ModLoader::loadMod(std::string const &path) { return loaderImpl.loadMod(path); }

void ModLoader::loadModsFromDirectory(std::string const &path) { loaderImpl.loadModsFromDirectory(path); }

void ModLoader::forEachMod(std::function<void(void *)> cb) {
  for (void *v : loaderImpl.mods) cb(v);
}

std::string ModLoaderImpl::findLib(std::string const &name) {
  for (fs::path const &dir : libDirs) {
    auto fullPath = dir / name;
    if (fs::exists(fullPath)) return fullPath;
  }
  return std::string();
}

void *ModLoaderImpl::loadLib(std::string const &path) {
  auto e = knownLoadedLibs.find(path);
  if (e != knownLoadedLibs.end()) return e->second;

  auto iof             = path.rfind('/');
  std::string fullPath = path;
  if (iof == std::string::npos) fullPath = findLib(path);

  if (!fullPath.empty()) {
    for (std::string const &dep : depelf(fullPath))
      if (!loadLib(dep)) {
        Log::error("ModLoader", "Abandon loading module %s, failed to load dependent module %s", fullPath.data(), dep.data());
        return nullptr;
      }
    Log::info("ModLoader", "Loading library: %s", fullPath.c_str());
  } else {
    fullPath = path;
  }

  void *ret = dlopen(fullPath.c_str(), RTLD_NOW);
  if (!ret) {
    Log::error("ModLoader", "Failed to load %s", dlerror());
    return nullptr;
  }
  std::string filename      = iof != std::string::npos ? path.substr(iof + 1) : path;
  knownLoadedLibs[filename] = ret;

  static std::set<bool (*)(char const *)> patches;

  auto do_patch = (bool (*)(char const *))dlsym(ret, "DO_PATCH");
  if (do_patch && !patches.contains(do_patch)) {
    patches.emplace(do_patch);
    size_t count = 0, ignored = 0;
    symelf(fullPath.c_str(), [&](std::string const &symname, std::string const &section) {
      if (section != ".patch" && !do_patch(symname.data())) return;
      void *src = dlsym(RTLD_DEFAULT, symname.data());
      if (src) {
        void *dst = dlsym(ret, symname.data());
        if (dst == src) return;
        void *temp;
        auto result = modloader_hook(src, dst, &temp);
        if (!result) throw std::runtime_error("Failed to patch: " + symname);
        count++;
        if (section == ".patch") Log::verbose("ModLoader/patch", "Patched(explicitly) %s", symname.data());
      } else if (section == ".patch") {
        throw std::runtime_error("Cannot found to patched symbol(explicitly): " + symname);
      } else {
        ignored++;
      }
    });
    Log::info("ModLoader/patch", "Force patched %zu, skipped %zu", count, ignored);
  }
  return ret;
}

void *ModLoaderImpl::loadMod(std::string const &path) {
  void *ret = loadLib(path);
  if (ret) mods.insert(ret);
  return ret;
}

void ModLoaderImpl::loadModsFromDirectory(std::string const &path) {
  Log::info("ModLoader", "Loading mods from directory: %s", path.c_str());
  DIR *dir = opendir(path.c_str());
  dirent *ent;
  if (dir == nullptr) {
    Log::info("ModLoader", "Directory does not exist");
    return;
  }
  while ((ent = readdir(dir)) != nullptr) {
    if (ent->d_name[0] == '.') continue;
    fs::path fileName(ent->d_name);
    if (fileName.extension() == ".so" || fileName.extension() == ".mod") loadMod(fileName);
  }
  closedir(dir);
  Log::info("ModLoader", "Loaded %li mods", mods.size());
}