#include <mods-quickjs/quickjs.hpp>

bool ScriptObjectBinder::serialize(ScriptEngine &engine, ScriptApi::ScriptObjectHandle &src) const {
  // Log::debug("QuickJS/ScriptObjectBinder", "serialize: %s", type.data()); // TODO: delete
  QCHECK(engine.setMember(src, TYPE_TAG, type));
  for (auto &component : components) { QCHECK(component->serialize(engine, src)); }
  return true;
}