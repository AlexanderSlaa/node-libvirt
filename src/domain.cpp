//
// Created by root on 2/7/24.
//

#include "domain.h"
#include "helper/assert.h"
#include "hypervisor.h"

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
                    /* Static Methods */
                    StaticMethod("FromXML", &Domain::FromXML)
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

Napi::Value Domain::FromXML(const Napi::CallbackInfo &info) {
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
    auto err = virGetLastError();
    if (err != nullptr) {
        Napi::Error::New(env,err->message).ThrowAsJavaScriptException();
        return env.Undefined();
    }
    return Domain::New(env, {Napi::External<virDomain>::New(env, domainPtr)});
}

//endregion

//region INSTANCE

//region INSTANCE METHODS

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

void Domain::Create(const Napi::CallbackInfo &info) {
    assert_void(this->_domain, "Domain is not defined")
    auto env = info.Env();
    int res;
    if (info[0] && info[0].IsNumber()) {
        res = virDomainCreateWithFlags(this->_domain, info[1].ToNumber().Uint32Value());
    } else {
        res = virDomainCreate(this->_domain);
    }
    if (res < 0) {
        Napi::Error::New(env, std::string(virGetLastError()->message)).ThrowAsJavaScriptException();
    }
}

void Domain::Shutdown(const Napi::CallbackInfo &info) {
    assert_void(this->_domain, "Domain not defined");
    Napi::Env env = info.Env();
    int ret;
    if (info.Length() > 0 && info[0].IsNumber()) { /* Check if shutdown request is with flags */
        ret = virDomainShutdownFlags(this->_domain, info[0].ToNumber().Uint32Value());
    } else {
        ret = virDomainShutdown(this->_domain);
    }
    if (ret < 0) {
        Napi::Error::New(env, std::string(virGetLastError()->message)).ThrowAsJavaScriptException();
    }
}

//endregion

//region ACCESSORS

Napi::Value Domain::Info(const Napi::CallbackInfo &info) {
    assert(this->_domain, "Domain not defined");
    auto env = info.Env();
//region request virDomainInfo
    virDomainInfo domainInfo;
    auto ret = virDomainGetInfo(this->_domain, &domainInfo);
    if (ret < 0) {
        Napi::Error::New(env, std::string(virGetLastError()->message)).ThrowAsJavaScriptException();
        return env.Undefined();
    }
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

Napi::Value Domain::Name(const Napi::CallbackInfo &info) {
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
        virErrorPtr err = virGetLastError();
        if (err != nullptr) {
            Napi::Error::New(env, std::string(err->message)).ThrowAsJavaScriptException();
            return env.Undefined();
        }
    }
//endregion
    return Napi::Number::New(env, id);
}

Napi::Value Domain::Id(const Napi::CallbackInfo &info) {
    assert(this->_domain, "Domain not defined");
    return Napi::String::New(info.Env(), virDomainGetName(this->_domain));
}

Napi::Value Domain::UUIDString(const Napi::CallbackInfo &info) {
    assert(this->_domain, "Domain not defined");
    auto env = info.Env();
    char uuid[VIR_UUID_STRING_BUFLEN];
    int res = virDomainGetUUIDString(this->_domain, uuid);
    if (res < 0) {
        Napi::Error::New(env, std::string(virGetLastError()->message)).ThrowAsJavaScriptException();
        return env.Undefined();
    }
    return Napi::String::New(Env(), uuid, VIR_UUID_STRING_BUFLEN - 1);
}

//endregion
//endregion



