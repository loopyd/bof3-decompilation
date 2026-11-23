"""Semantic-name grammar: caps, prohibitions and the placeholder exemption."""

from __future__ import annotations

from pathlib import Path

from harness.naming import schema


TOKENS = {
    "emi/bmagic/magic001/00": "BmagicMagic00100",
    "emi/world02/area108/14": "World02Area10814",
    "emi/plchar/plp467/00": "PlcharPlp46700",
}


def report(name: str, **kwargs) -> schema.NameReport:
    return schema.name_report(name, tokens_by_target=TOKENS, **kwargs)


def test_name_tokens_keeps_digits_in_their_token() -> None:
    assert schema.name_tokens("DrawBand64At") == ("Draw", "Band64", "At")
    assert schema.name_tokens("func_80100000") == ("func", "80100000")


def test_target_token_is_composed_from_the_overlay_id() -> None:
    assert schema.target_token("emi/bmagic/magic001/00") == "BmagicMagic00100"
    assert schema.target_token("emi/world02/area108/14") == "World02Area10814"
    assert schema.target_token("exe/slus_004_22") is None
    assert schema.target_token("exe/logo") is None


def test_short_descriptive_name_passes() -> None:
    assert report("renderDrawQuadBand", target="emi/bmagic/magic001/00").passed


def test_identifier_case_follows_the_owned_filename() -> None:
    assert report("RenderDrawQuadBand").violations == ("case",)


def test_caps_are_reported() -> None:
    too_many_tokens = report("stateAdvanceWorkByteTwoAndReparkOnRandom")
    assert too_many_tokens.violations == ("tokens",)
    exactly_four = report("stateAdvanceWorkByte2")
    assert exactly_four.violations == ()
    too_long = report("rendertranslucentgouraudquadbandstreamarraybuffer")
    assert too_long.violations == ("length",)


def test_category_denylist_is_refused() -> None:
    assert report("helperAdvanceWorkByte").violations == ("category",)


def test_own_and_foreign_target_tokens_are_refused() -> None:
    own = report("stateAdvanceBmagicMagic00100", target="emi/bmagic/magic001/00")
    assert own.violations == ("own-token",)
    foreign = report("stateAdvanceWorld02Area10814", target="emi/bmagic/magic001/00")
    assert foreign.violations == ("foreign-token",)


def test_address_suffix_is_refused() -> None:
    assert report("stateAdvanceWorkByte_801EEFA4").violations == ("address",)


def test_raw_placeholder_is_exempt() -> None:
    assert report("func_801EEFA4", target="emi/bmagic/magic001/00").passed
    assert report("D_801CE404", target="emi/plchar/plp467/00").passed


def test_data_role_word_is_required_only_for_proved_data() -> None:
    assert report("dispatchWorkByte", kind="data").violations == ("role",)
    assert report("dispatchWorkTable", kind="data").passed
    assert report("dispatchWorkByte", kind="function").passed
    assert report("dispatchWorkByte").passed


def test_violation_order_is_stable() -> None:
    name = "helperAdvanceWorkByteAndReparkBmagicMagic00100_801EEFA4"
    assert report(name, target="emi/bmagic/magic001/00").violations == (
        "length",
        "tokens",
        "category",
        "address",
        "own-token",
    )


def test_index_covers_only_overlay_targets() -> None:
    rows = schema.token_index(["emi/plchar/plp467/00", "exe/logo"])
    assert rows == {"emi/plchar/plp467/00": "PlcharPlp46700"}
