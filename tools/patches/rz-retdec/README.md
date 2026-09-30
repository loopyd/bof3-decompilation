# PSX RetDec customization

Status: accepted for isolated delivery with documented limitations. The single
`psx-toolchain.patch` passed clean application/build, bounded clean-plugin replay,
source-only reversal, and independent final packaging review.
See `acceptance.md` for evidence and limits. No production integration is claimed.

## Delivered files

- `psx-toolchain.patch`, the only customization patch.
- `manifest.json`, with both base revisions, the pinned base and prerequisite
  archive identities, the patch checksum, all 39 required base and content
  identities, and the accepted build and plugin identities.
- `README.md`, this file.
- `acceptance.md`, with per-corpus results, exclusions, and limitations.
- `division-guards.md`, with the case-053 guard proof and its assumptions.

## Source layout and supported base inputs

Use rz-retdec revision `4ac6b293553d7f5f00574e4dca4c21b799db63e1` with RetDec
revision `8272d0355794f8b8a63d8611b1dea50b4f2d87c3` at `deps/retdec/retdec/`.
The patch root is the rz-retdec root. Plugin paths stay `src/rz-plugin/...` and
every RetDec path is prefixed with `deps/retdec/retdec/`.

The supported base inputs are two non-Git tar archives with the pinned byte
identities recorded in `manifest.json` under `base_archives`.

| Component | Revision | Archive root | Files |
| --- | --- | --- | --- |
| plugin | `4ac6b293553d7f5f00574e4dca4c21b799db63e1` | `.` | 43 |
| retdec | `8272d0355794f8b8a63d8611b1dea50b4f2d87c3` | `deps/retdec/retdec` | 3851 |

The archives are `git archive` snapshots, so the preflight pins them by SHA-256
and by every required file's content instead of running a Git revision query on a
directory that is not a Git repository. The revisions are recorded as provenance.
An input that does not match is rejected and the differing path is named.

Development copies exist at `tmp/retdec-psx-preservation/{plugin,retdec}/base.tar`
with the extracted source at `tmp/retdec-psx-source/rz-retdec`. They are local
inputs. They are not delivered and do not replace the pinned archives.

## Prerequisites

The build uses the existing system GCC 13 compiler and CMake 3.28, and the three
retained dependency source archives recorded in `manifest.json` under
`prerequisites`.

| Prerequisite | Archive SHA-256 | Extracted source directory |
| --- | --- | --- |
| LLVM | `b5879b30768135e5fce84ccd8be356d2c55c940ab32ceb22d278b228e88c4c60` | `llvm-a776c2a976ef64d9cd84d7ee71d0e4a04aa117a1` |
| Capstone | `c47acdabb9ba4922a6d68b96eb7e14a431bfef7d7c57cea1e5881f87776228b2` | `capstone-5.0-rc2` |
| YARA | `ae1adad2ae33106f4c296cef32ddba2c93867010ef853028d30cad42548d0474` | `yara-4.2.0-rc1` |

The archives are validated before they are extracted, and the extracted trees are
compared against the archives before the build. Do not reuse an existing extracted
tree, which may contain build artifacts, and do not download or install a
dependency.

## Workflow

Run the steps in order from the repository root. Step 1 exports the pinned inputs
and defines `preflight()`. Every step that depends on a check invokes `preflight`
explicitly and exits on failure, so a rejected input stops the sequence instead of
continuing. The preflight writes nothing.

### Step 1. Select the pinned inputs and define the preflight

