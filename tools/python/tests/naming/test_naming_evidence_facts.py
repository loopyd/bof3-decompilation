"""Trusted naming-evidence fact derivation tests."""

import hashlib
from typing import Any

import harness.naming.evidence as facts
import pytest
from test_naming_table_consumer import DATA_CONSUMER_CASES, SPECS, _operation, _range
from harness.naming.consumers import decode_dispatch_consumer
from pathlib import Path

TARGET = "emi/battle/battle/15"
SELECTOR = f"{TARGET}@800AD26C"
DATA_SELECTOR = f"{TARGET}@80096994"


def _describe(*, selector=SELECTOR, role=None, **changes):  # type: ignore[reportUnknownParameterType]
    row = {
        "target": TARGET,
        "address": "0x800AD26C",
        "payload": {
            "contained": True,
            "payload_offset": "0x123",
            "file_offset": "0x923",
            "remaining_bytes": 64,
        },
        "splat": {
            "kind": "c",
            "start": "0x800AD26C",
            "end": "0x800AD300",
            "name": "func_800AD26C",
        },
        "symbol": {"name": "func_800AD26C", "kind": "function"},
        "storage": None,
        "references": [],
    }
    row.update(changes)
    return {
        "item": {"id": f"describe:{selector}"},
        "operation": "describe",
        "selector": selector,
        "supplemental": True,
        "role": role
        or (
            "selected_data_describe"
            if selector.lower() == DATA_SELECTOR.lower()
            else "access_function_describe"
        ),
        "payload": [row],
    }


def _access(**changes):
    row = {
        "source": 0x800AD278,
        "address": 0x80096994,
        "function_id": SELECTOR,
        "access_kind": "address",
        "opcode": "addiu",
    }
    row.update(changes)
    return {
        "item": {"id": f"access:{SELECTOR}"},
        "operation": "access",
        "selector": SELECTOR,
        "supplemental": False,
        "payload": [row],
    }


def _analyze(operations, *, row: dict[str, Any] | None = None):
    return facts._analyze_validated_records(
        facts._RUNNER_TOKEN,
        target=TARGET,
        report_digest=hashlib.sha256(b"report").hexdigest(),
        row=row or {"kind": "data", "name": "D_80096994"},
        operations=tuple(operations),
    )


def _external_owner(*, operation="owner", payload_target="emi/battle/battle/03"):
    selector = "emi/battle/battle/03@801EABB8"
    return {
        "item": {"id": f"owner:{selector}"},
        "operation": operation,
        "selector": selector,
        "supplemental": False,
        "payload": [{"target_id": payload_target}],
    }


def test_external_owner_stays_an_open_unavailable_gap():
    result = _analyze(
        [_external_owner()],
        row={"kind": "data", "name": "D_801EB2E8", "outside_payload": True},
    )
    assert result["target"] == TARGET
    assert result["facts"] == []
    assert {item["class"] for item in result["open"]} >= {"owner_resolution"}
    assert {item["operation"] for item in result["unavailable"]} >= {"owner"}
    assert result["conclusion_enabled"] is False


@pytest.mark.parametrize(
    ("operation", "outside"), [("access", True), ("owner", False), ("owner", None)]
)
def test_cross_target_exception_is_owner_outside_payload_only(operation, outside):
    row: dict[str, Any] = {"kind": "data", "name": "D_801EB2E8"}
    if outside is not None:
        row["outside_payload"] = outside
    with pytest.raises(ValueError, match="analyzer operation target mismatch"):
        _analyze([_external_owner(operation=operation)], row=row)


@pytest.mark.parametrize("payload_target", [TARGET, "emi/etc/game/00"])
def test_external_owner_payload_target_is_never_rewritten(payload_target):
    with pytest.raises(ValueError, match="external owner payload target mismatch"):
        _analyze(
            [_external_owner(payload_target=payload_target)],
            row={"kind": "data", "name": "D_801EB2E8", "outside_payload": True},
        )


