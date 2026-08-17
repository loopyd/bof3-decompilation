from __future__ import annotations

from pathlib import Path
import subprocess

from harness.decomp import preflight as dsp
from harness.decomp import status as decomp_status


def _target(
    root: Path,
    target: str,
    source_dir: str,
    *,
    sources: tuple[str, ...] = (),
    support_sources: tuple[str, ...] = (),
) -> None:
    manifest = root / "config" / "targets" / target / "target.toml"
    manifest.parent.mkdir(parents=True, exist_ok=True)
    claims = ""
    if sources or support_sources:
        claims += "sources = [" + ", ".join(f'"{s}"' for s in sources) + "]\n"
        claims += (
            "support_sources = [" + ", ".join(f'"{s}"' for s in support_sources) + "]\n"
        )
    manifest.write_text(
        "schema = 'harness.target/v2'\n"
        f"id = '{target}'\n"
        "kind = 'executable'\n"
        f"source_dir = '{source_dir}'\n"
        f"binary = 'out/binaries/{target}.bin'\n"
        f"splat = 'config/targets/{target}/splat.yaml'\n"
        "load_address = 0x80100000\n" + claims,
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


def test_preflight_reports_duplicate_claims_as_invalid(tmp_path: Path) -> None:
    _target(
        tmp_path,
        "exe/logo",
        "src/exe/logo",
        sources=("src/exe/logo/func_80100010.c", "src/exe/logo/dup.c"),
    )
    _source(tmp_path, "src/exe/logo", "80100010")
    dup = tmp_path / "src/exe/logo/dup.c"
    dup.write_text(
        "// @source 0x80100010\n// @behavior duplicate claim\n", encoding="utf-8"
    )
    manifests = decomp_status.select_manifests(tmp_path)
    ready, worklist = dsp._build_preflight(tmp_path, manifests, cache=None)
    assert any(
        r["reason"].startswith("duplicate address claim 0x80100010") for r in ready
    )
    # exactly one claimant reaches the worklist; the duplicate is rejected
    assert [item[2] for item in worklist["exe/logo"]].count(0x80100010) == 1


def test_preflight_skips_helper_files_and_flags_expected_lifts(
    tmp_path: Path,
) -> None:
    _target(
        tmp_path,
        "exe/logo",
        "src/exe/logo",
        sources=(
            "src/exe/logo/func_80100010.c",
            "src/exe/logo/initSelectionState.c",
        ),
        support_sources=("src/exe/logo/symbols.c",),
    )
    _source(tmp_path, "src/exe/logo", "80100010")
    source_dir = tmp_path / "src/exe/logo"
    (source_dir / "symbols.c").write_text("WEAK_SYMBOL_AT(x, 0x80100000);\n")
    expected = source_dir / "initSelectionState.c"
    expected.write_text("void f(void) {}\n", encoding="utf-8")
    splat = tmp_path / "config/targets/exe/logo/splat.yaml"
    splat.parent.mkdir(parents=True, exist_ok=True)
    splat.write_text(
        "segments:\n"
        "  - [0, c, func_80100010]\n"
        "  - [16, c, initSelectionState]\n"
        "  - [32]\n",
        encoding="utf-8",
    )
    manifests = decomp_status.select_manifests(tmp_path)
    ready, worklist = dsp._build_preflight(tmp_path, manifests, cache=None)
    # helper file contributes nothing; expected lift without metadata is invalid
    assert not any(r["function"] == "symbols.c" for r in ready)
    assert any(
        r["function"] == "initSelectionState"
        and "missing required metadata" in r["reason"]
        for r in ready
    )


def test_report_orders_lifts_and_aggregates_match_states(
    tmp_path: Path, monkeypatch
) -> None:
    _target(
        tmp_path, "exe/zeta", "src/exe/zeta", sources=("src/exe/zeta/func_80100030.c",)
    )
    _target(
        tmp_path,
        "emi/alpha/00",
        "src/emi/alpha/00",
        sources=(
            "src/emi/alpha/00/func_80100020.c",
            "src/emi/alpha/00/func_80100010.c",
        ),
    )
    _source(tmp_path, "src/exe/zeta", "80100030", metadata=False)
    _source(tmp_path, "src/emi/alpha/00", "80100020")
    _source(tmp_path, "src/emi/alpha/00", "80100010")
    monkeypatch.setattr(
        decomp_status,
        "index_coverage",
        lambda _root, _manifests: ({"emi/alpha/00": 7, "exe/zeta": 3}, {}),
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
    _target(
        tmp_path, "exe/keep", "src/exe/keep", sources=("src/exe/keep/func_80100010.c",)
    )
    _target(
        tmp_path, "exe/skip", "src/exe/skip", sources=("src/exe/skip/func_80100020.c",)
    )
    _source(tmp_path, "src/exe/keep", "80100010")
    _source(tmp_path, "src/exe/skip", "80100020")
    monkeypatch.setattr(
        decomp_status,
        "index_coverage",
        lambda _root, _manifests: ({"exe/keep": 1}, {}),
    )

    report = decomp_status.build_report(tmp_path, ("exe/keep",), diff_runner=_diff)

    assert [target["target"] for target in report["targets"]] == ["exe/keep"]
    assert report["lifts"]["total"] == 1
    assert report["indexed_functions"] == 1


def test_report_keeps_live_results_when_index_is_unavailable(
    tmp_path: Path, monkeypatch
) -> None:
    _target(
        tmp_path, "exe/logo", "src/exe/logo", sources=("src/exe/logo/func_80100010.c",)
    )
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


def test_report_reuses_a_content_addressed_cache(tmp_path: Path, monkeypatch) -> None:
    _target(
        tmp_path, "exe/logo", "src/exe/logo", sources=("src/exe/logo/func_80100010.c",)
    )
    _source(tmp_path, "src/exe/logo", "80100010")
    binary = tmp_path / "out/binaries/exe/logo.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(b"test")
    monkeypatch.setattr(
        decomp_status,
        "index_coverage",
        lambda _root, _manifests: ({"exe/logo": 1}, {}),
    )
    calls = 0

    def cached_diff(request: object) -> dict[str, object]:
        nonlocal calls
        calls += 1
        return _diff(request)

    first = decomp_status.build_report(tmp_path, ("exe/logo",), diff_runner=cached_diff)
    second = decomp_status.build_report(
        tmp_path, ("exe/logo",), diff_runner=cached_diff
    )

    assert calls == 1
    assert first == second


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


def test_build_preflight_separates_cache_misses_from_ready(
    tmp_path: Path, monkeypatch
) -> None:
    """Phase 2.3.1: invalid, cached, and valid-miss sources land correctly."""
    _target(
        tmp_path,
        "exe/logo",
        "src/exe/logo",
        sources=(
            "src/exe/logo/func_80100030.c",
            "src/exe/logo/func_80100010.c",
        ),
    )
    _target(
        tmp_path,
        "exe/other",
        "src/exe/other",
        sources=("src/exe/other/func_80100020.c",),
    )
    # Invalid: missing metadata
    _source(tmp_path, "src/exe/logo", "80100030", metadata=False)
    # Valid: will be a cache miss
    _source(tmp_path, "src/exe/logo", "80100010")
    _source(tmp_path, "src/exe/other", "80100020")
    binary = tmp_path / "out/binaries" / "exe/logo.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(b"test")

    manifests = decomp_status.select_manifests(tmp_path)

    ready, worklist = dsp._build_preflight(tmp_path, manifests, cache=None)

    assert len(ready) == 1  # one invalid (missing metadata)
    assert ready[0]["status"] == "invalid"
    assert ready[0]["function"] == "func_80100030"
    assert "exe/logo" in worklist
    # worklist item: (target, source, address, source_name, key, manifest)
    assert any(item[2] == 0x80100010 for item in worklist["exe/logo"])
    assert "exe/other" in worklist
    assert any(item[2] == 0x80100020 for item in worklist["exe/other"])


def test_build_preflight_reuses_cache_hits(tmp_path: Path, monkeypatch) -> None:
    """Phase 2.3.1: cache hits appear in ready, not in worklist."""
    _target(
        tmp_path,
        "exe/logo",
        "src/exe/logo",
        sources=(
            "src/exe/logo/func_80100010.c",
            "src/exe/logo/func_80100020.c",
        ),
    )
    _source(tmp_path, "src/exe/logo", "80100010")
    _source(tmp_path, "src/exe/logo", "80100020")
    binary = tmp_path / "out/binaries" / "exe/logo.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(b"test")

    from harness.match.status_cache import MatchStatusCache

    cache = MatchStatusCache(tmp_path)
    cache.put(
        "exe/logo",
        "src/exe/logo/func_80100010.c",
        "fake-fingerprint",
        {
            "target": "exe/logo",
            "function": "func_80100010",
            "address": "0x80100010",
            "source": "src/exe/logo/func_80100010.c",
            "status": "exact",
            "reason": "",
            "instruction_count": {"original": 4, "current": 4, "matching": 4},
            "match_percent": 100.0,
            "original_size": 16,
            "current_size": 16,
            "size_delta": 0,
        },
    )
    from harness.decomp import status as ds

    monkeypatch.setattr(dsp, "source_fingerprint", lambda _s, _t: "fake-fingerprint")
    monkeypatch.setattr(dsp, "target_fingerprint", lambda _r, _m: "fake-fingerprint")
    ready, worklist = dsp._build_preflight(
        tmp_path, ds.select_manifests(tmp_path), cache
    )
    cache.close()

    assert any(r["function"] == "func_80100010" for r in ready)
    # func_80100020 has no cache entry → appears in worklist
    assert "exe/logo" in worklist
    assert any(item[2] == 0x80100020 for item in worklist["exe/logo"])


def _batch_result() -> dict[str, object]:
    return {
        "byte_match": True,
        "instruction_count": {
            "original": 4,
            "current": 4,
            "matching": 4,
            "match_percent": 100.0,
        },
        "original_size": 16,
        "current_size": 16,
        "size_delta": 0,
    }


def test_batch_builds_fresh_misses_once_per_target(tmp_path: Path, monkeypatch) -> None:
    """Phase 2.3.2: one successful batch compares fresh objects in root."""
    _target(
        tmp_path,
        "exe/logo",
        "src/exe/logo",
        sources=(
            "src/exe/logo/func_80100010.c",
            "src/exe/logo/func_80100020.c",
        ),
    )
    _source(tmp_path, "src/exe/logo", "80100010")
    _source(tmp_path, "src/exe/logo", "80100020")
    binary = tmp_path / "out/binaries" / "exe/logo.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(b"test")

    from harness.decomp import status as ds

    batch_calls: list[list[str]] = []
    compared_roots: list[Path] = []

    def batch(root: Path, targets: list[str]) -> subprocess.CompletedProcess[str]:
        batch_calls.append(targets)
        return subprocess.CompletedProcess([], 0, "", "")

    def resolve(repo, request):
        object_path = request.source_path.with_suffix(".o")
        object_path.touch()
        return {
            "source_path": request.source_path,
            "address": request.address,
            "function_name": request.source_path.stem,
            "binary_path": request.binary_path,
            "load_address": request.load_address,
            "original_size": 16,
            "object_path": object_path,
            "output_dir": tmp_path / "out/matching/dummy",
        }

    def compare(repo, request, resolved):
        compared_roots.append(repo.root)
        return _batch_result()

    monkeypatch.setattr(dsp, "configure", lambda root: tmp_path / "build/cmake")
    monkeypatch.setattr(dsp, "batch_build", batch)
    monkeypatch.setattr(dsp, "_asm_diff_resolve", resolve)
    monkeypatch.setattr(dsp, "_asm_diff_compare", compare)
    monkeypatch.setattr(ds, "index_coverage", lambda _root, _manifests: ({}, {}))

    report = ds.build_report(tmp_path, use_cache=False, diff_runner=lambda _: {})

    assert len(batch_calls) == 1
    assert len(batch_calls[0]) == 2
    assert compared_roots == [tmp_path.resolve(), tmp_path.resolve()]
    assert report["lifts"] == {"exact": 2, "partial": 0, "invalid": 0, "total": 2}


def test_batch_resolve_failure_falls_back_once(tmp_path: Path, monkeypatch) -> None:
    """A successful batch cannot abort the audit on one resolve failure."""
    _target(
        tmp_path, "exe/logo", "src/exe/logo", sources=("src/exe/logo/func_80100010.c",)
    )
    _source(tmp_path, "src/exe/logo", "80100010")
    binary = tmp_path / "out/binaries" / "exe/logo.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(b"test")

    from harness.decomp import status as ds

    fallback = 0
    monkeypatch.setattr(dsp, "configure", lambda root: tmp_path / "build/cmake")
    monkeypatch.setattr(
        dsp,
        "batch_build",
        lambda root, targets: subprocess.CompletedProcess([], 0, "", ""),
    )
    monkeypatch.setattr(ds, "index_coverage", lambda _root, _manifests: ({}, {}))
    monkeypatch.setattr(
        dsp,
        "_asm_diff_resolve",
        lambda repo, request: (_ for _ in ()).throw(
            ValueError("cannot infer test size")
        ),
    )

    def diff(_request):
        nonlocal fallback
        fallback += 1
        raise ValueError("cannot infer test size")

    report = ds.build_report(tmp_path, use_cache=False, diff_runner=diff)

    assert fallback == 1
    assert report["lifts"] == {"exact": 0, "partial": 0, "invalid": 1, "total": 1}
    assert report["targets"][0]["functions"][0]["reason"] == "cannot infer test size"


def test_batch_stale_object_falls_back_once_without_duplicate_record(
    tmp_path: Path, monkeypatch
) -> None:
    _target(
        tmp_path, "exe/logo", "src/exe/logo", sources=("src/exe/logo/func_80100010.c",)
    )
    _source(tmp_path, "src/exe/logo", "80100010")
    binary = tmp_path / "out/binaries" / "exe/logo.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(b"test")

    from harness.decomp import status as ds

    fallback = 0
    monkeypatch.setattr(dsp, "configure", lambda root: tmp_path / "build/cmake")
    monkeypatch.setattr(
        dsp,
        "batch_build",
        lambda root, targets: subprocess.CompletedProcess([], 0, "", ""),
    )
    monkeypatch.setattr(
        dsp,
        "_asm_diff_resolve",
        lambda repo, request: {
            "source_path": request.source_path,
            "address": request.address,
            "function_name": request.source_path.stem,
            "binary_path": request.binary_path,
            "load_address": request.load_address,
            "original_size": 16,
            "object_path": request.source_path.with_suffix(".o"),
            "output_dir": tmp_path / "out/matching/dummy",
        },
    )
    monkeypatch.setattr(ds, "index_coverage", lambda _root, _manifests: ({}, {}))

    def diff(_request):
        nonlocal fallback
        fallback += 1
        return _batch_result()

    report = ds.build_report(tmp_path, use_cache=False, diff_runner=diff)

    assert fallback == 1
    assert report["lifts"] == {"exact": 1, "partial": 0, "invalid": 0, "total": 1}


def test_batch_failure_falls_back_per_source_with_error_attribution(
    tmp_path: Path, monkeypatch
) -> None:
    """Phase 2.3.2: failed batch falls back to per-source build+compare."""
    _target(
        tmp_path,
        "exe/logo",
        "src/exe/logo",
        sources=(
            "src/exe/logo/func_80100010.c",
            "src/exe/logo/func_80100020.c",
        ),
    )
    _source(tmp_path, "src/exe/logo", "80100010")
    _source(tmp_path, "src/exe/logo", "80100020")
    binary = tmp_path / "out/binaries" / "exe/logo.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(b"test")

    from harness.decomp import status as ds

    diff_calls: list[str] = []

    monkeypatch.setattr(dsp, "configure", lambda root: tmp_path / "build/cmake")
    monkeypatch.setattr(
        dsp,
        "batch_build",
        lambda root, targets: subprocess.CompletedProcess([], 1, "", "build error"),
    )
    monkeypatch.setattr(ds, "index_coverage", lambda _root, _manifests: ({}, {}))

    def diff(request):
        diff_calls.append(request.source_path.stem)
        exact = request.source_path.stem == "func_80100010"
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

    report = ds.build_report(tmp_path, use_cache=False, diff_runner=diff)

    # Both sources via per-source fallback, no duplicate
    assert sorted(diff_calls) == ["func_80100010", "func_80100020"]
    assert report["lifts"] == {"exact": 1, "partial": 1, "invalid": 0, "total": 2}


def test_no_batch_when_all_sources_are_cached(tmp_path: Path, monkeypatch) -> None:
    _target(
        tmp_path,
        "exe/logo",
        "src/exe/logo",
        sources=("src/exe/logo/func_80100010.c",),
    )
    _source(tmp_path, "src/exe/logo", "80100010")
    binary = tmp_path / "out/binaries" / "exe/logo.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(b"test")

    from harness.match.status_cache import MatchStatusCache

    cache = MatchStatusCache(tmp_path)
    cache.put(
        "exe/logo",
        "src/exe/logo/func_80100010.c",
        "fake-fp",
        {
            "target": "exe/logo",
            "function": "func_80100010",
            "address": "0x80100010",
            "source": "src/exe/logo/func_80100010.c",
            "status": "exact",
            "reason": "",
            "instruction_count": {"original": 4, "current": 4, "matching": 4},
            "match_percent": 100.0,
            "original_size": 16,
            "current_size": 16,
            "size_delta": 0,
        },
    )

    from harness.decomp import status as ds

    monkeypatch.setattr(dsp, "source_fingerprint", lambda _s, _t: "fake-fp")
    monkeypatch.setattr(dsp, "target_fingerprint", lambda _r, _m: "fake-fp")
    monkeypatch.setattr(
        ds,
        "index_coverage",
        lambda _root, _manifests: ({}, {}),
    )

    batch_calls: list[list[str]] = []

    def track_batch(root, targets):
        batch_calls.append(targets)
        raise RuntimeError("should not be called")

    monkeypatch.setattr(dsp, "batch_build", track_batch)

    report = ds.build_report(
        tmp_path, ("exe/logo",), use_cache=True, diff_runner=lambda r: {}
    )

    assert batch_calls == [], "batch_build was called despite all cache hits"
    assert report["lifts"]["total"] == 1
    assert report["lifts"]["exact"] == 1
    cache.close()


def test_source_change_invalidates_cache_and_recomputes(
    tmp_path: Path, monkeypatch
) -> None:
    """Phase 2.3.3: source change invalidates cache, produces one batch build."""
    _target(
        tmp_path,
        "exe/logo",
        "src/exe/logo",
        sources=(
            "src/exe/logo/func_80100010.c",
            "src/exe/logo/func_80100020.c",
        ),
    )
    _source(tmp_path, "src/exe/logo", "80100010")
    _source(tmp_path, "src/exe/logo", "80100020")
    binary = tmp_path / "out/binaries" / "exe/logo.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(b"test")

    from harness.decomp import status as ds
    from harness.match.status_cache import MatchStatusCache

    cache = MatchStatusCache(tmp_path)
    import hashlib

    monkeypatch.setattr(
        dsp,
        "source_fingerprint",
        lambda s, t: hashlib.sha256(s.read_bytes()).hexdigest()[:32],
    )
    monkeypatch.setattr(dsp, "target_fingerprint", lambda r, m: "fp-target")
    monkeypatch.setattr(ds, "index_coverage", lambda _root, _manifests: ({}, {}))

    batch_calls: list[list[str]] = []

    def batch(root, targets):
        batch_calls.append(targets)
        return subprocess.CompletedProcess([], 0, "", "")

    def resolve(repo, req):
        obj = req.source_path.with_suffix(".o")
        obj.touch()
        return {
            "source_path": req.source_path,
            "address": req.address,
            "function_name": req.source_path.stem,
            "binary_path": req.binary_path,
            "load_address": req.load_address,
            "original_size": 16,
            "object_path": obj,
            "output_dir": tmp_path / "out/matching/dummy",
        }

    monkeypatch.setattr(dsp, "configure", lambda root: tmp_path / "build/cmake")
    monkeypatch.setattr(dsp, "batch_build", batch)
    monkeypatch.setattr(dsp, "_asm_diff_resolve", resolve)
    monkeypatch.setattr(dsp, "_asm_diff_compare", lambda repo, rq, rs: _batch_result())

    # First run — cache miss for both sources
    first = ds.build_report(
        tmp_path, use_cache=True, diff_runner=lambda r: _batch_result()
    )
    assert first["lifts"] == {"exact": 2, "partial": 0, "invalid": 0, "total": 2}
    assert len(batch_calls) == 1, "first run: one batch build"

    # Second run — both cached, no build
    batch_calls.clear()
    second = ds.build_report(
        tmp_path, use_cache=True, diff_runner=lambda r: _batch_result()
    )
    assert second["lifts"] == first["lifts"]
    assert len(batch_calls) == 0, "second run: no batch build (all cached)"

    # Change one source
    src = tmp_path / "src/exe/logo/func_80100010.c"
    src.write_text("// @source 0x80100010\n// @behavior changed\n")

    # Third run — only changed source recomputed; one build for that target
    batch_calls.clear()
    third = ds.build_report(
        tmp_path, use_cache=True, diff_runner=lambda r: _batch_result()
    )
    assert third["lifts"] == {"exact": 2, "partial": 0, "invalid": 0, "total": 2}
    assert len(batch_calls) == 1, "source change: one batch"

    cache.close()


def test_compile_inputs_invalidate_only_affected_target_then_all_targets(
    tmp_path: Path, monkeypatch
) -> None:
    """Target inputs invalidate one target; shared headers invalidate both."""
    _target(
        tmp_path,
        "exe/logo",
        "src/exe/logo",
        sources=("src/exe/logo/func_80100010.c",),
    )
    _target(
        tmp_path,
        "exe/other",
        "src/exe/other",
        sources=("src/exe/other/func_80100020.c",),
    )
    _source(tmp_path, "src/exe/logo", "80100010")
    _source(tmp_path, "src/exe/other", "80100020")
    for target in ("exe/logo", "exe/other"):
        binary = tmp_path / "out/binaries" / f"{target}.bin"
        binary.parent.mkdir(parents=True, exist_ok=True)
        binary.write_bytes(b"test")
        symbols = tmp_path / "config/targets" / target / "symbols.txt"
        symbols.write_text("func_80100000 = 0x80100000;\n")
    header = tmp_path / "include/test.h"
    header.parent.mkdir()
    header.write_text("#define TEST 1\n")

    from harness.decomp import status as ds

    batches: list[list[str]] = []

    def batch(root, targets):
        batches.append(targets)
        return subprocess.CompletedProcess([], 0, "", "")

    def resolve(repo, request):
        object_path = request.source_path.with_suffix(".o")
        object_path.touch()
        return {
            "source_path": request.source_path,
            "address": request.address,
            "function_name": request.source_path.stem,
            "binary_path": request.binary_path,
            "load_address": request.load_address,
            "original_size": 16,
            "object_path": object_path,
            "output_dir": tmp_path / "out/matching/dummy",
        }

    monkeypatch.setattr(dsp, "configure", lambda root: tmp_path / "build/cmake")
    monkeypatch.setattr(dsp, "batch_build", batch)
    monkeypatch.setattr(dsp, "_asm_diff_resolve", resolve)
    monkeypatch.setattr(
        dsp, "_asm_diff_compare", lambda repo, req, resolved: _batch_result()
    )
    monkeypatch.setattr(ds, "index_coverage", lambda _root, _manifests: ({}, {}))

    ds.build_report(tmp_path, use_cache=True, diff_runner=lambda _: _batch_result())
    assert len(batches) == 2

    batches.clear()
    (tmp_path / "config/targets/exe/logo/symbols.txt").write_text(
        "func_80100000 = 0x80100004;\n"
    )
    ds.build_report(tmp_path, use_cache=True, diff_runner=lambda _: _batch_result())
    assert len(batches) == 1

    batches.clear()
    header.write_text("#define TEST 2\n")
    ds.build_report(tmp_path, use_cache=True, diff_runner=lambda _: _batch_result())
    assert len(batches) == 2
