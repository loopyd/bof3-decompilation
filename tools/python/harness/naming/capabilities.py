"""Code-owned reviewed Battle 15 naming analyzer specifications."""

from __future__ import annotations

from dataclasses import dataclass
from types import MappingProxyType
from typing import Mapping

from harness.naming.consumers import TableConsumerSpec


@dataclass(frozen=True)
class ExactCapabilitySpec:
    """One reviewed consumer and its exact required work item."""

    consumer: TableConsumerSpec
    required_work: str


ExactCapabilityRegistry = Mapping[tuple[str, str], ExactCapabilitySpec]

_TARGET = "emi/battle/battle/15"

# ponytail: this is an exact reviewed-spec registry, not automatic sibling discovery.
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


TABLE_CONSUMERS = MappingProxyType(
    {
        ("data:D_80096994", "emi/battle/battle/15@800ad26c"): TableConsumerSpec(
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
        ("data:D_800969A0", "emi/battle/battle/15@800ad69c"): TableConsumerSpec(
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
        ("data:D_800969AC", "emi/battle/battle/15@800ad9cc"): TableConsumerSpec(
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
        ("data:D_800969B8", "emi/battle/battle/15@800adcc4"): TableConsumerSpec(
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
        ("data:D_800969F8", "emi/battle/battle/15@800b01f0"): TableConsumerSpec(
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
        ("data:D_80096A08", "emi/battle/battle/15@800b09cc"): TableConsumerSpec(
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
        ("data:D_80096A34", "emi/battle/battle/15@800b138c"): TableConsumerSpec(
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

PRODUCTION_EXACT_CAPABILITIES: ExactCapabilityRegistry = MappingProxyType(
    {
        key: ExactCapabilitySpec(spec, f"access:{key[1]}")
        for key, spec in TABLE_CONSUMERS.items()
    }
)
