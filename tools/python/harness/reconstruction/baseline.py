"""Load the fixed reconstruction corpus and verify current original and fork inputs."""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

from harness.common.digests import file_record
from harness.common.artifacts import active_reader, read_bytes
from harness.domain.ids import normalize_target_id, parse_function_id
from harness.domain.psx import binary_offset_for, is_psx_exe, validate_psx_header
from harness.reconstruction import contract
from harness.toolchain.nativebuild import read_native_build


@dataclass(frozen=True)
class ImageFacts:
    """One distinct original image that members are carved from."""

    path: str
    content: bytes
    sha256: str
    label: str
    load_address: int
    payload_offset: int
    payload_size: int
    format: str


@dataclass(frozen=True)
class MemberFacts:
    """One revalidated admitted member: frozen identity plus exact byte range."""

    key: str
    case_id: str
    target: str
    address: int
    size: int
    range_sha256: str
    image: ImageFacts

    @property
    def end(self) -> int:
        return self.address + self.size


@dataclass(frozen=True)
class PinnedBaseline:
    """One verified baseline snapshot: bytes, digest and the parsed document.

    The parsed document and the retained bytes are derived from the same read,
    so a replacement after the pin check cannot change what preparation uses.
    """

    path: Path
    data: bytes
    sha256: str
    document: dict


def load_baseline(root: Path, path: Path) -> PinnedBaseline:
    """Read the maintained corpus once and reject changes to its fixed membership."""
    data = contract.read_verified(path, contract.PINNED_BASELINE_SHA256, "corpus")
    document = contract.parse_json(data, "corpus")
    contract.require(isinstance(document, dict), "corpus is not an object")
    contract.require(
        document.get("schema") == contract.BASELINE_SCHEMA, "corpus schema differs"
    )
    contract.require(
        isinstance(document.get("members"), list), "corpus members are missing"
    )
    contract.require(
        isinstance(document.get("exclusions"), list), "corpus exclusions are missing"
    )
    contract.require(
        len(document["members"]) == contract.ADMITTED, "corpus admitted count differs"
    )
    contract.require(
        len(document["exclusions"]) == contract.EXCLUSIONS,
        "corpus exclusion count differs",
    )
    contract.require(
        document.get("counts")
        == {"admitted_members": contract.ADMITTED, "exclusions": contract.EXCLUSIONS},
        "corpus counts differ",
    )
    return PinnedBaseline(
        path=path, data=data, sha256=contract.sha256_bytes(data), document=document
    )


def source_closure(root: Path, *, native_receipt: Path) -> list[dict]:
    """Bind current fork files and their verified native build receipt."""
    native_receipt = Path(native_receipt).resolve()
    receipt_identity = file_record(native_receipt)
    native = read_native_build(native_receipt)
    contract.require(
        native.plugin.root == root / "third_party/rz-psxdec"
        and native.retdec.root == root / "third_party/retdec-psx",
        "native delivery sources are not the current repository forks",
    )
    entries = [
        {
            "path": (source.root / row["path"]).relative_to(root).as_posix(),
            "sha256": row["sha256"],
        }
        for source in (native.plugin, native.retdec)
        for row in source.after
    ]
    contract.require(
        file_record(native_receipt) == receipt_identity,
        "native build receipt changed during verification",
    )
    entries.append(
        {
            "path": native_receipt.relative_to(root).as_posix(),
            "sha256": receipt_identity["sha256"],
        }
    )
    return entries


