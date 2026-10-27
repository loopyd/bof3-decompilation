"""Discover and decode complete original indexed byte-table consumers.

Discovery is code-owned: it scans the reviewed function boundaries of one target
image for a constant base that is combined with a dynamic index and then read by
a load instruction.  No caller-supplied function list, no index row and no
authored selector participates.

Two independent producers project that discovery, and each carries only its own
source:

* :func:`capture_byte_lookup_access` records the materialization window of every
  load site and the effective base it resolves to (address provenance) and yields
  ``selected_access`` facts.
* :func:`capture_byte_lookup_context` records each consumer's complete reviewed
  body effects (control flow, delay slots, memory events, opaque calls, loops,
  accumulation and comparison) through :mod:`harness.naming.lookupeffects` and
  yields ``one_level_beyond`` facts.

Neither projection can stand in for the other: they read different source
windows and their receipts are bound separately.  Every discovered consumer that
cannot be decoded completely stays an explicit capability gap, and no positive
fact claims object extent, a safe index bound, storage ownership or gameplay
meaning.
"""

from __future__ import annotations

import hashlib
import json
import struct
from collections import Counter
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from harness.analysis.project import prepare_target
from harness.domain.ids import parse_function_id
from harness.domain.layout import parse_splat_layout
from harness.domain.manifests import load_target_manifests
from harness.domain.mips import word_fields
from harness.naming.consumers import capture_reviewed_consumer
from harness.naming.description import capture_data_range
from harness.naming.lookupeffects import SCOPE, SHARED_PREMISES, decode_consumer_body
from harness.naming.staticeffects import REGISTER_NAMES

_MAX_FUNCTION_BYTES = 65536
_MAX_CONSUMERS = 8
_MAX_SITES = 32
_LUI_WINDOW = 12
_LOADS = {0x20: "lb", 0x21: "lh", 0x23: "lw", 0x24: "lbu", 0x25: "lhu"}
_STORES = {0x28: "sb", 0x29: "sh", 0x2B: "sw"}
_MEMORY = {**_LOADS, **_STORES}
_WIDTHS = {"lb": 1, "lbu": 1, "sb": 1, "lh": 2, "lhu": 2, "sh": 2, "lw": 4, "sw": 4}
_BRANCHES = frozenset({0x01, 0x04, 0x05, 0x06, 0x07})
_JUMPS = frozenset({0x02, 0x03})


def _hex(value: int) -> str:
    return f"0x{value & 0xFFFFFFFF:08X}"


def _register(number: int) -> str:
    return REGISTER_NAMES[number]


def _function_id(target: str, address: int) -> str:
    return str(parse_function_id(f"{target}@0x{address:08X}"))


def _words(code: bytes, start: int) -> list[int]:
    if not code or len(code) % 4 or len(code) > _MAX_FUNCTION_BYTES or start % 4:
        raise ValueError("byte lookup consumer is not a finite aligned body")
    return list(struct.unpack_from(f"<{len(code) // 4}I", code, 0))


def _branch_target(fields: Any, address: int) -> int | None:
    """Return the branch target of one already-extracted instruction."""

    if fields.opcode not in _BRANCHES:
        return None
    if fields.opcode == 0x01 and fields.rt > 1:
        return None
    return address + 4 + (fields.signed_immediate << 2)


# ---------------------------------------------------------------- discovery


@dataclass(frozen=True)
class _Site:
    """One materialized-base memory operation found in original bytes."""

    function: int
    start: int
    end: int
    site: int
    opcode: str
    fixed: int
    unsigned: int
    index_registers: tuple[int, ...]
    reason: str | None
    materialization: tuple[tuple[int, int], ...]


@dataclass(frozen=True)
class _Scan:
    """One target's original-image premises and materialized-base operations."""

    image_sha256: str
    splat_sha256: str
    replay_sha256: str
    functions: int
    function_bytes: int
    unreadable: int
    interrupted: int
    sites: tuple[_Site, ...]
    interruptions: Counter[str]
    settled: int
    boundaries: tuple[tuple[int, int | None, str], ...]


