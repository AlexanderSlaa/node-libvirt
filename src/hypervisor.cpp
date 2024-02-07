//
// Created by root on 1/12/24.
//

#include "hypervisor.h"
#include "helper/promise_worker.h"

#include <libvirt/libvirt.h>
#include <libvirt/virterror.h>


Napi::Object Hypervisor::Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func =
            DefineClass(env, "Hypervisor", {
                    /* Instance accessors */
//                    InstanceAccessor("name", &Hypervisor::GetCapabilities, nullptr),
//                    InstanceAccessor("hostname", &Hypervisor::GetHostname, nullptr),
//                    InstanceAccessor("defined", &Hypervisor::GetSysInfo, nullptr),
                    /* Instance Methods */
                    InstanceMethod("connect", &Hypervisor::connect),
//                    InstanceMethod("disconnect", &Hypervisor::disconnect)
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
        return;
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

Napi::Value Hypervisor::connect(const Napi::CallbackInfo &info) {
    auto env = info.Env();
    auto deferred = Napi::Promise::Deferred::New(env);

    auto worker = new PromiseWorker(deferred, [this](PromiseWorker *worker) {
        static int supported_cred_types[] = {
                VIR_CRED_AUTHNAME,
                VIR_CRED_PASSPHRASE,
        };

        virConnectAuth auth;
        auth.credtype = supported_cred_types;
        auth.ncredtype = sizeof(supported_cred_types) / sizeof(int);
        auth.cb = [](virConnectCredentialPtr cred, unsigned int ncred, void *data) {
            auto *hypervisor = static_cast<Hypervisor *>(data);
            for (unsigned int i = 0; i < ncred; ++i) {
                switch (cred[i].type) {
                    case VIR_CRED_AUTHNAME:
                        cred[i].result = strdup(hypervisor->_username.c_str());
                        if (cred[i].result == nullptr)
                            return -1;
                        cred[i].resultlen = strlen(cred[i].result);
                        break;

                    case VIR_CRED_PASSPHRASE:
                        cred[i].result = strdup(hypervisor->_password.c_str());
                        if (cred[i].result == nullptr)
                            return -1;
                        cred[i].resultlen = strlen(cred[i].result);
                        break;
                }
            }
            return 0;
        };
        auth.cbdata = this;
        this->_handle = virConnectOpenAuth(_uri.c_str(), &auth, this->_readonly ? VIR_CONNECT_RO : 0);
        if (!this->_handle) {
            throw std::runtime_error(virSaveLastError()->message);
        }
    });
    worker->Queue();
    return deferred.Promise();
}

Napi::Value Hypervisor::disconnect(const Napi::CallbackInfo &info) {
    auto env = info.Env();
    auto deferred = Napi::Promise::Deferred::New(env);

    auto worker = new PromiseWorker(deferred, [this](PromiseWorker *worker) {
        if (!this->_handle) {
            throw std::runtime_error("Hypervisor not connected");
        }
        int result = virConnectClose(this->_handle);
        if (result == -1) {
            throw std::runtime_error(virSaveLastError()->message);
        }
    });
    worker->Queue();
    return deferred.Promise();
}

Napi::Value Hypervisor::GetCapabilities(const Napi::CallbackInfo &info) {
    if (!this->_handle) {
        Napi::TypeError::New(info.Env(), "Hypervisor not connected").ThrowAsJavaScriptException();
    }
    return Napi::String::New(info.Env(), virConnectGetCapabilities(this->_handle));
}

Napi::Value Hypervisor::GetHostname(const Napi::CallbackInfo &info) {
    if (!this->_handle) {
        Napi::TypeError::New(info.Env(), "Hypervisor not connected").ThrowAsJavaScriptException();
    }
    return Napi::String::New(info.Env(), virConnectGetHostname(this->_handle));

}

Napi::Value Hypervisor::GetSysInfo(const Napi::CallbackInfo &info) {
    auto env = info.Env();
   if (!this->_handle) {
       Napi::TypeError::New(info.Env(), "Hypervisor not connected").ThrowAsJavaScriptException();
   }
    char *result = virConnectGetSysinfo(this->_handle, 0);
    if (result == nullptr) {
        Napi::Error::New(env, virSaveLastError()->message).ThrowAsJavaScriptException();
        return env.Undefined();
    }
    return Napi::String::New(env, result);
}
