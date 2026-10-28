from __future__ import annotations


import json


from pathlib import Path


import pytest


from harness.common.digests import digest


from native_result_fixture import exact_payload


TARGET = "exe/test"


def _partial_common() -> dict[str, object]:
    return {
        "status": "different",
        "exact_match": False,
        "byte_match": False,
        "source": "src/test/func_80100000.c",
        "function": "func_80100000",
        "address": "0x80100000",
        "original_size": 8,
        "current_size": 8,
        "size_delta": 0,
        "original_binary": "out/test.bin",
        "current_object": "build/test.o",
        "outputs": {},
    }


def test_partial_baseline_rejects_empty_missing_swapped_and_unselected_changes(
    tmp_path: Path,
) -> None:
    from harness.match.validation import _baseline_facts, _check, check_evidence

    selector = f"{TARGET}@0x80100000"
    check = _check(
        "byte-match",
        TARGET,
        selector,
        "func_80100000",
        "src/test/func_80100000.c",
    )
    payload = {"schema": "harness.byte-match-one/v1", **_partial_common()}
    raw = json.dumps(payload, sort_keys=True)
    facts = _baseline_facts(check, 1, raw, tmp_path)
    check["partial_baseline"] = {**facts, "digest": digest(facts)}
    assert check_evidence(check, 1, raw, tmp_path)["passed"]
    for changed in (
        {},
        {**payload, "function": "other"},
        {**payload, "outputs": {"unselected": "two"}},
    ):
        assert not check_evidence(
            check, 1, json.dumps(changed, sort_keys=True), tmp_path
        )["passed"]


@pytest.mark.parametrize("tool", ["asm-diff", "byte-match"])
def test_partial_evidence_rejects_every_malformed_field(
    tool: str, tmp_path: Path
) -> None:
    import copy

    from harness.match.validation import _baseline_facts, _check

    selector = f"{TARGET}@0x80100000"
    check = _check(
        tool,
        TARGET,
        selector,
        "func_80100000",
        "src/test/func_80100000.c",
    )
    payload = {"schema": "harness.byte-match-one/v1", **_partial_common()}
    if tool == "asm-diff":
        payload.update(
            schema="harness.asm-diff-one/v2",
            outputs={
                "directory": "out/diff",
                "summary": "out/diff/summary.json",
                "diff": "out/diff/diff.patch",
                "original": "out/diff/original.s",
                "current": "out/diff/current.s",
                "compiler": "out/diff/compiler.s",
                "original_bytes": "out/diff/original.bin",
                "build_log": "out/diff/build.log",
            },
            instruction_count={
                "original": 2,
                "current": 2,
                "matching": 1,
                "match_percent": 50.0,
            },
            first_mismatch={
                "original_index": 1,
                "current_index": 1,
                "original_offset": 4,
                "current_offset": 4,
                "original": "old",
                "current": "new",
            },
        )
    raw = json.dumps(payload, sort_keys=True)
    assert _baseline_facts(check, 1, raw, tmp_path)["function"] == "func_80100000"

    malformed = []
    for key in payload:
        missing = copy.deepcopy(payload)
        missing.pop(key)
        malformed.append(missing)
        wrong = copy.deepcopy(payload)
        wrong[key] = None
        malformed.append(wrong)
    malformed += [
        {**payload, "unexpected": True},
        {**payload, "status": "exact_match"},
        {**payload, "exact_match": True},
        {**payload, "byte_match": True},
        {**payload, "function": "other"},
        {**payload, "source": "src/test/other.c"},
        {**payload, "address": "0x80100004"},
        {**payload, "original_size": True},
        {**payload, "current_size": -1},
        {**payload, "size_delta": 1},
    ]
    if tool == "asm-diff":
        malformed += [
            {
                **payload,
                "instruction_count": {**payload["instruction_count"], "matching": 3},
            },
            {
                **payload,
                "instruction_count": {
                    **payload["instruction_count"],
                    "match_percent": 49.0,
                },
            },
            {
                **payload,
                "first_mismatch": {**payload["first_mismatch"], "original_index": 2},
            },
            {
                **payload,
                "first_mismatch": {**payload["first_mismatch"], "current_offset": 8},
            },
            {
                **payload,
                "first_mismatch": {**payload["first_mismatch"], "current": None},
            },
        ]
    for value in malformed:
        with pytest.raises(ValueError, match="partial"):
            _baseline_facts(check, 1, json.dumps(value, sort_keys=True), tmp_path)
    for exit_code in (0, 2, True, "1"):
        with pytest.raises(ValueError, match="exit 1"):
            _baseline_facts(check, exit_code, raw, tmp_path)


