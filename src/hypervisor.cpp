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
                    InstanceAccessor("capabilities", &Hypervisor::GetCapabilities, nullptr),
                    InstanceAccessor("hostname", &Hypervisor::GetHostname, nullptr),
                    InstanceAccessor("sysInfo", &Hypervisor::GetSysInfo, nullptr),
                    InstanceAccessor("maxVCPUs", &Hypervisor::GetMaxVCPUs, nullptr),
                    InstanceAccessor("info", &Hypervisor::GetInfo, nullptr),
                    /* Instance Methods */
                    InstanceMethod("connect", &Hypervisor::Connect),
                    InstanceMethod("disconnect", &Hypervisor::Disconnect),

                    InstanceMethod("domains", &Hypervisor::ListAllDomains),
                    InstanceMethod("lookupDomainById", &Hypervisor::LookupDomainById),
                    InstanceMethod("lookupDomainByName", &Hypervisor::LookupDomainByName),
                    InstanceMethod("lookupDomainByUUIDString", &Hypervisor::LookupDomainByUUIDString),
                    InstanceMethod("restoreDomain", &Hypervisor::RestoreDomain)

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
    if (this->_handle) {
        deferred.Reject(Napi::String::New(env, "Hypervisor already connected"));
    } else {
        auto worker = new PromiseWorker(deferred, [this](PromiseWorker *worker) {
            this->_handle = virConnectOpen(this->_uri.c_str());
            if (!this->_handle) {
                worker->Error(std::string(virGetLastError()->message));
            }
        });
        worker->Queue();
    }
    return deferred.Promise();
}

Napi::Value Hypervisor::Disconnect(const Napi::CallbackInfo &info) {
    assert(this->_handle, "Hypervisor not connected");

    auto env = info.Env();

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

Napi::Value Hypervisor::GetMaxVCPUs(const Napi::CallbackInfo &info) {
    assert(this->_handle, "Hypervisor not connected");

    if (info.Length() <= 0 || !info[0].IsString()) {
        Napi::TypeError::New(info.Env(), "Invalid argument").ThrowAsJavaScriptException();
        return info.Env().Undefined();
    }

    auto env = info.Env();
    auto result = virConnectGetMaxVcpus(this->_handle, info[0].ToString().Utf8Value().c_str());
    if (result < 0) {
        Napi::Error::New(env, virSaveLastError()->message).ThrowAsJavaScriptException();
        return env.Undefined();
    }
    return Napi::Number::New(env, result);
}

Napi::Value Hypervisor::GetInfo(const Napi::CallbackInfo &info) {
    assert(this->_handle, "Hypervisor not connected");

    auto env = info.Env();
    virNodeInfo nodeInfo;
    auto result = virNodeGetInfo(this->_handle, &nodeInfo);
    if (result < 0) {
        Napi::Error::New(env, virSaveLastError()->message).ThrowAsJavaScriptException();
        return env.Undefined();
    }

    Napi::Object infoObj = Napi::Object::New(env);
    infoObj.Set("model", Napi::String::New(env, nodeInfo.model));
    infoObj.Set("memory", Napi::Number::New(env, nodeInfo.memory));
    infoObj.Set("cpus", Napi::Number::New(env, nodeInfo.cpus));
    infoObj.Set("mhz", Napi::Number::New(env, nodeInfo.mhz));
    infoObj.Set("nodes", Napi::Number::New(env, nodeInfo.nodes));
    infoObj.Set("sockets", Napi::Number::New(env, nodeInfo.sockets));
    infoObj.Set("cores", Napi::Number::New(env, nodeInfo.cores));
    infoObj.Set("threads", Napi::Number::New(env, nodeInfo.threads));
    return infoObj;
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
        Napi::Object domain = Domain::New(env, {Napi::External<virDomain>::New(env, pVirDomains[i])});
        domains.Set(i, domain);
    }
    free(pVirDomains);
//endregion
    return domains;

}

Napi::Value Hypervisor::LookupDomainById(const Napi::CallbackInfo &info) {
    assert(this->_handle, "Hypervisor not connected");

    auto env = info.Env();

    if (info.Length() <= 0 || !info[0].IsNumber()) {
        Napi::Error::New(env, "Invalid arguments").ThrowAsJavaScriptException();
        return env.Undefined();
    }

    auto id = info[0].ToNumber().Int32Value();

    auto deferred = Napi::Promise::Deferred::New(env);
    auto worker = new PromiseWorker(deferred, [this, id](PromiseWorker *worker) {
        auto domainPtr = virDomainLookupByID(this->_handle, id);
        if (!domainPtr) {
            worker->Error(virSaveLastError()->message);
        }
        worker->Result(Domain::New(worker->env(), {Napi::External<virDomain>::New(worker->env(), domainPtr)}));
    });
    worker->Queue();
    return deferred.Promise();
}

Napi::Value Hypervisor::LookupDomainByName(const Napi::CallbackInfo &info) {
    assert(this->_handle, "Hypervisor not connected");

    auto env = info.Env();

    if (info.Length() <= 0 || !info[0].IsString()) {
        Napi::Error::New(env, "Invalid arguments").ThrowAsJavaScriptException();
        return env.Undefined();
    }

    auto name = info[0].ToString().Utf8Value();

    auto deferred = Napi::Promise::Deferred::New(env);
    auto worker = new PromiseWorker(deferred, [this, name](PromiseWorker *worker) {
        auto domainPtr = virDomainLookupByName(this->_handle, name.c_str());
        if (!domainPtr) {
            worker->Error(virSaveLastError()->message);
        }
        worker->Result(Domain::New(worker->env(), {Napi::External<virDomain>::New(worker->env(), domainPtr)}));
    });
    worker->Queue();
    return deferred.Promise();
}

Napi::Value Hypervisor::LookupDomainByUUIDString(const Napi::CallbackInfo &info) {
    assert(this->_handle, "Hypervisor not connected");

    auto env = info.Env();

    if (info.Length() <= 0 || !info[0].IsString()) {
        Napi::Error::New(env, "Invalid arguments").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    auto uuid = info[0].ToString().Utf8Value();


    auto deferred = Napi::Promise::Deferred::New(env);
    auto worker = new PromiseWorker(deferred, [this, uuid](PromiseWorker *worker) {
        auto domainPtr = virDomainLookupByName(this->_handle, uuid.c_str());
        if (!domainPtr) {
            worker->Error(virSaveLastError()->message);
        }
        worker->Result(Domain::New(worker->env(), {Napi::External<virDomain>::New(worker->env(), domainPtr)}));
    });
    worker->Queue();
    return deferred.Promise();
}

Napi::Value Hypervisor::RestoreDomain(const Napi::CallbackInfo &info) {
    assert(this->_handle, "Hypervisor not connected");

    auto env = info.Env();

    if (info.Length() <= 0 || !info[0].IsString()) {
        Napi::Error::New(env, "Invalid arguments").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    auto xml = info[0].ToString().Utf8Value();
    auto dxml = info[1].IsString() ? info[1].ToString().Utf8Value() : "";
    auto flags = info[2].IsString() ? info[2].ToNumber().Uint32Value() : 0;


    auto deferred = Napi::Promise::Deferred::New(env);
    auto worker = new PromiseWorker(deferred, [this, xml, dxml, flags](PromiseWorker *worker) {
        int result = virDomainRestoreFlags(this->_handle, xml.c_str(), dxml.empty() ? dxml.c_str() : nullptr, flags);
        if (result < 0) {
            worker->Error(virSaveLastError()->message);
        }
    });
    worker->Queue();
    return deferred.Promise();
}
