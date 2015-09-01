## Summary

Mosaic is a tool and library to configure hybrid storage to be used in
VM/CT environments. A storage is supposed to be suitable for both --
keeping lots of small files for VM/CT meta-data as well as fast and
scalable disks for VMs and CTs data.


## Definitions

Mosaic: a place where metadata and data are. Mosaic consists of one
(for now) place for metadata -- FS, and zero or more places for VM/CT
data -- tesserae.

Tessera: this is where VM/CT files are. It can be thought of as a disk
for VM or FS subtree for CT.
