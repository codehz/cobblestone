#pragma once

#include "StructureTemplateData.h"

class BlockSource;
class BlockPalette;
struct StructureSettings;

class StructureTemplate {
  std::string name;
  StructureTemplateData data;

public:
  inline StructureTemplate(std::string const &name = "") : name(name) {}

  std::unique_ptr<CompoundTag> save();
  bool load(std::unique_ptr<CompoundTag> const &);

  bool placeInWorld(BlockSource &, BlockPalette const &, BlockPos, StructureSettings const &) const;
  void fillFromWorld(BlockSource &, BlockPos const &, StructureSettings const &);
};