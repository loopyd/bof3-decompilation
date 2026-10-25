"""Occurrence-scoped observed pointer loads from one authenticated RAM history.

The owner admits only an aligned full-word ``LW`` from ordinary RAM into a nonzero
GPR when the recorded dispatch slice itself supplies the base value. Every fact stays
keyed by capture, occurrence sequence, recorded path and RAM interval: a recorded
dispatch proves neither retirement nor a data-read result, so the effective address,
backing bytes and destination visibility are derived separately and published as
conditional ``psx-observed-pointer-load/v1`` evidence.

Nothing here completes an entry, result, control, call, declaration, effect, object,
qualifier, placement or universal ``currentContents`` contract, and no pointee value
or device access is inferred. It also never substitutes for the BIOS-copy descriptor
profile owned by :mod:`harness.reconstruction.copies`.
"""

from __future__ import annotations

from collections import Counter
from dataclasses import dataclass
import hashlib
import json
from pathlib import Path
import re
from types import MappingProxyType

from harness.common.artifacts import read_bytes
from harness.domain.mips import word_fields
from harness.runtime.origin import decode_object


CAPTURE_KIND = "psx-observed-pointer-load-capture/v1"
RECORD_KIND = "psx-observed-pointer-load/v1"
MAPPING = "ordinary-ram-v1"
RAM_BYTES = 0x200000
EVENT_FIELDS = frozenset(
    {
        "sequence",
        "kind",
        "cycle",
        "fields",
        "payload",
        "device",
        "related",
        "request",
        "raw",
    }
)
FILE_LIMITS = {
    "receipt.json": 2 << 20,
    "history.json": 64 << 10,
    "events.json": 1 << 20,
    "events.ndjson": 64 << 20,
    "payload.bin": 8 << 20,
}
CONDITIONS = (
    "authenticated-original-media-and-emulator-identity",
    "authenticated-state-boundary-at-recording-start",
    "complete-ordered-recorded-dispatch-slice",
    "interval-initial-image-equals-boundary-ram",
    "ordered-committed-writes-replayed-and-endpoints-reconciled",
    "no-selected-cell-write-in-the-recorded-interval",
    "address-and-backing-word-inside-the-captured-ordinary-ram-interval",
    "original-instruction-words-remain-active",
    "supported-load-completes-normally-without-exception",
    "loaded-value-advances-after-the-next-recorded-instruction",
    "audited-recorder-and-host-exclusions-hold",
)
ISR_CONDITION = "recorded-isr-context-preserved"
USE_ROLE = "recorded-operand-use"
ZERO_DESTINATION = "zero-register-destination"
NOT_WORD_LOAD = "not-full-word-load"
BASE_UNPROVED = "base-value-not-locally-defined"
UNALIGNED = "unaligned-address"
NOT_ORDINARY = "address-not-ordinary-ram"
OUTSIDE_INTERVAL = "address-outside-captured-interval"
DELAY_CONSUMER = "load-delay-consumer-needs-new-value"
UNRECORDED_VISIBILITY = "destination-visibility-unrecorded"
UNKNOWN_DELAY_EFFECT = "unknown-effect-in-delay-window"
UNKNOWN_CONSUMER_EFFECT = "unknown-effect-in-consumer-slice"
CANCELLED_DESTINATION = "load-destination-cancelled-by-next-instruction"
REPLACED_DESTINATION = "load-destination-replaced-by-delayed-next-instruction"
LOAD_OPCODES = frozenset({0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x32})
DEFERRED_OPCODES = frozenset({0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26})
HI_REGISTER = 32
LO_REGISTER = 33


class PointerLoadRefused(ValueError):
    """The supplied history does not retain an authentic pointer-load derivation."""


@dataclass(frozen=True)
class PointerLoadCapture:
    """One pinned pointer-load capture and its authenticated replay."""

    descriptor: dict
    files: MappingProxyType
    replay: "Replay"

    @property
    def identity(self):
        return self.descriptor["capture"]


@dataclass(frozen=True)
class Replay:
    """Authenticated replay of one recorded RAM interval."""

    start: int
    length: int
    occurrences: tuple
    initial: bytes
    final: bytes
    writes: tuple


def _require(value, message):
    if not value:
        raise PointerLoadRefused(message)


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
        "pointer load cycle is not an exact decimal",
    )
    result = int(value)
    _require(result < 1 << 64, "pointer load cycle exceeds uint64")
    return result


