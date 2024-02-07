//
// Created by root on 1/12/24.
//

#include "hypervisor.h"
#include "domain.h"
#include "helper/promise_worker.h"
#include "helper/assert.h"

#include <libvirt/libvirt.h>
#include <libvirt/virterror.h>


Napi::Object Hypervisor::Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func =
            DefineClass(env, "Hypervisor", {
                    /* Instance accessors */
//                    InstanceAccessor("name", &Hypervisor::GetCapabilities, nullptr),
                    InstanceAccessor("hostname", &Hypervisor::GetHostname, nullptr),
                    InstanceAccessor("sysInfo", &Hypervisor::GetSysInfo, nullptr),
                    /* Instance Methods */
                    InstanceMethod("connect", &Hypervisor::Connect),
                    InstanceMethod("disconnect", &Hypervisor::Disconnect),
                    InstanceMethod("domains", &Hypervisor::ListAllDomains)
            });

    auto constructor = Napi::Persistent(func);
    constructor.SuppressDestruct(); // Prevent the destructor, as it will be handled by N-API.
    exports.Set("Hypervisor", func);
    return exports;
}

Hypervisor::Hypervisor(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Hypervisor>(info) {
    auto env = info.Env();
    if (info.Length() != 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "Invalid constructor call").ThrowAsJavaScriptException();
    }
    auto config = info[0].ToObject();
    if (!config.Has("uri")) {
        Napi::TypeError::New(env, "Config object doesn't have 'uri' property").ThrowAsJavaScriptException();
    }
    this->_uri = config.Get("uri").ToString().Utf8Value();
    this->_username = config.Has("username") ? config.Get("username").ToString().Utf8Value() : "";
    this->_password = config.Has("password") ? config.Get("password").ToString().Utf8Value() : "";
    this->_readonly = config.Has("readOnly") ? config.Get("readOnly").ToBoolean() : false;
}

Hypervisor::~Hypervisor() {
    this->_uri.clear();
    this->_username.clear();
    this->_password.clear();
}

Napi::Value Hypervisor::Connect(const Napi::CallbackInfo &info) {
    auto env = info.Env();
    auto deferred = Napi::Promise::Deferred::New(env);

    auto worker = new PromiseWorker(deferred, [this](PromiseWorker *worker) {
        this->_handle = virConnectOpen(this->_uri.c_str());
        if (!this->_handle) {
            worker->Error(std::string(virGetLastError()->message));
        }
    });
    worker->Queue();
    return deferred.Promise();
}

Napi::Value Hypervisor::Disconnect(const Napi::CallbackInfo &info) {
    auto env = info.Env();

    assert(this->_handle, "Hypervisor not connected");

    auto deferred = Napi::Promise::Deferred::New(env);

    auto worker = new PromiseWorker(deferred, [this](PromiseWorker *worker) {
        int result = virConnectClose(this->_handle);
        if (result == -1) {
            worker->Error(virSaveLastError()->message);
        }
    });
    worker->Queue();
    return deferred.Promise();
}

Napi::Value Hypervisor::GetCapabilities(const Napi::CallbackInfo &info) {
    assert(this->_handle, "Hypervisor not connected");
    return Napi::String::New(info.Env(), virConnectGetCapabilities(this->_handle));
}

Napi::Value Hypervisor::GetHostname(const Napi::CallbackInfo &info) {
    assert(this->_handle, "Hypervisor not connected");
    return Napi::String::New(info.Env(), virConnectGetHostname(this->_handle));
}

Napi::Value Hypervisor::GetSysInfo(const Napi::CallbackInfo &info) {
    assert(this->_handle, "Hypervisor not connected");
    auto env = info.Env();
    char *result = virConnectGetSysinfo(this->_handle, 0);
    if (result == nullptr) {
        Napi::Error::New(env, virSaveLastError()->message).ThrowAsJavaScriptException();
        return env.Undefined();
    }
    return Napi::String::New(env, result);
}

Napi::Value Hypervisor::ListAllDomains(const Napi::CallbackInfo &info) {
    assert(this->_handle, "Hypervisor not connected");
    auto env = info.Env();
//region extract flags
    int flags = 0;
    if (info.Length() > 0 && info[0].IsNumber()) {
        flags = info[0].ToNumber().Int32Value();
    }
//endregion

//region Get domains and create Javascript class object of type Domain
    virDomainPtr *pVirDomains;
    int numDomains = virConnectListAllDomains(this->_handle, &pVirDomains, flags);
    Napi::Array domains = Napi::Array::New(env);
    for (int i = 0; i < numDomains; i++) {
        Napi::Object domain = Domain::New(env,{Napi::External<virDomain>::New(env, pVirDomains[i])});
        domains.Set(i, domain);
    }
    free(pVirDomains);
//endregion
    return domains;

}
