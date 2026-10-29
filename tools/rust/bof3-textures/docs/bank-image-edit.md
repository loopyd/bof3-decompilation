# Edit a compressed sprite image

Change a decoded image and a stored palette word in original US
`BIN/BPLCHAR/BPLD012.EMI` entry 7. This example edits source records. It does not
identify the active character, image selector, or draw CLUT.

Build the current executable using the [build instructions](../README.md#build).
Supply the qualified original archive below. Create the output parent first and
keep every destination new and outside its inputs.

```sh
build/tools/rust/bof3-textures/release/bof3-textures unpack out/extracted/BIN/BPLCHAR/BPLD012.EMI --output out/textures/sprite-bank-package
build/tools/rust/bof3-textures/release/bof3-textures pack out/textures/sprite-bank-package --output out/textures/sprite-bank-unchanged --json
```

The unchanged output must match the complete original archive. Find the image
and palette document through their manifest entries:

```python
import json
from pathlib import Path

root = Path("out/textures/sprite-bank-package")
manifest = json.loads((root / "manifest.json").read_text())
resource = "SLUS_004.22:BIN/BPLCHAR/BPLD012.EMI#entry-007"
export = next(item for item in manifest["exports"] if item["resource"] == resource)
view = next(item for item in export["bank_images"]
            if item["root"] == 0 and item["image"] == 0)
print(root / view["member"])
print(root / export["sprite"])
```

Open the selected PNG in an editor that preserves indexed PNG data. Its original
dimensions are 256 by 8 pixels. Set the first six rows to palette index 0 and
leave the last two rows unchanged. Preserve the dimensions and diagnostic
grayscale palette. Each pixel stores one decoded byte; the image does not select
a GPU bit depth or a game palette. Save the PNG in indexed format.

To change the first stored color in root 0's first palette to STP black, run the
following after the discovery snippet:

```python
path = root / export["sprite"]
document = json.loads(path.read_text())
words = document["roots"][0]["palettes"][0]["words"]
assert len(words) == 32 and words[0] == 0
words[0] = 0x8000
path.write_text(json.dumps(document) + "\n")
```

Keep every other JSON field unchanged. In particular, image descriptors and
their `consumed_bytes` counters are read-only. Stored palette words preserve STP
exactly, but this palette edit does not prove which colors the image uses.

Pack the edits and verify the new archive tree:

```sh
build/tools/rust/bof3-textures/release/bof3-textures pack out/textures/sprite-bank-package --output out/textures/sprite-bank-packed --json
build/tools/rust/bof3-textures/release/bof3-textures verify out/textures/sprite-bank-packed --package out/textures/sprite-bank-package --json
build/tools/rust/bof3-textures/release/bof3-textures unpack out/textures/sprite-bank-packed --package out/textures/sprite-bank-package --output out/textures/sprite-bank-reopened
```

Inspect `byte_checks_passed`, `edited_reconstruction.status`, and
`unrelated_bytes.status`. Require `true`, `verified`, and `byte-identical`
respectively. Verification still exits 1 while full support remains incomplete;
exit 1 alone does not establish successful byte checks. The reopened PNG must
contain the intended index bytes, and its palette document must retain `0x8000`.

Both the inner image stream and outer bank must fit their original consumed
spans. A shorter result does not reduce the original capacity for later edits.
An oversized result is refused without a completed output. See
[Reopen packed edits](reopen-edits.md) before continuing from the new package.

## Edit cell records and animation steps

Use a separate package for this example. It changes a stored frame reference,
duration, anchor, U coordinate, and flip flag while preserving the image bytes.

```sh
build/tools/rust/bof3-textures/release/bof3-textures unpack out/extracted/BIN/BPLCHAR/BPLD012.EMI --output out/textures/sprite-record-package
```

Find entry 7's sprite document and change these existing records:

```python
import json
from pathlib import Path

root = Path("out/textures/sprite-record-package")
manifest = json.loads((root / "manifest.json").read_text())
resource = "SLUS_004.22:BIN/BPLCHAR/BPLD012.EMI#entry-007"
export = next(item for item in manifest["exports"] if item["resource"] == resource)
path = root / export["sprite"]
document = json.loads(path.read_text())
bank_root = document["roots"][0]
program = bank_root["programs"][0]
assert program["offset"] == 38906
assert program["steps"][0] == {"duration": 4, "operand": 0}
program["steps"][0] = {"duration": 5, "operand": 1}
cell = bank_root["cells"][0]
assert cell["offset"] == 38968
part = cell["parts"][0]
assert (part["x"], part["u"], part["horizontal_flip"]) == (-11, 0, False)
part.update(x=-10, u=8, horizontal_flip=True)
path.write_text(json.dumps(document) + "\n")
```

The step keeps its normal-frame role and refers to an existing cell. The first
stored cell receives a separate record edit. Keep other fields, list lengths,
offsets, and opaque flags unchanged. Durations count consumer updates, not a
promised number of displayed frames or milliseconds. This example does not
select a playback direction or establish which records a game entity uses.

Pack, verify, and re-export the new records:

```sh
build/tools/rust/bof3-textures/release/bof3-textures pack out/textures/sprite-record-package --output out/textures/sprite-record-packed --json
build/tools/rust/bof3-textures/release/bof3-textures verify out/textures/sprite-record-packed --package out/textures/sprite-record-package --json
build/tools/rust/bof3-textures/release/bof3-textures unpack out/textures/sprite-record-packed --package out/textures/sprite-record-package --output out/textures/sprite-record-reopened
```

Require the same successful byte-check fields described above. Inspect the
reopened `sprite.json`: its edited step and cell fields must match the requested
values, and other records must remain unchanged. Image PNGs and palette words
must match the unedited package. A valid stored UV or flip flag does not establish
its assembled appearance without the entity, source uploads, and draw state.

## Preserve source allocations

For other qualified records, mode 1 accepts index bytes from 0 to 31; mode 2 and
raw-word records accept bytes from 0 to 255. Keep dimensions, record counts,
offsets, image modes, headers, and diagnostic palettes unchanged. Shared views
must agree on their complete edited record. Raw and decoded bank views remain
read-only. Use the [sprite and animation rules](../README.md#edit-packages) for
cell or program edits. Physical loading, assembled appearance, animation timing,
and complete category coverage require their own evidence.