def physical(address):
    """Return the ordinary-RAM physical address, refusing every other mapping."""

    _require(
        _integer(address, 0, 0xFFFFFFFF)
        and address & 0xE0000000 in (0, 0x80000000, 0xA0000000)
        and address & 0x1FFFFFFF < RAM_BYTES,
        "pointer load address is outside ordinary RAM",
    )
    return address & 0x1FFFFFFF


def _little(words, offset):
    return int.from_bytes(words[offset : offset + 4], "little")


def _transfer_target(pc, word):
    """Return the taken target of a direct original transfer, else ``None``."""

    fields = word_fields(word)
    if fields.opcode in (2, 3):
        return (pc + 4) & 0xF0000000 | fields.target26 << 2
    if fields.opcode in (4, 5, 6, 7) or (fields.opcode == 1 and fields.rt in (0, 1)):
        return (pc + 4 + fields.signed_immediate * 4) & 0xFFFFFFFF
    return None


def is_transfer(word):
    fields = word_fields(word)
    return fields.opcode in (1, 2, 3, 4, 5, 6, 7) or (
        fields.opcode == 0 and fields.funct in (8, 9)
    )


def operand_use(word):
    """Return the GPR reads and writes of one decoded original word, or ``None``.

    The closed set mirrors the native ``originalOperation`` table so that both owners
    derive operand use from the same primary words instead of from each other. ``None``
    means the table holds no rule for the word, so its effects are unknown and nothing
    may be attributed through it.
    """

    fields = word_fields(word)
    opcode, rs, rt, rd, shift, funct = (
        fields.opcode,
        fields.rs,
        fields.rt,
        fields.rd,
        fields.shift,
        fields.funct,
    )
    if opcode == 0:
        if funct == 8 and rt == 0 and rd == 0 and shift == 0:
            return frozenset({rs}), frozenset()
        if funct == 9:
            if rt != 0 or shift != 0 or rd != 31 or rs == 31:
                return None
            return frozenset({rs}), frozenset({31})
        if funct in (0, 2, 3):
            return frozenset({rt}), frozenset({rd})
        if funct in (4, 6, 7, 42, 43) or 32 <= funct <= 39:
            return frozenset({rs, rt}), frozenset({rd})
        if funct in (24, 25, 26, 27) and rd == 0 and shift == 0:
            return frozenset({rs, rt}), frozenset({HI_REGISTER, LO_REGISTER})
        if funct in (16, 18) and rs == 0 and rt == 0 and shift == 0:
            return frozenset({HI_REGISTER if funct == 16 else LO_REGISTER}), frozenset(
                {rd}
            )
        return None
    if opcode in (2, 3):
        return frozenset(), frozenset({31}) if opcode == 3 else frozenset()
    if opcode in (4, 5):
        return frozenset({rs, rt}), frozenset()
    if opcode in (6, 7) or (opcode == 1 and rt in (0, 1)):
        return frozenset({rs}), frozenset()
    if 8 <= opcode <= 14:
        return frozenset({rs}), frozenset({rt})
    if opcode == 15:
        return frozenset(), frozenset({rt})
    if opcode in DEFERRED_OPCODES:
        return frozenset({rs}), frozenset({rt})
    if opcode in (40, 41, 42, 43, 46):
        return frozenset({rs, rt}), frozenset()
    return None


def _gpr(values):
    return {value for value in values if 0 <= value < 32}


