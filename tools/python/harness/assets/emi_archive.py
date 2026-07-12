from __future__ import annotations

import struct
from dataclasses import dataclass
from pathlib import Path


EMI_MAGIC = b"MATH_TBL"
EMI_SECTOR_SIZE = 0x800


@dataclass(frozen=True)
class EmiEntry:
    index: int
    size: int
    load_arg: int
    first_word: int
    type_id: int
    unk: int
    payload_offset: int

    @property
    def suffix(self) -> str:
        return entry_suffix(self.type_id)

    @property
    def default_name(self) -> str:
        return f"{self.index}{self.suffix}"


class EmiArchive:
    def __init__(self, path: Path) -> None:
        self.path = path
        self.bytes = path.read_bytes()
        if len(self.bytes) < EMI_SECTOR_SIZE or self.bytes[8:16] != EMI_MAGIC:
            raise ValueError(f"invalid EMI archive: {path}")

        entry_count = struct.unpack_from("<I", self.bytes, 0)[0]
        payload_offset = EMI_SECTOR_SIZE
        entries: list[EmiEntry] = []

        for index in range(entry_count):
            size, load_arg, first_word, type_id, unk = struct.unpack_from(
                "<IIIHH", self.bytes, 0x10 + index * 0x10
            )
            if payload_offset + size > len(self.bytes):
                raise ValueError(f"truncated EMI entry {index} in {path}")
            entries.append(
                EmiEntry(
                    index=index,
                    size=size,
                    load_arg=load_arg,
                    first_word=first_word,
                    type_id=type_id,
                    unk=unk,
                    payload_offset=payload_offset,
                )
            )
            payload_offset += (
                (size + EMI_SECTOR_SIZE - 1) // EMI_SECTOR_SIZE
            ) * EMI_SECTOR_SIZE

        self.entries = entries

    def entry(self, index: int) -> EmiEntry:
        return self.entries[index]

    def payload(self, index: int) -> bytes:
        entry = self.entry(index)
        return self.bytes[entry.payload_offset : entry.payload_offset + entry.size]


def entry_suffix(type_id: int) -> str:
    if type_id == 3:
        return ".img"
    if type_id == 6:
        return ".vh"
    if type_id == 7:
        return ".vb"
    if type_id == 10:
        return ".seq"
    return ".bin"