def _discovery_write(fields: Any) -> tuple[int | None, str | None]:
    """Classify only proven MIPS-I register writes and discovery barriers."""

    if fields.opcode in _BRANCHES | _JUMPS:
        return None, "control barrier"
    if fields.opcode == 0:
        if fields.funct in {0x08, 0x09, 0x0C, 0x0D}:
            return None, "control barrier"
        if fields.funct in {
            0x00,
            0x02,
            0x03,
            0x04,
            0x06,
            0x07,
            0x10,
            0x12,
            0x20,
            0x21,
            0x22,
            0x23,
            0x24,
            0x25,
            0x26,
            0x27,
            0x2A,
            0x2B,
        }:
            return fields.rd, None
        if fields.funct in {0x11, 0x13, 0x18, 0x19, 0x1A, 0x1B}:
            return None, None
    elif fields.opcode in {*range(0x08, 0x0F), *_LOADS}:
        return fields.rt, None
    elif fields.opcode == 0x0F and fields.rs == 0:
        return fields.rt, None
    elif fields.opcode in _STORES:
        return None, None
    return None, "unknown opcode barrier"


_INTERRUPTED = frozenset(
    {
        "materialization reaches the body end",
        "materialization exceeds the window",
        "control barrier",
        "unaccounted control slot",
        "unknown opcode barrier",
        "materialized base is clobbered",
        "dynamic addend is clobbered after addition",
        "base is added to itself",
    }
)


def _materialized_base(
    words: list[int], index: int, skip: int = 0
) -> tuple[int, int, tuple[int, ...], list[tuple[int, int]], str | None]:
    """Follow one base for at most twelve following instructions.

    Every register that provably holds the same materialized value stays tracked,
    so a copied base and a base that is used more than once are explained instead
    of being cut at the first memory operation.  ``skip`` selects which resolved
    memory use to report; when the window holds fewer uses the interruption record
    that ended the chain is returned instead, with ``_INTERRUPTED`` reasons.

    Proven non-clobbers remain in the receipt.  Unknown effects and transfers
    terminate the chain, not discovery evidence.  Multiple dynamic additions are
    retained but cannot yield a supported single-index access.
    """

    lui = word_fields(words[index])
    constant = lui.immediate << 16
    holders: dict[int, tuple[int, tuple[int, ...]]] = {lui.rt: (constant, ())}
    primary = (constant, ())
    window: list[tuple[int, int]] = [(index, lui.word)]
    uses: list[tuple[int, int, tuple[int, ...], int, str | None]] = []
    reason: str | None = None
    transfer_step: int | None = None
    for step in range(1, _LUI_WINDOW + 2):
        if step > _LUI_WINDOW and transfer_step != _LUI_WINDOW:
            reason = "materialization exceeds the window"
            break
        position = index + step
        if position >= len(words):
            reason = "materialization reaches the body end"
            break
        if transfer_step is not None and step > transfer_step + 1:
            reason = "control barrier"
            break
        candidate = word_fields(words[position])
        window.append((position, candidate.word))
        if candidate.opcode == 0x09 and candidate.rs in holders:
            value, indexes = holders[candidate.rs]
            if candidate.rt:
                holders[candidate.rt] = (
                    (value + candidate.signed_immediate) & 0xFFFFFFFF,
                    indexes,
                )
                if candidate.rt == lui.rt:
                    primary = holders[candidate.rt]
            continue
        if candidate.opcode == 0 and candidate.funct == 0x21 and candidate.shift == 0:
            source, addend, written = candidate.rs, candidate.rt, candidate.rd
            if source in holders and addend in holders:
                reason = "base is added to itself"
                break
            if source in holders and addend == 0:
                holders[written] = holders[source]
                continue
            if addend in holders and source == 0:
                holders[written] = holders[addend]
                continue
            if source in holders and written == source:
                holders[source] = (holders[source][0], (*holders[source][1], addend))
                if source == lui.rt:
                    primary = holders[source]
                continue
            if addend in holders and written == addend:
                holders[addend] = (holders[addend][0], (*holders[addend][1], source))
                if addend == lui.rt:
                    primary = holders[addend]
                continue
        if candidate.opcode in _MEMORY and candidate.rs in holders:
            value, indexes = holders[candidate.rs]
            unsigned = (value + candidate.immediate) & 0xFFFFFFFF
            fixed = (value + candidate.signed_immediate) & 0xFFFFFFFF
            uses.append(
                (
                    fixed,
                    unsigned,
                    indexes,
                    len(window),
                    "multiple dynamic addends" if len(indexes) > 1 else None,
                )
            )
            if candidate.opcode in _LOADS and candidate.rt in holders:
                del holders[candidate.rt]
                if not holders:
                    reason = "materialized base is clobbered"
                    break
            continue
        written, barrier = _discovery_write(candidate)
        if (
            barrier == "control barrier"
            and (
                candidate.opcode in _BRANCHES | {0x02, 0x03}
                or candidate.opcode == 0
                and candidate.funct in {0x08, 0x09}
            )
            and transfer_step is None
        ):
            transfer_step = step
            continue
        if barrier is not None:
            reason = barrier
            break
        if written in holders:
            del holders[written]
            if not holders:
                reason = "materialized base is clobbered"
                break
            continue
        if any(written in indexes for _, indexes in holders.values()):
            reason = "dynamic addend is clobbered after addition"
            break
    else:
        reason = "materialization exceeds the window"
    if transfer_step is not None:
        slot_present = window[-1][0] == index + transfer_step + 1
        slot_barrier = _discovery_write(word_fields(window[-1][1]))[1]
        if not slot_present:
            reason = "unaccounted control slot"
        elif slot_barrier is not None:
            reason = (
                "unknown opcode barrier"
                if slot_barrier == "unknown opcode barrier"
                else "unaccounted control slot"
            )
        elif reason in {None, "control barrier", "materialization exceeds the window"}:
            reason = "control barrier"
    if skip < len(uses):
        fixed, unsigned, indexes, length, reason = uses[skip]
        return fixed, unsigned, indexes, window[:length], reason
    # An interruption names the value that is still tracked.  A surviving copy is
    # preferred over the last value of the clobbered primary register, and a value
    # whose low half is still a bare page is preferred over a settled one, so no
    # unresolved form of this materialization is dropped.
    values = list(holders.values()) or [primary]
    fixed, indexes = next(
        (value for value in values if not value[0] & 0xFFFF), values[0]
    )
    return fixed, fixed, indexes, window, reason


