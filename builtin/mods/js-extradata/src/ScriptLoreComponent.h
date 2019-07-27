#pragma once
#include "common.h"

#include <minecraft/script/ScriptTemplateFactory.h>
#include <mods-quickjs/advcomponent.hpp>

class ScriptLoreComponent : public ScriptTemplateFactory<ScriptServerContext>::Component, public AdvanceComponent {
public:
  using ScriptTemplateFactory<ScriptServerContext>::Component::applyComponentTo;
  using AdvanceComponent::applyComponentTo;

  using ScriptTemplateFactory<ScriptServerContext>::Component::retrieveComponentFrom;
  using AdvanceComponent::retrieveComponentFrom;

  using ScriptTemplateFactory<ScriptServerContext>::Component::hasComponent;
  using AdvanceComponent::hasComponent;

  static std::string getName();

  ScriptLoreComponent();
  virtual ~ScriptLoreComponent() override;

  // * Actor * //
  virtual bool applyComponentTo(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &, Actor &, ScriptApi::ScriptObjectHandle const &) const override;
  virtual bool retrieveComponentFrom(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &, Actor &, ScriptApi::ScriptObjectHandle &) const override;
  virtual bool hasComponent(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &, Actor &, bool &) const override;

  // * ItemStack * //
  virtual bool applyComponentTo(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &, ItemInstance &, ScriptApi::ScriptObjectHandle const &) const override;
  virtual bool retrieveComponentFrom(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &, ItemInstance &, ScriptApi::ScriptObjectHandle &) const override;
  virtual bool hasComponent(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &, ItemInstance &, bool &) const override;
};