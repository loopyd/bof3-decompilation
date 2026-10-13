"""Verified original-PC provenance: immutable Problem binding and byte slices.

Task 3.1 admits original instructions only through an immutable snapshot of the
active approved Problem bundle.  :func:`open_problem_binding` re-verifies the
preparation pair, the sealed document bytes, the bundle and document digests,
the closed (empty) ``upstream`` and every identity/range the document claims,
then freezes one content-addressed :class:`ProblemBinding`.  Every reader uses the
binding, never a separately supplied document.

:func:`read_original_image` verifies complete original file bytes, including any
trailing payload bytes. :func:`original_slice` re-checks the active index and derives
one exact, word-aligned original-PC instruction slice from the bound image bytes; the
caller can only supply the extent, never the length or the bytes.
"""

from __future__ import annotations

from contextlib import nullcontext
from typing import Literal
from harness.common.failures import FailureEvent

import hashlib
import json
from collections.abc import Mapping
from dataclasses import dataclass, replace

from harness.domain import psx
from harness.reconstruction import bundles, contract, problemclosure, records
from harness.reconstruction.facterrors import FactRefused, refusing
from harness.reconstruction.factvalues import canonical_bytes

BINDING_PREFIX = "b1:"


def require(condition: object, message: str) -> None:
    if not condition:
        raise FactRefused(message)


@dataclass(frozen=True)
class BoundImage:
    image_id: str
    label: str
    sha256: str
    bytes: int
    format: str
    load_address: int
    payload_offset: int
    payload_size: int
    file_path: str
    file_sha256: str


@dataclass(frozen=True)
class BoundTarget:
    target: str
    kind: str
    image_id: str


@dataclass(frozen=True)
class BoundMember:
    key: str
    case_id: str
    target: str
    address: int
    start: int
    end: int
    range_sha256: str
    image_id: str


@dataclass(frozen=True)
class BoundRules:
    rules_id: str
    file_path: str
    file_sha256: str
    file_bytes: int
    entries: tuple


@dataclass(frozen=True)
class ProblemBinding:
    """One immutable snapshot of the verified active Problem bundle."""

    run_id: str
    problem_path: str
    problem_index_digest: str
    problem_doc_digest: str
    problem_doc_sha256: str
    problem_doc_bytes: int
    images: tuple
    targets: tuple
    members: tuple
    rules: BoundRules
    digest: str

    def image(self, image_id: str) -> BoundImage:
        for image in self.images:
            if image.image_id == image_id:
                return image
        raise FactRefused(f"image is not bound: {image_id!r}")

    def target(self, name: str) -> BoundTarget:
        for target in self.targets:
            if target.target == name:
                return target
        raise FactRefused(f"target is not bound: {name!r}")

    def member(self, key: str) -> BoundMember:
        for member in self.members:
            if member.key == key:
                return member
        raise FactRefused(f"member is not bound: {key!r}")


@dataclass(frozen=True)
class OriginalSlice:
    """One exact verified original-PC instruction slice for the active Problem."""

    run_id: str
    problem_index_digest: str
    problem_doc_digest: str
    image_id: str
    image_sha256: str
    format: str
    load_address: int
    payload_offset: int
    payload_size: int
    file_path: str
    file_sha256: str
    file_bytes: int
    pc: int
    extent: tuple
    length: int
    byte_offset: int
    instruction_bytes: bytes
    slice_sha256: str
    helper: bool

    def as_record(self) -> dict:
        return {
            "run_id": self.run_id,
            "problem_index_digest": self.problem_index_digest,
            "problem_doc_digest": self.problem_doc_digest,
            "image_id": self.image_id,
            "image_sha256": self.image_sha256,
            "format": self.format,
            "load_address": self.load_address,
            "payload_offset": self.payload_offset,
            "payload_size": self.payload_size,
            "file_path": self.file_path,
            "file_sha256": self.file_sha256,
            "file_bytes": self.file_bytes,
            "pc": self.pc,
            "extent": {"start": self.extent[0], "end": self.extent[1]},
            "length": self.length,
            "byte_offset": self.byte_offset,
            "instruction_bytes": self.instruction_bytes,
            "slice_sha256": self.slice_sha256,
            "helper": self.helper,
        }


