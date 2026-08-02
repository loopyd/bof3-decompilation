import subprocess
def test_report_reuses_a_content_addressed_cache(
    binary = tmp_path / "out/binaries/exe/logo.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(b"test")
        lambda _root, _manifests: {"exe/logo": 1},
    )
    calls = 0

    def cached_diff(request: object) -> dict[str, object]:
        nonlocal calls
        calls += 1
        return _diff(request)

    first = decomp_status.build_report(tmp_path, ("exe/logo",), diff_runner=cached_diff)
    second = decomp_status.build_report(tmp_path, ("exe/logo",), diff_runner=cached_diff)

    assert calls == 1
    assert first == second




def test_build_preflight_separates_cache_misses_from_ready(
    """Phase 2.3.1: invalid, cached, and valid-miss sources land correctly."""
    _target(tmp_path, "exe/other", "src/exe/other")
    # Invalid: missing metadata
    _source(tmp_path, "src/exe/logo", "80100030", metadata=False)
    # Valid: will be a cache miss
    _source(tmp_path, "src/exe/other", "80100020")
    binary = tmp_path / "out/binaries" / "exe/logo.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(b"test")

    manifests = decomp_status.select_manifests(tmp_path)
    from harness import decomp_status as ds

    ready, worklist = ds._build_preflight(tmp_path, manifests, cache=None)

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
    _source(tmp_path, "src/exe/logo", "80100020")
    binary = tmp_path / "out/binaries" / "exe/logo.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(b"test")

    from harness.match.status_cache import StatusCache

    cache = StatusCache(tmp_path)
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
    from harness import decomp_status as ds

    monkeypatch.setattr(ds, "source_fingerprint", lambda _s, _t: "fake-fingerprint")
    monkeypatch.setattr(ds, "target_fingerprint", lambda _r, _m: "fake-fingerprint")
    ready, worklist = ds._build_preflight(tmp_path, ds.select_manifests(tmp_path), cache)
    cache.close()

    assert any(r["function"] == "func_80100010" for r in ready)
    # func_80100020 has no cache entry → appears in worklist
    assert "exe/logo" in worklist
    assert any(item[2] == 0x80100020 for item in worklist["exe/logo"])


def _batch_result() -> dict[str, object]:
    return {
        "byte_match": True,
            "current": 4,
            "matching": 4,
            "match_percent": 100.0,
        },
        "original_size": 16,
        "current_size": 16,
        "size_delta": 0,
    }