def test_lowercase_native_selector_is_compared_by_identity():
    result = _analyze([_describe(selector=SELECTOR.lower()), _access()])
    assert any(item["class"] == "selected_access" for item in result["facts"])


def test_data_describe_and_function_access_produce_storage_and_access_facts():
    storage = {
        "kind": "rodata",
        "start": "0x80096994",
        "end": "0x800969A0",
        "file_offset": "0x194",
        "present_in_binary": True,
        "authority": ["reviewed_splat", "original_binary"],
    }
    described = _describe(
        selector=DATA_SELECTOR,
        address="0x80096994",
        payload={
            "contained": True,
            "payload_offset": "0x194",
            "file_offset": "0x194",
            "remaining_bytes": 132912,
        },
        splat={
            "kind": "rodata",
            "start": "0x80096800",
            "end": "0x80096AB0",
            "name": "T_80096800",
        },
        storage=storage,
        symbol={"name": "D_80096994", "kind": "data"},
    )
    result = _analyze([described, _describe(), _access()])
    classes = {item["class"] for item in result["facts"]}
    assert {"storage_class", "selected_access"} <= classes
    assert "selected_range" not in classes


def test_real_hex_describe_and_access_produce_positive_facts():
    result = _analyze([_describe(), _access()])
    access = next(
        item for item in result["facts"] if item["class"] == "selected_access"
    )
    assert access["value"]["site"] == "0x800AD278"
    assert "width" not in access["value"]
    assert result["conclusion_enabled"] is False


@pytest.mark.parametrize(
    "change",
    [
        {"source": True},
        {"source": -1},
        {"source": 0x800AD400},
        {"access_kind": "invented"},
        {"access_kind": "load", "opcode": "sw"},
        {"access_kind": "store", "opcode": "lw"},
        {"opcode": ""},
        {"opcode": True},
        {"opcode": "lw.fake", "access_kind": "load"},
        {"opcode": "xlw", "access_kind": "load"},
        {"opcode": "lw_suffix", "access_kind": "load"},
        {"function_id": "exe/logo@800AD26C"},
    ],
)
def test_malformed_access_emits_no_fact(change):
    result = _analyze([_describe(), _access(**change)])
    assert not [item for item in result["facts"] if item["class"] == "selected_access"]
    assert result["unavailable"]


@pytest.mark.parametrize(
    "change",
    [
        {"target": "exe/logo"},
        {"address": "800AD26C"},
        {
            "splat": {
                "kind": "c",
                "start": "0x800AD300",
                "end": "0x800AD400",
                "name": "text",
            }
        },
        {"address": "0x800AD270", "symbol": {"name": "other", "kind": "function"}},
    ],
)
def test_malformed_describe_emits_no_range(change):
    result = _analyze([_describe(**change)])
    assert not [item for item in result["facts"] if item["class"] == "selected_range"]
    assert result["unavailable"]


@pytest.mark.parametrize(
    "storage",
    [
        {
            "kind": "rodata",
            "start": "0x80096994",
            "end": "0x80096994",
            "file_offset": "0x194",
            "present_in_binary": True,
            "authority": ["reviewed_splat"],
        },
        {
            "kind": "rodata",
            "start": "0x800969A0",
            "end": "0x80096994",
            "file_offset": "0x194",
            "present_in_binary": True,
            "authority": ["reviewed_splat"],
        },
        {
            "kind": "rodata",
            "start": "0x80096994",
            "end": "0x800969A0",
            "file_offset": True,
            "present_in_binary": True,
            "authority": ["reviewed_splat"],
        },
        {
            "kind": "rodata",
            "start": "0x80096994",
            "end": "0x800969A0",
            "file_offset": "0x194",
            "present_in_binary": True,
            "authority": ["invented"],
        },
    ],
)
def test_malformed_storage_emits_no_storage_fact(storage):
    result = _analyze(
        [
            _describe(
                selector=DATA_SELECTOR,
                address="0x80096994",
                splat={
                    "kind": "rodata",
                    "start": "0x80096800",
                    "end": "0x80096AB0",
                    "name": "T_80096800",
                },
                storage=storage,
                symbol={"name": "D_80096994", "kind": "data"},
            )
        ]
    )
    assert not [item for item in result["facts"] if item["class"] == "storage_class"]


