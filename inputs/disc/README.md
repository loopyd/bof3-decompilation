# Legacy disc input

`inputs/disc/` is retained for compatibility with older local workspaces. The
supported `just` and `bin/harness` workflows discover one user-owned BIN/CUE set
under [`disks/`](../../disks/README.md).

## Expected files

Use one complete Breath of Fire III disc set:

- one `.cue` sheet;
- the two `.bin` tracks referenced by that cue sheet.

The cue filenames and its `FILE` entries must agree. Keep the original files
unchanged: extraction, rebuilding, and strict comparison depend on the exact
track bytes. Disc media is ignored and must never be committed.

## Legacy use

Do not add new media here. Existing local files can be extracted explicitly:

```sh
PYTHONPATH=tools/python .venv/bin/python -m harness.commands.disk \
  disk-extract --disc-dir inputs/disc --output out/extracted
```

Move the set to `disks/` when normalizing the workspace so `just extract` and
the default `bin/harness disk` commands can discover it.

## Identity and checksums

The repository does not track game-media hashes. Generate a local checksum
manifest under `out/` when the exact input identity needs to be retained:

```sh
PYTHONPATH=tools/python .venv/bin/python -m harness.commands.disk \
  disk-checksums --input-dir inputs/disc --output out/disk_checksums.json
```

Verify the same files against that manifest with `disk-verify`, passing the
same input directory and `--checksums out/disk_checksums.json`.

For strict rebuild comparison, compare the rebuilt combined image with the
source track bytes in cue order. When rebuilding from repacked EMI files,
preserve their original mtimes; ISO metadata timestamps can otherwise differ
even when the EMI payload bytes are identical.
