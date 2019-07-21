#pragma once

#include <minecraft/script/ScriptTemplateFactory.h>

class AdvanceComponent {
public:
  virtual bool retrieveComponentFrom(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &, Block const &, BlockSource &,
                                     BlockPos &, ScriptApi::ScriptObjectHandle &) const = 0;
  virtual bool hasComponent(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &, Block const &, BlockSource &, BlockPos &,
                            bool &) const                                               = 0;
};