```sh
export LLVM_ARCHIVE="${LLVM_ARCHIVE:-$PWD/out/retdec-evaluation/build/_deps/retdec-build/external/src/llvm.zip}"
export CAPSTONE_ARCHIVE="${CAPSTONE_ARCHIVE:-$PWD/out/retdec-evaluation/build/_deps/retdec-build/external/src/capstone.zip}"
export YARA_ARCHIVE="${YARA_ARCHIVE:-$PWD/out/retdec-evaluation/build/_deps/retdec-build/deps/yara/yara/src/yara.zip}"
export PLUGIN_BASE_ARCHIVE="${PLUGIN_BASE_ARCHIVE:-$PWD/tmp/retdec-psx-preservation/plugin/base.tar}"
export RETDEC_BASE_ARCHIVE="${RETDEC_BASE_ARCHIVE:-$PWD/tmp/retdec-psx-preservation/retdec/base.tar}"
preflight() {
  ROOT="$PWD" SOURCE_DIR="${1:-}" PREREQUISITE_ROOT="${2:-}" python3 - <<'PREFLIGHT' || return 1
"""Reject incompatible inputs before any source mutation. Reads only."""
import hashlib
import json
import os
import stat
import sys
import tarfile
import zipfile
from pathlib import Path, PurePosixPath

root = Path(os.environ.get("ROOT", ".")).resolve()
manifest = json.loads((root / "tools/patches/rz-retdec/manifest.json").read_text())
failures = []


def fail(message):
    failures.append(message)
    print("FAIL " + message)


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def sha256_file(path):
    digest = hashlib.sha256()
    with open(path, "rb") as stream:
        for block in iter(lambda: stream.read(1 << 20), b""):
            digest.update(block)
    return digest.hexdigest()


def located(record, variable):
    value = os.environ.get(variable) or record["archive"]
    path = Path(value)
    return path if path.is_absolute() else root / path


def kind_of(mode):
    if stat.S_ISDIR(mode):
        return "directory"
    if stat.S_ISREG(mode):
        return "file"
    if stat.S_ISLNK(mode):
        return "symlink"
    return "special"


def implied_dirs(files):
    parents = set()
    for name in files:
        parts = PurePosixPath(name).parts[:-1]
        for index in range(1, len(parts) + 1):
            parents.add("/".join(parts[:index]))
    return parents


def tar_entries(path, prefix):
    files, dirs = {}, set()
    with tarfile.open(path) as archive:
        for member in archive.getmembers():
            relative = PurePosixPath(member.name)
            if not relative.parts:
                continue
            if relative.is_absolute() or ".." in relative.parts:
                fail("unsafe archive member " + member.name)
            elif member.isdir():
                dirs.add(prefix + relative.as_posix())
            elif member.isfile():
                files[prefix + relative.as_posix()] = sha256(archive.extractfile(member).read())
            else:
                fail("%s: unsupported archive entry %s" % (path, member.name))
    return files, dirs


def zip_entries(path):
    files, dirs, modes = {}, set(), {}
    with zipfile.ZipFile(path) as archive:
        names = archive.namelist()
        top = names[0].split("/")[0] + "/" if names else ""
        for info in archive.infolist():
            if not info.filename.startswith(top):
                fail("%s: unexpected zip root %s" % (path, info.filename))
                continue
            relative = PurePosixPath(info.filename[len(top):])
            if not relative.parts:
                continue
            name = relative.as_posix()
            mode = info.external_attr >> 16
            if relative.is_absolute() or ".." in relative.parts:
                fail("%s: unsafe zip member %s" % (path, info.filename))
            elif stat.S_ISLNK(mode):
                fail("%s: unsupported symlink entry %s" % (path, info.filename))
            elif info.is_dir() or info.filename.endswith("/"):
                dirs.add(name)
            elif stat.S_ISREG(mode) or mode == 0:
                files[name] = sha256(archive.read(info))
                modes[name] = "100755" if mode & 0o111 else "100644"
            else:
                fail("%s: unsupported entry type for %s" % (path, info.filename))
    return files, dirs, modes


def scan_tree(path):
    files, dirs, special = {}, set(), []
    mode = path.lstat().st_mode
    if not stat.S_ISDIR(mode):
        special.append((".", kind_of(mode)))
        return files, dirs, special
    pending = [path]
    while pending:
        for entry in os.scandir(pending.pop()):
            item = Path(entry.path)
            relative = item.relative_to(path).as_posix()
            kind = kind_of(item.lstat().st_mode)
            if kind == "directory":
                dirs.add(relative)
                pending.append(item)
            elif kind == "file":
                files[relative] = sha256_file(item)
            else:
                special.append((relative, kind))
    return files, dirs, special


def compare_tree(label, base_files, base_dirs, tree_path):
    files, dirs, special = scan_tree(tree_path)
    for relative, kind in special:
        fail("%s: unsupported %s entry in source: %s" % (label, kind, relative))
    for name in sorted(base_files.keys() - files.keys()):
        fail("%s: missing base file: %s" % (label, name))
    for name in sorted(files.keys() - base_files.keys()):
        fail("%s: unexpected extra file: %s" % (label, name))
    for name in sorted(base_dirs - dirs):
        fail("%s: missing base directory: %s" % (label, name))
    for name in sorted(dirs - base_dirs):
        fail("%s: unexpected extra directory: %s" % (label, name))
    for name in sorted(base_files.keys() & files.keys()):
        if base_files[name] != files[name]:
            fail("%s: changed required base file: %s" % (label, name))
    return files, dirs


patch = root / manifest["patch"]["path"]
if not patch.is_file():
    fail("missing patch " + str(patch))
elif sha256_file(patch) != manifest["patch"]["sha256"]:
    fail("patch checksum differs from " + manifest["patch"]["sha256"])
else:
    print("patch " + manifest["patch"]["sha256"] + " ok")

prerequisite_maps = {}
for name, record in manifest["prerequisites"].items():
    path = located(record, record["env"])
    if not path.is_file():
        fail("%s: missing prerequisite archive %s" % (name, path))
        continue
    if sha256_file(path) != record["archive_sha256"]:
        fail("%s: prerequisite archive checksum differs" % name)
    files, dirs, modes = zip_entries(path)
    prerequisite_maps[name] = (files, dirs, modes)
    executable = sum(1 for mode in modes.values() if mode == "100755")
    print("prerequisite %s %s ok (%d files, %d executable in the archive mode model)"
          % (name, record["archive_sha256"], len(files), executable))

base_files, base_dirs = {}, set()
for component in ("plugin", "retdec"):
    record = manifest["base_archives"][component]
    path = located(record, record["env"])
    if not path.is_file():
        fail("%s: missing base archive %s" % (component, path))
        continue
    if sha256_file(path) != record["sha256"]:
        fail("%s: base archive %s is not the pinned revision %s bytes" % (component, path, record["revision"]))
    files, dirs = tar_entries(path, record["prefix"])
    base_files.update(files)
    base_dirs.update(dirs)
base_dirs |= implied_dirs(base_files)
print("complete base map: %d files, %d directories from the pinned archives"
      % (len(base_files), len(base_dirs)))

source_value = os.environ.get("SOURCE_DIR")
if not source_value:
    print("base source tree not checked: set SOURCE_DIR to the assembled base directory")
else:
    source = Path(source_value)
    if not source.is_absolute():
        source = root / source
    if not source.is_dir():
        fail("missing base source directory " + str(source))
    else:
        files, dirs = compare_tree("base source", base_files, base_dirs, source)
        present = set(files) | dirs
        for record in manifest["package_inputs"]:
            if record["status"] == "added" and record["path"] in present:
                fail("added-path collision: %s already exists in the base source" % record["path"])

prerequisite_root = os.environ.get("PREREQUISITE_ROOT")
if not prerequisite_root:
    print("prerequisite source trees not checked: set PREREQUISITE_ROOT to the fresh extraction root")
else:
    base_root = Path(prerequisite_root)
    if not base_root.is_absolute():
        base_root = root / base_root
    for name, record in manifest["prerequisites"].items():
        if name not in prerequisite_maps:
            continue
        tree = base_root / record["source_directory"]
        if not tree.is_dir():
            fail("%s: missing prerequisite source tree %s" % (name, tree))
            continue
        archive_files, archive_dirs, _ = prerequisite_maps[name]
        files, _ = compare_tree(name + " source", archive_files,
                                archive_dirs | implied_dirs(archive_files), tree)
        print("prerequisite %s source tree matches the pinned archive (%d files)" % (name, len(files)))

print("revisions: " + ", ".join(component + "=" + manifest["base_archives"][component]["revision"]
                               for component in ("plugin", "retdec")))
if failures:
    print("PREFLIGHT FAILED with %d problem(s); no source was modified" % len(failures))
    sys.exit(1)
print("PREFLIGHT OK; no source was modified")
PREFLIGHT
}
```