def validate_descriptor(value):
    _require(
        type(value) is dict
        and set(value)
        == {
            "kind",
            "function",
            "image_id",
            "image_sha256",
            "entry_pc",
            "stop_pc",
            "state",
            "interval",
            "begin_cycle",
            "end_cycle",
            "occurrences",
            "files",
        },
        "pointer load descriptor shape differs",
    )
    _require(
        value["kind"] == CAPTURE_KIND
        and type(value["function"]) is str
        and 0 < len(value["function"]) <= 256
        and type(value["image_id"]) is str
        and 0 < len(value["image_id"]) <= 256
        and _digest(value["image_sha256"]),
        "pointer load capture identity differs",
    )
    entry, stop = value["entry_pc"], value["stop_pc"]
    for address in (entry, stop):
        _require(
            _integer(address, 0, 0xFFFFFFFF) and address % 4 == 0,
            "pointer load capture address differs",
        )
        physical(address)
    state = value["state"]
    _require(
        type(state) is dict
        and set(state) == {"bytes", "sha256"}
        and _integer(state["bytes"], 1, 64 << 20)
        and _digest(state["sha256"]),
        "pointer load state identity differs",
    )
    interval = value["interval"]
    _require(
        type(interval) is dict
        and set(interval) == {"mapping_start", "bytes"}
        and _integer(interval["mapping_start"], 0, RAM_BYTES - 4)
        and interval["mapping_start"] % 4 == 0
        and _integer(interval["bytes"], 4, RAM_BYTES)
        and interval["bytes"] % 4 == 0
        and interval["mapping_start"] + interval["bytes"] <= RAM_BYTES
        and interval["mapping_start"] == (entry & 0x1FFFFFFF),
        "pointer load RAM interval differs",
    )
    _require(
        _cycle(value["begin_cycle"]) < _cycle(value["end_cycle"]),
        "pointer load cycle order differs",
    )
    rows = value["occurrences"]
    _require(
        type(rows) is list and 1 <= len(rows) <= 65536,
        "pointer load occurrence inventory differs",
    )
    previous = 0
    for index, row in enumerate(rows):
        _require(
            type(row) is dict
            and set(row) == {"sequence", "pc", "word", "branch_delay_slot"}
            and _integer(row["sequence"], 1, 65536)
            and row["sequence"] > previous
            and _integer(row["pc"], 0, 0xFFFFFFFF)
            and row["pc"] % 4 == 0
            and _integer(row["word"], 0, 0xFFFFFFFF)
            and type(row["branch_delay_slot"]) is bool,
            "pointer load occurrence differs",
        )
        previous = row["sequence"]
        if index == 0:
            _require(
                row["pc"] == entry, "pointer load slice does not start at its entry"
            )
    start = interval["mapping_start"]
    _require(
        all(
            start <= (row["pc"] & 0x1FFFFFFF) <= start + interval["bytes"] - 4
            for row in rows
        ),
        "pointer load occurrence escapes the captured interval",
    )
    _require(
        type(value["files"]) is dict and set(value["files"]) == set(FILE_LIMITS),
        "pointer load file inventory differs",
    )
    for name, limit in FILE_LIMITS.items():
        row = value["files"][name]
        _require(
            type(row) is dict
            and set(row) == {"path", "bytes", "sha256"}
            and row["path"] == f"input/pointer-{name}"
            and _integer(row["bytes"], 1, limit)
            and _digest(row["sha256"]),
            "pointer load file pin differs",
        )
    return value


