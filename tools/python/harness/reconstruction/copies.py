"""Bind observed RAM instruction windows to original BIOS bytes and capture scope."""

from __future__ import annotations

from collections import Counter
from dataclasses import dataclass
import hashlib
import json
from pathlib import Path
import re
import struct
from types import MappingProxyType

from harness.common.artifacts import encode_json, read_bytes
from harness.runtime.origin import decode_object


LIMITS = {
    "bios.bin": 512 << 10,
    "receipt.json": 2 << 20,
    "history.json": 64 << 10,
    "events.json": 1 << 20,
    "events.ndjson": 64 << 20,
    "payload.bin": 8 << 20,
    "observations.json": 64 << 10,
}
SCOPE_FIELDS = {
    "receipt_sha256",
    "caller_pc",
    "entry_pc",
    "stop_pc",
    "first_instruction_sequence",
    "last_instruction_sequence",
    "begin_cycle",
    "end_cycle",
}


class CopyRefused(ValueError):
    """The supplied instruction window does not retain its observed provenance."""


@dataclass(frozen=True)
class CopyObservation:
    descriptor: dict
    files: MappingProxyType


def _require(value, message):
    if not value:
        raise CopyRefused(message)


def _hash(raw):
    return hashlib.sha256(raw).hexdigest()


def _json(raw):
    return json.loads(raw, object_pairs_hook=decode_object)


def _digest(value):
    return type(value) is str and re.fullmatch(r"[0-9a-f]{64}", value) is not None


def _integer(value, first, last):
    return type(value) is int and first <= value <= last


def _cycle(value):
    _require(
        type(value) is str and re.fullmatch(r"0|[1-9][0-9]{0,19}", value),
        "copy cycle is not an exact decimal",
    )
    result = int(value)
    _require(result < 1 << 64, "copy cycle exceeds uint64")
    return result


def _canonical(address):
    _require(
        _integer(address, 0, 0xFFFFFFFF)
        and address & 0xE0000000 in (0, 0x80000000, 0xA0000000)
        and address & 0x1FFFFFFF < 0x200000,
        "copy address is outside standard RAM",
    )
    return address & 0x1FFFFFFF | 0x80000000


def descriptor_bytes(descriptor):
    return encode_json(descriptor, path="copied-original descriptor")


def validate_descriptor(value):
    _require(
        type(value) is dict
        and set(value)
        == {
            "kind",
            "source_offset",
            "bytes",
            "destination",
            "entry",
            "sha256",
            "scope",
            "files",
        },
        "copy descriptor shape differs",
    )
    length, offset = value["bytes"], value["source_offset"]
    _require(
        value["kind"] == "observed-original-copy/v1"
        and _integer(length, 4, 64 << 10)
        and length % 4 == 0
        and _integer(offset, 0, 512 << 10)
        and offset % 4 == 0
        and offset + length <= 512 << 10,
        "copy original extent differs",
    )
    destination = value["destination"]
    _require(
        _canonical(destination) == destination
        and destination % 4 == 0
        and destination + length <= 0x80200000
        and value["entry"] == destination
        and _digest(value["sha256"]),
        "copy instruction range differs",
    )
    scope = value["scope"]
    _require(
        type(scope) is dict
        and set(scope) == SCOPE_FIELDS
        and _digest(scope["receipt_sha256"]),
        "copy scope shape differs",
    )
    for name in ("caller_pc", "entry_pc", "stop_pc"):
        _require(
            _integer(scope[name], 0, 0xFFFFFFFF) and scope[name] % 4 == 0,
            "copy marker address differs",
        )
        _canonical(scope[name])
    _require(
        _canonical(scope["entry_pc"]) == value["entry"]
        and _integer(scope["first_instruction_sequence"], 1, 65536)
        and _integer(
            scope["last_instruction_sequence"],
            scope["first_instruction_sequence"],
            65536,
        )
        and _cycle(scope["begin_cycle"]) < _cycle(scope["end_cycle"]),
        "copy instruction scope differs",
    )
    _require(
        type(value["files"]) is dict and set(value["files"]) == set(LIMITS),
        "copy file inventory differs",
    )
    for name, limit in LIMITS.items():
        row = value["files"][name]
        _require(
            type(row) is dict
            and set(row) == {"path", "bytes", "sha256"}
            and row["path"] == f"input/copy-{name}"
            and _integer(row["bytes"], 1, limit)
            and _digest(row["sha256"]),
            "copy file pin differs",
        )
    _require(
        value["files"]["bios.bin"]["bytes"] == 512 << 10
        and value["files"]["receipt.json"]["sha256"] == scope["receipt_sha256"],
        "copy BIOS or receipt identity differs",
    )
    return value


