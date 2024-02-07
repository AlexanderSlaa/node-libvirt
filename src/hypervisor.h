//
// Created by root on 1/12/24.
//

#ifndef NODE_LIBVIRT_HYPERVISOR_H
#define NODE_LIBVIRT_HYPERVISOR_H

#include <napi.h>

#include <libvirt/libvirt.h>


class Hypervisor : public Napi::ObjectWrap<Hypervisor> {

private:
    std::string _username;
    std::string _password;
    bool _readonly = false;
    std::string _uri;

    virConnectPtr _handle = nullptr;

public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);

    explicit Hypervisor(const Napi::CallbackInfo &info);

    ~Hypervisor();

    virConnectPtr Handle(){
        return this->_handle;
    }

private:

    Napi::Value Connect(const Napi::CallbackInfo &info);

    Napi::Value Disconnect(const Napi::CallbackInfo &info);

    Napi::Value GetCapabilities(const Napi::CallbackInfo &info);

    Napi::Value GetHostname(const Napi::CallbackInfo &info);

    Napi::Value GetSysInfo(const Napi::CallbackInfo &info);

    Napi::Value ListAllDomains(const Napi::CallbackInfo &info);
};

#endif // NODE_LIBVIRT_HYPERVISOR_H
