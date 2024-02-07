//
// Created by root on 1/12/24.
//
#include <napi.h>

#include "domain.h"
#include "hypervisor.h"



Napi::Number GetVersion(const Napi::CallbackInfo &info) {
    return Napi::Number::New(info.Env(), LIBVIR_VERSION_NUMBER);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    virInitialize();
    Domain::Init(env,exports);
    Hypervisor::Init(env, exports);
    exports.Set("GetVersion", Napi::Function::New(env,GetVersion));
    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)