def validate_files(descriptor, files):
    validate_descriptor(descriptor)
    _require(
        type(files) in (dict, MappingProxyType) and set(files) == set(LIMITS),
        "copy captures missing",
    )
    for name, pin in descriptor["files"].items():
        raw = files[name]
        _require(
            type(raw) is bytes
            and len(raw) == pin["bytes"]
            and _hash(raw) == pin["sha256"],
            "copy capture bytes differ",
        )
    receipt, history, metadata, observations = (
        _json(files[name])
        for name in ("receipt.json", "history.json", "events.json", "observations.json")
    )
    _require(
        receipt.get("schema") == "psx.runtime-session/v2"
        and receipt.get("status") == "passed"
        and receipt.get("origin") is None
        and receipt["process"]["exit_code"] == 0
        and receipt["process"]["failure"] is None,
        "copy needs a successful original-only execution",
    )
    inputs = receipt["inputs"]
    _require(
        "bios" in inputs
        and "disc:cue" in inputs
        and all(
            name == "bios" or name == "script" or name.startswith("disc:")
            for name in inputs
        )
        and not receipt.get("assets"),
        "copy execution contains injected or restored inputs",
    )
    bios_pin = descriptor["files"]["bios.bin"]
    _require(
        all(inputs["bios"][key] == bios_pin[key] for key in ("bytes", "sha256")),
        "copy BIOS differs from executed original",
    )
    captures = receipt["completion"]["captures"]
    _require(
        receipt["completion"]["schema"] == "psx.runtime-completion/v1"
        and type(captures) is list
        and len({row["path"] for row in captures}) == len(captures),
        "copy capture completion differs",
    )
    for name in set(LIMITS) - {"bios.bin", "receipt.json"}:
        pin = descriptor["files"][name]
        matches = [row for row in captures if row["path"] == name]
        _require(
            len(matches) == 1
            and all(matches[0][key] == pin[key] for key in ("bytes", "sha256")),
            "copy capture is not receipt-bound",
        )
    _require(
        history.get("schema") == "psx.runtime-history/v2"
        and history.get("success") is True
        and history.get("started") is True
        and history.get("native_complete") is True
        and history.get("profile") == "ram-interval-v1"
        and history.get("reason") == "stop reached"
        and _integer(history.get("frames"), 0, history.get("frame_limit", -1))
        and _integer(history.get("frame_limit"), 1, 36000),
        "copy history did not reach its stop within guest bounds",
    )
    length, start = descriptor["bytes"], descriptor["destination"] & 0x1FFFFFFF
    _require(
        history.get("ram_start") == start
        and history.get("ram_length") == length
        and metadata.get("schema") == "psx.native-history/v2"
        and metadata.get("complete") is True
        and metadata.get("failure_bits") == 0
        and metadata.get("failures") == []
        and metadata.get("dropped_attempts") == 0,
        "copy RAM interval is incomplete",
    )
    scope = descriptor["scope"]
    _require(
        metadata["begin_cycle"] == scope["begin_cycle"]
        and metadata["end_cycle"] == scope["end_cycle"]
        and history.get("stop") == scope["stop_pc"]
        and metadata.get("build_identity") == history.get("build_identity")
        and metadata["build_identity"]["binary_sha256"]
        == receipt["emulator"]["binary_sha256"],
        "copy interval identity differs",
    )
    rows = [_json(line) for line in files["events.ndjson"].splitlines()]
    payload = files["payload.bin"]
    _require(
        2 <= len(rows) <= 65536
        and metadata["events"] == len(rows) <= history["event_limit"]
        and metadata["payload_bytes"] == len(payload) <= history["byte_limit"]
        and metadata["kinds"] == dict(Counter(row["kind"] for row in rows)),
        "copy event inventory differs",
    )
    bios = files["bios.bin"]
    offset = descriptor["source_offset"]
    original = bios[offset : offset + length]
    _require(_hash(original) == descriptor["sha256"], "copy original slice differs")
    first, last = rows[0], rows[-1]
    _require(
        first["kind"] == last["kind"] == "ram-context"
        and first["fields"]["phase"] == "begin"
        and last["fields"]["phase"] == "end"
        and first["cycle"] == scope["begin_cycle"]
        and last["cycle"] == scope["end_cycle"],
        "copy RAM boundaries differ",
    )
    image = None
    covered, instructions, writes = set(), [], []
    previous_cycle = _cycle(scope["begin_cycle"])
    for number, row in enumerate(rows, 1):
        _require(
            row["sequence"] == str(number)
            and previous_cycle <= _cycle(row["cycle"]) <= _cycle(scope["end_cycle"]),
            "copy event ordering differs",
        )
        previous_cycle = _cycle(row["cycle"])
        fields, span = row["fields"], row["payload"]
        _require(
            _integer(span["offset"], 0, len(payload))
            and _integer(span["bytes"], 0, len(payload) - span["offset"]),
            "copy event payload escapes",
        )
        data = payload[span["offset"] : span["offset"] + span["bytes"]]
        if row["kind"] == "ram-context":
            _require(
                fields["first"] == start
                and fields["byte_count"] == length
                and fields["setting_8mb"] == 0,
                "copy RAM mapping extent differs",
            )
            if number == 1:
                _require(len(data) == length, "copy initial image missing")
                image = bytearray(data)
            elif number == len(rows):
                _require(
                    len(data) == length and data == bytes(image) == original,
                    "copy final reconciliation differs",
                )
            else:
                _require(not data, "copy unexpected boundary image")
        elif row["kind"] == "ram-write":
            _require(
                image is not None
                and not instructions
                and fields["writer"] == "cpu-handler"
                and fields["store_bytes"] == fields["byte_count"] == len(data) == 4
                and fields["byte_mask"] == 15,
                "copy contains unsupported or subsequent writes",
            )
            source_pc = fields["instruction_pc"]
            _require(
                _integer(source_pc, 0xBFC00000, 0xBFC7FFFC)
                and source_pc % 4 == 0
                and fields["instruction_word"]
                == struct.unpack_from("<I", bios, source_pc - 0xBFC00000)[0],
                "copy writer is not an original BIOS instruction",
            )
            index = fields["backing_offset"] - start
            _require(
                _integer(index, 0, length - 4)
                and index % 4 == 0
                and _canonical(fields["address"]) == descriptor["destination"] + index
                and data == original[index : index + 4],
                "copy committed bytes differ from original",
            )
            image[index : index + 4] = data
            covered.update(range(index, index + 4))
            writes.append(row)
        elif row["kind"] == "ram-instruction":
            index = fields["backing_offset"] - start
            _require(
                image is not None
                and len(covered) == length
                and _integer(index, 0, length - 4)
                and index % 4 == 0
                and _canonical(fields["pc"]) == descriptor["destination"] + index
                and fields["consumed_word"]
                == fields["backing_word"]
                == struct.unpack_from("<I", image, index)[0]
                == struct.unpack_from("<I", original, index)[0],
                "copy consumed instruction differs from original backing",
            )
            instructions.append(row)
        else:
            raise CopyRefused("copy contains another observation profile")
    _require(
        instructions
        and writes
        and instructions[0]["sequence"] == str(scope["first_instruction_sequence"])
        and instructions[-1]["sequence"] == str(scope["last_instruction_sequence"])
        and instructions[0]["fields"]["pc"] == scope["entry_pc"]
        and {row["fields"]["backing_offset"] for row in instructions}
        == set(range(start, start + length, 4)),
        "copy observed instruction scope differs",
    )
    _require(
        observations.get("schema") == "bof3.original-handler-interval/v1"
        and type(observations.get("exclusions")) is str
        and observations["exclusions"]
        and type(observations.get("controller")) is str
        and observations["controller"],
        "copy mission assumptions are absent",
    )
    caller, entry, final = (
        observations[name]
        for name in ("caller_entry", "handler_entry", "final_registers")
    )
    for marker, pc in (
        (caller, scope["caller_pc"]),
        (entry, scope["entry_pc"]),
        (final, scope["stop_pc"]),
    ):
        _require(
            marker["schema"] == "psx.runtime-registers/v1"
            and marker["pc"] == pc
            and type(marker["registers"]) is list
            and len(marker["registers"]) == 32
            and all(_integer(word, 0, 0xFFFFFFFF) for word in marker["registers"]),
            "copy observation marker differs",
        )
    for marker in (caller, entry):
        sequence = marker["last_retained_event"]
        _require(
            _integer(sequence, 1, len(rows))
            and _integer(marker["cycles"], 0, (1 << 64) - 1),
            "copy marker count or cycle differs",
        )
        _require(
            _cycle(rows[sequence - 1]["cycle"]) <= marker["cycles"]
            and (
                sequence == len(rows)
                or marker["cycles"] <= _cycle(rows[sequence]["cycle"])
            ),
            "copy marker contradicts retained event cycles",
        )
    _require(
        _cycle(scope["begin_cycle"])
        <= caller["cycles"]
        < entry["cycles"]
        == _cycle(instructions[0]["cycle"])
        <= final["cycles"]
        == _cycle(scope["end_cycle"])
        and int(writes[-1]["sequence"])
        <= caller["last_retained_event"]
        <= entry["last_retained_event"]
        < scope["first_instruction_sequence"]
        and entry["registers"][31] == scope["stop_pc"],
        "copy use markers do not bracket the admitted execution",
    )
    return original