The exported archive variables are the only values used by both the extraction
commands and the preflight, so validation and extraction cannot select different
files. `preflight` takes the assembled base source directory as its first argument
and the prerequisite extraction root as its second; both are optional, and unset
values skip only those tree comparisons, never the patch or archive checks.

The preflight verifies the patch checksum, the prerequisite archive identities,
the pinned base archive identities, the complete base file and directory map
(every entry, not only the 39 changed files), every required base file's bytes,
and that no added path collides. It rejects symlinks and special entries,
including directory links and a `.git` link at the source root, in the archives
and in the source tree. It writes nothing.

### Step 2. Archive-only preflight (before any extraction)

```sh
preflight || exit 1
```

This runs the patch checksum, the prerequisite archive identities, the entry and
path validation of both archive sets, and the pinned base archive identities. No
destination directory is created before it passes.

### Step 3. Extract the prerequisites into a fresh root

```sh
DEST="$PWD/tmp/retdec-psx-prerequisites"
if [ -e "${DEST}" ] || [ -L "${DEST}" ]; then
  echo "refusing existing ${DEST}" >&2
  exit 1
fi
mkdir "$DEST" || exit 1
unzip -q "$LLVM_ARCHIVE" -d "$DEST" || exit 1
unzip -q "$CAPSTONE_ARCHIVE" -d "$DEST" || exit 1
unzip -q "$YARA_ARCHIVE" -d "$DEST" || exit 1
export PREREQUISITE_ROOT="$DEST"
export LLVM_SOURCE_DIR="$DEST/llvm-a776c2a976ef64d9cd84d7ee71d0e4a04aa117a1"
export CAPSTONE_SOURCE_DIR="$DEST/capstone-5.0-rc2"
export YARA_SOURCE_DIR="$DEST/yara-4.2.0-rc1"
```

