"""Native exact-match payloads for type and macro transaction integrations."""

import hashlib
from pathlib import Path

from harness.domain.manifests import load_target_manifests
from harness.match._asm_diff_payload import build_result_payload

_NATIVE_BUILD_DIGESTS: dict[Path, str] = {}


def _build_inputs_digest(root: Path) -> str:
    """Digest every input a real cmake/ninja build depends on for one test root."""
    digest = hashlib.sha256()
    manifest = root / "CMakeLists.txt"
    if manifest.is_file():
        digest.update(b"CMakeLists.txt\0")
        digest.update(manifest.read_bytes())
    for directory in ("src", "config", "include"):
        base = root / directory
        if not base.is_dir():
            continue
        for path in sorted(item for item in base.rglob("*") if item.is_file()):
            digest.update(path.relative_to(root).as_posix().encode())
            digest.update(b"\0")
            digest.update(path.read_bytes())
            digest.update(b"\0")
    return digest.hexdigest()


def exact_payload(root: Path, tool: str) -> dict:
    manifest = load_target_manifests(root)["exe/test"]
    payload = build_result_payload(
        source_path=root / "src/test/func_80100000.c",
        function_name="func_80100000",
        address=0x80100000,
        original_size=8,
        current_size=8,
        byte_match=True,
        binary_path=root / manifest.binary,
        object_path=root / "build/test.o",
        output_dir=root / "out/diff",
        original_lines=["nop", "nop"],
        current_lines=["nop", "nop"],
    )
    if tool == "byte-match":
        payload["schema"] = "harness.byte-match-one/v1"
        payload["outputs"] = {}
        del payload["instruction_count"], payload["first_mismatch"]
    return payload


def execution_inputs(root: Path) -> None:
    """Supply the real closure resolver's native configuration in synthetic owners."""
    import json

    manifest = load_target_manifests(root)["exe/test"]
    splat = root / manifest.splat
    splat.write_text(
        f"options:\n  target_path: {manifest.binary}\n"
        "  symbol_addrs_path: config/targets/exe/test/symbols.txt\n" + splat.read_text()
    )
    for name in (
        "bin",
        "toolchains/gcc-2.7.2-psx",
        "toolchains/psn00b_toolchain/bin",
        "config/compiler",
    ):
        (root / name).mkdir(parents=True, exist_ok=True)
    (root / "config/compiler/variants.json").write_text(json.dumps({"candidates": []}))
    (root / "config/compiler/object-flags.cmake").write_text("")
    (root / "CMakeLists.txt").write_text(
        "cmake_minimum_required(VERSION 3.20)\n"
        "project(context_fixture NONE)\n"
        'file(GLOB sources CONFIGURE_DEPENDS "src/*.c")\n'
    )
    import sys

    (root / ".venv").mkdir()
    (root / ".venv/.gitignore").write_text("*\n")
    (root / ".venv/bin").mkdir()
    (root / ".venv/bin/python").symlink_to(sys.executable)
    launcher = root / ".venv/bin/splat"
    launcher.write_text(f"#!{root}/.venv/bin/python\nprint('splat fixture')\n")
    launcher.chmod(0o755)
    (root / ".venv/pyvenv.cfg").write_text("include-system-site-packages = false\n")
    (root / ".venv/lib/site-packages").mkdir(parents=True)
    (root / ".venv/lib/site-packages/runtime.pth").write_text("# fixture\n")


def native_build(root: Path) -> None:
    """Exercise real CMake regeneration/Ninja glob scripts without BOF3 mutation.

    Repeated identical ``bin/harness build`` checks for one unchanged test root reuse the
    objects the first real build produced; any source/config change re-runs it.
    """
    import subprocess

    key = root.resolve()
    digest = _build_inputs_digest(root)
    if _NATIVE_BUILD_DIGESTS.get(key) == digest:
        return
    for argv in (
        ["cmake", "-S", str(root), "-B", str(root / "build/cmake"), "-G", "Ninja"],
        ["ninja", "-C", str(root / "build/cmake")],
    ):
        subprocess.run(argv, check=True, capture_output=True, timeout=30)
    _NATIVE_BUILD_DIGESTS[key] = digest
