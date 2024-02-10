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
     * Launch a defined domain. If the call succeeds the domain moves from the defined to the running domains pools.
     * * If the VIR_DOMAIN_START_PAUSED flag is set, or if the guest domain has a managed save image that requested paused state (see virDomainManagedSave()) the guest domain will be started, but its CPUs will remain paused. The CPUs can later be manually started using virDomainResume(). In all other cases, the guest domain will be running.
     * * If the VIR_DOMAIN_START_AUTODESTROY flag is set, the guest domain will be automatically destroyed when the virConnectPtr object is finally released. This will also happen if the client application crashes / loses its connection to the libvirtd daemon. Any domains marked for auto destroy will block attempts at migration. Hypervisors may also block save-to-file, or snapshots.
     * * If the VIR_DOMAIN_START_BYPASS_CACHE flag is set, and there is a managed save file for this domain (created by virDomainManagedSave()), then libvirt will attempt to bypass the file system cache while restoring the file, or fail if it cannot do so for the given system; this can allow less pressure on file system cache, but also risks slowing loads from NFS.
     * * If the VIR_DOMAIN_START_FORCE_BOOT flag is set, then any managed save file for this domain is discarded, and the domain boots from scratch.
     * * If flags includes VIR_DOMAIN_START_RESET_NVRAM, then libvirt will discard any existing NVRAM file and re-initialize NVRAM from the pristine template.
     *
     * domain: pointer to a defined domain
     * flags: bitwise-OR of supported virDomainCreateFlags
     * Returns: 0 in case of success, -1 in case of error
     * Access control parameter checks
     *

     * | Object                                             | Permission                                            | Condition |
     * |----------------------------------------------------|-------------------------------------------------------|-----------|
     * | [domain](https://libvirt.org/acl.html#object_domain) | [start](https://libvirt.org/acl.html#perm_domain_start) |x    |
     *
     * @param info
     */
    void Create(const Napi::CallbackInfo &info);

    void Save(const Napi::CallbackInfo &info);

    Napi::Value ToXML(const Napi::CallbackInfo &info);
    //endregion

private:

//region STATIC

    /**
     * Define a domain, but does not start it.
     *
     * This definition is persistent, until explicitly undefined with virDomainUndefine().
     *
     * A previous definition for this domain with the same UUID and name would be overridden if it already exists.
     * @param info
     * @return Domain
     */
    static Napi::Value DefineXML(const Napi::CallbackInfo &info);

    static Napi::Value CreateXML(const Napi::CallbackInfo &info);

    static Napi::Value LookupById(const Napi::CallbackInfo &info);

//endregion

private:
    virDomainPtr _domain = nullptr;

};


#endif //NODE_LIBVIRT_DOMAIN_H
