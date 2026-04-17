# Disc Images

This file records known-good source image references for the canonical US v1.1 target.

Use `processed/inventory/inventory.sqlite` for the canonical machine-readable disk inventory and `make verify_disk` for automated local verification. The verifier currently reads `processed/inventory/disk/checksums.json` as its checksum manifest.

## Canonical Disk Set

- `disk/Breath of Fire III (v1.1) (Track 1).bin`
  - MD5: `9dd9a7c934b8b59d0ce76b0f25d18176`
  - SHA-256: `94835d58c8b19c39b551039010ee9669861f1421958002b2f6927bb2d50f2f55`
- `disk/Breath of Fire III (v1.1) (Track 2).bin`
  - MD5: `2d7b5e8e94a91bf5423b2356f6a34863`
  - SHA-256: `ce5509fad13f6210656c9d29fb536b47abe5f824467177652c91b5c500470c77`
- `disk/Breath of Fire III (v1.1).cue`
  - MD5: `5e3ff7c1747a1dd658865084d037e6e9`
  - SHA-256: `1c24d737c6f9d2a1c7ad31fd08bd745a80ac6940958313a5db62cacb1ce3bf56`

## Reference Archive

The archive `Breath of Fire III (v1.1).7z` was extracted and compared byte-for-byte against the track data in the current `disk/` set. The tracked local cue sheet now keeps the original `Breath of Fire III (v1.1)` filenames.

- MD5: `b862836d01c300ab3a45215ac386f554`
- SHA-256: `e079877817cf72ccc73d9e9a9bf986a66f1142fbb6d6f4513a7fe7739a8cd8d4`

Status:

- extracted `Track 1.bin` matches the canonical `disk/` copy
- extracted `Track 2.bin` matches the canonical `disk/` copy
- the tracked `.cue` resolves the same two track payloads under the canonical `Breath of Fire III (v1.1)` filenames
