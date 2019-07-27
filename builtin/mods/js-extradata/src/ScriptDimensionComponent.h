#pragma once
#include "common.h"

#include <minecraft/script/ScriptTemplateFactory.h>

class ScriptDimensionComponent : public ScriptTemplateFactory<ScriptServerContext>::Component {
public:
  static std::string getName();

  ScriptDimensionComponent();
  virtual ~ScriptDimensionComponent() override;

  virtual bool applyComponentTo(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &, Actor &, ScriptApi::ScriptObjectHandle const &) const override;
  virtual bool retrieveComponentFrom(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &, Actor &, ScriptApi::ScriptObjectHandle &) const override;
  virtual bool hasComponent(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &, Actor &, bool &) const override;
};