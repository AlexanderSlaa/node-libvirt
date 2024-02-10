import {Hypervisor} from "./hypervisor";
import {libvirt} from "./index";

export type DomainInfo = { state: DomainState, maxMem: number, memory: number, nrVirtCpu: number, cpuTime: number };

export enum DomainState {
    /**
     * no state
     * @since 0.0.1
     */
    NOSTATE = 0,
    /** the domain is running
     * @since 0.0.1
     */
    RUNNING = 1,
    /**
     * the domain is blocked on resource
     * @since 0.0.1
     */
    BLOCKED = 2,
    /** the domain is paused by user (Since: 0.0.1) */
    PAUSED = 3,
    /** the domain is being shut down (Since: 0.0.1) */
    SHUTDOWN = 4,
    /** the domain is shut off (Since: 0.0.1) */
    SHUTOFF = 5,
    /** the domain is crashed (Since: 0.0.2) */
    CRASHED = 6,
    /** the domain is suspended by guest power management (Since: 0.9.11) */
    PMSUSPENDED = 7,
}

export enum DomainShutdownFlagValues {
    SHUTDOWN_DEFAULT = 0,        /* hypervisor choice (Since: 0.9.10) */
    SHUTDOWN_ACPI_POWER_BTN = (1 << 0), /* Send ACPI event (Since: 0.9.10) */
    SHUTDOWN_GUEST_AGENT = (1 << 1), /* Use guest agent (Since: 0.9.10) */
    SHUTDOWN_INITCTL = (1 << 2), /* Use initctl (Since: 1.0.1) */
    SHUTDOWN_SIGNAL = (1 << 3), /* Send a signal (Since: 1.0.1) */
    SHUTDOWN_PARAVIRT = (1 << 4), /* Use paravirt guest control (Since: 1.2.5) */
}

enum DomainCreateFlags {
    /**
     * Default behavior
     * (0x0)
     */
    NONE = 0,
    /**
     * Launch guest in paused state
     *
     * If flag is set, or if the guest domain has a managed save image that requested paused state (see [virDomainManagedSave()](https://libvirt.org/html/libvirt-libvirt-domain.html#virDomainManagedSave)) the guest domain will be started, but its CPUs will remain paused.
     * The CPUs can later be manually started using [virDomainResume()](https://libvirt.org/html/libvirt-libvirt-domain.html#virDomainResume).
     * In all other cases, the guest domain will be running.
     *
     * (0x1; 1 << 0)
     */
    START_PAUSED = 1,
    /**
     * Automatically kill guest when virConnectPtr is closed
     *
     * If flag is set, the guest domain will be automatically destroyed when the [virConnectPtr](https://libvirt.org/html/libvirt-libvirt-host.html#virConnectPtr) object is finally released.
     * This will also happen if the client application crashes / loses its connection to the libvirtd daemon.
     * Any domains marked for auto destroy will block attempts at migration.
     * Hypervisors may also block save-to-file, or snapshots.
     *
     * (0x2; 1 << 1)
     */
    START_AUTODESTROY = 2,
    /**
     * Avoid file system cache pollution
     *
     * If the flag is set, and there is a managed save file for this domain (created by [virDomainManagedSave()](https://libvirt.org/html/libvirt-libvirt-domain.html#virDomainManagedSave)), then libvirt will attempt to bypass the file system cache while restoring the file, or fail if it cannot do so for the given system;
     * this can allow less pressure on file system cache, but also risks slowing loads from NFS.
     *
     * (0x4; 1 << 2)
     */
    START_BYPASS_CACHE = 4,
    /**
     * Boot, discarding any managed save
     *
     * If the flag is set, then any managed save file for this domain is discarded, and the domain boots from scratch.
     *
     * (0x8; 1 << 3)
     */
    START_FORCE_BOOT = 8,
    /**
     * Validate the XML document against schema
     * (0x10; 1 << 4)
     */
    START_VALIDATE = 16,
    /**
     * Re-initialize NVRAM from template
     *
     * If flags includes VIR_DOMAIN_START_RESET_NVRAM, then libvirt will discard any existing NVRAM file and re-initialize NVRAM from the pristine template.
     *
     * (0x20; 1 << 5)
     */
    START_RESET_NVRAM = 32
}

export enum DomainSaveRestoreFlags {
    /**
     *  (0x1; 1 << 0)
     * Avoid file system cache pollution
     */
    SAVE_BYPASS_CACHE = 1,
    /**
     *  (0x2; 1 << 1)
     * Favor running over paused
     */
    SAVE_RUNNING = 2,
    /**
     * (0x4; 1 << 2)
     * Favor paused over running
     */
    SAVE_PAUSED = 4,
    /**
     * (0x8; 1 << 3)
     * Re-initialize NVRAM from template
     */
    SAVE_RESET_NVRAM = 8
}

export declare class Domain {
    /* Instance accessors */
    get id(): number;

    get info(): DomainInfo;

    get name(): string

    get uuid(): string

    /* Instance Methods */
    shutdown(flags?: DomainShutdownFlagValues): void

    /**
     * Launch a defined domain.
     * If the call succeeds the domain moves from the defined to the running domains pools.
     * @param flags bitwise-OR of supported DomainCreateFlags
     */
    create(flags?: DomainCreateFlags): void;

    /**
     * This method will suspend a domain and save its memory contents to a file on disk. After the call, if successful, the domain is not listed as running anymore (this ends the life of a transient domain). Use virDomainRestore() to restore a domain after saving.
     * If the hypervisor supports it, dxml can be used to alter host-specific portions of the domain XML that will be used when restoring an image. For example, it is possible to alter the backing filename that is associated with a disk device, in order to prepare for file renaming done as part of backing up the disk device while the domain is stopped.
     * If flags includes VIR_DOMAIN_SAVE_BYPASS_CACHE, then libvirt will attempt to bypass the file system cache while creating the file, or fail if it cannot do so for the given system; this can allow less pressure on file system cache, but also risks slowing saves to NFS.
     * Normally, the saved state file will remember whether the domain was running or paused, and restore defaults to the same state. Specifying VIR_DOMAIN_SAVE_RUNNING or VIR_DOMAIN_SAVE_PAUSED in flags will override what state gets saved into the file. These two flags are mutually exclusive.
     * A save file can be inspected or modified slightly with virDomainSaveImageGetXMLDesc() and virDomainSaveImageDefineXML().
     * Some hypervisors may prevent this operation if there is a current block job running; in that case, use virDomainBlockJobAbort() to stop the block job first.
     *
     * @param filename
     * @param dxml
     * @param flags
     */
    save(filename: string, dxml?: string, flags?: DomainSaveRestoreFlags): void;

    /* Static Methods */
    static FromXML(xml: string, context: Hypervisor, flags?: number): Domain
}
