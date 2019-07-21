#include <mods-quickjs/common.hpp>
#include <mods-quickjs/quickjs.hpp>

namespace ScriptApi {

bool ScriptFramework::setMember(ScriptObjectHandle const &handle, std::string const &name, bool value) {
  ScriptObjectHandle temp;
  setValue(temp, value);
  return setMember(handle, name, temp);
}

bool ScriptFramework::setMember(ScriptObjectHandle const &handle, std::string const &name, double value) {
  ScriptObjectHandle temp;
  setValue(temp, value);
  return setMember(handle, name, temp);
}

bool ScriptFramework::setMember(ScriptObjectHandle const &handle, std::string const &name, int value) {
  ScriptObjectHandle temp;
  setValue(temp, value);
  return setMember(handle, name, temp);
}

bool ScriptFramework::setMember(ScriptObjectHandle const &handle, std::string const &name, long value) {
  ScriptObjectHandle temp;
  setValue(temp, value);
  return setMember(handle, name, temp);
}

bool ScriptFramework::setMember(ScriptObjectHandle const &handle, std::string const &name, std::string const &value) {
  ScriptObjectHandle temp;
  setValue(temp, value);
  return setMember(handle, name, temp);
}

bool ScriptFramework::getMember(ScriptObjectHandle const &handle, std::string const &name, bool &value) {
  CLEANUP(QJS_FreeHandle) ScriptObjectHandle temp;
  QCHECK(getMember(handle, name, temp));
  return getValue(temp, value);
}

bool ScriptFramework::getMember(ScriptObjectHandle const &handle, std::string const &name, double &value) {
  CLEANUP(QJS_FreeHandle) ScriptObjectHandle temp;
  QCHECK(getMember(handle, name, temp));
  return getValue(temp, value);
}

bool ScriptFramework::getMember(ScriptObjectHandle const &handle, std::string const &name, int &value) {
  CLEANUP(QJS_FreeHandle) ScriptObjectHandle temp;
  QCHECK(getMember(handle, name, temp));
  return getValue(temp, value);
}

bool ScriptFramework::getMember(ScriptObjectHandle const &handle, std::string const &name, long &value) {
  CLEANUP(QJS_FreeHandle) ScriptObjectHandle temp;
  QCHECK(getMember(handle, name, temp));
  return getValue(temp, value);
}

bool ScriptFramework::getMember(ScriptObjectHandle const &handle, std::string const &name, std::string &value) {
  CLEANUP(QJS_FreeHandle) ScriptObjectHandle temp;
  QCHECK(getMember(handle, name, temp));
  return getValue(temp, value);
}

bool ScriptFramework::registerGlobalAPI(std::string const &name, ScriptCallbackInterface &cb, ScriptObjectHandle &handle) {
  QCHECK(createObject(handle));
  CLEANUP(QJS_FreeHandle) ScriptObjectHandle global;
  CLEANUP(QJS_FreeHandle) ScriptObjectHandle server;
  QCHECK(getGlobalObject(global));
  QCHECK(setMember(global, name, handle));
  ifce->callbacks = &cb;
  QCHECK(ifce->defineServerCallbacks(handle, getScriptReportQueue()));
  QCHECK(getMember(global, "server", server));
  for (auto &fn : quickjs_extras) fn(global, server);
  return true;
}

} // namespace ScriptApi