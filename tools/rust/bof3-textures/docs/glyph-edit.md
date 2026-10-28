# Edit a shared font glyph

The original US `BIN/ETC/FIRST.EMI` entry 3 contains a shared 4-bit glyph atlas.
The ordinary 8x12 consumer at `SLUS_004.22@0x8014F84C` maps byte `0x44`
to UV `(160, 72)` when both crop values are zero. The following example replaces
that 8x12 cell with palette index 1. Every consumer reading the same atlas words
can be affected. Text codes, layout records and palette words remain unchanged.

Use the [build and input instructions](../README.md#inputs-and-commands), then
unpack the qualified original archive into a new package:

```sh
build/tools/rust/bof3-textures/release/bof3-textures unpack out/original-us/BIN/ETC/FIRST.EMI --output out/textures/font-package
```

Find the resource in the package manifest instead of assuming an export directory
number. Its `words.json` is a row-major array of 64 words by 256 rows. Four
low-to-high nibbles in each word select four consecutive pixels. Setting a word
to `0x1111` selects palette index 1 for all four pixels:

```python
import json
from pathlib import Path

root = Path("out/textures/font-package")
manifest = json.loads((root / "manifest.json").read_text())
resource = "SLUS_004.22:BIN/ETC/FIRST.EMI#entry-003"
export = next(item for item in manifest["exports"] if item["resource"] == resource)
path = root / export["words"]
words = json.loads(path.read_text())
assert len(words) == 64 * 256
for y in range(72, 84):
	words[y * 64 + 40:y * 64 + 42] = [0x1111, 0x1111]
path.write_text(json.dumps(words) + "\n")
```

Keep the other source views and read-only previews unchanged. Packing applies
the changed words and preserves unrelated archive bytes:

```sh
build/tools/rust/bof3-textures/release/bof3-textures pack out/textures/font-package --output out/textures/font-packed --json
build/tools/rust/bof3-textures/release/bof3-textures verify out/textures/font-packed --package out/textures/font-package --json
```

Proceed only when `edited_reconstruction.status` is `verified`,
`unrelated_bytes.status` is `byte-identical`, and `byte_checks_passed` is `true`.
Exit 1 is expected while graphics acceptance is incomplete, but can also
accompany failed byte checks.
For CLUT `0x7800`, index 1 selects word 1 of FIRST entry 8. Other palette selectors
and drawing states can change the appearance of the same edited indices.

Use the full repository's media tool and its
[fixed-size replacement](../../bof3-disk/README.md#replace-a-file-in-a-copy)
with `BIN/ETC/FIRST.EMI` and the packed archive. Preserve the original media and
use a new image and CUE. Runtime checks need matching original and edited
observation boundaries; a black frame during a fade cannot establish appearance.
This workflow covers one source cell and consumer path. Other glyph layouts,
palettes, loading paths and graphics families retain their own verification needs.