_SCAN_REUSE: tuple[tuple, _Scan] | None = None


def _scan_target(root: Path, target: str) -> _Scan:
    """Read current premises and scan every reviewed function boundary."""

    global _SCAN_REUSE

    manifest = load_target_manifests(root)[target]
    layout = parse_splat_layout(root / manifest.splat, manifest.load_address)
    spec = prepare_target(root, target, manifest=manifest)
    image = spec.binary.read_bytes()
    # Recheck current premises before reuse, including the address-to-file mapping.
    key = (
        root.resolve(),
        target,
        hashlib.sha256(image).hexdigest(),
        layout.sha256,
        spec.replay_sha256,
        spec.binary_offset,
        spec.load_address,
    )
    if _SCAN_REUSE is not None and _SCAN_REUSE[0] == key:
        return _SCAN_REUSE[1]
    sites: list[_Site] = []
    interruptions: Counter[str] = Counter()
    settled = 0
    boundaries: list[tuple[int, int | None, str]] = []
    functions = 0
    function_bytes = 0
    unreadable = 0
    interrupted = 0
    for boundary in layout.boundaries:
        if not boundary.is_function:
            continue
        start, end = boundary.virtual_start, boundary.virtual_end
        if (
            end is None
            or start % 4
            or end % 4
            or not 0 < end - start <= _MAX_FUNCTION_BYTES
        ):
            unreadable += 1
            boundaries.append((start, end, "unreadable"))
            continue
        offset = spec.binary_offset + start - spec.load_address
        if offset < spec.binary_offset or offset + end - start > len(image):
            unreadable += 1
            boundaries.append((start, end, "unreadable"))
            continue
        boundaries.append((start, end, "scanned"))
        words = _words(image[offset : offset + end - start], start)
        functions += 1
        function_bytes += end - start
        for index, word in enumerate(words):
            fields = word_fields(word)
            if fields.opcode != 0x0F or fields.rs != 0 or fields.rt == 0:
                continue
            use = 0
            while True:
                fixed, unsigned, index_registers, window, reason = _materialized_base(
                    words, index, use
                )
                position = index + len(window) - 1
                candidate = word_fields(words[position])
                site = _Site(
                    function=start,
                    start=start,
                    end=end,
                    site=start + position * 4,
                    opcode=_MEMORY.get(candidate.opcode, "unresolved"),
                    fixed=fixed,
                    unsigned=unsigned,
                    index_registers=index_registers,
                    reason=reason,
                    materialization=tuple(
                        (start + offset * 4, value) for offset, value in window
                    ),
                )
                if reason in _INTERRUPTED:
                    interruptions[reason] += 1
                    # Classified uses settle only the bounded search. Unknown
                    # effects and missing/unsupported slots still hide possible
                    # uses, even after an earlier unrelated memory operation.
                    live_barrier = reason in {
                        "unknown opcode barrier",
                        "unaccounted control slot",
                    }
                    if use == 0 or live_barrier:
                        sites.append(site)
                        interrupted += 1
                    else:
                        settled += 1
                    break
                sites.append(site)
                use += 1
    scan = _Scan(
        image_sha256=hashlib.sha256(image).hexdigest(),
        splat_sha256=layout.sha256,
        replay_sha256=spec.replay_sha256,
        functions=functions,
        function_bytes=function_bytes,
        unreadable=unreadable,
        interrupted=interrupted,
        sites=tuple(sites),
        interruptions=interruptions,
        settled=settled,
        boundaries=tuple(boundaries),
    )
    _SCAN_REUSE = (key, scan)
    return scan


