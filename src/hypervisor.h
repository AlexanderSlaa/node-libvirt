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

    ~Hypervisor() override;

    virConnectPtr Handle() {
        return this->_handle;
    }

private:

    Napi::Value Connect(const Napi::CallbackInfo &info);

    Napi::Value Disconnect(const Napi::CallbackInfo &info);

    Napi::Value GetCapabilities(const Napi::CallbackInfo &info);

    Napi::Value GetHostname(const Napi::CallbackInfo &info);

    Napi::Value GetSysInfo(const Napi::CallbackInfo &info);

    Napi::Value GetMaxVCPUs(const Napi::CallbackInfo &info);

    /**
     * Extract hardware information about the node.
     * Use of this API is strongly discouraged as the information provided is not guaranteed to be accurate on all hardware platforms.
     * The mHZ value merely reflects the speed that the first CPU in the machine is currently running at.
     * This speed may vary across CPUs and changes continually as the host OS throttles.
     * The nodes/sockets/cores/threads data is potentially inaccurate as it assumes a symmetric installation.
     * If one NUMA node has more sockets populated that another NUMA node this information will be wrong.
     * It is also not able to report about CPU dies.
     * Applications are recommended to use the [virConnectGetCapabilities()](https://libvirt.org/html/libvirt-libvirt-host.html#virConnectGetCapabilities) call instead, which provides all the information except CPU mHZ, in a more accurate representation.
     * @param info
     * @return
     */
    Napi::Value GetInfo(const Napi::CallbackInfo &info);

    Napi::Value ListAllDomains(const Napi::CallbackInfo &info);

    Napi::Value LookupDomainById(const Napi::CallbackInfo &info);

    Napi::Value LookupDomainByName(const Napi::CallbackInfo &info);

    Napi::Value LookupDomainByUUIDString(const Napi::CallbackInfo &info);

    Napi::Value RestoreDomain(const Napi::CallbackInfo &info);
};

#endif // NODE_LIBVIRT_HYPERVISOR_H
