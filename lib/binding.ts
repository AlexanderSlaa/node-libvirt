import {type libvirt} from "./types";

const libvirt = require("./../build/Release/node-libvirt.node") as libvirt;


export const Hypervisor = libvirt.Hypervisor;

export function GetVersion(){
    return libvirt.GetVersion();
}