def _table_binding(root: Path, selector: str) -> tuple[dict[str, Any], int]:
    """Return the selected table's reviewed segment binding and its address."""

    captured = capture_data_range(root, selector)
    if captured["gap"] is not None:
        raise ValueError(str(captured["gap"]))
    binding = captured["binding"]
    return (
        {
            "scope": binding["scope"],
            "kind": binding["kind"],
            "start": binding["start"],
            "end": binding["end"],
            "image_sha256": binding["image_sha256"],
            "splat_sha256": binding["splat_sha256"],
            "replay_sha256": binding["replay_sha256"],
        },
        parse_function_id(selector).address,
    )


def _unsettled_low_half(base: int, selected: int) -> bool:
    """Decide whether an interrupted materialization can still reach the selected object.

    Only an unsettled low half is a candidate.  While the tracked register holds a
    bare page, a later 16-bit access offset can address the selected object on that
    page.  Once an explicit ``addiu`` settles the low half the register names a
    different object, and reaching the selected address would need a rebase that
    produces its own materialization - a counterfactual, not a discovered chain.
    """

    if base == selected:
        return True
    if base & 0xFFFF:
        return False
    delta = (selected - base) & 0xFFFFFFFF
    return delta <= 0xFFFF or delta >= 0xFFFF8000


def _classify(
    site: _Site, selected: int, target: str
) -> tuple[str, dict[str, Any] | None, dict[str, Any] | None]:
    """Sort one materialized operation into a consumer site or a refusal."""

    common = {
        "site": _hex(site.site),
        "function": _function_id(target, site.start),
        "opcode": site.opcode,
    }
    low = selected & 0xFFFF
    if site.reason is not None:
        if site.reason in _INTERRUPTED:
            if not _unsettled_low_half(site.fixed, selected):
                return "unrelated", None, None
            plausibility = (
                "the materialized page has no explicit constant offset yet, so a "
                "later signed or unsigned low half could address the selected object"
            )
        else:
            if selected not in {site.fixed, site.unsigned}:
                return "unrelated", None, None
            plausibility = "resolved fixed offset with unsupported addends"
        return (
            "refusal",
            None,
            {
                **common,
                "reason": site.reason,
                "selected_plausibility": plausibility,
                "fixed_base": _hex(site.fixed),
                "addends": [_register(register) for register in site.index_registers],
                "materialization": [
                    {"address": _hex(address), "word": f"0x{word:08X}"}
                    for address, word in site.materialization
                ],
            },
        )
    if site.fixed == selected:
        if site.opcode in _STORES.values():
            return (
                "refusal",
                None,
                {
                    **common,
                    "reason": (
                        "selected address is written, so the object cannot be a "
                        "read-only lookup table"
                    ),
                },
            )
        if site.opcode != "lbu":
            return "other", None, None
        return (
            "site",
            {
                **common,
                "fixed_base": _hex(site.fixed),
                "width": _WIDTHS[site.opcode],
                "materialization": [
                    {"address": _hex(address), "word": f"0x{word:08X}"}
                    for address, word in site.materialization
                ],
                "index": (
                    {"kind": "register", "register": _register(site.index_registers[0])}
                    if site.index_registers
                    else {"kind": "constant_element"}
                ),
            },
            None,
        )
    if site.unsigned == selected:
        return (
            "refusal",
            None,
            {
                **common,
                "reason": (
                    "unsigned low-half addition would reach the selected address "
                    "while the signed offset resolves elsewhere"
                ),
            },
        )
    if (site.fixed & 0xFFFF) == low:
        return (
            "refusal",
            None,
            {
                **common,
                "reason": (
                    "the same low offset is read from a different materialized "
                    "base, so a read of the selected object cannot be excluded"
                ),
            },
        )
    return "unrelated", None, None


