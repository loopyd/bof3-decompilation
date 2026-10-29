# Edit a terrain texture and its palette

Change a 16x16 index patch in original US `BIN/WORLD00/AREA024.EMI` entry 7
and one color in entry 8. The catalog conditionally associates this patch with
surface record 723 in entry 13. The observed page is `0x0097`, the CLUT is
`0x7980`, and the UV bounds are `(80,144)` through `(95,159)`.
See [catalog records](catalog.md) for the source and consumer qualifications.
This combined edit has produced a magenta surface patch in the opening mine.
Other contexts and complete shared uses remain unresolved.

Build the executable using the [build instructions](../README.md#build).
Create the output parent and keep each destination new and outside its inputs.
Export the qualified original archive:

```sh
build/tools/rust/bof3-textures/release/bof3-textures unpack out/extracted/BIN/WORLD00/AREA024.EMI --source BIN/WORLD00/AREA024.EMI --output out/textures/terrain-package
```

Find exports by their resource IDs in `manifest.json`. Numbered package
directories do not identify an archive or entry independently of the manifest.
The upload's `words.json` is a row-major 512x256 word grid. Each word holds two
8-bit indices. The selected patch occupies word columns 168 through 175 and
rows 144 through 159. Palette row 486 starts at entry 8 word 768.

Replace both indices in each selected word with 179. Change that index's color
from `0x7e10` to magenta `0x7c1f`, with STP clear:

```python
import hashlib
import json
from pathlib import Path

root = Path("out/textures/terrain-package")
manifest = json.loads((root / "manifest.json").read_text())
exports = {item["resource"]: item for item in manifest["exports"]}
prefix = "SLUS_004.22:BIN/WORLD00/AREA024.EMI#entry-"
texture_path = root / exports[prefix + "007"]["words"]
palette_path = root / exports[prefix + "008"]["words"]
words = json.loads(texture_path.read_text())
palette = json.loads(palette_path.read_text())
assert len(words) == 512 * 256 and len(palette) == 2560
positions = [y * 512 + x for y in range(144, 160) for x in range(168, 176)]
original = b"".join(words[i].to_bytes(2, "little") for i in positions)
assert hashlib.sha256(original).hexdigest() == (
    "c4a2bc09272deb630786bd5071669fd33593cca3d5933f2ebace37559068c01b"
)
assert palette[768 + 179] == 0x7e10
for i in positions:
    words[i] = 0xb3b3
palette[768 + 179] = 0x7c1f
texture_path.write_text(json.dumps(words) + "\n")
palette_path.write_text(json.dumps(palette) + "\n")
```

Keep the other editable views and read-only previews unchanged. Pack and verify:

```sh
build/tools/rust/bof3-textures/release/bof3-textures pack out/textures/terrain-package --output out/textures/terrain-packed --json
build/tools/rust/bof3-textures/release/bof3-textures verify out/textures/terrain-packed --package out/textures/terrain-package --json
```

Require `edited_reconstruction.status` to be `verified`,
`unrelated_bytes.status` to be `byte-identical`, and `byte_checks_passed` to be
`true`. Verification still returns exit 1 while full graphics support is
incomplete. Exit 1 can also report failed byte checks, so inspect these fields.
The edit must preserve every byte outside the selected texture and palette words.

Reopen the packed output with its edit package to inspect the recovered words:

```sh
build/tools/rust/bof3-textures/release/bof3-textures unpack out/textures/terrain-packed --package out/textures/terrain-package --output out/textures/terrain-reopened
```

Use the media tool's [fixed-size replacement](../../bof3-disk/README.md#replace-a-file-in-a-copy)
to put the new archive into a new disc copy. Preserve the original media.
Other consumers of this texture patch or palette color can also change.
The pack report lists known dependencies; it does not enumerate every runtime use.
Byte reconstruction does not prove visible color, palette sampling, geometry,
or complete terrain coverage. Verify those properties separately through a
bounded run with matching edited-media provenance.
