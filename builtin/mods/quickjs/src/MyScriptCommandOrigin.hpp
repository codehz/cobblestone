#include <mods-quickjs/quickjs.hpp>

#include <minecraft/commands/CommandOrigin.h>
#include <minecraft/level/Level.h>

class MyScriptCommandOrigin : public ScriptCommandOrigin {
  std::function<void(Json::Value &&)> callback;

public:
  INLINE MyScriptCommandOrigin(ServerLevel &level, ScriptEngine &engine, std::function<void(Json::Value &&)> callback) : ScriptCommandOrigin(level, engine), callback(callback) {}
  inline virtual ~MyScriptCommandOrigin() {}
  inline virtual void handleCommandOutputCallback(Json::Value &&value) { callback(std::move(value)); }
  inline virtual CommandPermissionLevel getPermissionsLevel() const override { return (CommandPermissionLevel)4; }
};