"""Canonical byte encoding for reconstruction provenance identities."""

from __future__ import annotations

from collections.abc import Mapping
from dataclasses import dataclass
from types import MappingProxyType

from harness.reconstruction.facterrors import FactRefused, require


@dataclass(frozen=True, eq=False)
class FrozenMap(Mapping):
    """Immutable string-keyed mapping: mapped, sorted, hashable, write-refused."""

    _pairs: tuple
    _index: object

    def __getitem__(self, key: str) -> object:
        return self._index[key]

    def __iter__(self):
        return iter(self._index)

    def __len__(self) -> int:
        return len(self._index)

    def __hash__(self) -> int:
        return hash(self._pairs)

    def __eq__(self, other: object) -> bool:
        return isinstance(other, FrozenMap) and self._pairs == other._pairs


def freeze(value: object) -> object:
    """Deep-freeze one value; an out-of-range integer refuses before any change."""

    if value is None or isinstance(value, bool):
        return value
    if isinstance(value, int):
        require(
            -(2**63) <= value <= 2**63 - 1,
            f"integer is outside the signed 64-bit encoding: {value}",
        )
        return value
    if isinstance(value, (str, bytes)):
        return value
    if isinstance(value, Mapping):
        pairs = []
        for key, item in value.items():
            require(isinstance(key, str), f"map key must be text: {key!r}")
            pairs.append((str(key), freeze(item)))
        ordered = tuple(sorted(pairs, key=lambda pair: pair[0].encode()))
        return FrozenMap(ordered, MappingProxyType(dict(ordered)))
    if isinstance(value, (list, tuple)):
        return tuple(freeze(item) for item in value)
    raise FactRefused(f"value cannot be frozen: {type(value).__name__}")


def _tagged(out: bytearray, tag: int, data: bytes) -> None:
    out.append(tag)
    out += len(data).to_bytes(4, "big") + data


def _encode(value: object, out: bytearray) -> None:
    if value is None:
        out.append(0x00)
    elif value is True:
        out.append(0x02)
    elif value is False:
        out.append(0x01)
    elif isinstance(value, int):
        out.append(0x03)
        out += int(value).to_bytes(8, "big", signed=True)
    elif isinstance(value, str):
        _tagged(out, 0x04, value.encode("utf-8"))
    elif isinstance(value, bytes):
        _tagged(out, 0x05, value)
    elif isinstance(value, tuple):
        out.append(0x06)
        out += len(value).to_bytes(4, "big")
        for item in value:
            _encode(item, out)
    elif isinstance(value, FrozenMap):
        out.append(0x07)
        out += len(value).to_bytes(4, "big")
        for key in value:
            encoded = key.encode("utf-8")
            out += len(encoded).to_bytes(4, "big") + encoded
            _encode(value[key], out)
    else:
        raise FactRefused(f"value has no canonical encoding: {type(value).__name__}")


def canonical_bytes(value: object) -> bytes:
    """Canonical record encoding: not JSON, never a transport ``v1:`` digest."""

    out = bytearray()
    _encode(freeze(value), out)
    return bytes(out)
