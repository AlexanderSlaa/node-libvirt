import {Hypervisor, Domain, GetVersion} from "../lib/binding";

const hypervisor = new Hypervisor({
    uri: "qemu:///system"
});

const domainXML =
`
<domain type='kvm'>
  <name>my-vm</name>
  <memory unit='MiB'>1024</memory>
  <vcpu>1</vcpu>
  <os>
    <type arch='x86_64' machine='pc-i440fx-2.12'>hvm</type>
    <boot dev='hd'/>
  </os>
  <devices>
    <disk type='file' device='disk'>
      <driver name='qemu' type='qcow2'/>
      <source file='/path/to/my-vm.qcow2'/>
      <target dev='vda' bus='virtio'/>
    </disk>
    <interface type='network'>
      <source network='default'/>
      <model type='virtio'/>
    </interface>
    <graphics type='vnc' port='-1' autoport='yes'/>
  </devices>
</domain>
`;


console.log(GetVersion());

async function main() {
    await hypervisor.connect()

    const domain = Domain.FromXML(domainXML, hypervisor);
    domain.create();

    console.log(hypervisor.hostname);

    const domains = hypervisor.domains();
    console.log(domains, "Amount:", domains.length);
    // console.log("hostname",hypervisor.hostname);
}

main().catch(console.error)


