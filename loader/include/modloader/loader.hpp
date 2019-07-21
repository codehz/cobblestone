#pragma once

#include <functional>
#include <string>

extern "C" {

const char *modloader_version();

typedef void (*modloader_foreach_fn)(void *handle, void *userdata);

void modloader_add_lib_search_path(const char *path);

void *modloader_load_mod(const char *path);

void modloader_load_mods_from_directory(const char *path);

void modloader_iterate_mods(modloader_foreach_fn cb, void *userdata);
}

class ModLoader {

public:
  static const char *getVersion();

  static void addLibSearchDir(std::string const &path);

  static void *loadMod(std::string const &path);

  static void loadModsFromDirectory(std::string const &path);

  static void forEachMod(std::function<void(void *)> cb);
};