def _replay(descriptor, rows, payload, *, boundary_ram):
    start, length = (
        descriptor["interval"]["mapping_start"],
        descriptor["interval"]["bytes"],
    )
    _require(
        len(payload) >= length
        and descriptor["files"]["payload.bin"]["bytes"] == len(payload),
        "pointer load payload extent differs",
    )
    image = bytearray(payload[:length])
    _require(
        len(boundary_ram) == RAM_BYTES
        and boundary_ram[start : start + length] == bytes(image),
        "pointer load initial image differs from the boundary RAM",
    )
    _require(
        len(rows) >= 2
        and rows[0].get("kind") == "ram-context"
        and rows[-1].get("kind") == "ram-context",
        "pointer load boundary events missing",
    )
    instructions, writes, final = [], [], None
    previous_cycle = _cycle(descriptor["begin_cycle"])
    for number, row in enumerate(rows, 1):
        _require(
            type(row) is dict and set(row) == EVENT_FIELDS,
            "pointer load event shape differs",
        )
        _require(
            type(row["kind"]) is str
            and row["sequence"] == str(number)
            and previous_cycle
            <= _cycle(row["cycle"])
            <= _cycle(descriptor["end_cycle"]),
            "pointer load event ordering differs",
        )
        previous_cycle = _cycle(row["cycle"])
        span, fields = row["payload"], row["fields"]
        _require(
            type(span) is dict
            and set(span) == {"offset", "bytes"}
            and _integer(span["offset"], 0, len(payload))
            and _integer(span["bytes"], 0, len(payload) - span["offset"])
            and type(fields) is dict,
            "pointer load event payload escapes",
        )
        data = payload[span["offset"] : span["offset"] + span["bytes"]]
        if row["kind"] == "ram-context":
            _require(
                fields.get("first") == start
                and fields.get("byte_count") == length
                and fields.get("setting_8mb") == 0,
                "pointer load RAM mapping extent differs",
            )
            if number == 1:
                _require(
                    fields.get("phase") == "begin"
                    and row["cycle"] == descriptor["begin_cycle"]
                    and data == bytes(image),
                    "pointer load initial image missing",
                )
            elif number == len(rows):
                _require(
                    final is None
                    and fields.get("phase") == "end"
                    and row["cycle"] == descriptor["end_cycle"]
                    and data == bytes(image)
                    and fields.get("pc") == descriptor["stop_pc"],
                    "pointer load endpoint reconciliation differs",
                )
                final = data
            else:
                _require(not data, "pointer load unexpected boundary image")
        elif row["kind"] == "ram-write":
            index = fields.get("backing_offset", -1) - start
            _require(
                final is None
                and 0 < len(data) <= 16
                and _integer(index, 0, length - 4)
                and index % 4 == 0
                and index + len(data) <= length
                and fields.get("byte_count") == len(data)
                and fields.get("store_bytes") == fields.get("byte_count"),
                "pointer load committed write differs",
            )
            image[index : index + len(data)] = data
            writes.append(
                {
                    "sequence": int(row["sequence"]),
                    "physical": start + index,
                    "bytes": len(data),
                }
            )
        elif row["kind"] == "ram-instruction":
            index = fields.get("backing_offset", -1) - start
            _require(
                _integer(index, 0, length - 4)
                and index % 4 == 0
                and (fields.get("pc") & 0x1FFFFFFF) == start + index
                and fields.get("consumed_word") == fields.get("backing_word")
                and fields["consumed_word"] == _little(bytes(image), index)
                and fields.get("delay_slot") in (0, 1),
                "pointer load consumed instruction differs",
            )
            instructions.append(
                {
                    "sequence": int(row["sequence"]),
                    "pc": fields["pc"] & 0xFFFFFFFF,
                    "word": fields["consumed_word"],
                    "branch_delay_slot": fields["delay_slot"] == 1,
                }
            )
        else:
            raise PointerLoadRefused("pointer load history has another event profile")
    _require(final is not None, "pointer load interval has no endpoint image")
    return Replay(
        start=start,
        length=length,
        occurrences=tuple(instructions),
        initial=bytes(payload[:length]),
        final=bytes(final),
        writes=tuple(writes),
    )


