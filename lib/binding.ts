import {type libvirt} from "./types";

const libvirt = require("./../build/Release/node-libvirt.node") as libvirt;


export const Hypervisor = libvirt.Hypervisor;
export const Domain = libvirt.Domain;

export function GetVersion(){
    return libvirt.GetVersion();
}

