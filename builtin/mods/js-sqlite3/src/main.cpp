#include "sqlite3.h"

#include <mods-quickjs/quickjs.hpp>

#include <filesystem>

namespace fs = std::filesystem;

static JSClassID sqlite3_class;
static JSClassDef sqlite3_class_def = {
  "SQLite3",
  +[](JSRuntime *rt, JSValue val) { sqlite3_close(reinterpret_cast<sqlite3 *>(JS_GetOpaque(val, sqlite3_class))); },
};

LAZY(class_id, JS_NewClassID(&sqlite3_class));

struct PropertiesSettings {
  std::string const &getLevelName() const;
};

static JSValue createSQLite3(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
  static fs::path root = fs::current_path() / "data" / "worlds" / refs<PropertiesSettings>->getLevelName();
  std::string name = ":memory:";
  if (argc >= 1) {
    if (!scriptengine->getValue(argv[0], name)) return JS_ThrowTypeError(ctx, "Require (string)");
    name = root / name;
  }
  Log::debug("SQLite3", "open %s", name.data());
  sqlite3 *db;
  auto res = sqlite3_open(name.data(), &db);
  if (res != SQLITE_OK) return JS_ThrowInternalError(ctx, "Failed to open database: %s", sqlite3_errstr(res));
  auto ret = JS_NewObjectProtoClass(ctx, JS_GetClassProto(ctx, sqlite3_class), sqlite3_class);
  JS_SetOpaque(ret, db);
  return ret;
}

static JSValue processExecute(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
  if (auto db = (sqlite3 *)JS_GetOpaque(this_val, sqlite3_class); db) {
    if (argc == 1) {
      std::string stmt;
      if (!scriptengine->getValue(argv[0], stmt)) return JS_ThrowTypeError(ctx, "Require (string)");
      char *errmsg;
      auto ret = sqlite3_exec(db, stmt.data(), nullptr, nullptr, &errmsg);
      if (ret == SQLITE_OK) return JS_TRUE;
      return JS_ThrowInternalError(js_context, "%s", errmsg);
    } else if (argc == 2) {
      std::string stmt;
      if (!scriptengine->getValue(argv[0], stmt) || !JS_IsFunction(js_context, argv[1])) return JS_ThrowTypeError(ctx, "Require (string, function)");
      auto cbn      = argv[1];
      auto callback = [&](int n, char **values, char **keys) -> bool {
        CLEANUP(QJS_FreeValue) JSValue val = JS_NewObject(js_context);
        for (int i = 0; i < n; i++) { JS_SetPropertyStr(js_context, val, keys[i], JS_NewString(js_context, values[i])); }
        CLEANUP(QJS_FreeValue) auto result = JS_Call(js_context, cbn, JS_UNDEFINED, 1, &val);
        if (!handle_exception(js_context, "SQLite3::exec")) return false;
        if (JS_IsBool(result) && !JS_ToBool(js_context, result)) return false;
        return true;
      };
      char *errmsg;
      auto ret = sqlite3_exec(
          db, stmt.data(),
          +[](void *user, int n, char **values, char **keys) -> int {
            auto cb = *(decltype(callback) *)user;
            return cb(n, values, keys) ? 0 : 1;
          },
          &callback, &errmsg);
      if (ret == SQLITE_OK) return JS_TRUE;
      return JS_ThrowInternalError(js_context, "%s", errmsg);
    }
    return JS_ThrowTypeError(ctx, "Require 1 or 2 arguments");
  }
  return JS_ThrowTypeError(ctx, "Require SQLite3 object");
}

static void fill_stmt(JSContext *ctx, sqlite3 *db, sqlite3_stmt *stmt, JSValue &obj) {
  CLEANUP(QJS_FreeValue) auto global  = JS_GetGlobalObject(ctx);
  CLEANUP(QJS_FreeValue) auto reflect = JS_GetPropertyStr(ctx, global, "Reflect");
  CLEANUP(QJS_FreeValue) auto ownKeys = JS_GetPropertyStr(ctx, reflect, "ownKeys");
  CLEANUP(QJS_FreeValue) auto arr     = JS_Call(ctx, ownKeys, reflect, 1, &obj);
  CLEANUP(QJS_FreeValue) auto propLen = JS_GetPropertyStr(ctx, arr, "length");
  for (int32_t i = 0; i < JS_VALUE_GET_INT(propLen); i++) {
    CLEANUP(QJS_FreeValue) auto key      = JS_GetPropertyUint32(ctx, arr, i);
    CLEANUP(QJS_FreeCString) auto keyStr = JS_ToCString(ctx, key);
    CLEANUP(QJS_FreeValue) auto value    = JS_GetPropertyStr(ctx, obj, keyStr);
    auto idx                             = sqlite3_bind_parameter_index(stmt, keyStr);
    if (idx != 0) {
      if (JS_IsString(value)) {
        auto vStr = JS_ToCString(ctx, value);
        sqlite3_bind_text(stmt, idx, vStr, -1, (void (*)(void *))QJS_FreeCString);
      } else if (JS_IsBool(value)) {
        sqlite3_bind_int(stmt, idx, JS_ToBool(ctx, value));
      } else if (JS_IsNull(value) || JS_IsUndefined(value)) {
        sqlite3_bind_null(stmt, idx);
      } else if (JS_IsInteger(value)) {
        int64_t vInt;
        JS_ToInt64(ctx, &vInt, value);
        sqlite3_bind_int64(stmt, idx, vInt);
      } else if (JS_IsNumber(value)) {
        double vFloat;
        JS_ToFloat64(ctx, &vFloat, value);
        sqlite3_bind_double(stmt, idx, vFloat);
      } else {
        throw JS_ThrowTypeError(ctx, "Failed to process stmt params");
      }
    }
  }
}

