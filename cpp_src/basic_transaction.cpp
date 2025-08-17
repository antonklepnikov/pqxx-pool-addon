#include "connection_pool.h"
#include "basic_transaction.h"


#include <iostream>

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
  std::string res{};
  bool queryWithParams = false;
  if (len == 2 && !info[1].IsString()) {
    Napi::TypeError::New(env, "Params must be string").ThrowAsJavaScriptException();
  }
  if (len == 2 && info[1].IsString()) {
    queryWithParams = true;
  }
  try {
    if (!queryWithParams) {
      res = this->basicTransaction_->query(query);
    } else {
      std::string params = info[1].As<Napi::String>().Utf8Value();
      res = this->basicTransaction_->query(query, params);
    }
  } catch (const std::exception& err) {
    Napi::Error::New(env, err.what()).ThrowAsJavaScriptException();
  }
  return Napi::String::New(info.Env(), res);
}
