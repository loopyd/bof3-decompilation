import re
_SHA256 = re.compile(r"[0-9a-f]{64}")

@dataclass(frozen=True)
class CompanionStaticCall:
    caller_address: int
    target_address: int


@dataclass(frozen=True)
class CompanionAbi:
    target_address: int
    prototype: str
    evidence: str


@dataclass(frozen=True)
class CompanionOverlay:
    target: TargetId
    disc_id: str
    payload_sha256: str
    load_address: int
    size: int
    static_calls: tuple[CompanionStaticCall, ...]
    evidence: str
    abi: CompanionAbi | None = None


    companions: tuple[CompanionOverlay, ...] = ()
        if self.companions and self.kind != "emi":
            raise ValueError("only EMI targets may declare companion overlays")
def _parse_companions(raw: dict[str, Any], caller: TargetId) -> tuple[CompanionOverlay, ...]:
    values = raw.get("companion_overlays", [])
    if not isinstance(values, list):
        raise ValueError("companion_overlays must be an array of tables")
    companions: list[CompanionOverlay] = []
    seen: set[str] = set()
    for value in values:
        if not isinstance(value, dict):
            raise ValueError("companion overlay must be a table")
        target = normalize_target_id(str(value["target"]))
        disc_id = str(value["disc_id"])
        if target.kind != "emi" or normalize_target_id(disc_id).value != target.value:
            raise ValueError(f"invalid companion overlay identity: {disc_id}")
        if target.value == caller.value:
            raise ValueError(f"companion overlay cannot reference itself: {target.value}")
        if target.value in seen:
            raise ValueError(f"duplicate companion overlay: {target.value}")
        seen.add(target.value)
        digest = str(value["payload_sha256"])
        if _SHA256.fullmatch(digest) is None:
            raise ValueError(f"invalid companion payload SHA-256: {target.value}")
        load_address = int(value["load_address"])
        size = int(value["size"])
        if load_address % 4 or not 0x80000000 <= load_address < 0x80200000:
            raise ValueError(f"invalid companion load address: {target.value}")
        if size <= 0 or load_address + size > 0x80200000:
            raise ValueError(f"invalid companion payload size: {target.value}")
        calls = value.get("static_calls", [])
        if not isinstance(calls, list) or not calls:
            raise ValueError(f"missing companion static calls: {target.value}")
        static_calls: list[CompanionStaticCall] = []
        seen_calls: set[tuple[int, int]] = set()
        for call in calls:
            if not isinstance(call, dict):
                raise ValueError(f"invalid companion static call: {target.value}")
            caller_address = int(call["caller_address"])
            target_address = int(call["target_address"])
            key = (caller_address, target_address)
            if caller_address % 4 or target_address % 4:
                raise ValueError(f"unaligned companion static call: {target.value}")
            if not load_address <= target_address < load_address + size:
                raise ValueError(f"companion call outside payload: {target.value}")
            if key in seen_calls:
                raise ValueError(f"duplicate companion static call: {target.value}")
            seen_calls.add(key)
            static_calls.append(CompanionStaticCall(*key))
        evidence = str(value["evidence"]).strip()
        if not evidence:
            raise ValueError(f"missing companion evidence: {target.value}")
        abi_raw = value.get("abi")
        abi = None
        if abi_raw is not None:
            if not isinstance(abi_raw, dict):
                raise ValueError(f"invalid companion ABI: {target.value}")
            target_address = int(abi_raw.get("target_address", 0))
            prototype = str(abi_raw.get("prototype", "")).strip()
            abi_evidence = str(abi_raw.get("evidence", "")).strip()
            call_targets = {call.target_address for call in static_calls}
            if target_address not in call_targets or not prototype or not abi_evidence:
                raise ValueError(f"missing companion ABI evidence: {target.value}")
            abi = CompanionAbi(target_address, prototype, abi_evidence)
        companions.append(
            CompanionOverlay(
                target=target,
                disc_id=disc_id,
                payload_sha256=digest,
                load_address=load_address,
                size=size,
                static_calls=tuple(static_calls),
                evidence=evidence,
                abi=abi,
            )
        )
    return tuple(companions)


def _validate_companions(manifests: dict[str, TargetManifest]) -> None:
    for manifest in manifests.values():
        for companion in manifest.companions:
            target = manifests.get(companion.target.value)
            if target is None:
                raise ValueError(f"unknown companion overlay: {companion.target.value}")
            if (
                target.kind != "emi"
                or target.disc_id != companion.disc_id
                or target.load_address != companion.load_address
            ):
                raise ValueError(
                    f"companion overlay identity mismatch: {companion.target.value}"
                )


            companions=_parse_companions(raw, target_id),
    _validate_companions(manifests)
