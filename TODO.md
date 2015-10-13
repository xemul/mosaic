This is mosaic TODO list.

## API

### Mosaic API
- [ ] Listing/getting functionality
  - [ ] Get the information about mosaic (from the `.mos` file)

### Volume API
- [ ] A way to ask for driver information/stats
  - [ ] What are the limitations? Number of snapshots, min/max volume size, possible filesystems etc.
- [ ] Status checks (`is_mounted`, etc.)
- [ ] Listing/getting functionality
  - [ ] `list_volumes`
  - [ ] `get_device` (not the same as `attach_device`)
  - [ ] `list_mount_points`
  - [ ] sizes, various stats, etc.
- [ ] Monitoring mode (subscribe to and be notified of new events)

## Library
- [ ] layout support (need some description first)
- [ ] zfs driver
- [ ] dm-thin driver
- [ ] Implement logging
- [ ] Error reporting when parsing .mos files
- [ ] Better (more verbose) error reporting (don't ever silently return an error)
- [ ] Implement reference counting for mount/umount, get/put etc.
- [ ] Per-volume private data

### Ploop driver
- [ ] Revise the ploop clone/snapshot model (new incompatible libploop?)

## Moctl tool
- [ ] Unified usage synopsis
- [ ] Various listing commands
- [ ] JSON output, especially for list commands
- [ ] Bash completion
- [ ] man page

## Integration
- [ ] Write a Go wrapper
- [ ] Write a Docker graph driver with mosaic as a backend
- [ ] Write a Docker volume driver?
- [ ] Implement `make dist` target
- [ ] Write a spec file (to make rpms)
- [ ] Write Debian rules (to make debs)

## Testing
- [ ] Write more tests
- [ ] Use Docker mosaic graph driver for testing
- [ ] Run the code through Coverity

## Documentation
- [ ] Auto-generate API documentation from sources

## DONE
- [x] ploop driver, including clone
- [x] moct: attach/detach
- [x] read-only volume mounts
- [x] tests for clone
- [x] rename tessera -> volume
- [x] detach: remove device argument
- [x] fsimg: enforce 1:1 mapping
- [x] lib: hide symbols not in public API
- [x] lib: add pkg-config file
- [x] Makefile: add `install` target with `DESTDIR` support
