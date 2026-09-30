"""Historical private integrity with real owner runs/Ninja and synthetic BOF3 payloads."""

import copy
import os
from pathlib import Path

import pytest
import test_application_review
from harness.common.execution import _evidence_paths
from harness.common.history import validate_reviewed_history

reviewed_run = test_application_review.reviewed_run


def test_private_history_preserves_proofs_without_accepting_current_drift(reviewed_run):
    root, owner, transactions, review, app, parent, environment = reviewed_run
    os.environ["PYTEST_CURRENT_TEST"] = environment
    envelope = review.review_application(root, app, parent, app["digest"])
    frozen = copy.deepcopy(envelope)

    def historical(value=envelope, pin=envelope["digest"]):
        return validate_reviewed_history(
            root, value, pin, owner, transactions._manifest
        )

    assert historical() is None  # No accepted/current/applied result surface.
    for name in ("unrelated.txt", "include/test.h", "build/cmake/build.ninja"):
        path = root / name
        before = path.read_bytes()
        path.write_bytes(before + b"\n/* later unreviewed work */\n")
        assert historical() is None
        with pytest.raises(ValueError):
            review.verify_reviewed_application(root, envelope, envelope["digest"])
        path.write_bytes(before)
    with pytest.raises(ValueError):
        historical(pin="v1:" + "0" * 64)
    with pytest.raises(ValueError):
        validate_reviewed_history(
            root, envelope, envelope["digest"], "wrong", transactions._manifest
        )

    # Every retained evidence path, not just a representative native receipt.
    paths = {root / name for name in _evidence_paths(app["manifest"])}
    paths.update(root / r["path"] for r in app["receipts"])
    paths.add(root / app["attestation"]["path"])
    paths.add(Path(parent["review_artifact"]["path"]))
    assert paths
    for path in sorted(paths):
        before, mode = path.read_bytes(), path.stat().st_mode
        path.write_bytes(b"replaced retained prerequisite\n")
        with pytest.raises(ValueError):
            historical()
        path.write_bytes(before)
        assert path.stat().st_mode == mode
        assert historical() is None

    for field in ("manifest", "receipts", "review_context", "attestation"):
        bad = copy.deepcopy(envelope)
        bad["application"][field] = {}
        with pytest.raises(ValueError):
            historical(bad)
    assert envelope == frozen
    assert review.verify_reviewed_application(root, envelope, envelope["digest"])[
        "accepted"
    ]
