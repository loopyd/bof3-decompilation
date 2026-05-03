from __future__ import annotations

from dataclasses import dataclass
from typing import Any


EMI_BINARY_RAM = "EMI_BINARY_RAM"
EMI_LARGE_RAM_BLOB = "EMI_LARGE_RAM_BLOB"
EMI_IMAGE_VRAM = "EMI_IMAGE_VRAM"
EMI_AUDIO_VH = "EMI_AUDIO_VH"
EMI_AUDIO_VB = "EMI_AUDIO_VB"
EMI_AUDIO_AUX = "EMI_AUDIO_AUX"
EMI_AUDIO_SEQ = "EMI_AUDIO_SEQ"
EMI_UNKNOWN = "EMI_UNKNOWN"


@dataclass(frozen=True)
class Classification:
    kind: str
    confidence: str
    score: int
    explanation: str

    def to_dict(self) -> dict[str, Any]:
        return {
            "confidence": self.confidence,
            "explanation": self.explanation,
            "kind": self.kind,
            "score": self.score,
        }


def emi_kind(raw_type: int, *, ram_ptr: int = 0, size: int = 0) -> str:
    if raw_type == 0:
        if ram_ptr >= 0x80000000:
            return EMI_LARGE_RAM_BLOB if size >= 0x20000 else EMI_BINARY_RAM
        return EMI_UNKNOWN
    if raw_type == 1:
        return EMI_LARGE_RAM_BLOB
    if raw_type == 3:
        return EMI_IMAGE_VRAM
    if raw_type == 6:
        return EMI_AUDIO_VH
    if raw_type == 7:
        return EMI_AUDIO_VB
    if raw_type == 8:
        return EMI_AUDIO_AUX
    if raw_type == 10:
        return EMI_AUDIO_SEQ
    return EMI_UNKNOWN


def classify_emi_entry(entry: dict[str, Any]) -> Classification:
    raw_type = int(entry.get("raw_type", entry.get("type", 0)) or 0)
    ram_ptr = int(entry.get("ram_ptr") or 0)
    size = int(entry.get("size") or 0)
    kind = emi_kind(raw_type, ram_ptr=ram_ptr, size=size)

    if kind == EMI_BINARY_RAM:
        if ram_ptr >= 0x80000000 and size >= 0x100:
            return Classification(
                kind=kind,
                confidence="high",
                score=90,
                explanation="type 0 payload with CPU RAM destination",
            )
        return Classification(
            kind=kind,
            confidence="medium",
            score=70,
            explanation="type 0 RAM payload, but size is small enough to be data",
        )
    if kind == EMI_LARGE_RAM_BLOB:
        return Classification(
            kind=kind,
            confidence="medium",
            score=55,
            explanation="large RAM payload; may contain code, data, or mixed content",
        )
    if kind == EMI_IMAGE_VRAM:
        return Classification(
            kind=kind,
            confidence="high",
            score=20,
            explanation="type 3 payload is treated as raw VRAM/image content",
        )
    if kind in {EMI_AUDIO_VH, EMI_AUDIO_VB, EMI_AUDIO_SEQ}:
        return Classification(
            kind=kind,
            confidence="high",
            score=5,
            explanation="audio payload type with known PSX sound role",
        )
    if kind == EMI_AUDIO_AUX:
        return Classification(
            kind=kind,
            confidence="medium",
            score=5,
            explanation="audio-side auxiliary payload type",
        )
    return Classification(
        kind=EMI_UNKNOWN,
        confidence="low",
        score=0,
        explanation=f"raw EMI type {raw_type} has no proven harness mapping",
    )
