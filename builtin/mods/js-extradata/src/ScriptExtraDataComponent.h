#pragma once
#include "common.h"

#include <minecraft/script/ScriptTemplateFactory.h>
#include <mods-quickjs/advcomponent.hpp>

class ScriptExtraDataComponent : public ScriptTemplateFactory<ScriptServerContext>::Component, public AdvanceComponent {
public:
  using ScriptTemplateFactory<ScriptServerContext>::Component::applyComponentTo;
  using AdvanceComponent::applyComponentTo;

  using ScriptTemplateFactory<ScriptServerContext>::Component::retrieveComponentFrom;
  using AdvanceComponent::retrieveComponentFrom;

  using ScriptTemplateFactory<ScriptServerContext>::Component::hasComponent;
  using AdvanceComponent::hasComponent;

  static std::string getName();

  ScriptExtraDataComponent();
  virtual ~ScriptExtraDataComponent() override;

  // * Actor * //
  virtual bool applyComponentTo(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &, Actor &, ScriptApi::ScriptObjectHandle const &) const override;
  virtual bool retrieveComponentFrom(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &, Actor &, ScriptApi::ScriptObjectHandle &) const override;
  virtual bool hasComponent(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &, Actor &, bool &) const override;

  // * Block * //
  virtual bool retrieveComponentFrom(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &, Block const &, BlockSource &, BlockPos &,
                                     ScriptApi::ScriptObjectHandle &) const override;
  virtual bool hasComponent(ScriptApi::ScriptVersionInfo const &, ScriptEngine &, ScriptServerContext &, Block const &, BlockSource &, BlockPos &, bool &) const override;
};