@pytest.mark.parametrize(
    ("symbol_kind", "splat_kind"),
    [("function", "rodata"), ("data", "c"), ("unknown", "rodata")],
)
def test_selected_data_requires_data_symbol_and_non_function_segment(
    symbol_kind, splat_kind
):
    described = _describe(
        selector=DATA_SELECTOR,
        address="0x80096994",
        splat={
            "kind": splat_kind,
            "start": "0x80096800",
            "end": "0x80096AB0",
            "name": "T_80096800",
        },
        symbol={"name": "D_80096994", "kind": symbol_kind},
    )
    result = _analyze([described])
    assert not [item for item in result["facts"] if item["class"] == "selected_range"]


def test_access_context_requires_function_symbol_and_text_segment():
    for change in (
        {"symbol": {"name": "func_800AD26C", "kind": "data"}},
        {
            "splat": {
                "kind": "data",
                "start": "0x800AD26C",
                "end": "0x800AD300",
                "name": "func_800AD26C",
            }
        },
    ):
        result = _analyze([_describe(**change), _access()])
        assert not [
            item for item in result["facts"] if item["class"] == "selected_access"
        ]


@pytest.mark.parametrize(
    ("storage", "payload"),
    [
        (
            {
                "kind": "rodata",
                "start": "0x80096994",
                "end": "0x800969A0",
                "file_offset": "0x194",
                "present_in_binary": True,
                "authority": ["reviewed_splat", "original_binary"],
            },
            {
                "contained": False,
                "payload_offset": None,
                "file_offset": None,
                "remaining_bytes": 0,
            },
        ),
        (
            {
                "kind": "rodata",
                "start": "0x80096994",
                "end": "0x800969A0",
                "file_offset": "0x194",
                "present_in_binary": True,
                "authority": ["reviewed_splat", "original_binary"],
            },
            {
                "contained": True,
                "payload_offset": "0x194",
                "file_offset": "0x194",
                "remaining_bytes": 4,
            },
        ),
        (
            {
                "kind": "bss",
                "start": "0x80096994",
                "end": "0x800969A0",
                "file_offset": None,
                "present_in_binary": False,
                "authority": ["reviewed_splat"],
            },
            {
                "contained": True,
                "payload_offset": "0x194",
                "file_offset": "0x194",
                "remaining_bytes": 12,
            },
        ),
    ],
)
def test_storage_presence_and_extent_must_agree(storage, payload):
    described = _describe(
        selector=DATA_SELECTOR,
        address="0x80096994",
        payload=payload,
        splat={
            "kind": storage["kind"],
            "start": "0x80096800",
            "end": "0x80096AB0",
            "name": "T_80096800",
        },
        storage=storage,
        symbol={"name": "D_80096994", "kind": "data"},
    )
    result = _analyze([described])
    assert not [
        item
        for item in result["facts"]
        if item["class"] in {"selected_range", "storage_class"}
    ]


def test_selected_data_nested_identity_must_match_row():
    described = _describe(
        selector=DATA_SELECTOR,
        address="0x80096994",
        payload={
            "contained": True,
            "payload_offset": "0x194",
            "file_offset": "0x194",
            "remaining_bytes": 132912,
        },
        splat={
            "kind": "rodata",
            "start": "0x80096800",
            "end": "0x80096AB0",
            "name": "T_80096800",
        },
        symbol={"name": "D_80096998", "kind": "data"},
    )
    result = _analyze([described])
    assert not [item for item in result["facts"] if item["class"] == "selected_range"]
    assert result["unavailable"]