def _load_image(root: Path, member: dict, cache: dict[str, ImageFacts]) -> ImageFacts:
    """Reload one original image and revalidate every recorded byte fact."""

    relative = contract.validate_rel_path(member.get("image"))
    if relative in cache:
        return cache[relative]
    image_path = root / relative
    contract.require(image_path.is_file(), f"original image is missing: {relative}")
    try:
        content = (
            image_path.read_bytes()
            if active_reader() is None
            else read_bytes(image_path, image_path.stat().st_size)
        )
    except OSError as error:
        raise contract.PreparationRefused(
            f"original image is unreadable: {relative} ({error})"
        ) from error
    sha256 = contract.sha256_bytes(content)
    contract.require(
        member.get("image_sha256") == sha256,
        f"original image digest differs for {relative}",
    )
    image_format = "psx-exe" if is_psx_exe(content) else "raw-payload"
    contract.require(
        image_format == member.get("image_format"),
        f"image format is not the format recorded for {relative}",
    )
    payload_offset = binary_offset_for(content)
    contract.require(
        payload_offset == member.get("payload_offset"),
        f"payload offset differs from the recorded offset for {relative}",
    )
    load_address = member.get("load_address")
    contract.require(
        isinstance(load_address, int), f"load address is missing for {relative}"
    )
    if image_format == "psx-exe":
        try:
            validate_psx_header(content, load_address, binary_name=relative)
        except (ValueError, TypeError) as error:
            raise contract.PreparationRefused(
                f"PS-X EXE header is invalid for {relative}: {error}"
            ) from error
    payload_size = len(content) - payload_offset
    contract.require(payload_size > 0, f"original payload is empty: {relative}")
    return ImageFacts(
        path=relative,
        content=content,
        sha256=sha256,
        label=image_path.name[:64],
        load_address=load_address,
        payload_offset=payload_offset,
        payload_size=payload_size,
        format=image_format,
    )


def member_facts(root: Path, baseline: dict) -> list[MemberFacts]:
    """Revalidate all 118 frozen identities and their reviewed byte ranges."""

    facts: list[MemberFacts] = []
    cache: dict[str, ImageFacts] = {}
    keys: set[str] = set()
    case_ids: set[str] = set()
    selectors: set[str] = set()
    for member in baseline["members"]:
        contract.require(isinstance(member, dict), "baseline member is not an object")
        case_id = contract.validate_ident(member.get("case_id"), "member case id")
        selector = member.get("selector")
        try:
            function = parse_function_id(str(selector))
        except (ValueError, TypeError) as error:
            raise contract.PreparationRefused(
                f"member selector is invalid: {selector!r} ({error})"
            ) from error
        try:
            target = normalize_target_id(str(member.get("target")))
        except (ValueError, TypeError) as error:
            raise contract.PreparationRefused(
                f"member target is invalid: {member.get('target')!r}"
            ) from error
        contract.require(
            function.target.value == target.value,
            f"member selector and target disagree: {selector!r} vs {member.get('target')!r}",
        )
        key = function.value
        contract.require(key not in keys, f"duplicate member key: {key}")
        contract.require(
            case_id not in case_ids, f"duplicate member case id: {case_id}"
        )
        contract.require(
            str(selector) not in selectors, f"duplicate member selector: {selector}"
        )
        keys.add(key)
        case_ids.add(case_id)
        selectors.add(str(selector))
        address = member.get("address")
        size = member.get("size")
        contract.require(
            isinstance(address, int) and address >= 0, f"invalid address for {key}"
        )
        contract.require(isinstance(size, int) and size > 0, f"invalid size for {key}")
        contract.require(
            address % 4 == 0 and size % 4 == 0, f"unaligned reviewed range for {key}"
        )
        recorded_range = member.get("range")
        contract.require(
            isinstance(recorded_range, list)
            and len(recorded_range) == 2
            and recorded_range[0] == address
            and recorded_range[1] == address + size,
            f"member range is inconsistent for {key}",
        )
        image = _load_image(root, member, cache)
        start = image.load_address
        contract.require(
            start <= address and address + size <= start + image.payload_size,
            f"member range escapes the payload for {key}",
        )
        offset = image.payload_offset + (address - start)
        slice_sha256 = contract.sha256_bytes(image.content[offset : offset + size])
        contract.require(
            member.get("range_sha256") == slice_sha256,
            f"reviewed range digest differs for {key}",
        )
        facts.append(
            MemberFacts(
                key=key,
                case_id=case_id,
                target=target.value,
                address=address,
                size=size,
                range_sha256=slice_sha256,
                image=image,
            )
        )
    contract.require(
        len(facts) == contract.ADMITTED, f"admitted member count is {len(facts)}"
    )
    return sorted(facts, key=lambda fact: fact.key)