_REAL_ASM_SCENA16 = {
    "schema": "harness.asm-diff-one/v2",
    "status": "different",
    "exact_match": False,
    "byte_match": False,
    "source": "{root}/src/bof3/scenario/clampPaletteChannels.c",
    "function": "clampPaletteChannels",
    "address": "0x801f83b0",
    "original_size": 172,
    "current_size": 172,
    "size_delta": 0,
    "original_binary": "{root}/out/binaries/emi/scenario/scena16/00.bin",
    "current_object": "{root}/build/src/bof3/scenario/clampPaletteChannels.o",
    "instruction_count": {
        "original": 43,
        "current": 43,
        "matching": 21,
        "match_percent": 48.84,
    },
    "first_mismatch": {
        "original_index": None,
        "current_index": 5,
        "original_offset": None,
        "current_offset": 20,
        "original": None,
        "current": "lui t6,0x8003",
    },
    "outputs": {
        "directory": "{root}/out/asm-diff/emi_scenario_scena16_00/clampPaletteChannels",
        "summary": "{root}/out/asm-diff/emi_scenario_scena16_00/clampPaletteChannels/summary.json",
        "diff": "{root}/out/asm-diff/emi_scenario_scena16_00/clampPaletteChannels/diff.patch",
        "original": "{root}/out/asm-diff/emi_scenario_scena16_00/clampPaletteChannels/original.s",
        "current": "{root}/out/asm-diff/emi_scenario_scena16_00/clampPaletteChannels/current.s",
        "compiler": "{root}/out/asm-diff/emi_scenario_scena16_00/clampPaletteChannels/compiler.s",
        "original_bytes": "{root}/out/asm-diff/emi_scenario_scena16_00/clampPaletteChannels/original.bin",
        "build_log": "{root}/out/asm-diff/emi_scenario_scena16_00/clampPaletteChannels/build.log",
        "linked": "{root}/out/asm-diff/emi_scenario_scena16_00/clampPaletteChannels/linked.s",
    },
}


_REAL_BYTE_SCENA16 = {
    "schema": "harness.byte-match-one/v1",
    "status": "different",
    "exact_match": False,
    "byte_match": False,
    "source": "{root}/src/bof3/scenario/clampPaletteChannels.c",
    "function": "clampPaletteChannels",
    "address": "0x801f83b0",
    "original_size": 172,
    "current_size": 172,
    "size_delta": 0,
    "original_binary": "{root}/out/binaries/emi/scenario/scena16/00.bin",
    "current_object": "{root}/build/src/bof3/scenario/clampPaletteChannels.o",
    "outputs": {},
}


_REAL_ASM_AREA008 = {
    "schema": "harness.asm-diff-one/v2",
    "status": "different",
    "exact_match": False,
    "byte_match": False,
    "source": "{root}/src/bof3/world/drawTexturedFrame.c",
    "function": "drawTexturedFrame",
    "address": "0x801f3d88",
    "original_size": 1356,
    "current_size": 1356,
    "size_delta": 0,
    "original_binary": "{root}/out/binaries/emi/world00/area008/13.bin",
    "current_object": "{root}/build/src/bof3/world/drawTexturedFrame.o",
    "instruction_count": {
        "original": 339,
        "current": 339,
        "matching": 326,
        "match_percent": 96.17,
    },
    "first_mismatch": {
        "original_index": None,
        "current_index": 85,
        "original_offset": None,
        "current_offset": 340,
        "original": None,
        "current": "sb s2,37(s0)",
    },
    "outputs": {
        "directory": "{root}/out/asm-diff/emi_world00_area008_13/drawTexturedFrame",
        "summary": "{root}/out/asm-diff/emi_world00_area008_13/drawTexturedFrame/summary.json",
        "diff": "{root}/out/asm-diff/emi_world00_area008_13/drawTexturedFrame/diff.patch",
        "original": "{root}/out/asm-diff/emi_world00_area008_13/drawTexturedFrame/original.s",
        "current": "{root}/out/asm-diff/emi_world00_area008_13/drawTexturedFrame/current.s",
        "compiler": "{root}/out/asm-diff/emi_world00_area008_13/drawTexturedFrame/compiler.s",
        "original_bytes": "{root}/out/asm-diff/emi_world00_area008_13/drawTexturedFrame/original.bin",
        "build_log": "{root}/out/asm-diff/emi_world00_area008_13/drawTexturedFrame/build.log",
        "linked": "{root}/out/asm-diff/emi_world00_area008_13/drawTexturedFrame/linked.s",
    },
}


