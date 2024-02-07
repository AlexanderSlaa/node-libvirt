import {type Domain} from "./domain";

export type Hypervisor ={

    new(config: {
        username?: string,
        password?: string,
        uri: string
        readOnly?: true
    });

    get hostname(): string

    connect(): Promise<void>;
    disconnect(): Promise<void>;

    domains(): Domain[]

}