def test_storage_redundancies_must_agree():
    storage = {
        "kind": "rodata",
        "start": "0x80096994",
        "end": "0x800969A0",
        "file_offset": "0x195",
        "present_in_binary": True,
        "authority": ["reviewed_splat", "original_binary"],
    }
    described = _describe(
        selector=DATA_SELECTOR,
        address="0x80096994",
        payload={
            "contained": True,
            "payload_offset": "0x194",
            "file_offset": "0x194",
            "remaining_bytes": 132912,
        },
        splat={
            "kind": "rodata",
            "start": "0x80096800",
            "end": "0x80096AB0",
            "name": "T_80096800",
        },
        storage=storage,
        symbol={"name": "D_80096994", "kind": "data"},
    )
    result = _analyze([described])
    assert not [
        item
        for item in result["facts"]
        if item["class"] in {"selected_range", "storage_class"}
    ]
    assert any("storage" in item["reason"] for item in result["unavailable"])


def test_canonical_bss_describe_produces_storage_without_original_range():
    storage = {
        "kind": "bss",
        "start": "0x80096994",
        "end": "0x800969A0",
        "file_offset": None,
        "present_in_binary": False,
        "authority": ["reviewed_splat"],
    }
    described = _describe(
        selector=DATA_SELECTOR,
        address="0x80096994",
        payload={
            "contained": False,
            "payload_offset": None,
            "file_offset": None,
            "remaining_bytes": 0,
        },
        splat={
            "kind": "bss",
            "start": "0x80096800",
            "end": "0x80096AB0",
            "name": "B_80096800",
        },
        storage=storage,
        symbol={"name": "D_80096994", "kind": "data"},
    )
    classes = {item["class"] for item in _analyze([described])["facts"]}
    assert classes == {"storage_class"}


@pytest.mark.parametrize(
    ("storage_change", "payload_change"),
    [
        ({"file_offset": "0x194"}, {}),
        ({"present_in_binary": True}, {}),
        ({"authority": ["reviewed_splat", "original_binary"]}, {}),
        ({}, {"contained": True}),
        ({}, {"payload_offset": "0x194"}),
        ({}, {"file_offset": "0x194"}),
        ({}, {"remaining_bytes": False}),
        ({}, {"remaining_bytes": True}),
        ({}, {"remaining_bytes": 0.0}),
        ({}, {"remaining_bytes": 1.0}),
        ({}, {"remaining_bytes": "0"}),
        ({}, {"remaining_bytes": None}),
        ({}, {"remaining_bytes": -1}),
        ({}, {"remaining_bytes": 1}),
    ],
)
def test_malformed_bss_emits_no_storage_fact(storage_change, payload_change):
    storage = {
        "kind": "bss",
        "start": "0x80096994",
        "end": "0x800969A0",
        "file_offset": None,
        "present_in_binary": False,
        "authority": ["reviewed_splat"],
        **storage_change,
    }
    payload = {
        "contained": False,
        "payload_offset": None,
        "file_offset": None,
        "remaining_bytes": 0,
        **payload_change,
    }
    result = _analyze(
        [
            _describe(
                selector=DATA_SELECTOR,
                address="0x80096994",
                payload=payload,
                splat={
                    "kind": "bss",
                    "start": "0x80096800",
                    "end": "0x80096AB0",
                    "name": "B_80096800",
                },
                storage=storage,
                symbol={"name": "D_80096994", "kind": "data"},
            )
        ]
    )
    assert not [
        item
        for item in result["facts"]
        if item["class"] in {"selected_range", "storage_class"}
    ]


def test_same_address_roles_do_not_confuse_data_and_function_describes():
    selected = _describe(
        selector=DATA_SELECTOR,
        address="0x80096994",
        splat={
            "kind": "rodata",
            "start": "0x80096800",
            "end": "0x80096AB0",
            "name": "T_80096800",
        },
        symbol={"name": "D_80096994", "kind": "data"},
        role="access_function_describe",
    )
    result = _analyze([selected])
    assert not [item for item in result["facts"] if item["class"] == "selected_range"]
    assert result["unavailable"]


def test_describe_role_mismatch_is_rejected():
    result = _analyze([_describe(role="selected_data_describe")])
    assert not result["facts"]
    assert result["unavailable"]


