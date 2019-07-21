#pragma once

#include "../ScriptBinderTemplate.h"
#include "../../core/types.h"

#include <memory>

namespace ScriptApi {
class ScriptObjectHandle;
}

class ScriptBinderComponentTemplate : public ScriptBinderTemplate {
public:
  virtual ~ScriptBinderComponentTemplate() override;
  virtual std::string const &getTemplateIdentifier() const override;
  virtual void applyTemplate(ScriptObjectBinder &) const override;

  static std::unique_ptr<ScriptObjectBinder> build(std::string const &, ScriptApi::ScriptObjectHandle &&);
};

class Block;

class ScriptBinderBlockTemplate : public ScriptBinderTemplate {
public:
  virtual ~ScriptBinderBlockTemplate() override;
  virtual std::string const &getTemplateIdentifier() const override;
  virtual void applyTemplate(ScriptObjectBinder &) const override;

  static std::unique_ptr<ScriptObjectBinder> build(Block const&, BlockPos const&, ScriptApi::ScriptObjectHandle &&);
};