The `[ -L ]` test refuses a dangling symlink as well as an existing path, and
every required `mkdir` exits on failure. The extracted trees are the build inputs.

### Step 4. Assemble the pinned base sources

```sh
DEST="$PWD/tmp/retdec-psx-base"
if [ -e "${DEST}" ] || [ -L "${DEST}" ]; then
  echo "refusing existing ${DEST}" >&2
  exit 1
fi
mkdir "$DEST" || exit 1
tar -xf "$PLUGIN_BASE_ARCHIVE" -C "$DEST" || exit 1
mkdir "$DEST/deps/retdec/retdec" || exit 1
tar -xf "$RETDEC_BASE_ARCHIVE" -C "$DEST/deps/retdec/retdec" || exit 1
export SOURCE_ROOT="$DEST"
export SOURCE_DIR="$DEST"
```

This extracts 3,894 files with the archive modes intact and no symlinks.

### Step 5. Full preflight before applying the patch

```sh
preflight "$SOURCE_DIR" "$PREREQUISITE_ROOT" || exit 1
```

This compares the complete assembled base tree and the consumed prerequisite trees
against the pinned archives, and checks the added-path collisions. The patch is not
applied unless this exits 0.

### Step 6. Apply the customization patch

```sh
test -d "$SOURCE_DIR" || { echo "missing $SOURCE_DIR" >&2; exit 1; }
patch -d "$SOURCE_DIR" -p1 --dry-run -i "$PWD/tools/patches/rz-retdec/psx-toolchain.patch" || exit 1
patch -d "$SOURCE_DIR" -p1 -i "$PWD/tools/patches/rz-retdec/psx-toolchain.patch" || exit 1
```