def _dispatch_consumer(**changes):
    spec = SPECS[0]
    operation = _operation(spec)
    operation.update(
        {
            "item": {"id": f"data_dispatch_consumer:{DATA_SELECTOR}:{SELECTOR}"},
            "operation": "data_dispatch_consumer",
            "supplemental": True,
            "role": "data_dispatch_consumer",
        }
    )
    operation["payload"].update(changes)
    return operation


def _original_range(spec=SPECS[0]):
    raw = bytearray(688)
    raw[spec.selected_address - 0x80096800 : spec.selected_end - 0x80096800] = (
        spec.table_bytes
    )
    return {
        "item": {
            "id": f"data_original_range:{spec.target}@{spec.selected_address:08x}"
        },
        "operation": "data_original_range",
        "selector": f"{spec.target}@{spec.selected_address:08x}",
        "supplemental": True,
        "role": "data_original_range",
        "payload": {
            "selector": f"{spec.target}@{spec.selected_address:08x}",
            "binding": {
                "scope": "containing_reviewed_segment",
                "kind": "rodata",
                "start": 0x80096800,
                "end": 0x80096AB0,
                "offset": 0,
                "image_sha256": spec.image_sha256,
                "splat_sha256": "a" * 64,
                "replay_sha256": "b" * 64,
            },
            "bytes": raw.hex(),
            "initializer": _range(spec)["initializer"],
            "gap": None,
        },
    }


def _assert_live_rejected(operation, monkeypatch):
    monkeypatch.setattr(
        facts, "capture_dispatch_consumer", lambda *_: _dispatch_consumer()["payload"]
    )
    with pytest.raises(ValueError, match="current original"):
        facts._validate_consumer_bytes(Path("/unused"), TARGET, operation["payload"])


def test_complete_battle15_facts_emit_typed_capability():
    storage = {
        "kind": "rodata",
        "start": "0x80096994",
        "end": "0x800969A0",
        "file_offset": "0x194",
        "present_in_binary": True,
        "authority": ["reviewed_splat", "original_binary"],
    }
    selected = _describe(
        selector=DATA_SELECTOR,
        address="0x80096994",
        payload={
            "contained": True,
            "payload_offset": "0x194",
            "file_offset": "0x194",
            "remaining_bytes": 12,
        },
        splat={
            "kind": "rodata",
            "start": "0x80096800",
            "end": "0x80096AB0",
            "name": "T_80096800",
        },
        storage=storage,
        symbol={"name": "D_80096994", "kind": "data"},
    )
    original_range = _original_range()
    result = _analyze(
        [original_range, selected, _describe(), _access(), _dispatch_consumer()]
    )
    capability = result["conclusion_capability"]
    assert result["conclusion_enabled"] is True
    assert capability["allowed_conclusion"] == "exhausted"
    assert capability["proposal_allowed"] is True
    assert set(capability["rungs"]) == {
        "selected_range",
        "selected_access",
        "storage_class",
        "one_level_beyond",
    }
    assert capability["required_work"] == ["access:emi/battle/battle/15@800AD26C"]


def test_unavailable_record_disables_complete_conclusion_capability():
    storage = {
        "kind": "rodata",
        "start": "0x80096994",
        "end": "0x800969A0",
        "file_offset": "0x194",
        "present_in_binary": True,
        "authority": ["reviewed_splat", "original_binary"],
    }
    selected = _describe(
        selector=DATA_SELECTOR,
        address="0x80096994",
        payload={
            "contained": True,
            "payload_offset": "0x194",
            "file_offset": "0x194",
            "remaining_bytes": 12,
        },
        splat={
            "kind": "rodata",
            "start": "0x80096800",
            "end": "0x80096AB0",
            "name": "T_80096800",
        },
        storage=storage,
        symbol={"name": "D_80096994", "kind": "data"},
    )
    invalid_access = _access()
    invalid_access["payload"][0]["opcode"] = "invalid"
    original_range = _original_range()
    result = _analyze(
        [
            original_range,
            selected,
            _describe(),
            _access(),
            _dispatch_consumer(),
            invalid_access,
        ]
    )

    assert result["open"] == []
    assert result["unavailable"] == [
        {"operation": "access", "reason": "access row failed domain validation"}
    ]
    assert result["conclusion_enabled"] is False
    assert "conclusion_capability" not in result


