import {Hypervisor} from "./hypervisor";
import {libvirt} from "./index";

export type DomainInfo = { state: DomainState, maxMem: number, memory: number, nrVirtCpu: number, cpuTime: number };

export enum DomainState {
    DOMAIN_NOSTATE = 0, /** no state (Since: 0.0.1) */
    DOMAIN_RUNNING = 1, /** the domain is running (Since: 0.0.1) */
    DOMAIN_BLOCKED = 2, /** the domain is blocked on resource (Since: 0.0.1) */
    DOMAIN_PAUSED = 3, /** the domain is paused by user (Since: 0.0.1) */
    DOMAIN_SHUTDOWN = 4, /** the domain is being shut down (Since: 0.0.1) */
    DOMAIN_SHUTOFF = 5, /** the domain is shut off (Since: 0.0.1) */
    DOMAIN_CRASHED = 6, /** the domain is crashed (Since: 0.0.2) */
    DOMAIN_PMSUSPENDED = 7, /** the domain is suspended by guest power management (Since: 0.9.11) */
}

export enum DomainShutdownFlagValues {
    SHUTDOWN_DEFAULT = 0,        /* hypervisor choice (Since: 0.9.10) */
    SHUTDOWN_ACPI_POWER_BTN = (1 << 0), /* Send ACPI event (Since: 0.9.10) */
    SHUTDOWN_GUEST_AGENT = (1 << 1), /* Use guest agent (Since: 0.9.10) */
    SHUTDOWN_INITCTL = (1 << 2), /* Use initctl (Since: 1.0.1) */
    SHUTDOWN_SIGNAL = (1 << 3), /* Send a signal (Since: 1.0.1) */
    SHUTDOWN_PARAVIRT = (1 << 4), /* Use paravirt guest control (Since: 1.2.5) */
}


export declare class Domain{
    /* Instance accessors */
    get id(): number;

    get info(): DomainInfo;

    get name(): string

    get uuid(): string

    /* Instance Methods */
    shutdown(flags?: DomainShutdownFlagValues): void

    create(): void;

    /* Static Methods */
    static FromXML(xml: string, context: Hypervisor, flags?: number): Domain
}