_REAL_BYTE_AREA008 = {
    "schema": "harness.byte-match-one/v1",
    "status": "different",
    "exact_match": False,
    "byte_match": False,
    "source": "{root}/src/bof3/world/drawTexturedFrame.c",
    "function": "drawTexturedFrame",
    "address": "0x801f3d88",
    "original_size": 1356,
    "current_size": 1356,
    "size_delta": 0,
    "original_binary": "{root}/out/binaries/emi/world00/area008/13.bin",
    "current_object": "{root}/build/src/bof3/world/drawTexturedFrame.o",
    "outputs": {},
}


def _real_payload(payload: dict[str, object], root: Path) -> dict[str, object]:
    return {
        key: (
            str(value).replace("{root}", str(root)) if isinstance(value, str) else value
        )
        for key, value in payload.items()
    }


def test_partial_baseline_captures_and_verifies_real_tool_identity(
    tmp_path: Path,
) -> None:
    """Real asm-diff/byte-match identity shapes capture and verify; forgeries
    (outside source, wrong source, symlinked source, wrong address) fail.
    """

    from harness.match.validation import _baseline_facts, _check, check_evidence

    for relative in (
        "src/bof3/scenario/clampPaletteChannels.c",
        "src/bof3/world/drawTexturedFrame.c",
    ):
        source = tmp_path / relative
        source.parent.mkdir(parents=True, exist_ok=True)
        source.write_text("/* @source 0x80100000 */\n", encoding="utf-8")

    functions = [
        {
            "selector": "scena16@0x801F83B0",
            "target": "scena16",
            "function": "clampPaletteChannels",
            "source": "src/bof3/scenario/clampPaletteChannels.c",
            "status": "partial",
        },
        {
            "selector": "area008/13@0x801F3D88",
            "target": "area008/13",
            "function": "drawTexturedFrame",
            "source": "src/bof3/world/drawTexturedFrame.c",
            "status": "partial",
        },
    ]
    responses = {
        "scena16@0x801F83B0": {
            "asm-diff": _real_payload(_REAL_ASM_SCENA16, tmp_path),
            "byte-match": _real_payload(_REAL_BYTE_SCENA16, tmp_path),
        },
        "area008/13@0x801F3D88": {
            "asm-diff": _real_payload(_REAL_ASM_AREA008, tmp_path),
            "byte-match": _real_payload(_REAL_BYTE_AREA008, tmp_path),
        },
    }

    for function in functions:
        for tool in ("asm-diff", "byte-match"):
            check = _check(
                tool,
                function["target"],
                function["selector"],
                function["function"],
                function["source"],
            )
            output = json.dumps(responses[function["selector"]][tool])
            facts = _baseline_facts(check, 1, output, tmp_path)
            check["partial_baseline"] = {**facts, "digest": digest(facts)}
            assert check_evidence(check, 1, output, tmp_path)["passed"]

    check = _check(
        "byte-match",
        "scena16",
        "scena16@0x801F83B0",
        "clampPaletteChannels",
        "src/bof3/scenario/clampPaletteChannels.c",
    )
    byte = _real_payload(_REAL_BYTE_SCENA16, tmp_path)
    assert _baseline_facts(check, 1, json.dumps(byte), tmp_path)["address"] == (
        "0x801f83b0"
    )
    # Hex-case-insensitive address identity is accepted, not loosened.
    assert (
        _baseline_facts(
            check, 1, json.dumps({**byte, "address": "0x801F83B0"}), tmp_path
        )["address"]
        == "0x801F83B0"
    )

    alias = tmp_path / "src/bof3/scenario/alias.c"
    alias.symlink_to(tmp_path / "src/bof3/scenario/clampPaletteChannels.c")
    outside = tmp_path.parent / f"{tmp_path.name}-alias.c"
    outside.symlink_to(tmp_path / "src/bof3/scenario/clampPaletteChannels.c")
    linked = tmp_path / "src/bof3/scenario/linked"
    linked.symlink_to(tmp_path / "src/bof3/scenario", target_is_directory=True)
    check_linked = _check(
        "byte-match",
        "scena16",
        "scena16@0x801F83B0",
        "clampPaletteChannels",
        "src/bof3/scenario/linked/clampPaletteChannels.c",
    )
    for forged in (
        {**byte, "source": str(tmp_path.parent / "outside.c")},
        {
            **byte,
            "source": str(tmp_path / "src/bof3/world/drawTexturedFrame.c"),
        },
        {**byte, "source": str(alias)},
        {**byte, "source": str(outside)},
        {**byte, "address": "0x801f83b4"},
    ):
        with pytest.raises(ValueError, match="partial"):
            _baseline_facts(check, 1, json.dumps(forged), tmp_path)
    # External symlink alias reached by a relative .. escape is also rejected.
    with pytest.raises(ValueError, match="partial"):
        _baseline_facts(
            check,
            1,
            json.dumps(
                {
                    **byte,
                    "source": f"../{tmp_path.name}-alias.c",
                }
            ),
            tmp_path,
        )
    # In-repo symlinked directory with an exactly matching lexical path fails.
    with pytest.raises(ValueError, match="partial"):
        _baseline_facts(
            check_linked,
            1,
            json.dumps({**byte, "source": str(linked / "clampPaletteChannels.c")}),
            tmp_path,
        )
    outside.unlink()


