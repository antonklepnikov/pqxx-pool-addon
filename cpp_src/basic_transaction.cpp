#include "basic_transaction.h"

Napi::FunctionReference BasicTransaction::constructor;

Napi::Object BasicTransaction::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "BasicTransaction", {
    InstanceMethod("commit", &BasicTransaction::Commit),
    InstanceMethod("query", &BasicTransaction::Query)
  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("BasicTransaction", func);
  return exports;
}

BasicTransaction::BasicTransaction(const Napi::CallbackInfo& info)
  : Napi::ObjectWrap<BasicTransaction>(info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  if (info.Length() < 1 || !info[0].IsObject()) {
      Napi::TypeError::New(env, "Pool object expected").ThrowAsJavaScriptException();
  }
  Napi::Object poolNapiObj = info[0].As<Napi::Object>();
  ConnectionPool* poolWrapped = Napi::ObjectWrap<ConnectionPool>::Unwrap(poolNapiObj);
  pqxxplint::ConnectionPool* pool = poolWrapped->GetInternalInstance();
  if (pool != NULL ) {
    this->basicTransaction_ = new pqxxplint::BasicTransaction(*pool);
  } else {
    Napi::Error::New(env, "Pool object is null-pointer").ThrowAsJavaScriptException();
  }
}

void BasicTransaction::Commit(const Napi::CallbackInfo& info) {
  this->basicTransaction_->commit();
}

Napi::Value BasicTransaction::Query(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  int len = info.Length();
  if (len < 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "Query string expected").ThrowAsJavaScriptException();
  }
  std::string query = info[0].As<Napi::String>().Utf8Value();   
  bool queryWithParams = false;
  if (len == 2) {
    if (!info[1].IsArray()) {
      Napi::TypeError::New(env, "Params must be an array").ThrowAsJavaScriptException();
    } else {
      queryWithParams = true;
    }
  }
  try {
    std::vector<std::string> params{};
    if (queryWithParams) {
      Napi::Array arr = info[1].As<Napi::Array>();
      std::size_t len = arr.Length();
      for (std::size_t i = 0; i < len; ++i) {
        params.emplace_back(static_cast<Napi::Value>(arr[i]).ToString().Utf8Value());
      }
    }
    const pqxx::result res = this->basicTransaction_->query(query, params);
    return convertPqxxResult(env, res);
  } catch (const std::exception& err) {
    Napi::Error::New(env, err.what()).ThrowAsJavaScriptException();
  }
  return env.Null();
}

Napi::Value BasicTransaction::convertPqxxField(const Napi::Env& env, const pqxx::field& field) {
  const pqxx::oid oid = field.type();
  try {
    // Compatibility is guaranteed only with Postgresql 16! 
    switch (oid) {
      case 16: // bool
        return Napi::Boolean::New(env, field.as<bool>());
      case 17: // bytea
        return Napi::Buffer<char>::Copy(env, field.c_str(), sizeof(field.c_str()));
      case 21: // int2
        return Napi::Number::New(env, field.as<short>());
      case 23: // int4
        return Napi::Number::New(env, field.as<int>());
      case 20: // int8
        return Napi::Number::New(env, field.as<long>());
      case 700: // float4
        return Napi::Number::New(env, field.as<float>());
      case 701: // int4
        return Napi::Number::New(env, field.as<double>());
      default:
        return Napi::String::New(env, field.c_str());
    }
  } catch (const std::exception& e) {
      // If conversion filed, return string
      return Napi::String::New(env, field.c_str());
  }
}

Napi::Value BasicTransaction::convertPqxxResult(
  const Napi::Env& env,
  const pqxx::result& result
) {
  try {
    const pqxx::result::size_type resSize = result.size();
    Napi::Array jsRes = Napi::Array::New(env, resSize);
    for (pqxx::result::size_type i = 0; i < resSize; ++i) {
      const pqxx::row& row = result[i];
      const pqxx::row::size_type rowSize = row.size();
      Napi::Object jsRow = Napi::Object::New(env);
      for (pqxx::row::size_type j = 0; j < rowSize; ++j) {
        const pqxx::field& field = row[j];
        std::string columnName = result.column_name(j);
        if (field.is_null()) {
          jsRow.Set(columnName, env.Null());
        } else {
          Napi::Value jsValue = convertPqxxField(env, field);
          jsRow.Set(columnName, jsValue);
        }
      }
      jsRes.Set(i, jsRow);
    }
    return jsRes;
  } catch (std::exception& err) {
    Napi::Error::New(env, err.what()).ThrowAsJavaScriptException();
  }
  return env.Null();
}
