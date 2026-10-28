# Reopen packed edits

Use `unpack --package` to export supported edits from a packed output tree into a
new self-contained package. The output bytes must match the reconstruction of
the supplied edit package. A modified archive alone is not a qualified original.

Build the current executable using the [build instructions](../README.md#build).
Keep the edit package and its packed output until re-export succeeds. Each
destination must be new, with an existing parent outside both inputs.

```sh
build/tools/rust/bof3-textures/release/bof3-textures pack out/textures/dragon-upload --output out/textures/packed --json
build/tools/rust/bof3-textures/release/bof3-textures unpack out/textures/packed --package out/textures/dragon-upload --output out/textures/dragon-reopened
build/tools/rust/bof3-textures/release/bof3-textures pack out/textures/dragon-reopened --output out/textures/repacked --json
build/tools/rust/bof3-textures/release/bof3-textures verify out/textures/repacked --package out/textures/dragon-reopened --json
```

The reopened package contains the edited words, index planes, cells, and animation
records supported by the source format. Packing it without further edits
reproduces the supplied packed archives exactly. Verification still exits 1 while
full coverage or runtime acceptance is incomplete. Check `byte_checks_passed`
separately from `complete`.

After re-export, you can move the former input tree, edit package, and packed
output. The new package retains the original snapshots and the baseline bytes
needed for further edits. Keep its snapshots and manifest unchanged. Edit its
supported views using the same allocation and reference rules as an original
package, then pack into another new destination.

## Original identity and current baseline

Package schema `bof3.textures-package/v6` separates `snapshots` from `baselines`.
Both lists follow the catalog's original source order and identify snapshot
members by SHA-256. In an original export, both refer to the same bytes. A reopened
package retains each qualified original snapshot and adds the reconstructed
baseline when its bytes differ.

The catalog and logical record identities describe the qualified originals.
Editable exports and read-only previews describe the package baseline. Previews
stay unchanged while you edit source views. Reopening the next packed output
refreshes those previews from the resulting bytes.

Loading validates each baseline against the original resource encoders and the
EMI owner's fixed-allocation replacement rules. Changes to opaque fields,
unsupported resources, archive padding, or unrelated entries are refused.
Changing a baseline hash in the manifest does not authorize those changes.
Compressed edits retain the qualified original allocation across re-exports.

Pack and verification reports continue to compare the resulting bytes with the
qualified originals. Baseline edits remain in the reported changed resources
even when you make no further edits. Re-export neither qualifies a different
release nor establishes palette loading, rendering, or complete graphics support.

Older package schemas and stale interpretation hashes are refused. Export again
from the qualified originals with the current executable before applying edits.
