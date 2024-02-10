import {Hypervisor, Domain, libvirt} from "../lib/binding";

const hypervisor = new Hypervisor({
    uri: "qemu:///system"
});

console.log(libvirt.GetVersionObject());

async function main() {
    await hypervisor.connect()

    console.log(hypervisor.info);
    console.log(hypervisor.hostname);

    //
    // const domain = Domain.FromXML(domainXML, hypervisor);
    // domain.create();
    //
    // console.log(hypervisor.hostname);
    //
    // const domains = hypervisor.domains();
    // console.log(domains, "Amount:", domains.length);
    // console.log("hostname",hypervisor.hostname);
}

main().catch(console.error)


