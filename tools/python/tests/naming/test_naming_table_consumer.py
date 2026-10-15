"""Tests for complete original dispatcher consumers and capture bindings."""

from __future__ import annotations

from dataclasses import dataclass, replace
from pathlib import Path
from types import MappingProxyType

import pytest
from harness.naming.consumers import decode_dispatch_consumer
from harness.naming import evidence


@dataclass(frozen=True)
class ConsumerCase:
    target: str
    row: str
    selected_address: int
    selected_end: int
    consumer_address: int
    consumer_end: int
    image_sha256: str
    table_bytes: bytes
    consumer_words: tuple[int, ...]
    pointer_addresses: tuple[int, ...]
    load_offsets: tuple[int, ...]


_TARGET = "emi/battle/battle/15"

_IMAGE_SHA256 = "77d963c56e3ba6b1619323e3007c25d70300e855ac9f4b088ddce2604f92e140"


def _consumer_words(selected_low: int) -> tuple[int, ...]:
    return (
        0x27BDFFD8,
        0xAFBF0020,
        0x3C058009,
        0x24A50000 | selected_low,
        0x8CA20000,
        0x8CA30004,
        0x8CA40008,
        0xAFA20010,
        0xAFA30014,
        0xAFA40018,
        0x3C02800F,
        0x3C031F80,
        0x8C630044,
        0x34420800,
        0x3C018014,
        0xAC2259F0,
        0x90620001,
        0x00000000,
        0x00021080,
        0x03A21021,
        0x8C420010,
        0x00000000,
        0x0040F809,
        0x00000000,
        0x3C02800D,
        0x34423800,
        0x3C018014,
        0xAC2259F0,
        0x8FBF0020,
        0x27BD0028,
        0x03E00008,
        0x00000000,
    )