def _discover(root: Path, selector: str) -> dict[str, Any]:
    """Discover every consumer of the selected object from original bytes."""

    selected_id = parse_function_id(selector)
    target = selected_id.target.value
    result: dict[str, Any] = {
        "selected": str(selected_id),
        "binding": None,
        "consumers": [],
        "scan": {
            "functions": 0,
            "bytes": 0,
            "sites": 0,
            "refusals": 0,
            "interrupted": 0,
        },
        "refusals": [],
        "gap": None,
    }
    try:
        binding, selected = _table_binding(root, selector)
        scan = _scan_target(root, target)
    except (OSError, ValueError) as error:
        result["gap"] = str(error)
        return result
    result["binding"] = binding
    if (
        binding["image_sha256"] != scan.image_sha256
        or binding["splat_sha256"] != scan.splat_sha256
        or binding["replay_sha256"] != scan.replay_sha256
    ):
        result["gap"] = "table segment and consumer image premises disagree"
        return result
    grouped: dict[int, list[dict[str, Any]]] = {}
    refusals: list[dict[str, Any]] = []
    other_width = 0
    for site in scan.sites:
        kind, value, refusal = _classify(site, selected, target)
        if kind == "site":
            assert value is not None
            grouped.setdefault(site.start, []).append(value)
        elif kind == "refusal":
            assert refusal is not None
            refusals.append(refusal)
        elif kind == "other":
            other_width += 1
    if scan.unreadable:
        refusals.append(
            {
                "site": None,
                "function": None,
                "opcode": None,
                "reason": (
                    f"{scan.unreadable} reviewed function boundaries have no "
                    "finite readable original range"
                ),
            }
        )
    sites = sum(len(values) for values in grouped.values())
    result["scan"] = {
        "functions": scan.functions,
        "bytes": scan.function_bytes,
        "sites": sites,
        "refusals": len(refusals),
        "interrupted": scan.interrupted,
        "interrupted_reasons": dict(sorted(scan.interruptions.items())),
        "settled_after_use": scan.settled,
        "other_width": other_width,
        "window_instructions": _LUI_WINDOW,
        "boundary_transfer_slot": "one ordinary slot after an in-window transfer",
        "coverage": "bounded materialization search; no absence proof beyond each window",
        "boundaries": [
            {
                "start": _hex(start),
                "end": _hex(end) if end is not None else None,
                "status": status,
            }
            for start, end, status in scan.boundaries
        ],
        "selected_plausible_interrupted": sum(
            "selected_plausibility" in refusal for refusal in refusals
        ),
    }
    result["refusals"] = refusals
    if len(grouped) > _MAX_CONSUMERS:
        result["gap"] = (
            f"discovered {len(grouped)} consumers, above the bounded limit of "
            f"{_MAX_CONSUMERS}"
        )
        return result
    if sites > _MAX_SITES:
        result["gap"] = (
            f"discovered {sites} selected-object sites, above the bounded limit "
            f"of {_MAX_SITES}"
        )
        return result
    for start in sorted(grouped):
        result["consumers"].append(
            {
                "consumer": _function_id(target, start),
                "sites": sorted(grouped[start], key=lambda item: item["site"]),
                "gap": None,
            }
        )
    return result