def _image_rows(images: tuple) -> list:
    return [
        {
            "image_id": image.image_id,
            "label": image.label,
            "sha256": image.sha256,
            "bytes": image.bytes,
            "format": image.format,
            "load_address": image.load_address,
            "payload_offset": image.payload_offset,
            "payload_size": image.payload_size,
            "file_path": image.file_path,
            "file_sha256": image.file_sha256,
        }
        for image in images
    ]


def _binding_rows(binding: ProblemBinding) -> dict:
    return {
        "run_id": binding.run_id,
        "problem_path": binding.problem_path,
        "problem_index_digest": binding.problem_index_digest,
        "problem_doc_digest": binding.problem_doc_digest,
        "problem_doc_sha256": binding.problem_doc_sha256,
        "problem_doc_bytes": binding.problem_doc_bytes,
        "images": _image_rows(binding.images),
        "targets": [
            {"target": t.target, "kind": t.kind, "image_id": t.image_id}
            for t in binding.targets
        ],
        "members": [
            {
                "key": m.key,
                "case_id": m.case_id,
                "target": m.target,
                "address": m.address,
                "start": m.start,
                "end": m.end,
                "range_sha256": m.range_sha256,
                "image_id": m.image_id,
            }
            for m in binding.members
        ],
        "rules": {
            "rules_id": binding.rules.rules_id,
            "file_path": binding.rules.file_path,
            "file_sha256": binding.rules.file_sha256,
            "file_bytes": binding.rules.file_bytes,
            "entries": list(binding.rules.entries),
        },
    }


def _binding_digest(binding: ProblemBinding) -> str:
    return (
        BINDING_PREFIX
        + hashlib.sha256(canonical_bytes(_binding_rows(binding))).hexdigest()
    )


def _bound_images(document: dict) -> tuple:
    images = []
    for image in document["images"]:
        require(
            image["payload_size"] == image["bytes"] - image["payload_offset"],
            f"image {image['image_id']} payload size is inconsistent with its bytes",
        )
        images.append(
            BoundImage(
                image_id=image["image_id"],
                label=image["label"],
                sha256=image["sha256"],
                bytes=image["bytes"],
                format=image["format"],
                load_address=image["load_address"],
                payload_offset=image["payload_offset"],
                payload_size=image["payload_size"],
                file_path=image["file"]["path"],
                file_sha256=image["file"]["sha256"],
            )
        )
    require(len({image.image_id for image in images}) == len(images), "repeated image")
    return tuple(images)


def _bound_targets(targets: list, images: dict) -> dict:
    bound = {}
    for target in targets:
        require(
            target["image_id"] in images,
            f"target {target['target']} references an unknown image",
        )
        bound[target["target"]] = BoundTarget(
            target["target"], target["kind"], target["image_id"]
        )
    return bound


def _bound_members(members: list, targets: dict, images: dict) -> tuple:
    bound = []
    for member in members:
        bound_target = targets.get(member["target"])
        require(
            bound_target is not None, f"member {member['key']} has an unknown target"
        )
        require(
            member["image_id"] == bound_target.image_id,
            f"member {member['key']} image disagrees with its target",
        )
        require(
            member["address"] == member["range"]["start"],
            f"member {member['key']} address must equal range.start",
        )
        start, end = member["range"]["start"], member["range"]["end"]
        require(end > start, f"member {member['key']} range must be half-open")
        image = images[member["image_id"]]
        require(
            image.load_address <= start
            and end <= image.load_address + image.payload_size,
            f"member {member['key']} range escapes its image payload",
        )
        bound.append(
            BoundMember(
                key=member["key"],
                case_id=member["case_id"],
                target=member["target"],
                address=member["address"],
                start=start,
                end=end,
                range_sha256=member["range_sha256"],
                image_id=member["image_id"],
            )
        )
    return tuple(bound)


def _bound_rules(document: dict) -> BoundRules:
    rules, source = document["rules"], document["rules"]["source"]
    return BoundRules(
        rules_id=rules["rules_id"],
        file_path=source["path"],
        file_sha256=source["sha256"],
        file_bytes=source["bytes"],
        entries=tuple(rules["entries"]),
    )