@pytest.mark.parametrize("tool", ["asm-diff", "byte-match"])
@pytest.mark.parametrize(
    "fault",
    [
        "warning",
        "missing",
        "unknown",
        "duplicate",
        "malformed",
        "source",
        "binary",
        "size",
        "boolean",
        "receipt",
    ],
)
def test_native_exact_gate_rejects_exit_zero_without_owner_evidence(
    tmp_path: Path, tool: str, fault: str
) -> None:
    from harness.match.validation import _check, check_evidence

    _repo(tmp_path)
    check = _check(
        tool,
        TARGET,
        f"{TARGET}@0x80100000",
        "func_80100000",
        "src/test/func_80100000.c",
    )
    payload = exact_payload(tmp_path, tool)
    if fault == "warning":
        payload["status"] = "warning"
    elif fault == "missing":
        del payload["status"]
    elif fault == "unknown":
        payload["unexpected"] = True
    elif fault == "source":
        payload["source"] = "src/other.c"
    elif fault == "binary":
        payload["original_binary"] = "out/other.bin"
    elif fault == "size":
        payload["current_size"] = 12
    elif fault == "boolean":
        payload["exact_match"] = 1
    output = json.dumps(payload)
    if fault == "duplicate":
        output = output.replace('"status":', '"status": "warning", "status":', 1)
    elif fault == "malformed":
        output = "not JSON"
    assert check_evidence(check, 0, output, tmp_path)["passed"] is (fault == "receipt")


def _repo(root: Path) -> None:
    config = root / "config/targets/exe/test/target.toml"
    config.parent.mkdir(parents=True)
    config.write_text(
        "schema='harness.target/v2'\nid='exe/test'\nkind='executable'\n"
        "source_dir='src/test'\nbinary='out/test.bin'\nload_address=0x80100000\n"
        "splat='config/targets/exe/test/splat.yaml'\n"
        "sources=['src/test/func_80100000.c']\nheaders=['include/test.h']\n",
        encoding="utf-8",
    )
    (config.parent / "splat.yaml").write_text(
        "segments:\n  - [0, c, func_80100000]\n  - [8]\n"
    )
    (config.parent / "symbols.txt").write_text("func_80100000 = 0x80100000;\n")
    binary = root / "out/test.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(b"\0" * 8)
    header = root / "include/test.h"
    header.parent.mkdir(parents=True)
    header.write_text("typedef unsigned int Test;\n")
    source = root / "src/test/func_80100000.c"
    source.parent.mkdir(parents=True)
    source.write_text(
        "/** @source 0x80100000\n * @behavior test lift\n */\n"
        "void func_80100000(void) {}\n"
    )