def has_byte_lookup_work(root: Path, selector: str) -> bool:
    """Activate selected candidates, not generic unavailable range coverage."""
    discovered = _discover(root, selector)
    return bool(
        discovered["consumers"]
        or discovered["scan"]["sites"]
        or any(
            refusal.get("function") is not None for refusal in discovered["refusals"]
        )
    )


def capture_byte_lookup_access(root: Path, selector: str) -> dict[str, Any]:
    """Capture per-site address provenance only, never a consumer body."""

    return _discover(root, selector)


def capture_byte_lookup_context(root: Path, selector: str) -> dict[str, Any]:
    """Capture each consumer's complete reviewed body and its decoded effects."""

    discovered = _discover(root, selector)
    payload: dict[str, Any] = {
        **discovered,
        "consumers": [
            {
                "consumer": consumer["consumer"],
                "binding": None,
                "bytes": None,
                "body": None,
                "gap": None,
            }
            for consumer in discovered["consumers"]
        ],
    }
    if discovered["gap"] is not None:
        return payload
    selected = parse_function_id(discovered["selected"]).address
    for entry in payload["consumers"]:
        captured = capture_reviewed_consumer(
            root, discovered["selected"], entry["consumer"]
        )
        entry.update({key: captured[key] for key in ("binding", "bytes", "gap")})
        if entry["gap"] is not None:
            continue
        if any(
            entry["binding"][key] != discovered["binding"][key]
            for key in ("image_sha256", "splat_sha256", "replay_sha256")
        ):
            entry["gap"] = "consumer capture and discovery premises disagree"
            continue
        try:
            entry["body"] = decode_consumer_body(
                bytes.fromhex(entry["bytes"]), entry["binding"]["start"], selected
            )
        except ValueError as error:
            entry["gap"] = str(error)
    return payload


# ---------------------------------------------------------------- decoding


def byte_lookup_gaps(payload: dict[str, Any], kind: str) -> list[dict[str, str]]:
    """Project every recorded refusal onto its blocking rung."""

    gaps: list[dict[str, str]] = []
    if payload.get("gap") is not None:
        gaps.append({"class": kind, "reason": f"capability gap: {payload['gap']}"})
    for refusal in payload.get("refusals") or ():
        gaps.append(
            {
                "class": kind,
                "reason": (
                    "capability gap: byte lookup refusal at "
                    f"{refusal.get('site') or 'unknown site'}: {refusal['reason']}"
                ),
            }
        )
    for consumer in payload.get("consumers") or ():
        if consumer.get("gap") is not None:
            gaps.append(
                {
                    "class": kind,
                    "reason": (
                        "capability gap: byte lookup consumer "
                        f"{consumer['consumer']}: {consumer['gap']}"
                    ),
                }
            )
    return gaps


def _premises(operation: dict[str, Any], target: str, selected_range: dict) -> dict:
    payload = operation.get("payload")
    if not isinstance(payload, dict):
        raise ValueError("byte lookup payload is invalid; recapture")
    if set(payload) != {
        "selected",
        "binding",
        "consumers",
        "scan",
        "refusals",
        "gap",
    }:
        raise ValueError("byte lookup payload shape is invalid; recapture")
    selected = parse_function_id(str(payload["selected"]))
    if selected.target.value != target or selected != parse_function_id(
        str(operation["selector"])
    ):
        raise ValueError("byte lookup payload identity mismatch")
    binding = payload["binding"]
    if not isinstance(binding, dict) or set(binding) != {
        "scope",
        "kind",
        "start",
        "end",
        "image_sha256",
        "splat_sha256",
        "replay_sha256",
    }:
        raise ValueError("byte lookup table binding is invalid")
    premises = {
        key: binding[key] for key in ("image_sha256", "splat_sha256", "replay_sha256")
    }
    if any(
        not isinstance(premises[key], str)
        or len(premises[key]) != 64
        or premises[key] != str(selected_range.get(key))
        for key in premises
    ):
        raise ValueError(
            "byte lookup receipt does not share the selected range image premises"
        )
    return payload


