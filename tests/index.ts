import {Hypervisor, GetVersion} from "../lib/binding";

const hypervisor = new Hypervisor({
    uri: "qemu:///system"
});

console.log(GetVersion());

async function main() {
    try {
        await hypervisor.connect()
    } catch (e) {
        console.error(e);
    }
    // console.log("hostname",hypervisor.hostname);
}

main().catch(console.error)


