#pragma once

#include "../core/types.h"

#include <memory>

class CompoundTag;

class StructureTemplateData {
  char filler[168];

public:
  StructureTemplateData();
  virtual ~StructureTemplateData();

  void setStructureWorldOrigin(BlockPos const &);
  void setSize(BlockPos const &);

  std::unique_ptr<CompoundTag> save();
  bool load(std::unique_ptr<CompoundTag> const &);
};