def validate_files(descriptor, files, *, boundary_ram):
    validate_descriptor(descriptor)
    _require(
        type(files) in (dict, MappingProxyType) and set(files) == set(FILE_LIMITS),
        "pointer load captures missing",
    )
    for name, pin in descriptor["files"].items():
        raw = files[name]
        _require(
            type(raw) is bytes
            and len(raw) == pin["bytes"]
            and _hash(raw) == pin["sha256"],
            "pointer load capture bytes differ",
        )
    receipt, history, metadata = (
        _json(files[name]) for name in ("receipt.json", "history.json", "events.json")
    )
    _require(
        receipt.get("schema") == "psx.runtime-session/v2"
        and receipt.get("status") == "passed"
        and receipt["process"]["exit_code"] == 0
        and receipt["process"]["failure"] is None
        and type(receipt.get("mission")) is dict
        and receipt["mission"].get("kind") == "builtin"
        and receipt["mission"].get("name") == "history",
        "pointer load needs a successful original-only history execution",
    )
    inputs = receipt["inputs"]
    state = inputs["file:STATE"]
    _require(
        set(inputs)
        <= {"bios", "disc:cue", "file:STATE", "origin"}
        | {name for name in inputs if name.startswith("disc:")}
        and "bios" in inputs
        and "disc:cue" in inputs
        and set(receipt.get("assets") or {}) == {"file:STATE"}
        and (state["bytes"], state["sha256"])
        == (descriptor["state"]["bytes"], descriptor["state"]["sha256"]),
        "pointer load executed state differs from the captured boundary",
    )
    continuation = receipt.get("origin")
    _require(
        type(continuation) is dict
        and continuation.get("schema") == "psx.runtime-origin/v1"
        and continuation.get("state_sha256") == descriptor["state"]["sha256"]
        and continuation.get("captures") == ["state.pbuf"]
        and type(inputs.get("origin")) is dict
        and set(inputs["origin"]) == {"path", "bytes", "sha256"},
        "pointer load origin continuation differs",
    )
    captures = receipt["completion"]["captures"]
    _require(
        receipt["completion"]["schema"] == "psx.runtime-completion/v1"
        and type(captures) is list
        and len({row["path"] for row in captures}) == len(captures),
        "pointer load capture completion differs",
    )
    for name in set(FILE_LIMITS) - {"receipt.json"}:
        pin = descriptor["files"][name]
        matches = [row for row in captures if row["path"] == name]
        _require(
            len(matches) == 1
            and all(matches[0][key] == pin[key] for key in ("bytes", "sha256")),
            "pointer load capture is not receipt-bound",
        )
    _require(
        history.get("schema") == "psx.runtime-history/v2"
        and history.get("success") is True
        and history.get("started") is True
        and history.get("native_complete") is True
        and history.get("profile") == "ram-interval-v1"
        and history.get("reason") == "stop reached"
        and history.get("ram_start") == descriptor["interval"]["mapping_start"]
        and history.get("ram_length") == descriptor["interval"]["bytes"]
        and history.get("stop") == descriptor["stop_pc"]
        and _integer(history.get("frames"), 0, history.get("frame_limit", -1)),
        "pointer load history did not reach its stop within guest bounds",
    )
    _require(
        metadata.get("schema") == "psx.native-history/v2"
        and metadata.get("complete") is True
        and metadata.get("failure_bits") == 0
        and metadata.get("failures") == []
        and metadata.get("dropped_attempts") == 0
        and metadata.get("begin_cycle") == descriptor["begin_cycle"]
        and metadata.get("end_cycle") == descriptor["end_cycle"]
        and metadata.get("build_identity") == history.get("build_identity")
        and metadata["build_identity"]["binary_sha256"]
        == receipt["emulator"]["binary_sha256"],
        "pointer load recorder identity differs",
    )
    rows = [_json(line) for line in files["events.ndjson"].splitlines()]
    payload = files["payload.bin"]
    _require(
        2 <= len(rows) <= 65536
        and metadata.get("events") == len(rows) <= history["event_limit"]
        and metadata.get("payload_bytes") == len(payload) <= history["byte_limit"]
        and metadata.get("kinds") == dict(Counter(row.get("kind") for row in rows)),
        "pointer load event inventory differs",
    )
    replay = _replay(descriptor, rows, payload, boundary_ram=boundary_ram)
    _require(
        [
            (row["sequence"], row["pc"], row["word"], row["branch_delay_slot"])
            for row in descriptor["occurrences"]
        ]
        == [
            (row["sequence"], row["pc"], row["word"], row["branch_delay_slot"])
            for row in replay.occurrences
        ],
        "pointer load occurrence slice differs from the recording",
    )
    return replay


def _continuity(occurrences):
    """Require a complete recorded dispatch slice: adjacent or taken-transfer path."""

    words = {row["pc"]: row["word"] for row in occurrences}
    for previous, following in zip(occurrences, occurrences[1:]):
        if following["pc"] == previous["pc"] + 4:
            continue
        _require(
            previous["branch_delay_slot"]
            and previous["pc"] - 4 in words
            and _transfer_target(previous["pc"] - 4, words[previous["pc"] - 4])
            == following["pc"],
            "pointer load recorded dispatch slice has a gap",
        )
    return words


def _delay_window_refusal(word, destination):
    """Return the refusal one intervening recorded instruction forces, else ``None``.

    The recorded CPU lands a pending load at the end of the following instruction unless
    that instruction writes the same GPR, and it defers its own destination write when it
    is itself a load. A word with no rule in the table leaves both possibilities open, so
    the load cannot be placed either.
    """

    use = operand_use(word)
    if use is None:
        return UNKNOWN_DELAY_EFFECT
    reads, writes = use
    if destination in _gpr(reads):
        return DELAY_CONSUMER
    if destination in _gpr(writes):
        if word_fields(word).opcode in DEFERRED_OPCODES:
            return REPLACED_DESTINATION
        return CANCELLED_DESTINATION
    return None