def test_dispatch_consumer_derives_layout_loads_and_one_level_fact():
    result = _analyze([_original_range(), _dispatch_consumer()])
    classes = [item["class"] for item in result["facts"]]
    assert classes == ["selected_range", "selected_access", "one_level_beyond"]
    assert result["facts"][1]["value"]["load_sites"] == [
        "0x800AD27C",
        "0x800AD280",
        "0x800AD284",
    ]
    assert not [item for item in result["open"] if item["class"] == "one_level_beyond"]
    assert result["conclusion_enabled"] is False


@pytest.mark.parametrize(
    ("target", "address", "change"),
    [
        (TARGET, 0x80096994, {"consumer": f"{TARGET}@800AD270"}),
        (TARGET, 0x80096998, {"selected": f"{TARGET}@80096998"}),
        (
            "emi/battle/battle/03",
            0x80096994,
            {
                "selected": "emi/battle/battle/03@80096994",
                "consumer": "emi/battle/battle/03@800AD26C",
            },
        ),
    ],
)
def test_dispatch_consumer_is_bound_to_reviewed_identities(
    target, address, change, monkeypatch
):
    operation = _dispatch_consumer(**change)
    _assert_live_rejected(operation, monkeypatch)


def test_dispatch_consumer_rejects_same_bytes_at_shifted_range(monkeypatch):
    operation = _dispatch_consumer(consumer=f"{TARGET}@800AD000")
    operation["payload"]["binding"].update(start=0x800AD000, end=0x800AD080)
    _assert_live_rejected(operation, monkeypatch)


@pytest.mark.parametrize("index", [2, 3, 10, 11, 12, 13, 14, 15, 24, 25, 26, 27])
def test_dispatch_consumer_rejects_altered_fixed_global(index, monkeypatch):
    _assert_live_rejected(
        _mutate_consumer_word(_dispatch_consumer(), index, 0), monkeypatch
    )


def _mutate_consumer_word(operation, index, word):
    payload = operation["payload"]
    code = bytearray.fromhex(payload["bytes"])
    code[index * 4 : index * 4 + 4] = word.to_bytes(4, "little")
    payload["bytes"] = code.hex()
    return operation


@pytest.mark.parametrize(
    ("index", "word"),
    [
        (4, 0x8CA00000),  # table source value is zero
        (7, 0xAFA00010),  # copied table value is zero
        (7, 0xAFA20014),  # copied table offset aliases next slot
        (7, 0xAFC20010),  # copied table uses a different frame base
        (16, 0x90600001),  # lbu destination is zero
        (17, 0x10000000),  # branch occupies selector delay slot
        (17, 0x24020001),  # selector register clobber
        (17, 0x241D0001),  # frame/table base clobber
        (18, 0x001D1080),  # sll source aliases frame base
        (18, 0x0002E880),  # sll destination aliases frame base
        (19, 0x00421020),  # signed add instead of exact addu
        (19, 0x10400000),  # branch can skip the dispatch
        (20, 0x24020001),  # computed dispatch address clobber
        (21, 0x24020001),  # loaded target clobber
        (21, 0x24030001),  # unrelated write interrupts the exact chain
        (23, 0x10000000),  # jalr delay slot is not nop
        (24, 0x1000FFFF),  # epilogue can re-enter the dispatch
    ],
)
def test_dispatch_consumer_rejects_clobbered_or_alternate_chain(
    index, word, monkeypatch
):
    _assert_live_rejected(
        _mutate_consumer_word(_dispatch_consumer(), index, word), monkeypatch
    )


def test_non_word_dispatch_without_reviewed_spec_stays_open(monkeypatch):
    operation = _dispatch_consumer(selected="emi/battle/battle/15@800B6F50")
    _assert_live_rejected(operation, monkeypatch)