def decode_byte_lookup_access(
    operation: dict[str, Any],
    target: str,
    selected_range: dict,
    input_digest: str,
) -> list[dict[str, Any]]:
    """Project one access receipt onto instruction-level address provenance."""

    payload = _premises(operation, target, selected_range)
    selected = parse_function_id(payload["selected"])
    facts: list[dict[str, Any]] = []
    for consumer in payload["consumers"]:
        if set(consumer) != {"consumer", "sites", "gap"}:
            raise ValueError("byte lookup access consumer shape is invalid")
        for site in consumer["sites"]:
            if set(site) != {
                "site",
                "function",
                "opcode",
                "fixed_base",
                "width",
                "materialization",
                "index",
            }:
                raise ValueError("byte lookup access site shape is invalid")
            if site["function"] != consumer["consumer"]:
                raise ValueError("byte lookup access site function mismatch")
            value = {
                "site": site["site"],
                "function": consumer["consumer"],
                "address": _hex(selected.address),
                "kind": "load",
                "opcode": site["opcode"],
                "width": site["width"],
                "effective_address": {
                    "fixed_base": site["fixed_base"],
                    "index": site["index"],
                    "materialization": site["materialization"],
                    "model": "constant materialization plus dynamic added index",
                },
            }
            facts.append(
                {
                    "class": "selected_access",
                    "polarity": "positive",
                    "value": value,
                    "source_id": _source(
                        "original-image:byte-lookup-access",
                        target,
                        value,
                        input_digest,
                    ),
                }
            )
    return facts


def decode_byte_lookup_context(
    operation: dict[str, Any],
    target: str,
    selected_range: dict,
    input_digest: str,
) -> list[dict[str, Any]]:
    """Project one context receipt onto complete consumer body effects."""

    payload = _premises(operation, target, selected_range)
    selected = parse_function_id(payload["selected"])
    facts: list[dict[str, Any]] = []
    for consumer in payload["consumers"]:
        if set(consumer) != {"consumer", "binding", "bytes", "body", "gap"}:
            raise ValueError("byte lookup context consumer shape is invalid")
        if consumer["gap"] is not None or consumer["body"] is None:
            continue
        binding = consumer["binding"]
        if not isinstance(binding, dict) or set(binding) != {
            "start",
            "end",
            "kind",
            "image_sha256",
            "splat_sha256",
            "replay_sha256",
        }:
            raise ValueError("byte lookup consumer binding is invalid")
        if any(
            binding[key] != payload["binding"][key]
            for key in ("image_sha256", "splat_sha256", "replay_sha256")
        ):
            raise ValueError("byte lookup consumer premises differ from the table")
        if not isinstance(consumer["bytes"], str) or len(consumer["bytes"]) != 2 * (
            binding["end"] - binding["start"]
        ):
            raise ValueError("byte lookup consumer bytes are invalid")
        body = consumer["body"]
        if not isinstance(body, dict) or body.get("scope") != SCOPE:
            raise ValueError("byte lookup body shape is invalid")
        # Recompute the complete body from the receipt's own bytes instead of
        # trusting the authored copy, so a changed or substituted body is refused.
        recomputed = decode_consumer_body(
            bytes.fromhex(consumer["bytes"]), binding["start"], selected.address
        )
        if recomputed != body:
            raise ValueError(
                "byte lookup consumer body does not match its captured original bytes"
            )
        value = {
            "consumer": consumer["consumer"],
            "consumer_range": [binding["start"], binding["end"]],
            "binding": binding,
            "body": body,
            "shared_premises": [
                "original image identity",
                "reviewed layout and target load mapping",
                *SHARED_PREMISES,
            ],
            "scope": SCOPE,
        }
        facts.append(
            {
                "class": "one_level_beyond",
                "polarity": "positive",
                "value": value,
                "source_id": _source(
                    "original-image:byte-lookup-context",
                    target,
                    value,
                    input_digest,
                ),
            }
        )
    return facts


def _source(family: str, target: str, value: object, input_digest: str) -> str:
    encoded = json.dumps(
        [target, value, input_digest], sort_keys=True, separators=(",", ":")
    ).encode()
    digest = hashlib.sha256(encoded).hexdigest()
    return f"{family}:{digest}"
