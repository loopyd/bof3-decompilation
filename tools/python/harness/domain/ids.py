"""Canonical target and function identity.

The disc spelling is intentionally retained on :class:`TargetId` so command
output can show the user's input alongside the normalized build identity.
"""

from __future__ import annotations

from dataclasses import dataclass
import re


_EXE_NAMES = {
    "slus_004.22": "exe/slus_004_22",
    "slus_004_22": "exe/slus_004_22",
    "logo/logo.exe": "exe/logo",
    "logo.exe": "exe/logo",
}
FUNCTION_ID_FORMAT = "TARGET@0xADDRESS"
FUNCTION_ID_HELP = (
    "TARGET@0xADDRESS; EMI targets may use BIN/FAMILY/ARCHIVE.EMI#INDEX@0xADDRESS"
)

_FUNCTION_RE = re.compile(r"^(?P<target>.+)@(?P<address>(?:0x)?[0-9a-fA-F]{8})$")


@dataclass(frozen=True)
class TargetId:
    """A normalized target plus the shipped identifier used to resolve it."""

    value: str
    shipped: str

    @property
    def kind(self) -> str:
        return "executable" if self.value.startswith("exe/") else "emi"

    def __str__(self) -> str:
        return self.value


@dataclass(frozen=True)
class FunctionId:
    target: TargetId
    address: int

    @property
    def value(self) -> str:
        return f"{self.target.value}@{self.address:08x}"

    def __str__(self) -> str:
        return self.value


def _normalize_emi(raw: str) -> TargetId:
    value = raw.strip().replace("\\", "/")
    if value.lower().startswith("bin/"):
        value = value[4:]
    if "#" not in value:
        # Already-normalized IDs are accepted directly.
        if value.lower().startswith("emi/"):
            return TargetId(value.lower(), raw)
        raise ValueError("EMI target must include an archive slot (#N)")
    archive, slot_text = value.rsplit("#", 1)
    slot = int(slot_text, 10)
    if slot < 0 or slot > 99:
        raise ValueError("EMI slot must be between 0 and 99")
    parts = [part for part in archive.split("/") if part]
    if len(parts) < 2:
        raise ValueError("EMI target must include a family and archive")
    family = parts[0].lower()
    archive_name = "/".join(parts[1:]).lower()
    if archive_name.endswith(".emi"):
        archive_name = archive_name[:-4]
    return TargetId(f"emi/{family}/{archive_name}/{slot:02d}", raw)


def normalize_target_id(value: str) -> TargetId:
    """Normalize a shipped executable or EMI identifier."""

    raw = value.strip()
    if not raw:
        raise ValueError("target identifier must not be empty")
    key = raw.replace("\\", "/").lower()
    if key in _EXE_NAMES:
        return TargetId(_EXE_NAMES[key], raw)
    if key.startswith("exe/"):
        normalized = key.replace(".", "_")
        return TargetId(normalized, raw)
    if key.startswith("emi/"):
        parts = key.split("/")
        if len(parts) == 4 and parts[-1].isdigit():
            return TargetId(f"emi/{parts[1]}/{parts[2]}/{int(parts[3]):02d}", raw)
    return _normalize_emi(raw)


def parse_function_id(value: str) -> FunctionId:
    """Parse the shared function selector accepted by harness commands.

    Executables use a target name such as ``SLUS_004.22@0x8014AE08``. An EMI
    entry uses its archive path and slot, for example
    ``BIN/BATTLE/BATL_END.EMI#0@0x800AF66C``.
    """
    match = _FUNCTION_RE.match(value.strip())
    if match is None:
        raise ValueError(
            f"function ID must be TARGET@8-digit-address ({FUNCTION_ID_HELP})"
        )
    return FunctionId(
        target=normalize_target_id(match.group("target")),
        address=int(match.group("address"), 16),
    )


def parse_address(value: str) -> int:
    """Parse the address spellings accepted by all function workflows."""

    raw = value.strip().removeprefix("func_")
    if not raw:
        raise ValueError("function address must not be empty")
    try:
        return int(raw, 0 if raw.lower().startswith("0x") else 16)
    except ValueError as exc:
        raise ValueError(f"invalid function address: {value}") from exc
