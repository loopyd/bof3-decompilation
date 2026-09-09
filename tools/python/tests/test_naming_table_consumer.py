"""Tests for the exact-qualified bounded table-consumer decoder."""

from __future__ import annotations

import copy
from dataclasses import replace

import pytest
from harness.naming.capabilities import PRODUCTION_EXACT_CAPABILITIES, TABLE_CONSUMERS
from harness.naming.consumers import TableConsumerSpec, decode_table_consumer

EXPECTED_KEYS = {
    ("data:D_80096994", "emi/battle/battle/15@800ad26c"),
    ("data:D_800969A0", "emi/battle/battle/15@800ad69c"),
    ("data:D_800969AC", "emi/battle/battle/15@800ad9cc"),
    ("data:D_800969B8", "emi/battle/battle/15@800adcc4"),
    ("data:D_800969F8", "emi/battle/battle/15@800b01f0"),
    ("data:D_80096A08", "emi/battle/battle/15@800b09cc"),
    ("data:D_80096A34", "emi/battle/battle/15@800b138c"),
}
SPECS = tuple(TABLE_CONSUMERS.values())


def test_exact_capability_registry_is_immutable_and_matches_consumers() -> None:
    assert set(PRODUCTION_EXACT_CAPABILITIES) == EXPECTED_KEYS
    assert {
        key: entry.consumer for key, entry in PRODUCTION_EXACT_CAPABILITIES.items()
    } == dict(TABLE_CONSUMERS)
    with pytest.raises(TypeError):
        PRODUCTION_EXACT_CAPABILITIES[("data:D_0", "exe/test@0")] = object()  # type: ignore[index]
    with pytest.raises(TypeError):
        TABLE_CONSUMERS[("data:D_0", "exe/test@0")] = object()  # type: ignore[index]


def _operation(spec: TableConsumerSpec) -> dict[str, object]:
    return {
        "selector": f"{spec.target}@{spec.consumer_address:08x}",
        "payload": {
            "selected": f"{spec.target}@{spec.selected_address:08x}",
            "consumer": f"{spec.target}@{spec.consumer_address:08x}",
            "selected_range": [spec.selected_address, spec.selected_end],
            "consumer_range": [spec.consumer_address, spec.consumer_end],
            "selected_bytes": spec.table_bytes.hex(),
            "consumer_bytes": b"".join(
                word.to_bytes(4, "little") for word in spec.consumer_words
            ).hex(),
            "image_sha256": spec.image_sha256,
            "pointer_descriptions": [
                [
                    {
                        "target": spec.target,
                        "address": f"0x{pointer:08X}",
                        "splat": {"kind": "c", "start": f"0x{pointer:08X}"},
                    }
                ]
                for pointer in spec.pointer_addresses
            ],
        },
    }


@pytest.mark.parametrize("spec", SPECS)
def test_representative_derives_layout_access_and_consumer(spec):
    facts = decode_table_consumer(_operation(spec), spec, "b" * 64)
    assert [fact["class"] for fact in facts] == [
        "reviewed_layout",
        "selected_access",
        "one_level_beyond",
    ]
    assert facts[0]["value"]["words"] == [
        f"0x{pointer:08X}" for pointer in spec.pointer_addresses
    ]
    encoded_offset = next(
        word & 0xFFFF
        for word in spec.consumer_words
        if word >> 26 == 0x24 and (word >> 21) & 0x1F and (word >> 16) & 0x1F
    )
    assert encoded_offset == (1 if spec.row < "data:D_800969F8" else 3)
    assert facts[1]["value"]["selector"]["offset"] == encoded_offset
    assert facts[2]["value"]["selector"]["offset"] == encoded_offset


def _mutate_selector(spec: TableConsumerSpec, *, rs=None, rt=None, immediate=None):
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
def test_selector_immediate_is_decoded_from_exact_words(spec):
    mutated = _mutate_selector(spec, immediate=7)
    facts = decode_table_consumer(_operation(mutated), mutated, "b" * 64)
    assert facts[1]["value"]["selector"]["offset"] == 7
    assert facts[2]["value"]["selector"]["offset"] == 7
    with pytest.raises(ValueError):
        decode_table_consumer(_operation(mutated), spec, "b" * 64)


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
        decode_table_consumer(_operation(mutated), mutated, "b" * 64)


@pytest.mark.parametrize("spec", SPECS)
@pytest.mark.parametrize(
    "field",
    [
        "selected",
        "consumer",
        "selected_range",
        "selected_bytes",
        "consumer_bytes",
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
        "selected_range": [spec.selected_address + 4, spec.selected_end],
        "selected_bytes": "00" * len(spec.table_bytes),
        "consumer_bytes": "00" * (len(spec.consumer_words) * 4),
        "image_sha256": "bad",
    }
    payload[field] = mutations[field]
    if field == "consumer":
        operation["selector"] = mutations[field]
    with pytest.raises(ValueError):
        decode_table_consumer(operation, spec, "b" * 64)


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
    payload["consumer_range"] = consumer_range(spec)
    with pytest.raises(ValueError):
        decode_table_consumer(operation, spec, "b" * 64)


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
    payload["image_sha256"] = digest
    with pytest.raises(ValueError):
        decode_table_consumer(operation, spec, "b" * 64)


@pytest.mark.parametrize("source", SPECS)
@pytest.mark.parametrize("destination", SPECS)
def test_cross_spec_reuse_fails_closed(source, destination):
    if source == destination:
        return
    with pytest.raises(ValueError):
        decode_table_consumer(_operation(source), destination, "b" * 64)


@pytest.mark.parametrize("spec", SPECS)
def test_pointer_description_mutation_fails_closed(spec):
    operation = copy.deepcopy(_operation(spec))
    payload = operation["payload"]
    assert isinstance(payload, dict)
    payload["pointer_descriptions"][0][0]["target"] = "emi/battle/battle/03"
    with pytest.raises(ValueError):
        decode_table_consumer(operation, spec, "b" * 64)


def test_only_independently_reviewed_representatives_are_allowlisted():
    assert set(TABLE_CONSUMERS) == EXPECTED_KEYS


def test_adjacent_non_sibling_rows_are_not_allowlisted():
    rows = {row for row, _consumer in TABLE_CONSUMERS}
    assert rows.isdisjoint(
        {
            "data:D_800969E4",  # five-pointer table
            "data:D_80096A14",  # five-pointer table
            "data:D_80096A28",  # unrelated large consumer
            "data:D_80096A40",  # mixed pointer/constant object
        }
    )
