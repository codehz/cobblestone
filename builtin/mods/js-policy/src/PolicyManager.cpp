#include <mods-js-policy/policy.h>

void PolicyManager::dispatch(BasePolicy &ptr) {
  auto &name       = ptr.getName();
  auto [low, high] = handlers.equal_range(name);
  if (low != high) {
    autoval data = ptr.build();
    for (auto it = low; it != high; it++) {
      JSValue temp[] = {
        JS_DupValue(js_context, data),
        JS_NewBool(js_context, ptr.result),
      };
      autoval ret = JS_Call(js_context, it->second, JS_UNDEFINED, 2, temp);
      if (handle_exception(js_context, "PolicyManager::dispatch") && JS_IsBool(ret)) scriptengine->getValue(ret, ptr.result);
    }
  }
}

bool PolicyManager::addListener(const std::string &name, JSValue callback) {
  if (!hasPolicy(name)) return false;
  handlers.emplace(name, JS_DupValue(js_context, callback));
  Log::debug("PolicyManager", "%s handler registered", name.data());
  return true;
}

void PolicyManager::registerPolicy(const std::string &name) {
  valid_policy.emplace(name);
  Log::debug("PolicyManager", "%s registered", name.data());
}

bool PolicyManager::hasPolicy(const std::string &name) { return valid_policy.contains(name); }