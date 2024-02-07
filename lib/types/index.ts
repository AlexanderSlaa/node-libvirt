import {type Hypervisor} from "./hypervisor";
import {type Domain} from "./domain";

export declare class External<T = unknown>{
    private constructor();
}

export type libvirt = {
    Hypervisor: Hypervisor
    Domain: typeof Domain
    GetVersion(): number;
}