def derive(replay):
    """Derive every occurrence fact from the replayed interval and original words."""

    occurrences = list(replay.occurrences)
    words = _continuity(occurrences)
    first, last = occurrences[0]["sequence"], occurrences[-1]["sequence"]
    for row in occurrences:
        previous = words.get(row["pc"] - 4)
        _require(
            row["branch_delay_slot"]
            == (previous is not None and is_transfer(previous)),
            "pointer load delay-slot marker contradicts the recorded slice",
        )
    admitted, excluded = [], []
    for index, row in enumerate(occurrences):
        pc, word, sequence = row["pc"], row["word"], row["sequence"]
        fields = word_fields(word)
        prefix = occurrences[index - 1] if index else None
        prefix_fields = word_fields(prefix["word"]) if prefix else None
        if fields.opcode not in LOAD_OPCODES:
            continue
        reason, address = None, None
        if fields.opcode != 0x23:
            reason = NOT_WORD_LOAD
        elif fields.rt == 0:
            reason = ZERO_DESTINATION
        elif (
            fields.rs == 0
            or prefix_fields is None
            or prefix_fields.opcode != 15
            or prefix_fields.rt != fields.rs
        ):
            reason = BASE_UNPROVED
        else:
            address = (
                (prefix_fields.immediate << 16) + fields.signed_immediate
            ) & 0xFFFFFFFF
            if address % 4:
                reason = UNALIGNED
            elif address & 0xE0000000 not in (0, 0x80000000, 0xA0000000) or (
                address & 0x1FFFFFFF >= RAM_BYTES
            ):
                reason = NOT_ORDINARY
            elif not (
                replay.start
                <= (address & 0x1FFFFFFF)
                <= replay.start + replay.length - 4
            ):
                reason = OUTSIDE_INTERVAL
        if reason is None and index + 2 >= len(occurrences):
            reason = UNRECORDED_VISIBILITY
        if reason is None:
            reason = _delay_window_refusal(occurrences[index + 1]["word"], fields.rt)
        consumers = []
        if reason is None:
            for later in occurrences[index + 2 :]:
                use = operand_use(later["word"])
                if use is None:
                    reason = UNKNOWN_CONSUMER_EFFECT
                    break
                if fields.rt in _gpr(use[0]):
                    consumers.append(
                        {
                            "sequence": later["sequence"],
                            "pc": later["pc"],
                            "role": USE_ROLE,
                        }
                    )
                if fields.rt in _gpr(use[1]):
                    break
        if reason is not None:
            excluded.append(
                {"sequence": sequence, "pc": pc, "word": word, "reason": reason}
            )
            continue
        offset = (address & 0x1FFFFFFF) - replay.start
        visible = occurrences[index + 2]
        admitted.append(
            {
                "sequence": sequence,
                "pc": pc,
                "word": word,
                "branchDelaySlot": row["branch_delay_slot"],
                "path": {
                    "firstSequence": first,
                    "sequence": sequence,
                    "lastSequence": last,
                },
                "reaching": {
                    "sequence": prefix["sequence"],
                    "pc": prefix["pc"],
                    "word": prefix["word"],
                },
                "address": {
                    "canonical": (address & 0x1FFFFFFF) | 0x80000000,
                    "physical": address & 0x1FFFFFFF,
                    "mapping": MAPPING,
                },
                "value": list(replay.initial[offset : offset + 4]),
                "destination": fields.rt,
                "visibleFromPc": visible["pc"],
                "visibleFromSequence": visible["sequence"],
                "consumers": consumers,
            }
        )
    for write in replay.writes:
        end = write["physical"] + write["bytes"]
        for occurrence in admitted:
            cell = occurrence["address"]["physical"]
            _require(
                end <= cell or write["physical"] >= cell + 4,
                "pointer load selected cell was rewritten inside the interval",
            )
    return admitted, excluded


def record(descriptor, *, isr, admitted, excluded):
    """Assemble the tagged record body shared by both owners."""

    return {
        "kind": RECORD_KIND,
        "capture": capture_identity(descriptor),
        "stateHash": descriptor["state"]["sha256"],
        "entryPc": descriptor["entry_pc"],
        "stopPc": descriptor["stop_pc"],
        "interval": {
            "mappingStart": descriptor["interval"]["mapping_start"],
            "bytes": descriptor["interval"]["bytes"],
        },
        "beginCycle": descriptor["begin_cycle"],
        "endCycle": descriptor["end_cycle"],
        "isr": isr,
        "conditions": list(CONDITIONS) + ([ISR_CONDITION] if isr else []),
        "occurrences": admitted,
        "excluded": excluded,
    }


def capture_identity(descriptor):
    """Digest the descriptor exactly as the native request re-serializes it."""

    body = {name: value for name, value in descriptor.items() if name != "capture"}
    return _hash(json.dumps(body, sort_keys=True, separators=(",", ":")).encode())