def test_reviewed_dispatch_spec_rejects_non_word_payload(monkeypatch):
    operation = _dispatch_consumer(bytes="00")
    _assert_live_rejected(operation, monkeypatch)


def test_dispatch_consumer_rejects_unrelated_local_table_base(monkeypatch):
    _assert_live_rejected(
        _mutate_consumer_word(_dispatch_consumer(), 19, 0x02021021), monkeypatch
    )


@pytest.mark.parametrize("size", [1, 2, 3, 5, 95, 97, 1028])
def test_dispatch_consumer_rejects_malformed_consumer_extent(size, monkeypatch):
    operation = _dispatch_consumer(bytes="00" * size)
    operation["payload"]["binding"]["end"] = 0x800AD26C + size
    _assert_live_rejected(operation, monkeypatch)


@pytest.mark.parametrize("value", [None, False, 1, [], {}])
def test_dispatch_consumer_rejects_non_string_consumer_bytes(value, monkeypatch):
    _assert_live_rejected(_dispatch_consumer(bytes=value), monkeypatch)


@pytest.mark.parametrize(
    "change",
    [
        {"selected_bytes": "00" * 12},
        {"consumer_bytes": "00" * 96},
        {"selected_range": [0x80096994, 0x800969A4]},
        {"consumer": f"{TARGET}@800AD270"},
        {"image_sha256": "bad"},
        {"pointer_descriptions": []},
    ],
)
def test_dispatch_consumer_tampering_fails_closed(change, monkeypatch):
    _assert_live_rejected(_dispatch_consumer(**change), monkeypatch)


@pytest.mark.parametrize(
    ("row", "consumer"),
    [
        ("data:D_800969AC", f"{TARGET}@800AD9CC"),
        ("data:D_800969B8", f"{TARGET}@800ADCC4"),
    ],
)
def test_next_table_consumer_specs_accept_only_exact_original_evidence(row, consumer):
    spec = DATA_CONSUMER_CASES[(row, consumer.lower())]
    operation = _operation(spec)
    derived = decode_dispatch_consumer(operation, TARGET, _range(spec), "0" * 64)
    assert [fact["class"] for fact in derived] == [
        "selected_access",
        "one_level_beyond",
    ]
    operation["payload"]["binding"]["end"] += 4
    with pytest.raises(ValueError, match="binding"):
        decode_dispatch_consumer(operation, TARGET, _range(spec), "0" * 64)


@pytest.mark.parametrize(
    ("row", "consumer"),
    [
        ("data:D_800969AC", f"{TARGET}@800AD9CC"),
        ("data:D_800969B8", f"{TARGET}@800ADCC4"),
    ],
)
def test_next_table_consumer_specs_require_complete_capability_evidence(row, consumer):
    spec = DATA_CONSUMER_CASES[(row, consumer.lower())]
    operation = _operation(spec)
    operation.update(
        {
            "item": {"id": f"data_dispatch_consumer:{row}:{consumer}"},
            "operation": "data_dispatch_consumer",
            "supplemental": True,
            "role": "data_dispatch_consumer",
        }
    )
    result = facts._analyze_validated_records(
        facts._RUNNER_TOKEN,
        target=TARGET,
        report_digest=hashlib.sha256(b"report").hexdigest(),
        row={"kind": "data", "name": row.removeprefix("data:")},
        operations=(operation,),
    )
    assert result["facts"] == []
    assert result["conclusion_enabled"] is False
    assert result["unavailable"]


def test_internal_token_required():
    with pytest.raises(ValueError, match="repository/report"):
        facts._analyze_validated_records(
            object(),
            target=TARGET,
            report_digest="0" * 64,
            row={"kind": "data", "name": "D_80096994"},
            operations=(),
        )


def test_empty_indexed_access_is_not_negative_coverage():
    access = _access()
    access["payload"] = []
    result = _analyze([_describe(), access])
    assert not any(fact["class"] == "selected_access" for fact in result["facts"])
    assert any(item["class"] == "selected_access" for item in result["open"])
    assert result["conclusion_enabled"] is False
    assert "conclusion_capability" not in result
