#include <mods-quickjs/quickjs.hpp>

#include <minecraft/actor/Actor.h>
#include <minecraft/block/BlockSource.h>
#include <minecraft/level/Level.h>
#include <minecraft/level/TickingArea.h>
#include <minecraft/script/EventInfo.h>
#include <minecraft/script/ScriptBinderEventDataTemplate.h>
#include <minecraft/script/ScriptEventCoordinator.h>
#include <minecraft/script/ScriptEventData.h>
#include <minecraft/script/ScriptQueryBinderComponent.h>
#include <minecraft/script/ScriptVersionInfo.h>
#include <minecraft/script/WORKAROUNDS.h>
#include <minecraft/script/details/BinderTemplates.h>

bool ScriptEngineWithContext<ScriptServerContext>::fireEvent(const ScriptEventData &data) {
  EventInfo info;
  if (!getEventTrackings().contains(data.getName()))
    return true;
  autohandle handle;
  if (!data.serialize(*scriptengine, info, handle)) {
    getScriptReportQueue().addError("failed to serialize event: " + data.getName());
    return false;
  }
  fireEventToScript(info, std::move(handle));
  return true;
}

bool ScriptEngineWithContext<ScriptServerContext>::onEventReceivedFromScriptEngine(ScriptApi::ScriptVersionInfo const &version, std::string const &name, ScriptApi::ScriptObjectHandle const &handle) {
  auto &event = getScriptEventCoordinator();
  ScriptApi::ScriptObjectHandle data;
  QCHECK(scriptengine->getMember(handle, "data", data));
  auto builder = getScriptTemplateFactory().events.get(name).lock();
  bool success = false;
  if (builder) {
    success = builder->receivedEvent(version, *scriptengine, getScriptServerContext(), name, data);
    event.sendScriptBroadcastEvent(name, RegistrationType::SYSTEM, success);
  } else {
    builder = getScriptTemplateFactory().custtom_event;
    if (builder) {
      success = builder->receivedEvent(version, *scriptengine, getScriptServerContext(), name, data);
      event.sendScriptBroadcastEvent(name, RegistrationType::CUSTOM, success);
    } else {
      event.sendScriptBroadcastEvent(name, RegistrationType::UNKNOWN, false);
    }
  }
  return success;
}

bool ScriptEngineWithContext<ScriptServerContext>::createEventData(ScriptApi::ScriptVersionInfo const &version, std::string const &name, ScriptApi::ScriptObjectHandle &handle) {
  bool success = false;
  if (_validateObjectIdentifier(false, name)) {
    ScriptApi::ScriptObjectHandle temp;
    if (getScriptOnlyEventsData().isDefined(name)) {
      success = getScriptOnlyEventsData().createEventData(version, *scriptengine, getScriptServerContext(), name, temp);
    } else {
      if (auto ptr = getScriptTemplateFactory().events.get(name).lock(); ptr) {
        createObject(temp);
        success = ptr->getEventData(version, *scriptengine, getScriptServerContext(), name, temp);
        success = handle_exception(js_context, "getEventData");
      }
    }
    if (success) {
      handle = JS_NewObject(js_context);
      ScriptBinderEventDataTemplate::build(name, std::move(temp))->serialize(*scriptengine, handle);
      success = handle_exception(js_context, "createEventData");
    }
  } else {
    getScriptReportQueue().addWarning("Invalid event identifier '" + name + "'");
    return false;
  }
  return success;
}

bool ScriptEngineWithContext<ScriptServerContext>::createComponent(ScriptApi::ScriptVersionInfo const &version, ScriptApi::ScriptObjectHandle const &source, std::string const &name,
                                                                   ScriptApi::ScriptObjectHandle &handle) {
  ScriptApi::ScriptObjectHandle temp;
  bool success = false;
  if (getScriptOnlyComponents().isDefined(name)) {
    success = getScriptOnlyComponents().createComponent(version, *scriptengine, getScriptServerContext(), source, name, temp);
  } else {
    if (auto ptr = getScriptTemplateFactory().components.get(name).lock(); ptr) {
      success = ptr->retrieveComponentFrom(version, *scriptengine, getScriptServerContext(), source, temp);
    }
  }
  if (success) {
    handle = JS_NewObject(js_context);
    ScriptBinderComponentTemplate::build(name, std::move(temp))->serialize(*scriptengine, handle);
    success = handle_exception(js_context, "createComponent");
  }
  if (!success) {
    getScriptReportQueue().addError("createComponent");
  }
  return success;
}

