#pragma once

#include <minecraft/item/ItemInstance.h>
#include <minecraft/script/ScriptTemplateFactory.h>

class AdvanceComponent {
public:
  virtual bool retrieveComponentFrom(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &, Block const &, BlockSource &, BlockPos &, ScriptApi::ScriptObjectHandle &) const;
  virtual bool hasComponent(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &, Block const &, BlockSource &, BlockPos &, bool &) const;

  virtual bool applyComponentTo(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &, ItemInstance &, ScriptApi::ScriptObjectHandle const &) const;
  virtual bool retrieveComponentFrom(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &, ItemInstance &, ScriptApi::ScriptObjectHandle &) const;
  virtual bool hasComponent(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &, ItemInstance &, bool &) const;
};