## Summary

Mosaic is a tool and library to configure hybrid storage to be used in
VM/CT environments. A storage is supposed to be suitable for both --
keeping lots of small files for VM/CT meta-data as well as fast and
scalable disks for VMs and CTs data.


## Definitions

Mosaic: a place where metadata and data are. Mosaic consists of one
(for now) place for metadata -- FS, and zero or more places for VM/CT
data -- volumes.

Volume: this is where VM/CT files are. It can be thought of as a disk
for VM or FS subtree for CT.


## Overview

Both moctl (the tool) and libmosaic work on a pre-created storage that
is described by a config file (in yaml). The config states the storage
type (btrfs, loop, ploop, dmthin, etc.) and data location -- a string
that is interpreted in type-specific manner. Other config options are,
well, optional and also type-dependent.

When working with mosaic one should load config and then do whatever
he wants. The moctl tool loads config on every start since all of the
operations are "stateless" from the library point of view.


## Config file example

    type: btrfs
    location: /foo/bar

Describes a btrfs mosaic located at /foo/bar. The metadata part is then
in the /foo/bar, so are the volumes. Adding

    layout:
        fs: subdir

will mean that the metadata is at /foo/bar/subdir. Volumes remain in
the /foo/bar and thus do not intersect with metadata.

The detailed description of config file is in doc/config.txt


## Tool usage

The moctl usage is

    moctl <config_file> <action> <arguments>

The config file should exist, moctl loads one before doing actions. The
actions are mount, umount, new, clone and drop.

The first two mount and umount metadata or volume, new and clone create
new empty volume or clone one from the existing, drop removes the
volume. Each action accepts its own set of arguments.

The detailed description of moctl is in doc/moctl.txt


## Library API

The API is declared in include/uapi/mosaic.h. Typical workflow with it
looks like

    mosaic_t m;

    m = mosaic_open(config_file, 0);
    mosaic_make_vol(m, name, size_in_bytes, 0);
    ...
    mosaic_close(m, 0);


The detailed description of the API is in doc/libmosaic.txt
