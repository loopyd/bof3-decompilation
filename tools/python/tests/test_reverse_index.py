import json
def test_status_rejects_pre_jal_snapshot_schema(tmp_path: Path) -> None:
    snapshot = tmp_path / "out/reverse/emi/test/archive/00/snapshot.json"
    assert status(tmp_path, TARGET)["fresh"] is True

    payload = json.loads(snapshot.read_text(encoding="utf-8"))
    payload["schema"] = "bof3.analysis-snapshot/v2"
    snapshot.write_text(json.dumps(payload), encoding="utf-8")