def open_problem_binding(
    run_dir,
    *,
    io_mode: Literal["normalized", "exact"] = "normalized",
    events: list[FailureEvent] | None = None,
    max_bytes: int | None = None,
) -> ProblemBinding:
    """Verify the active approved Problem and freeze one immutable snapshot.

    The preparation pair is verified first (Answer *index* only).  Empty
    ``upstream`` and the closed inventory/reference policy are enforced before any
    nested edge is resolved, so a disallowed closure is refused before any read.
    """

    with (
        contract.bounded_reads(max_bytes),
        refusing() if io_mode == "normalized" else nullcontext(),
    ):
        bundles.verify_preparation_bindings(run_dir, io_mode=io_mode, events=events)
        index = bundles.load_index(run_dir, "problem", io_mode=io_mode, events=events)
        primary = problemclosure.require_closed_index(index, run_dir.name)
        document = bundles.resolve_document(
            run_dir,
            contract.doc_ref("problem", primary["digest"], None),
            owner="problem",
            io_mode=io_mode,
            events=events,
        )
        problemclosure.require_closed_document(document)
        sealed = json.dumps(document, indent=2, sort_keys=True).encode() + b"\n"
        require(
            contract.sha256_bytes(sealed) == primary["sha256"],
            "the sealed Problem bytes cannot be reproduced",
        )
        require(
            contract.doc_digest(document) == primary["digest"],
            "the resolved Problem document digest differs from the index",
        )
        require(
            contract.bundle_digest(index) == index["bundle_digest"],
            "the Problem bundle digest does not reproduce",
        )
        images = _bound_images(document)
        by_image = {image.image_id: image for image in images}
        targets = _bound_targets(document["targets"], by_image)
        members = _bound_members(document["members"], targets, by_image)
        binding = ProblemBinding(
            run_id=index["run"],
            problem_path=f"problem/{primary['path']}",
            problem_index_digest=index["bundle_digest"],
            problem_doc_digest=primary["digest"],
            problem_doc_sha256=primary["sha256"],
            problem_doc_bytes=len(sealed),
            images=images,
            targets=tuple(targets[name] for name in sorted(targets)),
            members=members,
            rules=_bound_rules(document),
            digest="",
        )
        problemclosure.require_closed_inventory(index, document)
        problemclosure.require_closed_references(
            run_dir, document, io_mode=io_mode, events=events
        )
    return replace(binding, digest=_binding_digest(binding))


def require_active_binding(
    binding: ProblemBinding,
    run_dir,
    *,
    io_mode: Literal["normalized", "exact"] = "normalized",
    events: list[FailureEvent] | None = None,
    max_bytes: int | None = None,
) -> ProblemBinding:
    """Require a supplied snapshot to equal the independently opened Problem.

    The shared reader and ``factrevisions.build_revision`` boundary:
    it reopens the sealed Problem once and compares every row, so a forged copy
    that keeps the genuine digest cannot supply false image/load/member metadata.
    """

    active = open_problem_binding(
        run_dir, io_mode=io_mode, events=events, max_bytes=max_bytes
    )
    require(
        binding == active,
        "the supplied binding does not reproduce the active sealed Problem",
    )
    return active


def _extent_bounds(value: object) -> tuple:
    if isinstance(value, Mapping):
        fields = records._fields(value, {"start", "end"}, "extent")
        start, end = fields["start"], fields["end"]
    elif isinstance(value, (list, tuple)) and len(value) == 2:
        start, end = value
    else:
        raise FactRefused(f"extent must be a half-open pair: {value!r}")
    return records._u32(start, "extent.start"), records._u32(end, "extent.end")


def _image_ref(image: BoundImage) -> dict:
    return {
        "bundle": "problem",
        "path": image.file_path,
        "sha256": image.file_sha256,
        "bytes": image.bytes,
        "expected_bundle": None,
    }


def _read_image(
    image: BoundImage,
    run_dir,
    *,
    io_mode: Literal["normalized", "exact"] = "normalized",
    events: list[FailureEvent] | None = None,
    max_bytes: int | None = None,
) -> bytes:
    data = bundles.resolve_file(
        run_dir,
        _image_ref(image),
        owner="problem",
        io_mode=io_mode,
        events=events,
        max_bytes=max_bytes,
    )
    require(
        len(data) == image.bytes and contract.sha256_bytes(data) == image.sha256,
        "the resolved image bytes differ from the bound image",
    )
    require(
        contract.sha256_bytes(data) == image.file_sha256,
        "the resolved image bytes differ from the bound file",
    )
    derived = "psx-exe" if psx.is_psx_exe(data) else "raw-payload"
    require(derived == image.format, "the image format was re-derived differently")
    require(
        psx.binary_offset_for(data) == image.payload_offset,
        "the payload offset was re-derived differently",
    )
    require(
        image.payload_size == len(data) - image.payload_offset,
        "the payload size was re-derived differently",
    )
    if image.format == "psx-exe":
        try:
            psx.validate_psx_header(
                data, image.load_address, binary_name=image.image_id
            )
        except ValueError as error:
            raise FactRefused(f"the PS-X EXE header is invalid: {error}") from error
    return data


