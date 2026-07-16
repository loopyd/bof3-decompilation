# Disc input

The read-only `bin/bof3-disk` workflow uses one user-owned BIN/CUE set under
`inputs/disc/`.

## Expected files

Use one complete Breath of Fire III disc set:

- one `.cue` sheet;
- the two `.bin` tracks referenced by that cue sheet.

The cue filenames and its `FILE` entries must agree. Keep the original files
unchanged: extraction and any local comparison depend on the exact track
bytes. Disc media is ignored and must never be committed.

## Identity and checksums

The repository does not track game-media hashes. Use `bin/bof3-disk --help`
for its read-only extraction and checksum operations. Keep any local checksum
manifest below `out/`; it is evidence, not tracked project state.
