# Build the sibling RetDec forks

Build the customized plugin through the existing harness lifecycle.
The [rz-psxdec fork](../../third_party/rz-psxdec/README.md) supplies the Rizin
plugin; [retdec-psx](../../third_party/retdec-psx/README.md) supplies its backend.
The [preparation guide](preparation.md) defines the pinned corpus and input
boundary; the [reconstruction guide](reconstruction.md) describes original-only
runs and their evidence limits.

```sh
bin/harness setup --component rz-psxdec --force
bin/harness doctor --component rz-psxdec
```

Setup prints the actual DSO and receipt paths after verification. Every fresh
attempt has its own directory under `build/third_party/rz-psxdec/attempt-*`.
The DSO is `build/src/rz-plugin/rz-psxdec.so` inside that attempt. The receipt
is `native-build.json` beside the attempt's `build` directory. Failed attempts
retain their command logs and publish no successful receipt.

Without `--force`, setup can reuse a complete receipt only after rehashing its
inputs. Doctor verifies a matching receipt and explicitly loads its DSO with
managed Rizin. It does not rebuild.

## Supply the existing prerequisites

The component uses the existing managed Rizin prefix at `toolchains/rizin` and
the system C and C++ compilers, CMake, Make, Git, Autotools, and linker tools.
It requires their current files to exist. It does not install packages or copy
the DSO into an installed plugin directory.

The three cached archives remain pinned by SHA-256. Their default paths are:

| Archive | Default cache path |
| --- | --- |
| LLVM | `toolchains/downloads/retdec-psx/llvm.zip` |
| Capstone | `toolchains/downloads/retdec-psx/capstone.zip` |
| YARA | `toolchains/downloads/retdec-psx/yara.zip` |

To select another copy of the same archive, set `LLVM_ARCHIVE`,
`CAPSTONE_ARCHIVE`, or `YARA_ARCHIVE`. The digest must match the pin in
[nativebuild.py](../python/harness/toolchain/nativebuild.py). Each attempt
copies and verifies the archives, then extracts disposable prerequisite trees.
YARA's patch and generated files remain in its disposable tree. The receipt
records its complete before and after inventories and every observed mutation.
Owned fork sources must remain unchanged.

## Inspect the build evidence

The plugin CMake dependency owner selects sibling `third_party/retdec-psx`
through `RETDEC_SOURCE_DIR`. It rejects a missing sibling. There is no nested
RetDec source, FetchContent fallback, or wrapper patch application.
The plugin refuses `BUILD_BUNDLED_RETDEC=OFF`, so branded backend metadata
always identifies the sibling sources used by this build.

Setup configures the plugin from `third_party/rz-psxdec`, uses
`BUILD_BUNDLED_RETDEC=ON`, and builds only the `rz-psxdec` target with
`cmake --build` and `--parallel 24`. The configure and build argv, working
directory, raw exit, failure, and output identities remain in the attempt.
The component sets `CMAKE_INSTALL_PREFIX` to the attempt's `prefix` directory.
RetDec's configure-time support cleanup stays in that disposable directory.
The component does not run a top-level install or mutate installed support paths.

The receipt binds both actual fork commits and complete source inventories.
The configure flags `RZ_PSXDEC_REVISION` and `RETDEC_PSX_REVISION` must equal
their source HEADs. The generated plugin version is
`rz-psxdec@<plugin-40-SHA>+retdec-psx@<backend-40-SHA>`; `Lcj` reports it.
It also binds the prerequisite archives, prepared trees, compiler and build
tools, configured external libraries, link inputs, generated build files, and
DSO. Declared Rizin inputs have before and after observations. Additional
consumed compiler headers come from actual CMake and YARA depfiles after
compilation. Their receipt field states that observation point.

The reader re-derives the compiler and link inventories and rehashes the
recorded files. A changed fork commit, source file, dependency, tool, build
input, log, or DSO invalidates reuse. A successful build receipt establishes
native build provenance. Reconstruction and semantic acceptance require their
own execution evidence.

## Use the plugin interactively

Set `dso` to the verified path printed by setup. Extract a PS-X EXE payload or
prove an overlay base before setting `payload` and `load_address`:

```sh
RZ_NOPLUGINS=1 toolchains/rizin/bin/rizin \
  -N -n -a mips -b 32 -E little -m "$load_address" \
  -l "$dso" "$payload"
```

Inspect `pdz?` and `pdza?`. `pdz` decompiles the current Rizin function, `pdzo`
shows offsets, and `pdz*` prints commands that add decompilation comments.
These commands use supplied Rizin context. Original-only generation uses
`pdzar /absolute/workspace` with the validated fixed inputs described below.

## Run original-only reconstruction

Use the receipt path printed by setup as `native_receipt`. Supply the prepared
run, fixed native configuration, frozen member key, and a fresh destination.

```sh
bin/harness reconstruction run --prepared-run "$prepared" \
  --destination "$fresh_run" --member "$member" \
  --configuration "$configuration" --native-receipt "$native_receipt"
```

Reconstruction validates the receipt, snapshots both forks and the native build
inputs, and loads the selected DSO with `RZ_NOPLUGINS=1`. It rechecks the
snapshots before native execution. Its generic compilation remains owned by
[build/recipe.py](../python/harness/build/recipe.py).

Keep original-only inputs separate from fresh supplied-context captures. Authored
references belong to the independent evaluator. A build or assisted comparison
grants no original-only reconstruction credit.

## Prepare with the current fork receipt

Preparation requires the same verified native receipt as generation.

```sh
bin/harness reconstruction prepare --run "$new_run" --baseline tools/retdec-psx/corpus.json \
  --native-receipt "$native_receipt"
```

The maintained [corpus manifest](corpus.json) fixes all 118 admitted function
identities and original image/range hashes, seven exclusions and evaluator-only
reference metadata. Preparation checks that manifest and the current fork receipt.
It does not read historical audit bundles, collectors, patch records or old scores.
Missing or changed current inputs refuse preparation.
