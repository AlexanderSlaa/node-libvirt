import {type libvirt as virt} from "./types";

const $ = require("./../build/Release/node-libvirt.node") as virt;


export const Hypervisor = $.Hypervisor;
export const Domain = $.Domain;

export const libvirt = {
    GetVersion: $.GetVersion,
    GetVersionObject: () => {
        const version = $.GetVersion();
        return {
            major: version / 1000000,
            minor: (version / 1000) % 1000,
            micro: version % 1000
        }
    }
}
