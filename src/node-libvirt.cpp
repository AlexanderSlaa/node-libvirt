//
// Created by root on 1/12/24.
//

#include <napi.h>

#include "hypervisor.h"
#include "helper/promise_worker.h"


Napi::Number GetVersion(const Napi::CallbackInfo &info) {
    return Napi::Number::New(info.Env(), LIBVIR_VERSION_NUMBER);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    virInitialize();

    Hypervisor::Init(env, exports);
    exports.Set("GetVersion", Napi::Function::New(env,GetVersion));
//    exports.Set("GetGlobalConfigItem", Napi::Function::New(env, GetGlobalConfigItem));
//    exports.Set("ListAllContainers", Napi::Function::New(env, ListAllContainer));
//    exports.Set("ListAllDefinedContainers", Napi::Function::New(env, ListAllDefinedContainer));
//    exports.Set("ListAllActiveContainers", Napi::Function::New(env, ListAllActiveContainers));
    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)