#include <mods-js-policy/policy.h>

ScriptPolicy::ScriptPolicy(std::string const &name, JSValue value, bool def)
    : BasePolicy(def)
    , name(name)
    , value(JS_DupValue(js_context, value)) {}

ScriptPolicy::~ScriptPolicy() { JS_FreeValue(js_context, value); }

std::string const &ScriptPolicy::getName() const { return name; }

JSValue ScriptPolicy::build() { return JS_DupValue(js_context, value); }
