import {Hypervisor, Domain, libvirt} from "../lib/binding";

const hypervisor = new Hypervisor({
    uri: "qemu:///system"
});

console.log(libvirt.GetVersionObject());

const lxcDomain = `
<domain type='lxc'>
  <name>vm1</name>
  <memory>500000</memory>
  <os>
    <type>exe</type>
    <init>/bin/sh</init>
  </os>
  <vcpu>1</vcpu>
  <clock offset='utc'/>
  <on_poweroff>destroy</on_poweroff>
  <on_reboot>restart</on_reboot>
  <on_crash>destroy</on_crash>
  <devices>
    <emulator>/usr/libexec/libvirt_lxc</emulator>
    <interface type='network'>
      <source network='default'/>
    </interface>
    <console type='pty' />
  </devices>
</domain>
`

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