def read_original_image(
    binding: ProblemBinding,
    run_dir,
    *,
    target: str,
    image_id: str,
    io_mode: Literal["normalized", "exact"] = "normalized",
    events: list[FailureEvent] | None = None,
    max_bytes: int | None = None,
) -> bytes:
    """Read complete verified original file bytes without instruction alignment."""

    with (
        contract.bounded_reads(max_bytes),
        refusing() if io_mode == "normalized" else nullcontext(),
    ):
        active = require_active_binding(
            binding, run_dir, io_mode=io_mode, events=events
        )
        target = records._target(target, "target")
        image_id = records._image_id(image_id, "image_id")
        require(
            active.target(target).image_id == image_id,
            "the image id does not belong to the bound target",
        )
        return _read_image(
            active.image(image_id), run_dir, io_mode=io_mode, events=events
        )


def original_slice(
    binding: ProblemBinding,
    run_dir,
    *,
    target: str,
    image_id: str,
    pc: int,
    extent: object,
    subject: object = None,
    helper: bool = False,
    io_mode: Literal["normalized", "exact"] = "normalized",
    events: list[FailureEvent] | None = None,
    max_bytes: int | None = None,
) -> OriginalSlice:
    """Derive one exact word-aligned original-PC slice, or refuse before use."""

    with (
        contract.bounded_reads(max_bytes),
        refusing() if io_mode == "normalized" else nullcontext(),
    ):
        require_active_binding(binding, run_dir, io_mode=io_mode, events=events)
        start, end = _extent_bounds(extent)
        require(pc == start, "pc must equal extent.start")
        require(
            pc % 4 == 0 and start % 4 == 0 and end % 4 == 0,
            "the extent must be word aligned",
        )
        require(end > start, "the extent must be non-empty")
        length = end - start
        target = records._target(target, "target")
        image_id = records._image_id(image_id, "image_id")
        bound_target = binding.target(target)
        require(
            bound_target.image_id == image_id,
            "the image id does not belong to the bound target",
        )
        image = binding.image(image_id)
        require(
            image.load_address <= start
            and end <= image.load_address + image.payload_size,
            "the extent escapes the image payload",
        )
        require(isinstance(helper, bool), "helper must be a boolean")
        if helper:
            require(subject is None, "a helper slice must not name a member")
        else:
            member = binding.member(records._member_key(subject, "subject"))
            require(
                member.target == target
                and member.image_id == image_id
                and member.address == pc,
                "the member identity disagrees with the requested slice",
            )
            require(
                member.start <= start and end <= member.end,
                "the extent escapes the member range",
            )
        data = _read_image(image, run_dir, io_mode=io_mode, events=events)
        byte_offset = image.payload_offset + (pc - image.load_address)
        require(byte_offset >= 0, "the byte offset is negative")
        require(byte_offset + length <= len(data), "the slice escapes the image")
        instruction_bytes = data[byte_offset : byte_offset + length]
        require(
            len(instruction_bytes) == length,
            "the instruction slice is not the exact derived length",
        )
        return OriginalSlice(
            run_id=binding.run_id,
            problem_index_digest=binding.problem_index_digest,
            problem_doc_digest=binding.problem_doc_digest,
            image_id=image_id,
            image_sha256=image.sha256,
            format=image.format,
            load_address=image.load_address,
            payload_offset=image.payload_offset,
            payload_size=image.payload_size,
            file_path=image.file_path,
            file_sha256=image.file_sha256,
            file_bytes=image.bytes,
            pc=pc,
            extent=(start, end),
            length=length,
            byte_offset=byte_offset,
            instruction_bytes=instruction_bytes,
            slice_sha256=contract.sha256_bytes(instruction_bytes),
            helper=helper,
        )