GNU `patch` creates the ten added files and modifies the 29 changed files. The
dry-run stops before the real application, and neither command touches a Git
index. The additions are non-executable; the 18 executable base files keep their
archive content because the patch does not touch them.

Task 6.1 executed both commands on clean archive-bound sources, then verified
all 3,904 patched files. Task 6.3 independently verified the intermediate patched
state before reversal. Task 5.2 also exercised `git apply --check` on disposable
copies.

### Step 7. Build

Choose a fresh build directory so the clean build cannot reuse the accepted
development tree at `out/retdec-psx/build`.

```sh
export CC=/usr/bin/cc
export CXX=/usr/bin/c++
fs_archive="$($CXX -print-file-name=libstdc++fs.a)"
test -f "$fs_archive" || exit 1
fs_directory="$(dirname "$fs_archive")"
export CMAKE_LIBRARY_PATH="$fs_directory${CMAKE_LIBRARY_PATH:+:$CMAKE_LIBRARY_PATH}"
export LIBRARY_PATH="$fs_directory${LIBRARY_PATH:+:$LIBRARY_PATH}"

DEST="$PWD/out/retdec-psx-task6/build"
if [ -e "${DEST}" ] || [ -L "${DEST}" ]; then
  echo "refusing existing ${DEST}" >&2
  exit 1
fi
/usr/bin/cmake \
  -S "$SOURCE_DIR" \
  -B "$DEST" \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_CUTTER_PLUGIN=OFF \
  -DCMAKE_C_COMPILER="$CC" \
  -DCMAKE_CXX_COMPILER="$CXX" \
  -DCMAKE_PREFIX_PATH="$PWD/toolchains/rizin" \
  -DRIZIN_INSTALL_PLUGDIR="$PWD/out/retdec-psx-task6/plugins" \
  -DLLVM_LOCAL_DIR="$LLVM_SOURCE_DIR" \
  -DCAPSTONE_LOCAL_DIR="$CAPSTONE_SOURCE_DIR" \
  -DYARA_LOCAL_DIR="$YARA_SOURCE_DIR" || exit 1
export BUILD_DIR="$DEST"
/usr/bin/cmake --build "$BUILD_DIR" --target core_retdec --parallel 6 || exit 1
```

CMake's `find_library` needs `CMAKE_LIBRARY_PATH`; the compiler's own default
search path does not guarantee that CMake finds the archive. Do not suppress the
missing-library message or install a replacement dependency. The verified
configuration finds `/usr/lib/gcc/x86_64-linux-gnu/13/libstdc++fs.a`, and the
plugin link command includes `-lstdc++fs`. These arguments and environment were
exercised by the verified configuration and build. No installation command is
run. The `core_retdec` target does not build the configured but unused YaraMod
download target, and the plugin is loaded explicitly from
`$BUILD_DIR/src/rz-plugin/core_retdec.so`. Record that plugin's SHA-256 and bind
it before any replay.

### Step 8. Evaluate

Task 6.2 completed the bounded replay with the task-6.1 clean plugin, SHA-256
`c77978fe373ed1d39ae764bfdc141f3c6d4f8dc38fccd0c502a408e0e530b653`,
51,579,472 bytes. All 125 broad and six ordinary assisted emissions equal the
accepted section-4 outputs. The separately qualified counter required the
invocation correction described below. `acceptance.md` records exact native and
runtime results, failed attempts, and remaining limitations.