def _sources(receipt_path, files):
    return [
        {
            "path": str(
                receipt_path if name == "receipt.json" else receipt_path.parent / name
            ),
            "bytes": len(raw),
            "sha256": _hash(raw),
        }
        for name, raw in files.items()
    ]


def capture(receipt_path):
    """Authenticate one RAM history and pin its pointer-load capture files."""

    from harness.build import recipe
    from harness.runtime import completion, origin

    receipt_path = Path(receipt_path).resolve(strict=True)
    raw = read_bytes(receipt_path, FILE_LIMITS["receipt.json"])
    receipt = _json(raw)
    _require(
        receipt.get("schema") == "psx.runtime-session/v2"
        and receipt.get("status") == "passed"
        and completion.validate_completion(receipt_path.parent) == receipt["completion"]
        and origin.validate_selection(receipt).get("name") == "history",
        "pointer load requires a passed native RAM history capture",
    )
    inputs = receipt["inputs"]
    source = inputs["origin"]
    _require(
        _digest(source["sha256"])
        and _integer(source["bytes"], 1, FILE_LIMITS["receipt.json"]),
        "pointer load history names no original state capture",
    )
    origin_raw = read_bytes(Path(source["path"]), FILE_LIMITS["receipt.json"])
    _require(
        len(origin_raw) == source["bytes"] and _hash(origin_raw) == source["sha256"],
        "pointer load original state receipt changed",
    )
    continuation = origin.verify_origin(
        Path(source["path"]), inputs, receipt["emulator"]
    )
    _require(
        receipt.get("origin") == continuation
        and continuation["state_sha256"] == inputs["file:STATE"]["sha256"],
        "pointer load history does not continue its captured original state",
    )
    files = {
        name: read_bytes(receipt_path.parent / name, limit)
        for name, limit in FILE_LIMITS.items()
        if name != "receipt.json"
    }
    files["receipt.json"] = raw
    metadata, history = _json(files["events.json"]), _json(files["history.json"])
    rows = [_json(line) for line in files["events.ndjson"].splitlines()]
    _require(
        len(rows) >= 2
        and rows[0]["kind"] == rows[-1]["kind"] == "ram-context"
        and rows[0]["fields"]["phase"] == "begin"
        and rows[-1]["fields"]["phase"] == "end",
        "pointer load history has no RAM boundaries",
    )
    descriptor = {
        "kind": CAPTURE_KIND,
        "function": "",
        "image_id": "",
        "image_sha256": "",
        "entry_pc": rows[0]["fields"]["pc"] & 0xFFFFFFFF,
        "stop_pc": rows[-1]["fields"]["pc"] & 0xFFFFFFFF,
        "state": {
            "bytes": inputs["file:STATE"]["bytes"],
            "sha256": continuation["state_sha256"],
        },
        "interval": {
            "mapping_start": history.get("ram_start"),
            "bytes": history.get("ram_length"),
        },
        "begin_cycle": metadata.get("begin_cycle"),
        "end_cycle": metadata.get("end_cycle"),
        "occurrences": [
            {
                "sequence": int(row["sequence"]),
                "pc": row["fields"]["pc"] & 0xFFFFFFFF,
                "word": row["fields"]["consumed_word"],
                "branch_delay_slot": row["fields"]["delay_slot"] == 1,
            }
            for row in rows
            if row["kind"] == "ram-instruction"
        ],
        "files": {
            name: {
                "path": f"input/pointer-{name}",
                "bytes": len(data),
                "sha256": _hash(data),
            }
            for name, data in files.items()
        },
    }
    recipe.verify_inputs(_sources(receipt_path, files))
    return descriptor, MappingProxyType(files)


def bind(descriptor, *, contexts, overlay_mounts):
    """Name the observed function and image owner of one pinned history."""

    entry = descriptor["entry_pc"]
    candidates = [
        image
        for image in contexts.images
        if image.image_id in overlay_mounts
        and image.load_address <= entry < image.load_address + image.payload_size
    ]
    _require(candidates, "pointer load history has no mounted original image owner")
    image = candidates[-1]
    targets = [
        target.target
        for target in contexts.targets
        if target.image_id == image.image_id
    ]
    _require(len(targets) == 1, "pointer load image target is ambiguous")
    return {
        **descriptor,
        "function": f"{targets[0]}@{entry:08x}",
        "image_id": image.image_id,
        "image_sha256": image.file_sha256,
    }


