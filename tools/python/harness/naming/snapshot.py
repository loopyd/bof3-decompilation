"""Pinned PRE snapshots for reviewed naming identity transactions."""

from __future__ import annotations

import hashlib
import stat
from pathlib import Path

from harness.common.deadlines import check_deadline
from harness.common.files import atomic_write
from harness.common.git import capture_git_index
from harness.common.inputs import file_state
from harness.common.inventory import CaptureBudget, capture_file
from harness.common.lease import acquire_writer, require_writer
from harness.naming.application import collect_function_checks, directory
from harness.naming.audit import validate
from harness.naming.editing import report_mutation
from harness.naming.history import (
    compute_sha256,
    encode,
    read_snapshot_states,
    read_state,
    require_pin,
)
from harness.naming.inputs import frozen, state, transaction_paths
from harness.naming.proposal import canonical_report_path


def create_snapshot(
    root: Path,
    target: str,
    report_path: Path,
    transaction: str,
    *,
    expected_report_sha256: str,
) -> Path:
    """Capture exact validated PRE files without editing source or report bytes."""
    root = root.resolve()
    path = canonical_report_path(root, report_path)
    require_pin(expected_report_sha256)
    with acquire_writer(root), report_mutation(path, report=True):
        report, row, binding = frozen(root, target, path, transaction)
        if binding["report_state"]["sha256"] != expected_report_sha256:
            raise ValueError("snapshot report differs from expected SHA-256")
        baseline = state(root, target, row)
        index = capture_git_index(root)
        if index is None or index.content is None:
            raise ValueError("naming snapshot requires an existing Git index")
        validate(root, target, report, transaction=transaction, report_path=path)
        if row["kind"] == "function":
            collect_function_checks(root, row)
        budget = CaptureBudget()
        names = transaction_paths(row)
        observations = {
            name: capture_file(root, name, budget, missing_ok=True)
            for name in sorted(names)
        }
        facts = row["pre_apply"]["facts"]
        destination = facts["destination"]
        for name, (metadata, _) in observations.items():
            missing = name == destination and name != facts["scope"]["definition"]
            if (metadata is None) != missing:
                raise ValueError(f"naming snapshot is not the expected PRE: {name}")
            if row["kind"] == "data" and (
                metadata["sha256"] != facts["data"]["files"][name]["before"]
                or stat.S_IMODE(metadata["mode"])
                != facts["data"]["files"][name]["mode"]
            ):
                raise ValueError(f"data snapshot differs from prepared PRE: {name}")
        frozen_files = {
            name: capture_file(root, name, budget)
            for name in [binding["report"], *facts["reviewed"]]
        }
        if frozen_files[binding["report"]][0]["sha256"] != expected_report_sha256:
            raise ValueError("snapshot report changed during capture")
        output = directory(root)
        files = []
        for name, (metadata, content) in observations.items():
            item = {"path": name, "exists": metadata is not None}
            if metadata is not None:
                mode = stat.S_IMODE(metadata["mode"])
                atomic_write(output, name, content, exclusive=True, mode=mode)
                item.update(sha256=metadata["sha256"], mode=mode)
            files.append(item)
        pinned = {
            name: metadata["sha256"] for name, (metadata, _) in frozen_files.items()
        }
        pinned[".git/index"] = hashlib.sha256(index.content).hexdigest()
        payload = {"files": files, "frozen": pinned}

        def verify_inputs() -> None:
            for name, expected in {**observations, **frozen_files}.items():
                if capture_file(root, name, budget, missing_ok=True) != expected:
                    raise ValueError(f"naming PRE changed during snapshot: {name}")
            if (
                state(root, target, row) != baseline
                or capture_git_index(root) != index
                or file_state(path) != binding["report_state"]
            ):
                raise ValueError("naming inputs changed during snapshot")
            require_writer(root)
            check_deadline()

        for item in files:
            expected = (
                {"sha256": item["sha256"], "mode": item["mode"]}
                if item["exists"]
                else None
            )
            if file_state(output / item["path"]) != expected:
                raise ValueError("retained PRE changed before manifest publication")
        verify_inputs()
        content = encode(payload)
        atomic_write(output, "snapshot.json", content, exclusive=True)
        result = output / "snapshot.json"
        read_snapshot_states(result)
        verify_inputs()
        if read_state(result)["sha256"] != compute_sha256(content):
            raise ValueError("published snapshot differs from captured PRE")
        require_writer(root)
        check_deadline()
        return result