def capture_copy(receipt_path, source_offset):
    from harness.runtime.session import inspect_runtime_capture

    receipt_path = Path(receipt_path).resolve(strict=True)
    observed = inspect_runtime_capture(receipt_path.parent, read_bytes)
    _require(observed.status == "matched", "copy runtime ownership inspection failed")
    receipt_raw = read_bytes(receipt_path, LIMITS["receipt.json"])
    receipt = _json(receipt_raw)
    files = {
        name: read_bytes(receipt_path.parent / name, limit)
        for name, limit in LIMITS.items()
        if name not in {"bios.bin", "receipt.json"}
    }
    files.update(
        {
            "receipt.json": receipt_raw,
            "bios.bin": read_bytes(
                Path(receipt["inputs"]["bios"]["path"]), LIMITS["bios.bin"]
            ),
        }
    )
    history, metadata, observations = (
        _json(files[name])
        for name in ("history.json", "events.json", "observations.json")
    )
    instructions = [
        row
        for row in (_json(line) for line in files["events.ndjson"].splitlines())
        if row["kind"] == "ram-instruction"
    ]
    _require(instructions, "copy history has no consumed instructions")
    length, start = history["ram_length"], history["ram_start"]
    _require(
        _integer(source_offset, 0, len(files["bios.bin"]))
        and _integer(length, 4, 64 << 10),
        "copy source extent differs",
    )
    descriptor = dict(
        kind="observed-original-copy/v1",
        source_offset=source_offset,
        bytes=length,
        destination=_canonical(start),
        entry=_canonical(observations["handler_entry"]["pc"]),
        sha256=_hash(files["bios.bin"][source_offset : source_offset + length]),
        scope=dict(
            receipt_sha256=_hash(receipt_raw),
            caller_pc=observations["caller_entry"]["pc"],
            entry_pc=observations["handler_entry"]["pc"],
            stop_pc=observations["final_registers"]["pc"],
            first_instruction_sequence=int(instructions[0]["sequence"]),
            last_instruction_sequence=int(instructions[-1]["sequence"]),
            begin_cycle=metadata["begin_cycle"],
            end_cycle=metadata["end_cycle"],
        ),
        files={
            name: dict(path=f"input/copy-{name}", bytes=len(raw), sha256=_hash(raw))
            for name, raw in files.items()
        },
    )
    validate_files(descriptor, files)
    return CopyObservation(descriptor, MappingProxyType(files))


def read_copy(descriptor, directory, *, reader=None):
    if descriptor is None:
        return None
    validate_descriptor(descriptor)
    files = {
        name: (reader or read_bytes)(Path(directory) / f"copy-{name}", pin["bytes"], [])
        for name, pin in descriptor["files"].items()
    }
    validate_files(descriptor, files)
    return CopyObservation(descriptor, MappingProxyType(files))
