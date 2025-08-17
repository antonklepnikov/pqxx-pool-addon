#ifndef BASIC_TRANSACTION_H_AK
#define BASIC_TRANSACTION_H_AK


#include <napi.h>
#include "pqxx_pool_int/pqxx_pool_int.hpp"


class BasicTransaction : public Napi::ObjectWrap<BasicTransaction> {
  private:
    static Napi::FunctionReference constructor;
    void Commit(const Napi::CallbackInfo& info);
    Napi::Value Query(const Napi::CallbackInfo& info);
    pqxxplint::BasicTransaction *basicTransaction_;
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    BasicTransaction(const Napi::CallbackInfo& info);
};


/* query, commit */

#endif