#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <filesystem>

class ModLoaderImpl {

public:
  std::vector<std::filesystem::path> libDirs;
  std::unordered_map<std::string, void *> knownLoadedLibs;
  std::unordered_set<void *> mods;

  void addLibSearchDir(std::string const &path) { libDirs.push_back(path); }

  std::string findLib(std::string const &name);

  void *loadLib(std::string const &path);

  void *loadMod(std::string const &path);

  void loadModsFromDirectory(std::string const &path);
};