bool ScriptEngineWithContext<ScriptServerContext>::applyComponentChanges(ScriptApi::ScriptVersionInfo const &version, ScriptApi::ScriptObjectHandle const &source,
                                                                         ScriptApi::ScriptObjectHandle const &component) {
  bool success = false;
  if (auto binder = getScriptBinderTemplateController()->deserialize(*this, component); binder) {
    auto identifier_component = binder->getComponent<ScriptIdentifierBinderComponent>();
    auto component_binder = binder->getComponent<ScriptComponentBinderComponent>();
    if (identifier_component && component_binder) {
      auto identifier = identifier_component->getIdentifier();
      auto data = component_binder->getData();
      if (!getScriptOnlyComponents().isDefined(identifier)) {
        if (auto factory = getScriptTemplateFactory().components.get(identifier).lock(); factory) {
          success = factory->applyComponentTo(version, *this, getScriptServerContext(), source, data);
        }
      } else {
        success = getScriptOnlyComponents().applyComponentTo(version, *this, getScriptServerContext(), source, identifier, data);
      }
    }
  }
  if (!success) {
    getScriptReportQueue().addError("applyComponentChanges");
  }
  return success;
}

bool ScriptEngineWithContext<ScriptServerContext>::getComponent(ScriptApi::ScriptVersionInfo const &version, ScriptApi::ScriptObjectHandle const &source, std::string const &name,
                                                                ScriptApi::ScriptObjectHandle &target) {
  ScriptApi::ScriptObjectHandle handle;
  if (getScriptOnlyComponents().isDefined(name)) {
    if (!getScriptOnlyComponents().retrieveComponentFrom(version, *this, getScriptServerContext(), source, name, handle)) {
      getScriptReportQueue().addWarning("Failed to get script only component " + name);
      return false;
    }
  } else {
    if (auto factory = getScriptTemplateFactory().components.get(name).lock(); factory) {
      if (!factory->retrieveComponentFrom(version, *this, getScriptServerContext(), source, handle)) {
        getScriptReportQueue().addWarning("Failed to get pre-defined component " + name);
        return false;
      }
    } else {
      getScriptReportQueue().addWarning("Failed to get template factory for " + name);
      return false;
    }
  }
  if (auto binder = ScriptBinderComponentTemplate::build(name, std::move(handle)); binder) {
    if (!binder->serialize(*this, target)) {
      getScriptReportQueue().addWarning("Failed to serialize component " + name);
      return false;
    }
  } else {
    getScriptReportQueue().addWarning("Failed to get component binder");
    return false;
  }
  return true;
}

bool ScriptEngineWithContext<ScriptServerContext>::getBlock(ScriptApi::ScriptVersionInfo const &version, ScriptApi::ScriptObjectHandle &target, BlockPos const &pos,
                                                            ScriptApi::ScriptObjectHandle const &ticking_area) {
  if (auto binder = getScriptBinderTemplateController()->deserialize(*this, ticking_area); binder) {
    auto level_ticking = binder->getComponent<ScriptLevelAreaBinderComponent>();
    auto actor_ticking = binder->getComponent<ScriptActorAreaBinderComponent>();
    if (auto source = _helpGetBlockSourceFromBinder(*getScriptServerContext().level, level_ticking, actor_ticking); source) {
      if (source->hasBlock(pos)) {
        auto &block = source->getBlock(pos);
        autohandle temp = ticking_area;
        if (auto obj_template = ScriptBinderBlockTemplate::build(block, pos, temp.transfer()); obj_template) {
          if (!obj_template->serialize(*this, target)) {
            getScriptReportQueue().addWarning("Failed to serialize block");
            return false;
          }
          return true;
        } else {
          getScriptReportQueue().addWarning("Failed to create block template binder");
          return false;
        }
      } else {
        getScriptReportQueue().addWarning("Failed to get block: no block");
        return false;
      }
    } else {
      getScriptReportQueue().addWarning("Failed to get block source");
      return false;
    }
  } else {
    getScriptReportQueue().addWarning("Failed to get template binder for ticking area");
    return false;
  }
}