def seal(descriptor, files, *, contexts, overlay_mounts, boundary_ram):
    """Bind, validate and digest one capture before it enters a native request."""

    bound = bind(descriptor, contexts=contexts, overlay_mounts=overlay_mounts)
    replay = validate_files(bound, files, boundary_ram=boundary_ram)
    return PointerLoadCapture(bound, files, replay)


def read(descriptor, directory, *, boundary_ram, reader=None):
    """Re-read one pinned capture from retained inputs and replay it independently."""

    if descriptor is None:
        return None
    validate_descriptor(descriptor)
    files = {
        name: (reader or read_bytes)(
            Path(directory) / f"pointer-{name}", pin["bytes"], []
        )
        for name, pin in descriptor["files"].items()
    }
    replay = validate_files(descriptor, files, boundary_ram=boundary_ram)
    return PointerLoadCapture(descriptor, MappingProxyType(files), replay)


def open_captures(descriptor, files, *, boundary_ram):
    """Validate one pinned capture from the bytes already carried by a request."""

    validate_descriptor(descriptor)
    _require(
        type(files) in (dict, MappingProxyType) and set(files) == set(FILE_LIMITS),
        "pointer load captures missing",
    )
    replay = validate_files(descriptor, files, boundary_ram=boundary_ram)
    return PointerLoadCapture(descriptor, MappingProxyType(files), replay)


def expected_record(capture, *, isr):
    """Derive the exact record the native producer must publish for this capture."""

    admitted, excluded = derive(capture.replay)
    return record(capture.descriptor, isr=isr, admitted=admitted, excluded=excluded)


def validate_record(value):
    """Check the closed shape of one native ``psx-observed-pointer-load/v1`` record."""

    _require(
        type(value) is dict
        and set(value)
        == {
            "kind",
            "capture",
            "stateHash",
            "entryPc",
            "stopPc",
            "interval",
            "beginCycle",
            "endCycle",
            "isr",
            "conditions",
            "occurrences",
            "excluded",
        }
        and value["kind"] == RECORD_KIND
        and _digest(value["capture"])
        and _digest(value["stateHash"]),
        "observed pointer load record shape differs",
    )
    _require(
        _integer(value["entryPc"], 0, 0xFFFFFFFF)
        and value["entryPc"] % 4 == 0
        and _integer(value["stopPc"], 0, 0xFFFFFFFF)
        and type(value["interval"]) is dict
        and set(value["interval"]) == {"mappingStart", "bytes"}
        and type(value["isr"]) is bool
        and type(value["conditions"]) is list
        and type(value["occurrences"]) is list
        and type(value["excluded"]) is list,
        "observed pointer load record fields differ",
    )
    _cycle(value["beginCycle"])
    _cycle(value["endCycle"])
    for row in value["excluded"]:
        _require(
            type(row) is dict
            and set(row) == {"sequence", "pc", "word", "reason"}
            and type(row["reason"]) is str,
            "observed pointer load exclusion differs",
        )
    for row in value["occurrences"]:
        _require(
            type(row) is dict
            and set(row)
            == {
                "sequence",
                "pc",
                "word",
                "branchDelaySlot",
                "path",
                "reaching",
                "address",
                "value",
                "destination",
                "visibleFromPc",
                "visibleFromSequence",
                "consumers",
            }
            and type(row["path"]) is dict
            and set(row["path"]) == {"firstSequence", "sequence", "lastSequence"}
            and type(row["reaching"]) is dict
            and set(row["reaching"]) == {"sequence", "pc", "word"}
            and type(row["address"]) is dict
            and set(row["address"]) == {"canonical", "physical", "mapping"}
            and row["address"]["mapping"] == MAPPING
            and type(row["value"]) is list
            and len(row["value"]) == 4
            and all(_integer(byte, 0, 0xFF) for byte in row["value"])
            and _integer(row["destination"], 1, 31)
            and type(row["consumers"]) is list,
            "observed pointer load occurrence differs",
        )
        for consumer in row["consumers"]:
            _require(
                type(consumer) is dict
                and set(consumer) == {"sequence", "pc", "role"}
                and consumer["role"] == USE_ROLE,
                "observed pointer load consumer differs",
            )
    return value
