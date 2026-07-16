# bof3-disk v1 parity gates

Authority is the imported `third_party/deprecated/bof3-disk` implementation and real-disc
output. Fixture tests prove only the bounded contracts named below.

## Proven slices

- ISO9660 extraction from cooked 2048 and raw 2352 Mode 1/Mode 2 Form 1.
- Bounded directory records and both-endian ISO fields.
- Raw license payload extraction.
- Multi-file CUE parsing with track modes, INDEX 00/01, and pregaps.
- CDDA sector PCM conversion through `hound` to stereo 44.1 kHz 16-bit WAV.
- Typed `disc_lba.json` rows with v1 lexical key order, LBA ordering, XA sector
  length, family classification, and EMI manifest mapping.
- Portable streaming checksums and verification.

## Canonical replacement blockers

- XA directory classification and 2336-byte extent counting are implemented;
  sector subheader/Form 1/Form 2 validation and real BOF3 stream byte comparison
  remain required.
- The project model must preserve PVD identifiers and dates, `xa_edc`,
  `new_type`, PS2 flag, record timestamps/GMT offsets, XA permissions/GID/UID,
  hidden flags, explicit LBA ordering, dummy gaps/ECC flags, license path, and
  audio track/pregap metadata. The current `Entry` model cannot express these,
  so current project XML is intentionally provisional.
- Rebuild must write valid Mode 2 raw sectors including sync/header/subheader,
  EDC/ECC, ISO9660 descriptors/path tables/directories, license sectors, gaps,
  CDDA tracks, and exact CUE sidecars.
- Extraction and rebuild must be compared against v1 on the user-owned BOF3
  image, including every ordinary/XA payload, WAV, XML, JSON, CUE, and rebuilt
  track checksum.

Do not switch the harness integration until every blocker has authoritative
real-disc evidence.
