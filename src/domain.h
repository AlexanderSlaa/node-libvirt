//
// Created by root on 2/7/24.
//

#ifndef NODE_LIBVIRT_DOMAIN_H
#define NODE_LIBVIRT_DOMAIN_H

#include "napi.h"
#include <libvirt/libvirt.h>
#include <libvirt/virterror.h>
#include <libvirt/libvirt-domain.h>

class Domain : public Napi::ObjectWrap<Domain> {

public:

    static Napi::Object Init(Napi::Env env, Napi::Object exports);

    static Napi::Object New(Napi::Env env, const std::initializer_list<napi_value> &args);

    explicit Domain(const Napi::CallbackInfo &info);

    ~Domain() override;


private:

//region ACCESSORS

    Napi::Value Info(const Napi::CallbackInfo &info);

    Napi::Value Id(const Napi::CallbackInfo &info);

    Napi::Value Name(const Napi::CallbackInfo &info);

    Napi::Value UUIDString(const Napi::CallbackInfo &info);
//endregion

//region INSTANCE METHODS

    void Shutdown(const Napi::CallbackInfo &info);

    /**
     * Launch a defined domain.
     * If the call succeeds the domain moves from the defined to the running domains pools.
     * The domain will be paused only if restoring from managed state created from a paused domain.
     * @param info
     */
    void Create(const Napi::CallbackInfo &info);
    //endregion

private:

//region STATIC

    static Napi::Value FromXML(const Napi::CallbackInfo &info);
//endregion

private:
    virDomainPtr _domain = nullptr;

};


#endif //NODE_LIBVIRT_DOMAIN_H
