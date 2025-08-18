#ifndef PQXX_POOL_H_AK
#define PQXX_POOL_H_AK


#include "pqxx_pool_int/pqxx_pool_int.hpp"
#include <napi.h>


class ConnectionPool : public Napi::ObjectWrap<ConnectionPool> {
  private:
    static Napi::FunctionReference constructor;
    pqxxplint::ConnectionPool *connectionPool_;
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    ConnectionPool(const Napi::CallbackInfo& info);
    Napi::Value GetPoolStatus(const Napi::CallbackInfo& info);
    pqxxplint::ConnectionPool* GetInternalInstance();
};

/* 
ConnectionPool
BasicTransaction / query, commit, abort
tx
*/

#endif