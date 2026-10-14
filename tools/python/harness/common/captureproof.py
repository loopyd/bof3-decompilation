"""Verify retained capture records and files without executing a process."""

from __future__ import annotations

from collections.abc import Callable, Mapping, Sequence
import hashlib
import json
import math
from pathlib import Path
from typing import Literal


class CaptureProofRefused(ValueError):
    pass


def _read(path, limit, events, reader=None):
    local, stream, data = [], None, None
    try:
        regular = not path.is_symlink() and path.is_file()
    except Exception as error:
        local.append(("ordinary", "capture-stat", str(path), error))
        regular = False
    if not local and not regular:
        raise CaptureProofRefused("capture file is absent or invalid")
    if not local:
        try:
            if reader is None:
                stream = path.open("rb")
                data = stream.read(limit + 1)
            else:
                data = reader(path, limit, local)
        except Exception as error:
            if not any(row[3] is error for row in local):
                local.append(("ordinary", "capture-read", str(path), error))
    if stream is not None:
        try:
            stream.close()
        except Exception as error:
            local.append(("ordinary", "capture-close", str(path), error))
    if events is not None:
        events.extend(
            (
                origin,
                {"read": "capture-read", "close": "capture-close"}.get(phase, phase),
                path,
                error,
            )
            for origin, phase, path, error in local
        )
    if local:
        raise local[0][3]
    return data


def verify_capture(
    record: dict,
    cwd: Path,
    argv: Sequence[str | Path],
    *,
    environment: Mapping[str, str],
    output_limit: int,
    name: str,
    origin: Literal["direct", "persisted"],
    events: list[tuple[str, str, str, Exception]] | None = None,
    reader: Callable[[Path, int, list[tuple[str, str, str, Exception]] | None], bytes]
    | None = None,
) -> Literal["observed"]:
    if type(origin) is not str or origin not in {"direct", "persisted"}:
        raise CaptureProofRefused("capture record origin differs")
    expected_argv = argv if origin == "direct" else [str(item) for item in argv]
    expected = dict(
        argv=expected_argv,
        cwd=str(cwd),
        environment=environment,
        raw_exit_owner="common.process owned supervisor",
    )
    extra = set(
        "exit_code raw_exit failure owner_observation capture_complete streams".split()
    )
    if (
        type(record) is not dict
        or set(record) != set(expected) | extra
        or any(record[key] != value for key, value in expected.items())
        or type(record["argv"]) is not type(expected_argv)
        or type(record["cwd"]) is not str
        or type(record["environment"]) is not dict
        or any(
            type(key) is not str or type(value) is not str
            for key, value in record["environment"].items()
        )
        or type(record["raw_exit_owner"]) is not str
        or type(record["capture_complete"]) is not bool
        or record["failure"] is not None
        and type(record["failure"]) is not str
        or record["capture_complete"] != (record["failure"] is None)
        or type(record["streams"]) is not dict
        or set(record["streams"]) != {"stdout", "stderr"}
    ):
        raise CaptureProofRefused("capture identity or completion proof differs")
    raw_streams = {}
    for channel, identity in record["streams"].items():
        path = cwd / f"{name}.{channel}.bin"
        if (
            type(identity) is not dict
            or set(identity) != {"path", "bytes", "sha256"}
            or type(identity["bytes"]) is not int
            or type(identity["path"]) is not str
            or type(identity["sha256"]) is not str
        ):
            raise CaptureProofRefused("capture stream identity differs")
        raw_streams[channel] = _read(path, output_limit, events, reader)
        raw = raw_streams[channel]
        if identity != dict(
            path=str(path), bytes=len(raw), sha256=hashlib.sha256(raw).hexdigest()
        ):
            raise CaptureProofRefused("capture stream bytes differ")
    if sum(map(len, raw_streams.values())) > output_limit:
        raise CaptureProofRefused("capture streams exceed aggregate bound")
    owner = record["owner_observation"]
    if owner is not None:
        observed = dict(
            argv=[str(item) for item in argv],
            **{
                channel: raw.decode("utf-8", "replace")
                for channel, raw in raw_streams.items()
            },
        )
        if (
            type(owner) is not dict
            or set(owner) != set(observed) | {"failure", "pid", "seconds"}
            or any(owner[key] != value for key, value in observed.items())
            or type(owner["argv"]) is not list
            or any(type(item) is not str for item in owner["argv"])
            or any(type(owner[channel]) is not str for channel in raw_streams)
            or type(owner["pid"]) is not int
            or owner["pid"] <= 0
            or type(owner["seconds"]) not in {int, float}
            or not math.isfinite(owner["seconds"])
            or owner["seconds"] < 0
            or owner["failure"] is not None
            and type(owner["failure"]) is not str
        ):
            raise CaptureProofRefused("capture owner observation differs")
    limit = (6 * output_limit if owner is not None else 0) + (64 << 10)
    try:
        encoded = (
            json.dumps(record, sort_keys=True, indent=2, allow_nan=False).encode()
            + b"\n"
        )
    except Exception as error:
        if events is not None:
            events.append(
                ("ordinary", "capture-serialize", str(cwd / f"{name}.json"), error)
            )
        raise
    path = cwd / f"{name}.json"
    if len(encoded) > limit:
        raise CaptureProofRefused("capture receipt exceeds bound or is invalid")
    raw = _read(path, limit, events, reader)
    if raw != encoded:
        raise CaptureProofRefused("capture receipt differs")
    if (
        owner is not None
        and record["raw_exit"] is record["exit_code"] is None
        and not record["capture_complete"]
    ):
        return "observed"
    if (
        owner is None
        and type(record["raw_exit"]) is type(record["exit_code"]) is int
        and record["raw_exit"] == record["exit_code"]
    ):
        return "observed"
    raise CaptureProofRefused("capture launch proof is unknown")