static void free_stmt(sqlite3_stmt **pstmt) {
  if (pstmt && *pstmt) sqlite3_finalize(*pstmt);
}

static JSValue processQuery(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
  if (auto db = (sqlite3 *)JS_GetOpaque(this_val, sqlite3_class); db) {
    if (argc < 1) return JS_ThrowTypeError(ctx, "Require 1 or 2 arguments");
    std::string sql;
    if (!scriptengine->getValue(argv[0], sql)) return JS_ThrowTypeError(ctx, "Require (string, [object])");
    CLEANUP(free_stmt) sqlite3_stmt *stmt = nullptr;
    if (auto res = sqlite3_prepare_v2(db, sql.data(), sql.size(), &stmt, nullptr); res != SQLITE_OK)
      return JS_ThrowInternalError(ctx, "Cannot create stmt: %s", sqlite3_errstr(res));
    if (argc > 1) {
      if (!JS_IsObject(argv[1])) return JS_ThrowTypeError(ctx, "Require (string, object)");
      try {
        fill_stmt(ctx, db, stmt, argv[1]);
      } catch (JSValue const &v) { return v; }
    }
    int ret                                                   = 0;
    int idx                                                   = 0;
    CLEANUP(QJS_FreeHandle) ScriptApi::ScriptObjectHandle arr = JS_NewArray(ctx);
    while ((ret = sqlite3_step(stmt)) == SQLITE_ROW) {
      CLEANUP(QJS_FreeHandle) ScriptApi::ScriptObjectHandle obj = JS_NewObject(ctx);
      int count                                                 = sqlite3_column_count(stmt);
      for (int i = 0; i < count; i++) {
        auto key   = (char const *)sqlite3_column_name(stmt, i);
        auto vtype = sqlite3_column_type(stmt, i);
        CLEANUP(QJS_FreeHandle) ScriptApi::ScriptObjectHandle value;
        switch (vtype) {
        case SQLITE_INTEGER: value = JS_NewInt64(ctx, sqlite3_column_int64(stmt, i)); break;
        case SQLITE_FLOAT: value = JS_NewFloat64(ctx, sqlite3_column_double(stmt, i)); break;
        case SQLITE_TEXT: value = JS_NewString(ctx, (char const *)sqlite3_column_text(stmt, i)); break;
        case SQLITE_BLOB: value = JS_NewArrayBufferCopy(ctx, (uint8_t *)sqlite3_column_blob(stmt, i), sqlite3_column_bytes(stmt, i)); break;
        case SQLITE_NULL: value = JS_NULL;
        default: value = JS_UNDEFINED;
        }
        JS_SetPropertyStr(ctx, obj, key, value.transfer());
      }
      JS_SetPropertyInt64(ctx, arr, idx++, obj.transfer());
    }
    if (ret == SQLITE_DONE) {
      return arr.transfer();
    } else {
      return JS_ThrowInternalError(ctx, "Failed to perform query: %s", sqlite3_errmsg(db));
    }
  }
  return JS_ThrowTypeError(ctx, "Require SQLite3 object");
}

static JSValue processUpdate(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
  if (auto db = (sqlite3 *)JS_GetOpaque(this_val, sqlite3_class); db) {
    if (argc < 1) return JS_ThrowTypeError(ctx, "Require 1 or 2 arguments");
    std::string sql;
    if (!scriptengine->getValue(argv[0], sql)) return JS_ThrowTypeError(ctx, "Require (string, [object])");
    CLEANUP(free_stmt) sqlite3_stmt *stmt = nullptr;
    if (auto res = sqlite3_prepare_v2(db, sql.data(), sql.size(), &stmt, nullptr); res != SQLITE_OK)
      return JS_ThrowInternalError(ctx, "Cannot create stmt: %s", sqlite3_errstr(res));
    if (argc > 1) {
      if (!JS_IsObject(argv[1])) return JS_ThrowTypeError(ctx, "Require (string, object)");
      try {
        fill_stmt(ctx, db, stmt, argv[1]);
      } catch (JSValue const &v) { return v; }
    }
    int ret = sqlite3_step(stmt);
    switch (ret) {
    case SQLITE_DONE: return JS_NewInt32(ctx, sqlite3_changes(db));
    case SQLITE_ROW: return JS_ThrowSyntaxError(ctx, "Cannot perform query in this context");
    default: return JS_ThrowInternalError(ctx, "Failed to serialize sqlite result table: %s", sqlite3_errmsg(db));
    }
  }
  return JS_ThrowTypeError(ctx, "Require SQLite3 object");
}

static JSCFunctionListEntry methods[] = {
  JS_CFUNC_DEF("exec", 1, processExecute),
  JS_CFUNC_DEF("query", 1, processQuery),
  JS_CFUNC_DEF("update", 1, processUpdate),
};

static void preload(JSValue const &global) {
  JS_NewClass(js_runtime, sqlite3_class, &sqlite3_class_def);
  auto proto = JS_NewObject(js_context);
  JS_SetPropertyFunctionList(js_context, proto, methods, countof(methods));
  JS_SetClassProto(js_context, sqlite3_class, proto);

  JS_SetPropertyStr(js_context, global, "SQLite3", JS_NewCFunction2(js_context, createSQLite3, "SQLite3", 0, JS_CFUNC_constructor, 0));
}

LAZY(register, quickjs_preloads.emplace_back(preload));