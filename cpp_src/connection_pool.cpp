#include "connection_pool.h"


Napi::FunctionReference ConnectionPool::constructor;

Napi::Object ConnectionPool::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "ConnectionPool", {});
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("ConnectionPool", func);
  return exports;
}

ConnectionPool::ConnectionPool(const Napi::CallbackInfo& info)
	: Napi::ObjectWrap<ConnectionPool>(info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  if (info.Length() != 1 || !info[0].IsObject()) {
    Napi::TypeError::New(env, "Configuration params object expected").ThrowAsJavaScriptException();
  }
  Napi::Object inputParams = info[0].As<Napi::Object>();
  pqxxplint::ConnectionParams params;
  params.dbName = inputParams.Get("dbName").As<Napi::String>().Utf8Value();
  params.user = inputParams.Get("user").As<Napi::String>().Utf8Value();
  params.password = inputParams.Get("password").As<Napi::String>().Utf8Value();
	if (inputParams.Has("hostAddr")) {
		params.hostAddr = inputParams.Get("hostAddr").As<Napi::String>().Utf8Value();
	}
	if (inputParams.Has("port")) {
		params.port = inputParams.Get("port").As<Napi::String>().Utf8Value();
	}
	if (inputParams.Has("poolSize")) {
		params.poolSize = inputParams.Get("poolSize").As<Napi::Number>().Int32Value();
  }
	try {
		this->connectionPool_ = new pqxxplint::ConnectionPool(params);
	} catch (const std::exception& err) {
		std::string rawMessage = err.what();
		auto pos = rawMessage.find('\n');
		std::string errMessage{};
		if (pos != std::string::npos) {
			errMessage = rawMessage.erase(pos);
		} else {
			errMessage = rawMessage;
		}
		Napi::Error::New(env, errMessage + '\n').ThrowAsJavaScriptException();
	} 
}

pqxxplint::ConnectionPool* ConnectionPool::GetInternalInstance() {
	return this->connectionPool_;
}