DATA_CONSUMER_CASES = MappingProxyType(
    {
        ("data:D_80096994", "emi/battle/battle/15@800ad26c"): ConsumerCase(
            target=_TARGET,
            row="data:D_80096994",
            selected_address=0x80096994,
            selected_end=0x800969A0,
            consumer_address=0x800AD26C,
            consumer_end=0x800AD2EC,
            image_sha256=_IMAGE_SHA256,
            table_bytes=bytes.fromhex("ecd20a8034d40a807cd60a80"),
            consumer_words=_consumer_words(0x6994),
            pointer_addresses=(0x800AD2EC, 0x800AD434, 0x800AD67C),
            load_offsets=(4, 5, 6),
        ),
        ("data:D_800969A0", "emi/battle/battle/15@800ad69c"): ConsumerCase(
            target=_TARGET,
            row="data:D_800969A0",
            selected_address=0x800969A0,
            selected_end=0x800969AC,
            consumer_address=0x800AD69C,
            consumer_end=0x800AD71C,
            image_sha256=_IMAGE_SHA256,
            table_bytes=bytes.fromhex("1cd70a8008d80a80acd90a80"),
            consumer_words=_consumer_words(0x69A0),
            pointer_addresses=(0x800AD71C, 0x800AD808, 0x800AD9AC),
            load_offsets=(4, 5, 6),
        ),
        ("data:D_800969AC", "emi/battle/battle/15@800ad9cc"): ConsumerCase(
            target=_TARGET,
            row="data:D_800969AC",
            selected_address=0x800969AC,
            selected_end=0x800969B8,
            consumer_address=0x800AD9CC,
            consumer_end=0x800ADA4C,
            image_sha256=_IMAGE_SHA256,
            table_bytes=bytes.fromhex("4cda0a8074db0a80a4dc0a80"),
            consumer_words=_consumer_words(0x69AC),
            pointer_addresses=(0x800ADA4C, 0x800ADB74, 0x800ADCA4),
            load_offsets=(4, 5, 6),
        ),
        ("data:D_800969B8", "emi/battle/battle/15@800adcc4"): ConsumerCase(
            target=_TARGET,
            row="data:D_800969B8",
            selected_address=0x800969B8,
            selected_end=0x800969C4,
            consumer_address=0x800ADCC4,
            consumer_end=0x800ADD44,
            image_sha256=_IMAGE_SHA256,
            table_bytes=bytes.fromhex("44dd0a806cde0a809cdf0a80"),
            consumer_words=_consumer_words(0x69B8),
            pointer_addresses=(0x800ADD44, 0x800ADE6C, 0x800ADF9C),
            load_offsets=(4, 5, 6),
        ),
        ("data:D_800969F8", "emi/battle/battle/15@800b01f0"): ConsumerCase(
            target=_TARGET,
            row="data:D_800969F8",
            selected_address=0x800969F8,
            selected_end=0x80096A04,
            consumer_address=0x800B01F0,
            consumer_end=0x800B0250,
            image_sha256=_IMAGE_SHA256,
            table_bytes=bytes.fromhex("50020b8098030b8010040b80"),
            consumer_words=(
                0x3C028015,
                0x8C428648,
                0x27BDFFD8,
                0xAFBF0020,
                0x3C068009,
                0x24C669F8,
                0x8CC30000,
                0x8CC40004,
                0x8CC50008,
                0xAFA30010,
                0xAFA40014,
                0xAFA50018,
                0x90420003,
                0x00000000,
                0x00021080,
                0x03A21021,
                0x8C420010,
                0x00000000,
                0x0040F809,
                0x00000000,
                0x8FBF0020,
                0x27BD0028,
                0x03E00008,
                0x00000000,
            ),
            pointer_addresses=(0x800B0250, 0x800B0398, 0x800B0410),
            load_offsets=(6, 7, 8),
        ),
        ("data:D_80096A08", "emi/battle/battle/15@800b09cc"): ConsumerCase(
            target=_TARGET,
            row="data:D_80096A08",
            selected_address=0x80096A08,
            selected_end=0x80096A14,
            consumer_address=0x800B09CC,
            consumer_end=0x800B0A2C,
            image_sha256=_IMAGE_SHA256,
            table_bytes=bytes.fromhex("2c0a0b80540a0b80b00a0b80"),
            consumer_words=(
                0x3C028015,
                0x8C428648,
                0x27BDFFD8,
                0xAFBF0020,
                0x3C068009,
                0x24C66A08,
                0x8CC30000,
                0x8CC40004,
                0x8CC50008,
                0xAFA30010,
                0xAFA40014,
                0xAFA50018,
                0x90420003,
                0x00000000,
                0x00021080,
                0x03A21021,
                0x8C420010,
                0x00000000,
                0x0040F809,
                0x00000000,
                0x8FBF0020,
                0x27BD0028,
                0x03E00008,
                0x00000000,
            ),
            pointer_addresses=(0x800B0A2C, 0x800B0A54, 0x800B0AB0),
            load_offsets=(6, 7, 8),
        ),
        ("data:D_80096A34", "emi/battle/battle/15@800b138c"): ConsumerCase(
            target=_TARGET,
            row="data:D_80096A34",
            selected_address=0x80096A34,
            selected_end=0x80096A40,
            consumer_address=0x800B138C,
            consumer_end=0x800B13EC,
            image_sha256=_IMAGE_SHA256,
            table_bytes=bytes.fromhex("ec130b80bc140b8004150b80"),
            consumer_words=(
                0x3C028015,
                0x8C428648,
                0x27BDFFD8,
                0xAFBF0020,
                0x3C068009,
                0x24C66A34,
                0x8CC30000,
                0x8CC40004,
                0x8CC50008,
                0xAFA30010,
                0xAFA40014,
                0xAFA50018,
                0x90420003,
                0x00000000,
                0x00021080,
                0x03A21021,
                0x8C420010,
                0x00000000,
                0x0040F809,
                0x00000000,
                0x8FBF0020,
                0x27BD0028,
                0x03E00008,
                0x00000000,
            ),
            pointer_addresses=(0x800B13EC, 0x800B14BC, 0x800B1504),
            load_offsets=(6, 7, 8),
        ),
    }
)

EXPECTED_KEYS = {
    ("data:D_80096994", "emi/battle/battle/15@800ad26c"),
    ("data:D_800969A0", "emi/battle/battle/15@800ad69c"),
    ("data:D_800969AC", "emi/battle/battle/15@800ad9cc"),
    ("data:D_800969B8", "emi/battle/battle/15@800adcc4"),
    ("data:D_800969F8", "emi/battle/battle/15@800b01f0"),
    ("data:D_80096A08", "emi/battle/battle/15@800b09cc"),
    ("data:D_80096A34", "emi/battle/battle/15@800b138c"),
}
SPECS = tuple(DATA_CONSUMER_CASES.values())


