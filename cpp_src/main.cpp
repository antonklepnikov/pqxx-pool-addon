#include <napi.h>
#include "connection_pool.h"
#include "basic_transaction.h"

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  ConnectionPool::Init(env, exports);
  return BasicTransaction::Init(env, exports);
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, InitAll)