def test_batch_builds_fresh_misses_once_per_target(
    """Phase 2.3.2: one successful batch compares fresh objects in root."""
    _source(tmp_path, "src/exe/logo", "80100020")
    binary = tmp_path / "out/binaries" / "exe/logo.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(b"test")

    from harness import decomp_status as ds

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

    monkeypatch.setattr(ds, "configure", lambda root: tmp_path / "build/cmake")
    monkeypatch.setattr(ds, "batch_build", batch)
    monkeypatch.setattr(ds, "_asm_diff_resolve", resolve)
    monkeypatch.setattr(ds, "_asm_diff_compare", compare)
    monkeypatch.setattr(ds, "index_coverage", lambda _root, _manifests: {})

    report = ds.build_report(tmp_path, use_cache=False, diff_runner=lambda _: {})

    assert len(batch_calls) == 1
    assert len(batch_calls[0]) == 2
    assert compared_roots == [tmp_path.resolve(), tmp_path.resolve()]
    assert report["lifts"] == {"exact": 2, "partial": 0, "invalid": 0, "total": 2}


def test_batch_resolve_failure_falls_back_once(
    """A successful batch cannot abort the audit on one resolve failure."""
    binary = tmp_path / "out/binaries" / "exe/logo.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(b"test")

    from harness import decomp_status as ds

    fallback = 0
    monkeypatch.setattr(ds, "configure", lambda root: tmp_path / "build/cmake")
    monkeypatch.setattr(
        ds,
        "batch_build",
        lambda root, targets: subprocess.CompletedProcess([], 0, "", ""),
    )
    monkeypatch.setattr(ds, "index_coverage", lambda _root, _manifests: {})
    monkeypatch.setattr(
        ds,
        "_asm_diff_resolve",
        lambda repo, request: (_ for _ in ()).throw(ValueError("cannot infer test size")),
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
    binary = tmp_path / "out/binaries" / "exe/logo.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(b"test")

    from harness import decomp_status as ds

    fallback = 0
    monkeypatch.setattr(ds, "configure", lambda root: tmp_path / "build/cmake")
    monkeypatch.setattr(
        ds,
        "batch_build",
        lambda root, targets: subprocess.CompletedProcess([], 0, "", ""),
    )
    monkeypatch.setattr(
        ds,
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
    monkeypatch.setattr(ds, "index_coverage", lambda _root, _manifests: {})

    def diff(_request):
        nonlocal fallback
        fallback += 1
        return _batch_result()

    report = ds.build_report(tmp_path, use_cache=False, diff_runner=diff)

    assert fallback == 1
    assert report["lifts"] == {"exact": 1, "partial": 0, "invalid": 0, "total": 1}


def test_batch_failure_falls_back_per_source_with_error_attribution(
    """Phase 2.3.2: failed batch falls back to per-source build+compare."""
    _source(tmp_path, "src/exe/logo", "80100020")
    binary = tmp_path / "out/binaries" / "exe/logo.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(b"test")

    from harness import decomp_status as ds

    diff_calls: list[str] = []

    monkeypatch.setattr(ds, "configure", lambda root: tmp_path / "build/cmake")
    monkeypatch.setattr(
        ds,
        "batch_build",
        lambda root, targets: subprocess.CompletedProcess([], 1, "", "build error"),
    )
    monkeypatch.setattr(ds, "index_coverage", lambda _root, _manifests: {})

    def diff(request):
        diff_calls.append(request.source_path.stem)
        exact = request.source_path.stem == "func_80100010"
    report = ds.build_report(tmp_path, use_cache=False, diff_runner=diff)

    # Both sources via per-source fallback, no duplicate
    assert sorted(diff_calls) == ["func_80100010", "func_80100020"]
    assert report["lifts"] == {"exact": 1, "partial": 1, "invalid": 0, "total": 2}


def test_no_batch_when_all_sources_are_cached(tmp_path: Path, monkeypatch) -> None:
    """Phase 2.3.2: all-cache-hit target must not invoke batch build."""
    _target(tmp_path, "exe/logo", "src/exe/logo")
    _source(tmp_path, "src/exe/logo", "80100010")
    binary = tmp_path / "out/binaries" / "exe/logo.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(b"test")

    from harness.match.status_cache import StatusCache

    cache = StatusCache(tmp_path)
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

    from harness import decomp_status as ds

    monkeypatch.setattr(ds, "source_fingerprint", lambda _s, _t: "fake-fp")
    monkeypatch.setattr(ds, "target_fingerprint", lambda _r, _m: "fake-fp")
    monkeypatch.setattr(
        ds,
        "index_coverage",
        lambda _root, _manifests: {},
    )

    batch_calls: list[list[str]] = []

    def track_batch(root, targets):
        batch_calls.append(targets)
        raise RuntimeError("should not be called")

    monkeypatch.setattr(ds, "batch_build", track_batch)

    report = ds.build_report(
        tmp_path, ("exe/logo",), use_cache=True, diff_runner=lambda r: {}
    )

    assert batch_calls == [], "batch_build was called despite all cache hits"
    assert report["lifts"]["total"] == 1
    assert report["lifts"]["exact"] == 1
    cache.close()


def test_source_change_invalidates_cache_and_recomputes(
    """Phase 2.3.3: source change invalidates cache, produces one batch build."""
    _source(tmp_path, "src/exe/logo", "80100020")
    binary = tmp_path / "out/binaries" / "exe/logo.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(b"test")

    from harness import decomp_status as ds
    from harness.match.status_cache import StatusCache

    cache = StatusCache(tmp_path)
    import hashlib
    monkeypatch.setattr(ds, "source_fingerprint", lambda s, t: hashlib.sha256(s.read_bytes()).hexdigest()[:32])
    monkeypatch.setattr(ds, "target_fingerprint", lambda r, m: "fp-target")
    monkeypatch.setattr(ds, "index_coverage", lambda _root, _manifests: {})

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

    monkeypatch.setattr(ds, "configure", lambda root: tmp_path / "build/cmake")
    monkeypatch.setattr(ds, "batch_build", batch)
    monkeypatch.setattr(ds, "_asm_diff_resolve", resolve)
    monkeypatch.setattr(ds, "_asm_diff_compare", lambda repo, rq, rs: _batch_result())

    # First run — cache miss for both sources
    first = ds.build_report(tmp_path, use_cache=True, diff_runner=lambda r: _batch_result())
    assert first["lifts"] == {"exact": 2, "partial": 0, "invalid": 0, "total": 2}
    assert len(batch_calls) == 1, "first run: one batch build"

    # Second run — both cached, no build
    batch_calls.clear()
    second = ds.build_report(tmp_path, use_cache=True, diff_runner=lambda r: _batch_result())
    assert second["lifts"] == first["lifts"]
    assert len(batch_calls) == 0, "second run: no batch build (all cached)"

    # Change one source
    src = tmp_path / "src/exe/logo/func_80100010.c"
    src.write_text("// @source 0x80100010\n// @behavior changed\n")

    # Third run — only changed source recomputed; one build for that target
    batch_calls.clear()
    third = ds.build_report(tmp_path, use_cache=True, diff_runner=lambda r: _batch_result())
    assert third["lifts"] == {"exact": 2, "partial": 0, "invalid": 0, "total": 2}
    assert len(batch_calls) == 1, "source change: one batch"

    cache.close()


def test_compile_inputs_invalidate_only_affected_target_then_all_targets(
    """Target inputs invalidate one target; shared headers invalidate both."""
    _target(tmp_path, "exe/other", "src/exe/other")
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

    from harness import decomp_status as ds

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

    monkeypatch.setattr(ds, "configure", lambda root: tmp_path / "build/cmake")
    monkeypatch.setattr(ds, "batch_build", batch)
    monkeypatch.setattr(ds, "_asm_diff_resolve", resolve)
    monkeypatch.setattr(ds, "_asm_diff_compare", lambda repo, req, resolved: _batch_result())
    monkeypatch.setattr(ds, "index_coverage", lambda _root, _manifests: {})

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
