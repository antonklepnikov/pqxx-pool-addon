#ifndef BASIC_TRANSACTION_H_AK
#define BASIC_TRANSACTION_H_AK


#include <napi.h>
#include <pqxx/pqxx>
#include "pqxx_pool_int/pqxx_pool_int.hpp"
#include "connection_pool.h"


class BasicTransaction : public Napi::ObjectWrap<BasicTransaction> {
  private:
    static Napi::FunctionReference constructor;
    pqxxplint::BasicTransaction *basicTransaction_;
    void Commit(const Napi::CallbackInfo& info);
    Napi::Value Query(const Napi::CallbackInfo& info);
    Napi::Value convertPqxxField(const Napi::Env& env, const pqxx::field& field);
    Napi::Value convertPqxxResult(const Napi::Env& env, const pqxx::result& result);
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    BasicTransaction(const Napi::CallbackInfo& info);
};


/* query, commit */

#endif