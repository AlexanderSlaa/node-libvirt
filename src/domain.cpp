//
// Created by root on 2/7/24.
//

#include "domain.h"
#include "helper/assert.h"
#include "helper/error.h"
#include "hypervisor.h"
#include "helper/promise_worker.h"

//region STATIC

Napi::Object Domain::Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func =
            DefineClass(env, "Domain", {
                    /* Instance accessors */
                    InstanceAccessor("info", &Domain::Info, nullptr),
                    InstanceAccessor("id", &Domain::Id, nullptr),
                    InstanceAccessor("name", &Domain::Name, nullptr),
                    InstanceAccessor("uuid", &Domain::UUIDString, nullptr),
                    /* Instance Methods */
                    InstanceMethod("shutdown", &Domain::Shutdown),
                    InstanceMethod("create", &Domain::Create),
                    InstanceMethod("save", &Domain::Save),
                    InstanceMethod("toXML", &Domain::ToXML),


                    /* Static Methods */
                    StaticMethod("DefineXML", &Domain::DefineXML),
                    StaticMethod("CreateXML", &Domain::CreateXML)
            });


    auto *constructor = new Napi::FunctionReference();
    *constructor = Napi::Persistent(func);
    env.SetInstanceData(constructor);
    exports.Set("Domain", func);
    return exports;
}

Napi::Object Domain::New(Napi::Env env, const std::initializer_list<napi_value> &args) {
    Napi::EscapableHandleScope scope(env);
    Napi::Object obj = env.GetInstanceData<Napi::FunctionReference>()->New(args);
    return scope.Escape(napi_value(obj)).ToObject();
}


