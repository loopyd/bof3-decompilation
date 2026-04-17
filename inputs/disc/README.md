# Disc Images

Place exactly one Breath of Fire III disc image set in this directory when using the root `Makefile` or `bin/` commands:

- One `.cue` file with the referenced track `.bin` files. This is the preferred input.
- Two `.bin` files with a matching `.cue` sheet file beside it.

The tracked canonical US v1.1 files currently committed in this repo are:

- `Breath of Fire III (v1.1).cue`
- `Breath of Fire III (v1.1) (Track 1).bin`
- `Breath of Fire III (v1.1) (Track 2).bin`

The extract workflow auto-detects the single disc set under `inputs/disc/`; the cue sheet and track files only need to agree with each other and the committed checksum manifest.

`inputs/disc/` is the active runtime input path.
If you use the private importer flow, it downloads and extracts under `external/private-assets/...` first, then copies the selected cue/bin set here.

## Strict Identity Notes

For a strict byte-identical rebuild check, compare the rebuilt combined image against `track01 + track02` bytes from the source cue/bin set.

When rebuilding from a staged extracted tree that contains repacked `.EMI` files, preserve original `.EMI` mtimes before rebuild. `make pack` does this automatically; otherwise use `cp -p` or `touch -r`. Without preserved mtimes, ISO metadata timestamps can differ even if all `.EMI` files are byte-identical.

## Checksums

Run `make disk_checksums` to regenerate the tracked checksum manifest for the files in `inputs/disc/`.

Run `make verify_disk` to confirm your local cue/bin set matches the committed checksums in:

- the tracked disk checksum manifest used by `make verify_disk`