def _operation(spec: ConsumerCase) -> dict:
    return {
        "selector": f"{spec.target}@{spec.consumer_address:08x}",
        "payload": {
            "selected": f"{spec.target}@{spec.selected_address:08x}",
            "consumer": f"{spec.target}@{spec.consumer_address:08x}",
            "binding": {
                "start": spec.consumer_address,
                "end": spec.consumer_end,
                "kind": "c",
                "image_sha256": spec.image_sha256,
                "splat_sha256": "a" * 64,
                "replay_sha256": "b" * 64,
            },
            "bytes": b"".join(
                w.to_bytes(4, "little") for w in spec.consumer_words
            ).hex(),
            "gap": None,
        },
    }


def _range(spec: ConsumerCase) -> dict:
    return {
        "address": f"0x{spec.selected_address:08X}",
        "image_sha256": spec.image_sha256,
        "splat_sha256": "a" * 64,
        "replay_sha256": "b" * 64,
        "initializer": {
            "words": [
                {
                    "address": spec.selected_address + i * 4,
                    "value": pointer,
                    "function_end": pointer + 4,
                    "kind": "c",
                }
                for i, pointer in enumerate(spec.pointer_addresses)
            ],
            "stop": "containing segment boundary",
        },
    }


def _decode(operation, spec):
    return decode_dispatch_consumer(operation, spec.target, _range(spec), "b" * 64)


@pytest.mark.parametrize("spec", SPECS)
def test_representative_derives_layout_access_and_consumer(spec):
    facts = _decode(_operation(spec), spec)
    assert [fact["class"] for fact in facts] == [
        "selected_access",
        "one_level_beyond",
    ]
    assert [
        word["value"] for word in facts[0]["value"]["initializer_words_read"]
    ] == list(spec.pointer_addresses)
    encoded_offset = next(
        word & 0xFFFF
        for word in spec.consumer_words
        if word >> 26 == 0x24 and (word >> 21) & 0x1F and (word >> 16) & 0x1F
    )
    assert encoded_offset == (1 if spec.row < "data:D_800969F8" else 3)
    assert facts[0]["value"]["body"]["selector"]["offset_bytes"] == encoded_offset
    assert facts[1]["value"]["body"]["selector"]["offset_bytes"] == encoded_offset


def _mutate_selector(spec: ConsumerCase, *, rs=None, rt=None, immediate=None):
    words = list(spec.consumer_words)
    index = next(index for index, word in enumerate(words) if word >> 26 == 0x24)
    word = words[index]
    if rs is not None:
        word = (word & ~(0x1F << 21)) | (rs << 21)
    if rt is not None:
        word = (word & ~(0x1F << 16)) | (rt << 16)
    if immediate is not None:
        word = (word & ~0xFFFF) | (immediate & 0xFFFF)
    words[index] = word
    return replace(spec, consumer_words=tuple(words))


@pytest.mark.parametrize("spec", SPECS)
def test_selector_immediate_is_decoded_from_current_words(spec, monkeypatch):
    monkeypatch.setattr(
        evidence, "capture_dispatch_consumer", lambda *_: _operation(spec)["payload"]
    )
    mutated = _mutate_selector(spec, immediate=7)
    facts = _decode(_operation(mutated), mutated)
    assert facts[0]["value"]["body"]["selector"]["offset_bytes"] == 7
    assert facts[1]["value"]["body"]["selector"]["offset_bytes"] == 7
    with pytest.raises(ValueError):
        evidence._validate_consumer_bytes(
            Path("/unused"), spec.target, _operation(mutated)["payload"]
        )


@pytest.mark.parametrize("spec", SPECS)
@pytest.mark.parametrize(
    ("rs", "rt", "immediate"),
    ((0, None, None), (None, 0, None), (0, 0, None), (7, None, None), (None, None, -1)),
    ids=(
        "zero-base",
        "zero-destination",
        "both-zero",
        "wrong-base",
        "negative-immediate",
    ),
)
def test_invalid_selector_fields_fail_closed(spec, rs, rt, immediate):
    mutated = _mutate_selector(spec, rs=rs, rt=rt, immediate=immediate)
    with pytest.raises(ValueError, match="selector"):
        _decode(_operation(mutated), mutated)


