from __future__ import annotations

from pathlib import Path

from harness import decomp_status


def _target(root: Path, target: str, source_dir: str) -> None:
    manifest = root / "config" / "targets" / target / "target.toml"
    manifest.parent.mkdir(parents=True, exist_ok=True)
    manifest.write_text(
        "schema = 'harness.target/v2'\n"
        f"id = '{target}'\n"
        "kind = 'executable'\n"
        f"source_dir = '{source_dir}'\n"
        f"binary = 'out/binaries/{target}.bin'\n"
        f"splat = 'config/targets/{target}/splat.yaml'\n"
        "load_address = 0x80100000\n",
        encoding="utf-8",
    )


def _source(root: Path, directory: str, address: str, *, metadata: bool = True) -> None:
    path = root / directory / f"func_{address}.c"
    path.parent.mkdir(parents=True, exist_ok=True)
    text = ""
    if metadata:
        text = f"// @source 0x{address}\n// @behavior test behavior\n"
    path.write_text(text + "void f(void) {}\n", encoding="utf-8")


def _diff(request: object) -> dict[str, object]:
    source = request.source_path  # type: ignore[attr-defined]
    exact = source.stem == "func_80100010"
    return {
        "byte_match": exact,
        "instruction_count": {
            "original": 4,
            "current": 4 if exact else 5,
            "matching": 4 if exact else 3,
            "match_percent": 100.0 if exact else 60.0,
        },
        "original_size": 16,
        "current_size": 16 if exact else 20,
        "size_delta": 0 if exact else 4,
    }


def test_report_orders_lifts_and_aggregates_match_states(
    tmp_path: Path, monkeypatch
) -> None:
    _target(tmp_path, "exe/zeta", "src/exe/zeta")
    _target(tmp_path, "emi/alpha/00", "src/emi/alpha/00")
    _source(tmp_path, "src/exe/zeta", "80100030", metadata=False)
    _source(tmp_path, "src/emi/alpha/00", "80100020")
    _source(tmp_path, "src/emi/alpha/00", "80100010")
    monkeypatch.setattr(
        decomp_status,
        "index_coverage",
        lambda _root, _manifests: {"emi/alpha/00": 7, "exe/zeta": 3},
    )

    report = decomp_status.build_report(
        tmp_path, ("exe/zeta", "emi/alpha/00"), diff_runner=_diff
    )

    assert [target["target"] for target in report["targets"]] == [
        "emi/alpha/00",
        "exe/zeta",
    ]
    assert [record["function"] for record in report["targets"][0]["functions"]] == [
        "func_80100010",
        "func_80100020",
    ]
    assert report["lifts"] == {
        "exact": 1,
        "partial": 1,
        "invalid": 1,
        "total": 3,
    }
    assert report["targets"][0]["indexed_functions"] == 7
    assert "EXACT func_80100010" in decomp_status.render_text(report)


def test_report_filters_to_requested_target(tmp_path: Path, monkeypatch) -> None:
    _target(tmp_path, "exe/keep", "src/exe/keep")
    _target(tmp_path, "exe/skip", "src/exe/skip")
    _source(tmp_path, "src/exe/keep", "80100010")
    _source(tmp_path, "src/exe/skip", "80100020")
    monkeypatch.setattr(
        decomp_status,
        "index_coverage",
        lambda _root, _manifests: {"exe/keep": 1},
    )

    report = decomp_status.build_report(tmp_path, ("exe/keep",), diff_runner=_diff)

    assert [target["target"] for target in report["targets"]] == ["exe/keep"]
    assert report["lifts"]["total"] == 1
    assert report["indexed_functions"] == 1


def test_report_keeps_live_results_when_index_is_unavailable(
    tmp_path: Path, monkeypatch
) -> None:
    _target(tmp_path, "exe/logo", "src/exe/logo")
    _source(tmp_path, "src/exe/logo", "80100010")

    def unavailable(_root: Path, _manifests: object) -> dict[str, int]:
        raise FileNotFoundError("reverse index not found; run just index")

    monkeypatch.setattr(decomp_status, "index_coverage", unavailable)
    report = decomp_status.build_report(tmp_path, diff_runner=_diff)

    assert report["lifts"] == {
        "exact": 1,
        "partial": 0,
        "invalid": 0,
        "total": 1,
    }
    assert report["indexed_functions"] is None
    assert report["coverage_error"] == "reverse index not found; run just index"
    assert "index coverage: unavailable" in decomp_status.render_text(report)


def test_context_detail_keeps_full_report_available() -> None:
    report = {
        "schema": "bof3.decomp-status/v1",
        "lifts": {"exact": 1, "partial": 0, "invalid": 0, "total": 1},
        "indexed_functions": 3,
        "coverage_error": None,
        "targets": [
            {
                "target": "exe/test",
                "lifts": {"exact": 1, "partial": 0, "invalid": 0, "total": 1},
                "indexed_functions": 3,
                "functions": [
                    {
                        "status": "exact",
                        "function": "func_80100010",
                        "address": "0x80100010",
                    }
                ],
            }
        ],
    }

    assert decomp_status.render_text(report, "minimal") == (
        "lifts: exact=1 partial=0 invalid=0 total=1"
    )
    normal = decomp_status.project_report(report, "normal")
    assert normal["targets"][0]["invalid"] == []
    assert decomp_status.project_report(report, "full") is report
