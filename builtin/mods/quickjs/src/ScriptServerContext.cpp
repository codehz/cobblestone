#include <mods-quickjs/quickjs.hpp>

MakeStaticReference(ScriptServerContext);

THook(void, _ZN19ScriptServerContext5resetEv) {}
THook(void, _ZN19ScriptServerContextC2Ev, ScriptServerContext *self) {
  refs<ScriptServerContext> = self;
  original(self);
}