The frozen evaluation inputs are local and are not delivered. They are:

- `out/retdec-evaluation/remainders-02/final/evaluation.json`
  (SHA-256 `21ef7b0e1cf10249623e33891311c084052d671e8bb71bfdc9ab9415a96ed8ed`) and
  `out/retdec-evaluation/symbolic-final/results.json`
  (SHA-256 `3eaf0c1f7e454bec5052002bd7bc8ecd0017ec13308ed164b3354b1f70930c68`),
  which carry the frozen broad and metadata-assisted generation commands.
- The accepted emitted-source references under `out/retdec-psx/task-4-1/replay-02/`
  and `out/retdec-psx/provenance-replay/`.
- The retained drivers under `tmp/`, including `tmp/retdec-psx-task-4-1/task-4-1-replay.py`
  (SHA-256 `8577be24230a71aab8a6fede6a744c6937bf050147de6fb98177eca59abacdfd`),
  `task-4-1-compile.py`, `task-4-1-link.py`, `task-4-1-compare.py`,
  `task-4-1-bof3-runtime.py`, `tmp/retdec-psx-task-4-4/task-4-4-run.py`,
  `tmp/retdec-psx-task-4-5/synthetic-fixtures.py`, and
  `tmp/retdec-psx-task-4-5/clean-c-scan.py`.

These are workspace inputs. The delivery does not distribute the corpus, the game
bytes, or the evaluation drivers, and no private game bytes appear here.

Generation replay for the clean build uses the local wrapper
`tmp/retdec-psx-task-5-3/replay-clean-build.py`
(SHA-256 `1102b952b29c0e077065537c0eb42dfc3ec1ca29da7132ef4d07c22f8a13002f`). The
wrapper binds the retained driver by hash and substitutes its repository-root
line, its frozen plugin path, and its frozen output root with exact-count
replacements, so the relocated copy resolves the workspace root correctly. It
writes the parameterized copy under a fresh output root and returns the driver's
real exit code. It never edits the retained driver, the fixtures, the assertions,
or the expected sets.

```sh
python3 tmp/retdec-psx-task-5-3/replay-clean-build.py \
  --plugin "$BUILD_DIR/src/rz-plugin/core_retdec.so" \
  --output-root "$PWD/out/retdec-psx-task6/replay" \
  --label task6
```

The retained driver compares emitted C against the task-3.1 operational baseline
and against the original evaluation. For the final accepted rules the expected
result is `changed vs task 3.1 baseline (1): ['accumulateWorkCounter']` and
`changed vs original evaluation (2): [('broad', 'case-053'), ('assisted',
'accumulateWorkCounter')]`, with exit 1. That exit is the driver reporting the two
accepted per-function changes, not a harness failure, and it must be distinguished
from a Python traceback or another early failure. Task 6 must check that the
changed set is exactly that set, that every other emitted source is byte-identical,
and that the changed emissions match the accepted section-4 identities. The
completed comparison is `out/retdec-psx/task-6-2/section4-compare-62/comparisons.json`.
Do not
require exit 0 from the baseline comparison and do not relabel the accepted changes
as passing.

Compilation, native comparison, and runtime replay reuse the remaining retained
drivers. They carry frozen per-case inputs and, in
`tmp/retdec-psx-task-4-4/task-4-4-verify.py`, an accepted-plugin predicate
`0239663f8ce9af45d2ffe8e62bad493ca7b457c77fbc1ccee2acdc84e9f53c7d`. A clean build
may not reproduce that byte identity. Task 6.2 re-established those predicates
and input paths through disposable exact-count substitutions without editing the
retained drivers. Cases
016 and 041 keep their historical score-only decreases, and no broad
function-specific runtime acceptance is claimed.

#### Retained clean-replay commands

