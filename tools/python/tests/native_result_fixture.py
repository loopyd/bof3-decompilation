"""Native comparison payloads for existing validation assertions."""

from pathlib import Path

from harness.domain.manifests import load_target_manifests
from harness.match._asm_diff_payload import build_result_payload


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
