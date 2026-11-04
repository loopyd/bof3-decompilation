"""Repack exactly three declared C08 replicas from accepted primary payloads."""
from contextlib import ExitStack
from dataclasses import asdict
import hashlib
import json
from pathlib import Path
from harness.build.payload import _create_output, _repack, _verify_pins
from harness.common.files import atomic_write
from harness.common.inputs import InputBatch
from harness.common.lease import acquire_writer
from harness.common.process import ProcessCleanupError
from harness.domain.manifests import _parse_manifest
from harness.emi.catalog_verify import resolve_shipped_entry
from harness.match.extraction import read_function_bytes

root = Path(__file__).resolve().parents[2]
assert Path.cwd() == root
sha = lambda body: hashlib.sha256(body).hexdigest()
LIMIT = 64 * 1024 * 1024
OWNERS = {'tools/python/harness/build/payload.py': '4d1d5c49f3db01e8d7ecb97c6943600cb94a0304df38002cba0f961938b1d72f', 'tools/python/harness/emi/catalog_verify.py': 'ebc0f016e49af6eff4c781dad3b2fb710e51d2d12592fe2ef336e037f5b04e94', 'tools/python/harness/common/inputs.py': 'c889678879d896aa5f98d79575d9868027f4a219c8d0543cac922d75a869fba9', 'tools/python/harness/common/lease.py': '2293c4c2b1446bac858cea9f969f19ab9176764349200565ce18505aec004c99', 'tools/python/harness/common/process.py': '16f7c08e1c6f0c1b7f87537047bb1ce36139e8d4a5c140d51c45ff8f27bbe1e3'}
PRIMARY = {
    "boss014": "146f758270ce4459b89e1b9a22f4ed3a1124ef882709af98bc325bce2b3f0d26",
    "boss018": "e9f1c97b801c6f2a2d0720f66bacd38b06520d79beca37d6b232d5e3260b1475",
}
REPLICAS = (("boss014", "BIN/BOSS/BOSS046.EMI#16", "c08-replica-boss046-16"), ("boss018", "BIN/BOSS/BOSS019.EMI#16", "c08-replica-boss019-16"), ("boss018", "BIN/BOSS/BOSS020.EMI#16", "c08-replica-boss020-16"))
evidence = root / "out/source-refinement/c08-replica-repack"
assert not evidence.exists() and all(not (root / "out/payloads" / name).exists() for _, _, name in REPLICAS)
evidence.mkdir()
summary = {"status": "running", "replicas": [], "new_function_credit": 0, "new_source_byte_credit": 0}
def save(path, value):
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")
save(evidence / "summary.json", summary)
try:
    with acquire_writer(root), ExitStack() as stack:
        batch = stack.enter_context(InputBatch(root))
        states, pins = {}, {}
        def bind(name, digest=None, expected=None):
            path = Path(name)
            if path.is_absolute():
                name = path.relative_to(root).as_posix()
            state, body = batch.read(root / name, max_bytes=LIMIT, include_metadata=True, single_link=True)
            assert name not in states or states[name] == state, name
            if digest is not None:
                assert state is not None and state["sha256"] == digest, name
            if expected is not None:
                assert state == expected, name
            states[name] = state
            pins[name] = None if state is None else state["sha256"]
            return body
        def guard():
            for name in list(states):
                bind(name)
            _verify_pins(batch, pins)
        bind(Path(__file__).resolve())
        for name, digest in OWNERS.items():
            bind(name, digest)
        primaries = {}
        for boss, report_sha in PRIMARY.items():
            report_path = f"out/payloads/c08-emi-boss-{boss}-16/report.json"
            report = json.loads(bind(report_path, report_sha))
            target = f"emi/boss/{boss}/16"
            assert report["schema"] == "harness.build-payload/v1" and report["target"] == target and report["whole_payload_equal"]
            for name, digest in report["inputs"].items():
                body = bind(name, digest)
                assert (body is None) == (digest is None), name
            manifest_name = f"config/targets/{target}/target.toml"
            manifest = _parse_manifest(root / manifest_name, bind(manifest_name))
            payload = bind(report["output"], report["sha256"])
            assert payload == bind(report["original"]) == bind(manifest.binary) and len(payload) == report["size"]
            intervals = sorted([*report["compiled"], *report["retained"]], key=lambda row: row["start"])
            assert intervals[0]["start"] == 0 and intervals[-1]["end"] == len(payload)
            assert all(a["end"] == b["start"] and a["start"] < a["end"] for a, b in zip(intervals, intervals[1:]))
            assert report["compiled_bytes"] + report["retained_bytes"] == len(payload)
            for item in report["source_comparisons"]:
                comparison = item["comparison"]
                assert comparison["fresh_compilation"] and comparison["byte_match"] and comparison["exact_match"] and comparison["size_delta"] == 0
                for key in ("native_inputs", "include_inputs", "native_outputs"):
                    for name, state in comparison[key].items():
                        bind(name, None if state is None else state["sha256"], state)
                        assert states[name] == state, name
                elf = bind(Path(comparison["current_object"]).with_suffix(".linked.o"))
                raw = read_function_bytes(elf, function_name=comparison["function"], address=int(comparison["address"], 16)).content
                matches = [r for r in report["compiled"] if r["source"] == item["source"] and r["symbol"] == comparison["function"]]
                assert len(matches) == 1
                interval = matches[0]
                assert raw == payload[interval["start"]:interval["end"]] and sha(raw) == interval["sha256"] == comparison["function_image"]["current_sha256"] == comparison["function_image"]["original_sha256"]
            archive = report["archive"]
            assert archive["whole_archive_equal"] and archive["entry_equal"]
            assert bind(archive["output"], archive["sha256"]) == bind(archive["original"])
            assert bind(str(Path(report["output"]).parent / "entry.bin")) == payload
            primaries[boss] = (manifest, report_path, report, payload)
        prepared = []
        for boss, disc_id, name in REPLICAS:
            manifest, report_path, report, payload = primaries[boss]
            declarations = [r for r in manifest.replicas if r.disc_id == disc_id]
            assert len(declarations) == 1
            declared = declarations[0]
            entry = resolve_shipped_entry(root, disc_id)
            bind(entry["manifest_path"])
            original_archive = bind(entry["archive_path"])
            original = bind(entry["payload_path"])
            assert resolve_shipped_entry(root, disc_id) == entry and entry["payload_kind"] == "ram" and entry["index"] == 16
            assert entry["load_address"] == declared.load_address == manifest.load_address == 0x800C1800
            assert entry["size"] == declared.size == len(payload) and entry["sha256"] == declared.payload_sha256 == sha(payload) and original == payload
            assert entry["entry"]["size"] == len(payload) and entry["entry"]["ram_ptr"] == manifest.load_address and entry["entry"]["type"] == 0
            prepared.append((boss, disc_id, name, declared, entry, original_archive))
        guard()
        save(evidence / "starting-input-states.json", states)
        for boss, disc_id, name, declared, entry, original_archive in prepared:
            guard()
            manifest, report_path, report, payload = primaries[boss]
            output = root / "out/payloads" / name
            _create_output(root, output)
            archive = _repack(root, output, entry, original_archive, root / report["output"], payload, batch, pins, stack)
            assert archive["tool_state"] == report["archive"]["tool_state"]
            rebuilt = bind(archive["output"], archive["sha256"])
            extracted = bind(output / "entry.bin")
            assert rebuilt == original_archive and extracted == payload
            summary["replicas"].append({"primary_target": manifest.id.value, "primary_report": report_path, "primary_report_sha256": PRIMARY[boss], "primary_payload": report["output"], "primary_payload_sha256": sha(payload), "declared_replica": asdict(declared), "replica_entry": entry, "archive": archive, "extracted_entry": str((output / "entry.bin").relative_to(root)), "entry_sha256": sha(extracted), "archive_size": len(rebuilt), "size": len(payload), "whole_archive_equal": True, "entry_equal": True, "new_function_credit": 0, "new_source_byte_credit": 0})
            guard()
            save(evidence / "summary.json", summary)
        def terminal(error_type, error, traceback):
            if error_type is None:
                guard()
        stack.push(terminal)
        guard()
        save(evidence / "terminal-input-states.json", states)
        save(evidence / "artifact-digests.json", pins)
    summary.update(status="three replica repacks complete; independent acceptance pending", guards_and_writer_closed=True)
    for row in summary["replicas"]:
        output = root / row["archive"]["output"]
        atomic_write(root, (output.parent / "report.json").relative_to(root).as_posix(), (json.dumps(row, indent=2, sort_keys=True) + "\n").encode(), exclusive=True)
    save(evidence / "summary.json", summary)
except BaseException as error:
    summary.update(status="failed; retain all outputs and fix cause", error=f"{type(error).__name__}: {error}")
    if isinstance(error, ProcessCleanupError):
        summary["cleanup"] = "unconfirmed; stop native work"
        summary["cleanup_observation"] = error.observed
    save(evidence / "summary.json", summary)
    raise
