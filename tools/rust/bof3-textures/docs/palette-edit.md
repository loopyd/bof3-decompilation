# Edit an opening-scene palette

Change one stored color used by the right-hand miner in the opening mine scene.
The qualified original US source is `BIN/WORLD00/AREA024.EMI` entry 6.
In the observed 8-bit draw context, CLUT `0x7c02` starts at VRAM word X 32,
row 496. Index 17 selects source word 49. Other draw contexts need their own
palette selection evidence.

Build the current executable using the [build instructions](../README.md#build).
Supply the qualified original archive at the input path below. Create the output
parent first, and keep every destination new and outside its inputs.

```sh
build/tools/rust/bof3-textures/release/bof3-textures unpack out/extracted/BIN/WORLD00/AREA024.EMI --output out/textures/mine-palette-package
```

Find the palette through its resource ID in the manifest. Preserve all 768 words
except word 49. This example replaces `0x1886` with red `0x001f`, with STP clear:

```python
import json
from pathlib import Path

root = Path("out/textures/mine-palette-package")
manifest = json.loads((root / "manifest.json").read_text())
resource = "SLUS_004.22:BIN/WORLD00/AREA024.EMI#entry-006"
export = next(item for item in manifest["exports"] if item["resource"] == resource)
path = root / export["words"]
words = json.loads(path.read_text())
assert len(words) == 768 and words[49] == 0x1886
words[49] = 0x001f
path.write_text(json.dumps(words) + "\n")
```

Keep other editable views and read-only previews unchanged. Pack and verify the
new archive tree:

```sh
build/tools/rust/bof3-textures/release/bof3-textures pack out/textures/mine-palette-package --output out/textures/mine-palette-packed --json
build/tools/rust/bof3-textures/release/bof3-textures verify out/textures/mine-palette-packed --package out/textures/mine-palette-package --json
```

Proceed only when `edited_reconstruction.status` is `verified`,
`unrelated_bytes.status` is `byte-identical`, and `byte_checks_passed` is `true`.
Exit 1 also covers failed byte checks, so inspect those fields even though full
graphics verification remains incomplete. The output retains the original
allocation and changes only the two bytes of this palette word.

To inspect or continue the edit, export the packed output with its edit package:

```sh
build/tools/rust/bof3-textures/release/bof3-textures unpack out/textures/mine-palette-packed --package out/textures/mine-palette-package --output out/textures/mine-palette-reopened
```

The reopened palette contains `0x001f` at word 49. It retains the qualified
original snapshots for further edits. See [Reopen packed edits](reopen-edits.md)
for baseline validation and reconstruction after moving former inputs.

Use the repository's media tool and its
[fixed-size replacement](../../bof3-disk/README.md#replace-a-file-in-a-copy)
to replace `BIN/WORLD00/AREA024.EMI` in a new disc image. Preserve the original
media and use a matching new CUE. Enter the opening scene through the ordinary
new-game menus to inspect the affected color on the right-hand miner.
Any draw that uses the same palette word can change; this example does not
enumerate all shared uses.

This selection binds one observed field context to its source palette. It does
not assign palettes to every cell or diagnostic image. Geometry, animation,
other loading paths, and complete field coverage retain separate verification
requirements. The byte verifier does not import external runtime observations.
