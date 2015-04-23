## Summary

Mosaic too makes a Linux file tree from of individual sub-trees each of
which can be a set of snapshots. There are several use cases where this 
approach can be used, e.g. trying an update of a system, flexible multi
user environment, containers with shared files, nested containers,
multi-tennant applications etc.


## Definitions

* Mosaic: this is the files tree built using the described technology.
The mosaic can be mounted either as system root or as some sub-tree within
it. The directory mosaic is mounted too can be either empty or contain
anything.

* Tessera: an individual sub-tree that can be put into mosaic. Each tessera
has a source where data comes from depending on tessera type, and can
contain several data snapshots called ages. There's always one base age
and a set of increments (in a form of tree). There can be only a base
snapshot though.

* Element: it's a tessera that is put into a mosaic. Element references a
certain tessera age and destination –- the name under which the user sees
the tessera's root inside mosaic.

### Translating into Linux language

Tessera is a file system or block device optionally supporting snapshots.
Mosaic is a set of mountpoints each of which is a particular snaphot of
a tessera tree.

Specifically, the tesserae can be stored using several technologies,
such as overlayfs, dm-thin, btrfs and others.

## Library

The project introduces the libmosaic library written in C that provides
the calls to

* manage mosaics, tesserae and elements
* mounting and umounting them
* working with tesserae snapshots
* keeping the configuration between restarts
* tracking the objects states

The API is described in the include/uapi/mosaic.h header.

## MOCTL tool

This is the command-line tool that manipulates mosaic objects
using the library API. The tool is in moctl/ directory and
it's syntax resembles the one of the iproute2 tool

```shell
$ moctl <object> <action> [<arguments>]
```

where object is either mosaic or tessera and actions include
adding, removing, modifying, mounting, umounting, etc.