The following are the executed task-6.2 command forms, not a one-command portable
suite. `tmp/retdec-psx-task-6-2/run.py` binds each retained driver by hash and records
its exact substitutions and arguments under `out/retdec-psx/task-6-2/steps/`.
Successful destinations already exist. Do not rerun into them. A new evaluation
needs fresh destinations and exact-count path substitutions, while preserving
inputs, profiles, fixtures, and expected outcomes.

| Domain | Executed local command | Successful evidence directory under `out/retdec-psx/task-6-2/` |
| --- | --- | --- |
| Broad native | `python3 tmp/retdec-psx-task-6-2/run.py broad-verify` | `verify-62` |
| Ordinary assisted | `python3 tmp/retdec-psx-task-6-2/run.py assisted-compare` | `compare-62` |
| Qualified native | `python3 tmp/retdec-psx-task-6-2/run.py qualified-compare` | `compare-62q` |
| Synthetic | `python3 tmp/retdec-psx-task-6-2/run.py synthetic` | `synthetic-62` |
| Division and ABI | `python3 tmp/retdec-psx-task-6-2/run.py runtime-3-2` | `task-3-2-runtime/runtime-62` |
| Shifts | `python3 tmp/retdec-psx-task-6-2/run.py shift-candidates`, then `python3 tmp/retdec-psx-task-6-2/run.py shift-runtime` | See corresponding `steps/` records and captures |
| Merge memory | `python3 tmp/retdec-psx-task-6-2/run.py memory-probes unaligned` | `unaligned-62b` |
| Load delay | `python3 tmp/retdec-psx-task-6-2/run.py memory-probes delayslot` | `delayslot-62b` |
| Narrowed UDIV | `python3 tmp/retdec-psx-task-6-2/run.py native-divu` | `native-divu-62` |
| BOF3 ordinary | `python3 tmp/retdec-psx-task-6-2/run.py bof3-ordinary` | `bof3-runtime-62.json` |
| BOF3 qualified | `python3 tmp/retdec-psx-task-6-2/run.py bof3-qualified` | `bof3-runtime-62q.json` |

The original `run.py qualified-regen` command is unsafe as standalone acceptance.
It reports the clean plugin hash while loading the development plugin embedded
in its copied command. The corrected invocation is recorded in
`qualified-parent-01/process.json` by
`python3 tmp/retdec-psx-task-6-2/parent-qualified-generation.py`.
The saved process exited 0. Its first post-generation audit failed on a missing
historical field; the corrected `--verify-existing` invocation passed 17 checks.
The actual clean-plugin stdout is byte-identical to the qualified native and
runtime inputs. This identity bridge accepts those consumers without claiming
that the old adapter used the clean plugin.

The direct receipt audits are
`python3 tmp/retdec-psx-task-6-2/parent-direct-audits.py`,
`python3 tmp/retdec-psx-task-6-2/parent-replay-audit.py`, and
`python3 tmp/retdec-psx-task-6-2/parent-final-bindings.py`.
They bind actual exits, executable and capture hashes, complete compiler profiles,
source bodies, and the interrupted-run inventory. Their successful output roots
must also remain intact. See `acceptance.md` for the exact counts and limits.

### Step 9. Verify reversal on a disposable source copy

Keep the accepted source, build, and prerequisite trees intact. Copy the patched
source into a fresh destination, then reverse only that copy. Do not copy a build
directory into the source tree.

```sh
REVERSE_DIR="$PWD/tmp/retdec-psx-reverse"
if [ -e "$REVERSE_DIR" ] || [ -L "$REVERSE_DIR" ]; then
  echo "refusing existing $REVERSE_DIR" >&2
  exit 1
fi
test -d "$SOURCE_DIR" && test ! -L "$SOURCE_DIR" || exit 1
preflight || exit 1
cp -a "$SOURCE_DIR" "$REVERSE_DIR" || exit 1
patch -d "$REVERSE_DIR" -p1 -R --dry-run --no-backup-if-mismatch -i "$PWD/tools/patches/rz-retdec/psx-toolchain.patch" || exit 1
patch -d "$REVERSE_DIR" -p1 -R --no-backup-if-mismatch -i "$PWD/tools/patches/rz-retdec/psx-toolchain.patch" || exit 1
preflight "$REVERSE_DIR" "" || exit 1
```

