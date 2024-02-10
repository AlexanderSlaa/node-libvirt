import {type Domain, DomainSaveRestoreFlags} from "./domain";
import {NodeInfo} from "./nodeinfo";

export type Hypervisor = {

    new(config: {
        username?: string,
        password?: string,
        uri: string
        readOnly?: true
    });

    get capabilities(): string
    get hostname(): string
    get sysInfo(): string
    get maxVCPUs(): number
    get info(): NodeInfo

    connect(): Promise<void>;
    disconnect(): Promise<void>;

    domains(): Domain[]
    lookupDomainById(id: number): Promise<Domain>
    lookupDomainByName(name: string): Promise<Domain>
    lookupDomainByUUIDString(uuid: string): Promise<Domain>
    restoreDomain(xml: string, dxml?: string, flags?: DomainSaveRestoreFlags): Promise<Domain>
}