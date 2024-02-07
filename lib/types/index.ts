import {Hypervisor} from "./hypervisor";

export type libvirt = {
    Hypervisor: Hypervisor
    GetVersion(): number;
}