The final `PREFLIGHT OK` verifies the complete pinned source map and absence of
all ten additions. The empty second argument deliberately skips prerequisite-tree
comparison, but still verifies all archive and patch identities. YARA builds
in its source directory, so its post-build tree is not a pristine archive copy.
Source reversal neither restores nor validates those build-generated changes.
Do not clean the accepted prerequisite tree to force this check to pass.

Task 6.3 verified 3,894 restored files and 567 directories against both the pinned
archives and the pre-application inventory, using fresh unbuilt prerequisites.
Task 6.4 executed this corrected recipe against a fresh copy of the accepted
task-6.1 source, with only `REVERSE_DIR` relocated. It passed complete base
verification, and the accepted source inventory remained identical. Evidence is
under `out/retdec-psx/task-6-4/verify-01/`.

## Preflight behavior

On correct pinned inputs the preflight prints the verified identities and exits 0.
On an incompatible input it prints one `FAIL` line per problem and exits 1 without
mutating any source, and the enclosing step exits because every invocation is
guarded. A source copy whose `src/rz-plugin/data.cpp` bytes differ reports
`changed required base file: src/rz-plugin/data.cpp`. A file or an empty directory
at an added path reports `added-path collision: ...`, and any directory link,
including a `.git` link at the source root, reports `unsupported symlink entry in
source: ...`. A wrong base archive reports `plugin: base archive ... is not the
pinned revision ... bytes`.

## Mode representation

The pinned base and prerequisite archives record executable bits. In the archive
model the two base archives contain 18 executable files, and the prerequisite
archives contain 325 (LLVM), 84 (Capstone), and 7 (YARA) executable files. The
preflight reports those counts, and `manifest.json` records the Git mode of every
changed or added file as `package_inputs[].git_mode` (all `100644`). These archive
and Git modes are the authoritative executable-bit model.

The workspace is on a `fuseblk` NTFS mount where `chmod` is a no-op, so a file
written there keeps mode `0770` regardless of the requested mode. On-disk Unix
mode bits are therefore not meaningful on this workspace, and the preflight
compares entry membership and content hashes rather than on-disk modes. Do not
normalize the accepted tree or rewrite modes, and do not claim an on-disk
Unix-mode identity for the applied copy.

## Limitations

- The optional stateful frontend includes `stateful.h`, `stateful.cpp`,
  `stateful_main.cpp`, and their CMake wiring. `stateful.cpp` is compiled into the
  static library; only the separate `retdec-mips-stateful` executable is excluded
  from the default build (`EXCLUDE_FROM_ALL`). The files are preserved prior work,
  are not required for ordinary-C output, and are not accepted
  exception-resumption support. Full guest exception resumption is out of scope.
- Historical per-file pin coverage is 38 of 39. `value_protect.cpp` has no
  established historical digest; only its current hash
  `f58d85b03034170dedd13873575761bd29b6a4d1f6996d1fd4fd1003681d4435` is bound.
- Seven non-exact cases retain unsatisfied `.rodata` placements: `026, 031, 034,
  037, 046, 051, 062`. Cases 016 and 041 retain unresolved score-only decreases
  (14.08 to 13.24 and 31.08 to 22.17 percent) and add no semantic evidence.
- Case 053 is not byte-exact (55 of 295 instructions) and full function-specific
  runtime equivalence is not claimed.
- Clean build, bounded replay, and source-only reversal passed tasks 6.1 through
  6.3. This does not establish universal matching, full case-053 runtime equivalence,
  or blanket supplied-type preservation. Task 6.4 accepts packaging within those
  limits, not broader semantic support.
