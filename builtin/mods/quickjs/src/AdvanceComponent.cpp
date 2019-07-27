#include <mods-quickjs/advcomponent.hpp>

bool AdvanceComponent::retrieveComponentFrom(const ScriptApi::ScriptVersionInfo &, ScriptEngine &, ScriptServerContext &, const Block &, BlockSource &, BlockPos &,
                                             ScriptApi::ScriptObjectHandle &) const {
  return false;
}
bool AdvanceComponent::hasComponent(const ScriptApi::ScriptVersionInfo &, ScriptEngine &, ScriptServerContext &, const Block &, BlockSource &, BlockPos &, bool &) const { return false; }

bool AdvanceComponent::applyComponentTo(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &, ItemInstance &, ScriptApi::ScriptObjectHandle const &) const { return false; }
bool AdvanceComponent::retrieveComponentFrom(const ScriptApi::ScriptVersionInfo &, ScriptEngine &, ScriptServerContext &, ItemInstance &, ScriptApi::ScriptObjectHandle &) const { return false; }
bool AdvanceComponent::hasComponent(const ScriptApi::ScriptVersionInfo &, ScriptEngine &, ScriptServerContext &, ItemInstance &, bool &) const { return false; }