Napi::Value Domain::DefineXML(const Napi::CallbackInfo &info) {
    auto env = info.Env();
    if (info.Length() <= 0 || !info[0].IsString() || !info[1].IsObject()) {
        Napi::Error::New(env, "Invalid arguments").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    auto xml = info[0].ToString().Utf8Value();
    auto pHypervisor = Napi::ObjectWrap<Hypervisor>::Unwrap(info[1].ToObject());
    virDomainPtr domainPtr;
    if (info[2].IsNumber()) {
        domainPtr = virDomainDefineXMLFlags(pHypervisor->Handle(), xml.c_str(), info[2].ToNumber().Uint32Value());
    } else {
        domainPtr = virDomainDefineXML(pHypervisor->Handle(), xml.c_str());
    }
    virt_error_check_last()
    return Domain::New(env, {Napi::External<virDomain>::New(env, domainPtr)});
}

Napi::Value Domain::CreateXML(const Napi::CallbackInfo &info) {
    auto env = info.Env();
    if (info.Length() <= 0 || !info[0].IsString() || !info[1].IsObject()) {
        Napi::Error::New(env, "Invalid arguments").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    auto xml = info[0].ToString().Utf8Value();
    virDomainPtr domainPtr;
    auto pHypervisor = Napi::ObjectWrap<Hypervisor>::Unwrap(info[1].ToObject());
    if (info[2].IsNumber()) {
        domainPtr = virDomainCreateXML(pHypervisor->Handle(), xml.c_str(), info[2].ToNumber().Uint32Value());
    } else {
        domainPtr = virDomainCreateXML(pHypervisor->Handle(), xml.c_str(), 0);
    }
    virt_error_check(domainPtr);
    return Domain::New(env, {Napi::External<virDomain>::New(env, domainPtr)});
}

//endregion

//region INSTANCE


Domain::Domain(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Domain>(info) {
    Napi::Env env = info.Env();
    if (info.Length() <= 0 || !info[0].IsExternal()) {
        Napi::TypeError::New(env, "Expected an external.")
                .ThrowAsJavaScriptException();
        return;
    }
    this->_domain = info[0].As<Napi::External<virDomain>>().Data();
}

Domain::~Domain() {
    if (this->_domain) virDomainFree(this->_domain);
}

//region INSTANCE METHODS


void Domain::Create(const Napi::CallbackInfo &info) {
    assert_void(this->_domain, "Domain is not defined")
    int ret;
    if (info[0] && info[0].IsNumber()) {
        ret = virDomainCreateWithFlags(this->_domain, info[1].ToNumber().Uint32Value());
    } else {
        ret = virDomainCreate(this->_domain);
    }
    virt_error_check_void(ret < 0)
}

void Domain::Save(const Napi::CallbackInfo &info) {
    assert_void(this->_domain, "Domain is not defined")

    auto env = info.Env();
    if (info.Length() <= 0 || !info[0].IsString()) {
        Napi::Error::New(env, "Invalid arguments").ThrowAsJavaScriptException();
        return;
    }
    virt_error_check_void(
            virDomainSaveFlags(
                    this->_domain,
                    info[0].ToString().Utf8Value().c_str(),
                    info[2].IsString() ? info[2].ToString().Utf8Value().c_str() : nullptr,
                    info[1].IsNumber() ? info[1].ToNumber().Int32Value() : 0)
            < 0)
}

void Domain::Shutdown(const Napi::CallbackInfo &info) {
    assert_void(this->_domain, "Domain not defined");
    int ret;
    if (info.Length() > 0 && info[0].IsNumber()) { /* Check if shutdown request is with flags */
        ret = virDomainShutdownFlags(this->_domain, info[0].ToNumber().Uint32Value());
    } else {
        ret = virDomainShutdown(this->_domain);
    }
    virt_error_check_void(ret < 0);
}

Napi::Value Domain::ToXML(const Napi::CallbackInfo &info) {
    assert(this->_domain, "Domain not defined");

    if (info.Length() <= 0 || !info[0].IsNumber()) {
        Napi::Error::New(info.Env(), "Invalid arguments").ThrowAsJavaScriptException();
        return info.Env().Undefined();
    }

    auto flags = info[0].ToNumber().Uint32Value();

    auto xmlDesc = virDomainGetXMLDesc(this->_domain, flags);
    virt_error_check(xmlDesc == nullptr);
    auto ret = Napi::String::New(info.Env(), xmlDesc);
    free(xmlDesc);
    return ret;
}

//endregion

//region ACCESSORS

Napi::Value Domain::Info(const Napi::CallbackInfo &info) {
    assert(this->_domain, "Domain not defined");
    auto env = info.Env();
//region request virDomainInfo
    virDomainInfo domainInfo;
    virt_error_check(virDomainGetInfo(this->_domain, &domainInfo) < 0);
//endregion
//region virDomainInfo to Javascript object
    auto infoObj = Napi::Object::New(env);
    infoObj.Set("state", Napi::Number::New(env, domainInfo.state));
    infoObj.Set("maxMem", Napi::Number::New(env, domainInfo.maxMem));
    infoObj.Set("memory", Napi::Number::New(Env(), domainInfo.memory));
    infoObj.Set("nrVirtCpu", Napi::Number::New(Env(), domainInfo.nrVirtCpu));
    infoObj.Set("cpuTime", Napi::Number::New(Env(), domainInfo.cpuTime));
    return infoObj;
//endregion
}

Napi::Value Domain::Id(const Napi::CallbackInfo &info) {
    assert(this->_domain, "Domain not defined");

    auto env = info.Env();
//region request Domain id
    auto id = virDomainGetID(this->_domain);
    /*
     * Inactive domains don't have an id, so virDomainGetID will return -1,
     * but not set an error. This will cause SetVirtError to throw an
     * "Unknown error". But in this case it might be better to just return
     * null as id.
     */
    if (id == static_cast<unsigned int>(-1)) {
        virt_error_check_last();
    }
//endregion
    return Napi::Number::New(env, id);
}

Napi::Value Domain::Name(const Napi::CallbackInfo &info) {
    assert(this->_domain, "Domain not defined");
    return Napi::String::New(info.Env(), virDomainGetName(this->_domain));
}

Napi::Value Domain::UUIDString(const Napi::CallbackInfo &info) {
    assert(this->_domain, "Domain not defined");
    char uuid[VIR_UUID_STRING_BUFLEN];
    virt_error_check(virDomainGetUUIDString(this->_domain, uuid) < 0);
    return Napi::String::New(Env(), uuid, VIR_UUID_STRING_BUFLEN - 1);
}

//endregion
//endregion