@pytest.mark.parametrize("spec", SPECS)
@pytest.mark.parametrize(
    "field",
    [
        "selected",
        "consumer",
        "binding",
        "bytes",
        "image_sha256",
    ],
)
def test_identity_range_and_byte_mutations_fail_closed(spec, field):
    operation = _operation(spec)
    payload = operation["payload"]
    assert isinstance(payload, dict)
    mutations = {
        "selected": f"{spec.target}@{spec.selected_address + 4:08x}",
        "consumer": f"{spec.target}@{spec.consumer_address + 4:08x}",
        "binding": {**payload["binding"], "start": spec.consumer_address + 4},
        "bytes": "00" * (len(spec.consumer_words) * 4),
        "image_sha256": "bad",
    }
    if field == "image_sha256":
        payload["binding"][field] = mutations[field]
    else:
        payload[field] = mutations[field]
    if field == "consumer":
        operation["selector"] = mutations[field]
    with pytest.raises(ValueError):
        _decode(operation, spec)


@pytest.mark.parametrize("spec", SPECS)
@pytest.mark.parametrize(
    ("case", "consumer_range"),
    [
        ("shifted-start", lambda spec: [spec.consumer_address + 4, spec.consumer_end]),
        ("shifted-end", lambda spec: [spec.consumer_address, spec.consumer_end + 4]),
        ("zero", lambda spec: [spec.consumer_address, spec.consumer_address]),
        ("reversed", lambda spec: [spec.consumer_end, spec.consumer_address]),
        ("wrong-size", lambda spec: [spec.consumer_address, spec.consumer_address + 4]),
        ("oversized", lambda spec: [spec.consumer_address, spec.consumer_end + 0x100]),
        (
            "unrelated",
            lambda spec: [spec.consumer_address + 0x1000, spec.consumer_end + 0x1000],
        ),
    ],
    ids=lambda value: value if isinstance(value, str) else None,
)
def test_consumer_range_mutations_fail_closed(spec, case, consumer_range):
    operation = _operation(spec)
    payload = operation["payload"]
    assert isinstance(payload, dict)
    payload["binding"]["start"], payload["binding"]["end"] = consumer_range(spec)
    with pytest.raises(ValueError):
        _decode(operation, spec)


@pytest.mark.parametrize("spec", SPECS)
@pytest.mark.parametrize(
    "digest",
    [
        "A" * 64,
        "g" * 64,
        "0" * 64,
        "",
        "3d9b32ab3b0ddee595839ef177892cff68e2fe8f9fea7c87e990f4eb0c881f9f",
    ],
)
def test_image_digest_mutations_fail_closed(spec, digest):
    operation = _operation(spec)
    payload = operation["payload"]
    assert isinstance(payload, dict)
    payload["binding"]["image_sha256"] = digest
    with pytest.raises(ValueError):
        _decode(operation, spec)


@pytest.mark.parametrize("source", SPECS)
@pytest.mark.parametrize("destination", SPECS)
def test_cross_spec_reuse_fails_closed(source, destination):
    if source == destination:
        return
    with pytest.raises(ValueError):
        _decode(_operation(source), destination)


@pytest.mark.parametrize("spec", SPECS)
def test_pointer_prefix_truncation_fails_closed(spec):
    selected_range = _range(spec)
    selected_range["initializer"]["words"].pop()
    with pytest.raises(ValueError, match="prefix"):
        decode_dispatch_consumer(
            _operation(spec), spec.target, selected_range, "b" * 64
        )


def test_other_target_uses_the_same_decoder():
    spec = replace(SPECS[0], target="exe/test")
    assert len(_decode(_operation(spec), spec)) == 2


def test_other_selected_address_uses_the_same_decoder():
    spec = SPECS[0]
    words = list(spec.consumer_words)
    words[3] += 4
    moved = replace(
        spec,
        selected_address=spec.selected_address + 4,
        selected_end=spec.selected_end + 4,
        consumer_words=tuple(words),
    )
    assert len(_decode(_operation(moved), moved)) == 2
