"""Target-qualified image coverage for unresolved direct-call reports."""

from __future__ import annotations

from dataclasses import asdict, dataclass
import hashlib
import json
from pathlib import Path
import re

from harness.common.artifacts import read_bytes
from harness.reconstruction import (
    callcontracts,
    copies,
    objectfile,
    proofvalues,
    provenance,
)
from harness.reconstruction.proofvalues import _keys, _require, _rows

CATALOG_LIMIT = 64 << 10
BINDINGS_LIMIT = 2 << 20
MEMBER_FIELDS = tuple("key target image_id start end range_sha256".split())
SCALAR_ADDRESS_VIEW = "conditional-scalar-address-view/v1"
BYTE_WRITE_ADDRESS_VIEW = "conditional-byte-write-address-view/v1"


@dataclass(frozen=True)
class NativeContexts:
    problem_sha256: str
    catalog_sha256: str
    images: tuple[provenance.BoundImage, ...]
    targets: tuple[provenance.BoundTarget, ...]
    members: tuple[provenance.BoundMember, ...]
    input_directory: Path


def original_direct_functions(functions, selected):
    targets = {
        ((row["pc"] + 4) & 0xF0000000) | ((row["word"] & 0x03FFFFFF) << 2)
        for row in selected["returnDisposition"]["evidence"]["instructions"]
        if row["word"] >> 26 == 3
    }
    names, addresses = {}, {}
    for row in functions:
        address = proofvalues._address(row.get("startAddr"))
        names.setdefault(row.get("name"), []).append(address)
        addresses.setdefault(address, []).append(row.get("name"))
    direct = {}
    for name, entries in names.items():
        if not any(address in targets for address in entries):
            continue
        _require(
            type(name) is str and len(entries) == 1,
            "original direct-call function name is ambiguous",
        )
        address = entries[0]
        _require(
            type(address) is int
            and 0 <= address < 1 << 32
            and address % 4 == 0
            and len(addresses[address]) == 1,
            "original direct-call function address is ambiguous",
        )
        direct[name] = address
    return direct


def _address_views(row):
    if "originalAddressViews" not in row:
        return {}
    views = _rows(row["originalAddressViews"], 4096, "conditional address views")
    ordinary = row.get("originalBodyCandidate") == "mips-ordinary-body/v1"
    evidence, summary = (
        row["returnDisposition"]["evidence"],
        row["originalCallSummary"],
    )
    for view in views:
        _keys(view, {"kind", "symbol", "start", "width", "accessPcs"}, "address view")
        _require(
            view["kind"] in (SCALAR_ADDRESS_VIEW, BYTE_WRITE_ADDRESS_VIEW),
            "conditional address view kind differs",
        )
    kinds = {view["kind"] for view in views}
    _require(len(kinds) == 1, "conditional address view modes differ")
    byte_writes = kinds == {BYTE_WRITE_ADDRESS_VIEW}
    _require(
        views
        and (ordinary or row["returnDisposition"]["kind"] == "value")
        and evidence["selection"] == "selected-root"
        and evidence["decodeComplete"] is evidence["analysisComplete"] is True
        and proofvalues._complete(summary["entry"])
        and (
            (ordinary and byte_writes)
            or (
                not byte_writes
                and proofvalues._complete(summary["control"])
                and not evidence["calls"]
            )
        ),
        "byte write address view lacks its supported original body"
        if byte_writes
        else "conditional address view lacks a closed call-free original body",
    )
    erased_link = None
    if ordinary:
        if byte_writes:
            erased_link = callcontracts._validate_ordinary_body_candidate(row)
        else:
            _require(
                callcontracts.validate_body_candidate(row), "address view body proposal"
            )
    else:
        callcontracts.validate_projected_declaration(row)
    accesses = {item["pc"]: item for item in summary["accesses"]}
    projected = set()
    if byte_writes:
        _require(
            len(accesses) == len(summary["accesses"])
            and summary["pendingEntry"] == "requires-no-pending-gpr-loads"
            and bool(summary["exits"]),
            "byte write view access census or settled entry differs",
        )
        if erased_link:
            save_pc, restore_pc, _, entry_offset = erased_link
            for pc, read in ((save_pc, False), (restore_pc, True)):
                _require(pc in accesses, "byte write view omits projected link access")
                access = accesses[pc]
                address = access["address"]
                _require(
                    access["kind"] == ("read" if read else "write")
                    and access["width"] == 32
                    and access["extension"] == "none"
                    and access["delayed"] is read
                    and address.get("kind")
                    in {
                        "entry-register-relative-prefix/v1",
                        "entry-register-relative-flow/v1",
                    }
                    and address["entryRegister"] == 29
                    and address["offset"] == (entry_offset & 0xFFFFFFFF)
                    and address["equalityEvidence"] == [evidence["start"]],
                    "byte write view projected link address differs",
                )
                projected.add(pc)
        _require(
            {
                item["pc"]
                for item in evidence["instructions"]
                if 32 <= item["word"] >> 26 <= 46
            }
            == set(accesses),
            "byte write view omits an original memory instruction",
        )
    instructions = {item["pc"]: item for item in evidence["instructions"]}
    bound, seen, spans = {}, set(), []
    mixed = False
    for view in views:
        symbol, start, width = (view[field] for field in ("symbol", "start", "width"))
        _require(
            type(symbol) is str
            and re.fullmatch(r"[A-Za-z_][A-Za-z_0-9]*", symbol)
            and symbol not in bound
            and type(start) is int
            and type(width) is int
            and width in {8, 16, 32}
            and 0x80000000 <= start <= 0x80200000 - width // 8
            and start % (width // 8) == 0,
            "conditional address view identity, width or RAM interval differs",
        )
        size = width // 8
        pcs = proofvalues._references(view["accessPcs"], 0xFFFFFFFF)
        _require(pcs and not seen.intersection(pcs), "address view access ownership")
        if byte_writes:
            _require(
                width == 8 and len(pcs) == 1 and not projected.intersection(pcs),
                "byte write view width or access ownership differs",
            )
        anchor, mapping = False, None
        for pc in pcs:
            _require(pc in accesses, "address view names an absent original access")
            access = accesses[pc]
            address = access["address"]
            if byte_writes:
                _require(
                    pc in instructions
                    and instructions[pc]["word"] >> 26 == 40
                    and access["kind"] == "write"
                    and access["width"] == 8
                    and access["extension"] == "none"
                    and access["delayed"] is False
                    and not access["addressEntries"],
                    "byte write view differs from its original SB",
                )
            _require(
                address.get("kind")
                in {"entry-fallthrough-prefix/v1", "instruction-fixed-flow/v1"}
                and proofvalues._complete(address["alignment"])
                and address["fileBytes"].get("kind") == "file-backed-initial-bytes"
                and access["width"] in {8, width}
                and start <= address["value"]
                and address["value"] + access["width"] // 8 <= start + size,
                "address view differs from a fixed, aligned, backed original access",
            )
            backing = address["fileBytes"]
            origin = (
                backing["mappingStart"],
                backing["mappingBytes"],
                backing["payloadOffset"] - address["value"],
            )
            _require(mapping in (None, origin), "address view has conflicting backing")
            mapping = origin
            anchor |= access["width"] == width and address["value"] == start
            mixed |= width > 8 and access["width"] == 8
        _require(anchor, "address view extends beyond its observed scalar access")
        _require(
            all(start + size <= first or last <= start for first, last in spans),
            "conditional scalar views overlap",
        )
        spans.append((start, start + size))
        seen.update(pcs)
        bound[symbol] = (start, size)
    _require(
        seen | projected == set(accesses), "address views omit original memory accesses"
    )
    byte_read = (
        not ordinary
        and len(views) == len(summary["accesses"]) == len(accesses) == 1
        and proofvalues._complete(summary["result"])
        and proofvalues._complete(summary["declaration"])
        and summary["pendingEntry"] == "requires-no-pending-gpr-loads"
        and bool(summary["exits"])
        and views[0]["width"] == 8
        and summary["accesses"][0]["kind"] == "read"
        and summary["accesses"][0]["width"] == 8
        and summary["accesses"][0]["extension"] == "zero"
        and summary["accesses"][0]["delayed"] is True
        and not summary["accesses"][0]["addressEntries"]
        and len(evidence["obligations"]) == 1
        and all(
            obligation["domain"] == "memory"
            and obligation["pc"] == summary["accesses"][0]["pc"]
            and obligation["reason"]
            == "contents-object-address-alias-effect-placement-unresolved"
            for obligation in evidence["obligations"]
        )
    )
    _require(
        mixed or byte_read or byte_writes,
        "address views lack an eligible backed access",
    )
    return bound


def _data_view_bindings(entry, views, *, independent_byte_writes=False):
    bindings = []
    for symbol, (start, size) in sorted(views.items()):
        uses = [row for row in entry.relocations if row.symbol_name == symbol]
        _require(uses, "address view lacks emitted relocation storage")
        _require(
            len({(row.target_section, row.offset) for row in uses}) == len(uses),
            "address view has duplicate relocation storage",
        )
        byte_read = not independent_byte_writes and len(views) == 1 and size == 1
        if byte_read or independent_byte_writes:
            _require(
                size == 1 and len(uses) == 2 and [row.kind for row in uses] == [5, 6],
                "byte address view requires one HI16/LO16 pair",
            )
        byte_register, byte_offset = 0, -1
        pending = []
        paired = False
        for row in uses:
            offset = row.offset - entry.value
            _require(
                row.symbol_section == 0
                and row.kind in {5, 6}
                and row.target_section == entry.section_index
                and row.offset % 4 == 0
                and offset % 4 == 0
                and 0 <= offset <= len(entry.data) - 4
                and row.addend is None,
                "address view has an unsupported or partial relocation",
            )
            word = int.from_bytes(entry.data[offset : offset + 4], "little")
            opcode = word >> 26
            if row.kind == 5:
                _require(
                    opcode == 15 and (word >> 21) & 31 == 0,
                    "address view HI16 is not LUI",
                )
                if byte_read or independent_byte_writes:
                    byte_register, byte_offset = (word >> 16) & 31, offset
                pending.append((row.section_index, word & 0xFFFF))
                continue
            _require(pending or paired, "address view LO16 lacks an established HI16")
            widths = {32: 1, 36: 1, 33: 2, 37: 2, 35: 4, 40: 1, 41: 2, 43: 4}
            _require(
                (opcode == 9 or opcode in widths) and (word >> 21) & 31 != 0,
                "address view LO16 instruction is unsupported",
            )
            if byte_read or independent_byte_writes:
                _require(
                    opcode == (40 if independent_byte_writes else 36)
                    and offset == byte_offset + 4
                    and byte_register != 0
                    and (word >> 21) & 31 == byte_register,
                    "byte write address view lacks adjacent LUI-to-SB register flow"
                    if independent_byte_writes
                    else "byte address view lacks adjacent LUI-to-LBU register flow",
                )
            low = word & 0xFFFF
            low -= 0x10000 if low & 0x8000 else 0
            _require(
                0 <= low < size and low + widths.get(opcode, 0) <= size,
                "address view LO16 addend escapes its observed span",
            )
            for table, high in pending:
                addend = (high << 16) + low
                _require(
                    table == row.section_index
                    and 0 <= addend < size
                    and addend + widths.get(opcode, 0) <= size,
                    "address view relocation addend escapes its observed span",
                )
            pending.clear()
            paired = True
        _require(not pending, "address view has an unpaired HI16")
        bindings.append([symbol, start])
    return bindings


def _measurement_bindings(unit, entry):
    from harness.reconstruction.nativeoutput import CONFIG_LIMIT, parse_json

    if unit is None:
        return []
    configuration = parse_json(unit.configuration, CONFIG_LIMIT)
    functions = _rows(configuration.get("functions"), 4096, "measurement functions")
    selected = [row for row in functions if row.get("name") == unit.entry_symbol]
    _require(len(selected) == 1, "measurement selected function is not unique")
    root = selected[0]
    ordinary = root.get("originalBodyCandidate") == "mips-ordinary-body/v1"
    views = _address_views(root)
    if not ordinary and not views:
        return []
    if ordinary:
        _require(
            callcontracts.project_validated_contract(unit) is None,
            "measurement body proposal acquired a validated contract",
        )
    direct = original_direct_functions(functions, root)
    _require(not set(direct).intersection(views), "measurement symbol has two owners")
    symbols = {
        row.symbol_name
        for row in entry.relocations
        if row.symbol_section == 0
        and row.kind == 4
        and row.target_section == entry.section_index
        and entry.value <= row.offset < entry.value + len(entry.data)
    }
    bindings = []
    for symbol in sorted(symbols, key=lambda name: name or ""):
        _require(
            type(symbol) is str and symbol in direct,
            "measurement direct-call symbol has no unique native function",
        )
        address = direct[symbol]
        for row in entry.relocations:
            if row.symbol_name != symbol:
                continue
            offset = row.offset - entry.value
            _require(
                row.symbol_section == 0
                and row.kind == 4
                and row.target_section == entry.section_index
                and row.offset % 4 == 0
                and offset % 4 == 0
                and 0 <= offset <= len(entry.data) - 4
                and row.addend in (None, 0),
                "measurement direct-call symbol has a noncall or partial use",
            )
            word = int.from_bytes(entry.data[offset : offset + 4], "little")
            _require(
                word >> 26 == 3 and word & 0x03FFFFFF == 0,
                "measurement direct-call relocation is not an unadjusted JAL",
            )
        bindings.append([symbol, address])
    if views:
        _require(
            all(
                row.symbol_name in direct or row.symbol_name in views
                for row in entry.relocations
                if row.symbol_section == 0
                and row.target_section == entry.section_index
                and entry.value <= row.offset < entry.value + len(entry.data)
            ),
            "measurement relocation has no native call or address view",
        )
    byte_writes = bool(views) and all(
        view["kind"] == BYTE_WRITE_ADDRESS_VIEW
        for view in root.get("originalAddressViews", [])
    )
    return bindings + _data_view_bindings(
        entry, views, independent_byte_writes=byte_writes
    )


def measurement_placement(blob, entry_symbol, original, limit, *, unit):
    _require(
        unit is None
        or (
            unit.entry_symbol == entry_symbol
            and unit.member.start == original.start
            and unit.member.end == original.end
        ),
        "measurement native unit differs from the original interval",
    )
    entry = objectfile.extract_function(blob, entry_symbol, limit=limit)
    sections = objectfile.inspect_sections(blob, entry_symbol, limit=limit)
    section = next(item for item in sections if item.index == entry.section_index)
    text_base = original.start - entry.value
    placeable = (
        entry.section_name == ".text"
        and 0 <= text_base < 2**32
        and section.alignment > 0
        and text_base % section.alignment == 0
    )
    facts = dict(
        entry_section=entry.section_name,
        entry_offset=entry.value,
        text_base=text_base,
        sections=[asdict(item) for item in sections],
        relocations=[asdict(item) for item in entry.relocations],
        text_relocations=sum(
            item.target_section == entry.section_index for item in entry.relocations
        ),
        bindings=_measurement_bindings(unit, entry),
        section_addresses={},
    )
    return facts, placeable


def _member_record(member):
    return {name: getattr(member, name) for name in MEMBER_FIELDS}


def _context_file(directory, index):
    return directory / "images" / f"{index:03d}.image"


def _original_file(path, image):
    _require(0 < image.bytes <= 16 << 20, "catalog original file bound")
    data = read_bytes(path, image.bytes)
    _require(
        len(data) == image.bytes
        and hashlib.sha256(data).hexdigest() == image.file_sha256 == image.sha256,
        "catalog complete original file identity",
    )
    return data


def _catalog_record(contexts):
    _require(
        0 < len(contexts.images) <= 128
        and 0 < len(contexts.targets) <= 128
        and len(contexts.members) == 118,
        "catalog inventory bound",
    )
    images, total = [], 0
    for index, image in enumerate(contexts.images):
        data = _original_file(_context_file(contexts.input_directory, index), image)
        payload = data[image.payload_offset : image.payload_offset + image.payload_size]
        total += len(data) + len(payload)
        _require(
            0 < image.payload_size <= 16 << 20
            and len(payload) == image.payload_size
            and 0 <= image.load_address < image.load_address + len(payload) <= 1 << 32
            and total <= 64 << 20,
            "catalog physical payload bound",
        )
        for member in contexts.members:
            if member.image_id != image.image_id:
                continue
            first, last = (
                member.start - image.load_address,
                member.end - image.load_address,
            )
            _require(
                0 <= first < last <= len(payload)
                and member.start % 4 == member.end % 4 == 0
                and last - first <= 64 << 10
                and hashlib.sha256(payload[first:last]).hexdigest()
                == member.range_sha256,
                "catalog reviewed original range identity",
            )
        images.append(
            {
                "image_id": image.image_id,
                "file_sha256": image.file_sha256,
                "file_bytes": image.bytes,
                "payload_offset": image.payload_offset,
                "payload_sha256": hashlib.sha256(payload).hexdigest(),
                "payload_bytes": image.payload_size,
                "load_address": image.load_address,
                "payload_index": index,
            }
        )
    return {
        "schema": "retdec118.original-image-catalog/v1",
        "problem_sha256": contexts.problem_sha256,
        "images": images,
        "targets": [
            {"target": t.target, "kind": t.kind, "image_id": t.image_id}
            for t in contexts.targets
        ],
        "members": [_member_record(member) for member in contexts.members],
    }


def _write_context(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("xb") as stream:
        _require(stream.write(data) == len(data), "catalog copy incomplete")


def prepare_native_contexts(binding, run_dir, destination):
    images = tuple(sorted(binding.images, key=lambda image: image.image_id))
    targets = tuple(
        sorted(binding.targets, key=lambda target: (target.target, target.image_id))
    )
    members = tuple(sorted(binding.members, key=lambda member: member.key))
    _require(
        all(
            0 < image.bytes <= 16 << 20 and 0 < image.payload_size <= 16 << 20
            for image in images
        )
        and sum(image.bytes + image.payload_size for image in images) <= 64 << 20
        and 0 < len(images) <= 128
        and 0 < len(targets) <= 128
        and len(members) == 118,
        "catalog preallocation bounds",
    )
    destination.mkdir()
    for index, image in enumerate(images):
        target = next(t.target for t in targets if t.image_id == image.image_id)
        data = provenance.read_original_image(
            binding, run_dir, target=target, image_id=image.image_id
        )
        _write_context(_context_file(destination, index), data)
    contexts = NativeContexts(
        binding.problem_doc_sha256, "", images, targets, members, destination
    )
    data = (
        json.dumps(_catalog_record(contexts), sort_keys=True, indent=2, allow_nan=False)
        + "\n"
    ).encode()
    _require(len(data) <= CATALOG_LIMIT, "catalog JSON bound")
    _write_context(destination / "image-catalog.json", data)
    return NativeContexts(
        contexts.problem_sha256,
        hashlib.sha256(data).hexdigest(),
        images,
        targets,
        members,
        destination,
    )


def verify_native_contexts(contexts, *, directory=None):
    selected = (
        contexts
        if directory is None
        else NativeContexts(
            contexts.problem_sha256,
            contexts.catalog_sha256,
            contexts.images,
            contexts.targets,
            contexts.members,
            directory,
        )
    )
    data = read_bytes(selected.input_directory / "image-catalog.json", CATALOG_LIMIT)
    catalog = _catalog_record(selected)
    expected = (
        json.dumps(catalog, sort_keys=True, indent=2, allow_nan=False) + "\n"
    ).encode()
    _require(
        data == expected
        and hashlib.sha256(data).hexdigest() == contexts.catalog_sha256,
        "catalog active Problem inventory or digest differs",
    )
    return catalog


def stage_native_contexts(contexts, directory):
    verify_native_contexts(contexts)
    _write_context(
        directory / "image-catalog.json",
        (contexts.input_directory / "image-catalog.json").read_bytes(),
    )
    for index, image in enumerate(contexts.images):
        _write_context(
            _context_file(directory, index),
            _original_file(_context_file(contexts.input_directory, index), image),
        )
    verify_native_contexts(contexts, directory=directory)


def _binding_fact(value, *, complete=False):
    _keys(value, {"kind", "remaining"}, "binding fact")
    reasons = _rows(value["remaining"], 16, "binding reasons")
    _require(
        all(
            type(reason) is str
            and 0 < len(reason) <= 128
            and all(32 <= ord(c) <= 126 for c in reason)
            for reason in reasons
        )
        and len(set(reasons)) == len(reasons),
        "binding reason text or uniqueness",
    )
    _require(
        value["kind"] == ("complete" if complete else "partial")
        and bool(reasons) != complete,
        "unsupported binding fact completeness",
    )


def validate_native_bindings(
    *, document, request, request_sha256, contexts, original_payload, source_form=None
):
    catalog = verify_native_contexts(contexts)
    images = {image.image_id: image for image in contexts.images}
    metadata = {image["image_id"]: image for image in catalog["images"]}
    overlays = type(request) is dict and request.get("schema") in {
        "retdec118.native-request/v5",
        "retdec118.native-request/v6",
        "retdec118.native-request/v7",
        "retdec118.native-request/v8",
    }
    forms = type(request) is dict and (
        request.get("schema") == "retdec118.native-request/v4"
        or (overlays and "source_form" in request)
    )
    fields = set(
        "schema operation request_id config_sha256 member image problem_sha256 callees image_catalog_sha256".split()
    )
    if forms:
        fields.add("source_form")
    if request.get("schema") in {
        "retdec118.native-request/v6",
        "retdec118.native-request/v8",
    }:
        fields.add("entry_observation")
    if request.get("schema") == "retdec118.native-request/v8":
        fields.add("pointer_load")
    if request.get("schema") == "retdec118.native-request/v7":
        fields.add("copied_original")
        _require(not forms, "copied original context acquired a source form")
        copies.validate_descriptor(request["copied_original"])
    if overlays:
        fields.add("overlay_mounts")
        mounts = _rows(
            request["overlay_mounts"],
            127 if request["schema"] == "retdec118.native-request/v7" else 128,
            "declared overlay mounts",
        )
        _require(
            mounts
            and all(type(image) is str and image in images for image in mounts)
            and len(set(mounts)) == len(mounts),
            "declared overlay mounts differ from permitted images",
        )
    _keys(request, fields, "binding request")
    _require(
        request["schema"]
        in {
            "retdec118.native-request/v3",
            "retdec118.native-request/v4",
            "retdec118.native-request/v5",
            "retdec118.native-request/v6",
            "retdec118.native-request/v7",
            "retdec118.native-request/v8",
        }
        and request["operation"] == "generate",
        "binding request version/operation",
    )
    _require(
        (
            forms
            and source_form is not None
            and request["source_form"] == source_form.value
        )
        or (not forms and source_form is None),
        "binding request source form differs from validated selection",
    )
    members = {member.key: _member_record(member) for member in contexts.members}
    root = request["member"]
    _require(
        type(root) is dict
        and root.get("key") in members
        and root == members[root["key"]],
        "binding root identity",
    )
    root_image = metadata[root["image_id"]]
    image_fields = set(
        "file_sha256 file_bytes payload_sha256 payload_offset payload_bytes load_address".split()
    )
    _keys(request["image"], image_fields, "binding request image")
    _require(
        request["image"] == {key: root_image[key] for key in image_fields}
        and all(
            type(request["image"][key]) is int
            for key in image_fields - {"file_sha256", "payload_sha256"}
        ),
        "binding request image",
    )
    _require(
        hashlib.sha256(original_payload).hexdigest() == root_image["payload_sha256"],
        "binding root payload identity",
    )
    peers = [
        member
        for member in members.values()
        if member["key"] != root["key"]
        and member["target"] == root["target"]
        and member["image_id"] == root["image_id"]
    ]
    _require(
        sorted(request["callees"], key=lambda row: row["key"]) == peers,
        "binding same-image input scope",
    )
    _keys(
        document,
        set(
            "schema request_sha256 problem_sha256 image_catalog_sha256 root scan".split()
        ),
        "binding document",
    )
    _require(
        document["schema"] == "retdec118.original-bindings/v1"
        and document["request_sha256"] == request_sha256
        and document["problem_sha256"]
        == request["problem_sha256"]
        == contexts.problem_sha256
        and document["image_catalog_sha256"]
        == request["image_catalog_sha256"]
        == contexts.catalog_sha256
        and document["root"] == root,
        "binding document input identity",
    )
    scan = document["scan"]
    _require(type(scan) is dict, "binding scan object")
    if scan.get("kind") == "partial":
        _binding_fact(scan)
        return
    _keys(scan, {"kind", "words", "calls"}, "complete binding scan")
    _require(
        scan["kind"] == "complete"
        and type(scan["words"]) is int
        and scan["words"] == (root["end"] - root["start"]) // 4,
        "binding raw word coverage",
    )
    load = root_image["load_address"]
    words = [
        (pc, int.from_bytes(original_payload[pc - load : pc - load + 4], "little"))
        for pc in range(root["start"], root["end"], 4)
    ]
    calls = [
        (pc, word) for pc, word in words if word >> 26 == 3 or word & 0xFC1F07FF == 9
    ]
    observed = _rows(scan["calls"], 4096, "binding calls")
    _require(len(observed) == len(calls), "binding complete call inventory")
    addresses = {
        (((pc + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2))
        for pc, word in calls
        if word >> 26 == 3
    }
    entry_words = {}
    for index, image in enumerate(contexts.images):
        data = _original_file(_context_file(contexts.input_directory, index), image)
        for address in addresses:
            offset = address - image.load_address
            if 0 <= offset < image.payload_size:
                entry_words[image.image_id, address] = (
                    int.from_bytes(
                        data[
                            image.payload_offset + offset : image.payload_offset
                            + offset
                            + 4
                        ],
                        "little",
                    )
                    if offset + 4 <= image.payload_size
                    else None
                )
    scoped = {member["key"] for member in [root, *request["callees"]]}
    total = 0
    for call, (pc, word) in zip(observed, calls, strict=True):
        _keys(
            call,
            set(
                "pc word link_register target reachability alternatives active_target scoped_observation universal_binding".split()
            ),
            "binding call",
        )
        _require(
            type(call["pc"]) is type(call["word"]) is type(call["link_register"]) is int
            and call["pc"] == pc
            and call["word"] == word,
            "binding original call word/PC",
        )
        direct = word >> 26 == 3
        _require(
            call["link_register"] == (31 if direct else (word >> 11) & 31),
            "binding original link register",
        )
        for field in (
            "reachability",
            "active_target",
            "scoped_observation",
            "universal_binding",
        ):
            _binding_fact(call[field])
        if direct:
            address = ((pc + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2)
            _keys(call["target"], {"kind", "address"}, "direct binding target")
            _require(
                call["target"]["kind"] == "direct"
                and type(call["target"]["address"]) is int
                and call["target"]["address"] == address,
                "binding direct destination",
            )
            covering = [
                target
                for target in contexts.targets
                if (target.image_id, address) in entry_words
            ]
        else:
            _keys(
                call["target"], {"kind", "register", "value"}, "register binding target"
            )
            _require(
                call["target"]["kind"] == "register"
                and type(call["target"]["register"]) is int
                and call["target"]["register"] == (word >> 21) & 31,
                "binding original source register",
            )
            _binding_fact(call["target"]["value"])
            covering = []
        alternatives = _rows(call["alternatives"], 128, "binding alternatives")
        total += len(alternatives)
        _require(
            len(alternatives) == len(covering) and total <= 4096,
            "binding covering inventory/aggregate bound",
        )
        for alternative, target in zip(alternatives, covering, strict=True):
            image = images[target.image_id]
            _keys(
                alternative,
                set(
                    "target image_id payload_sha256 address payload_offset address_coverage entry_word reviewed_entry analysis_context residency callee_contract declaration".split()
                ),
                "binding alternative",
            )
            offset = address - image.load_address
            _require(
                alternative["target"] == target.target
                and alternative["image_id"] == target.image_id
                and alternative["payload_sha256"]
                == metadata[target.image_id]["payload_sha256"]
                and type(alternative["address"])
                is type(alternative["payload_offset"])
                is int
                and alternative["address"] == address
                and alternative["payload_offset"] == offset,
                "qualified binding alternative identity/span",
            )
            _binding_fact(alternative["address_coverage"], complete=True)
            for field in ("residency", "callee_contract", "declaration"):
                _binding_fact(alternative[field])
            entry = entry_words[target.image_id, address]
            if entry is None:
                _binding_fact(alternative["entry_word"])
            else:
                _keys(
                    alternative["entry_word"],
                    {"kind", "payload_offset", "word"},
                    "binding entry word",
                )
                _require(
                    type(alternative["entry_word"]["word"])
                    is type(alternative["entry_word"]["payload_offset"])
                    is int
                    and alternative["entry_word"]
                    == {
                        "kind": "four-file-bytes",
                        "payload_offset": offset,
                        "word": entry,
                    },
                    "binding four physical entry bytes",
                )
            reviewed = next(
                (
                    member
                    for member in members.values()
                    if member["target"] == target.target
                    and member["image_id"] == target.image_id
                    and member["start"] == address
                ),
                None,
            )
            _require(
                alternative["reviewed_entry"] == reviewed,
                "binding exact reviewed entry",
            )
            context = (
                {"kind": "same-image-reviewed-scope", "member": reviewed["key"]}
                if reviewed is not None and reviewed["key"] in scoped
                else {"kind": "not-analyzed"}
            )
            _require(
                alternative["analysis_context"] == context,
                "binding same-image input marker",
            )


def _validate_graph_links(rows, root, bindings):
    if bindings["scan"]["kind"] != "complete":
        return
    calls = {call["pc"]: call for call in bindings["scan"]["calls"]}
    for call in rows[root["key"]]["returnDisposition"]["evidence"]["calls"]:
        callee = call["contract"]["callee"]
        if not callee:
            continue
        _require(
            call["pc"] in calls,
            "qualified backend call missing from raw binding inventory",
        )
        alternatives = calls[call["pc"]]["alternatives"]
        derived = rows.get(callee, {}).get("originalDiscovered")
        if derived is not None:
            _require(
                any(
                    alternative["image_id"] == derived["imageId"]
                    and alternative["address"] == call["target"]
                    for alternative in alternatives
                ),
                "native discovered callee is missing from original binding alternatives",
            )
            continue
        _require(
            any(
                alternative["target"] == root["target"]
                and alternative["image_id"] == root["image_id"]
                and alternative["address"] == call["target"]
                and alternative["analysis_context"]
                == {"kind": "same-image-reviewed-scope", "member": callee}
                for alternative in alternatives
            ),
            "backend call/callee qualified binding linkage",
        )


def _validate_reviewed_member(row, root, payload, load):
    _keys(
        row,
        set("key target image_id start end range_sha256".split()),
        "reviewed function",
    )
    for key in ("key", "target", "image_id"):
        _require(
            type(row[key]) is str and 0 < len(row[key].encode()) <= 4096,
            "reviewed function identity",
        )
    _require(
        row["target"] == root["target"] and row["image_id"] == root["image_id"],
        "same-image reviewed function",
    )
    for key in ("start", "end"):
        _require(
            type(row[key]) is int and 0 <= row[key] <= 1 << 32,
            "reviewed function address",
        )
    first, last = row["start"] - load, row["end"] - load
    _require(
        0 <= first < last <= len(payload)
        and first % 4 == last % 4 == 0
        and last - first <= 64 << 10,
        "reviewed original extent",
    )
    _require(
        type(row["range_sha256"]) is str
        and re.fullmatch(r"[0-9a-f]{64}", row["range_sha256"])
        and hashlib.sha256(payload[first:last]).hexdigest() == row["range_sha256"],
        "reviewed original slice identity",
    )


def _overlay_owner(context, address, count):
    canonical = (address & 0x1FFFFFFF) | 0x80000000
    for interval in context["visible_intervals"]:
        if interval["start"] <= canonical < interval["end"]:
            return (
                interval["image_id"] if count <= interval["end"] - canonical else None
            )
    return None


def validate_overlay_context(
    configuration, request, contexts, *, copied_original=None, copied_payload=None
):
    _require(contexts is not None, "overlay context lacks permitted original images")
    catalog = verify_native_contexts(contexts)
    copied = request.get("schema") == "retdec118.native-request/v7"
    _require(
        copied == (copied_original is not None)
        and copied == (copied_payload is not None),
        "native copied context lacks validated original capture",
    )
    context_fields = {"schema", "identity_sha256", "mounts", "visible_intervals"}
    if copied:
        context_fields.add("instruction_scope_sha256")
    context = _keys(
        configuration["decompParams"].get("psxMemoryContext"),
        context_fields,
        "native overlay context",
    )
    _require(
        context["schema"] == "psx.memory-context/v1", "native overlay context schema"
    )
    unsigned = dict(context, identity_sha256="")
    _require(
        context["identity_sha256"]
        == hashlib.sha256(
            json.dumps(
                unsigned, ensure_ascii=False, separators=(",", ":"), allow_nan=False
            ).encode()
        ).hexdigest(),
        "native overlay context digest differs",
    )
    mounts = _rows(context["mounts"], 128, "native overlay mounts")
    copy_hash = (
        hashlib.sha256(copies.descriptor_bytes(copied_original)).hexdigest()
        if copied
        else None
    )
    copy_id = f"observed-copy:{copy_hash}" if copied else None
    if copied:
        _require(
            copied_original == request["copied_original"]
            and context["instruction_scope_sha256"] == copy_hash,
            "native instruction scope differs from validated copy",
        )
    _require(
        [mount["image_id"] for mount in mounts]
        == [*request["overlay_mounts"], *([copy_id] if copied else [])],
        "native overlay mount order differs from request",
    )
    metadata = {image["image_id"]: image for image in catalog["images"]}
    images = {image.image_id: image for image in contexts.images}
    payloads = {}
    boundaries = set()
    for mount in mounts:
        _keys(
            mount,
            set(
                "image_id target file_path file_sha256 file_bytes payload_offset payload_sha256 payload_bytes load_address mapping_kind mapping_path mapping_sha256".split()
            ),
            "native overlay image",
        )
        if copied and mount["image_id"] == copy_id:
            bios = copied_original["files"]["bios.bin"]
            expected = {
                "image_id": copy_id,
                "target": "bios/" + bios["sha256"],
                "file_path": str(
                    contexts.input_directory.absolute().parent / bios["path"]
                ),
                "file_sha256": bios["sha256"],
                "file_bytes": bios["bytes"],
                "payload_offset": copied_original["source_offset"],
                "payload_sha256": copied_original["sha256"],
                "payload_bytes": copied_original["bytes"],
                "load_address": copied_original["destination"],
                "mapping_kind": "observed-original-copy/v1",
                "mapping_path": str(
                    contexts.input_directory.absolute() / "copy-descriptor.json"
                ),
                "mapping_sha256": copy_hash,
            }
            _require(
                mount == expected,
                "native copied mount differs from validated descriptor",
            )
            _require(
                type(copied_payload) is bytes
                and len(copied_payload) == copied_original["bytes"]
                and hashlib.sha256(copied_payload).hexdigest()
                == copied_original["sha256"],
                "native copied original slice differs",
            )
            payloads[copy_id] = copied_payload, copied_original["destination"]
            boundaries.update(
                (mount["load_address"], mount["load_address"] + len(copied_payload))
            )
            continue
        row = metadata[mount["image_id"]]
        _require(
            all(
                mount[key] == row[key]
                for key in (
                    "file_sha256",
                    "file_bytes",
                    "payload_offset",
                    "payload_sha256",
                    "payload_bytes",
                    "load_address",
                )
            )
            and mount["mapping_kind"] == "original-catalog"
            and mount["mapping_sha256"] == contexts.catalog_sha256,
            "native overlay metadata differs from permitted catalog",
        )
        targets = [
            target["target"]
            for target in catalog["targets"]
            if target["image_id"] == mount["image_id"]
        ]
        _require(
            mount["target"] == (targets[0] if len(targets) == 1 else mount["image_id"]),
            "native overlay target differs from catalog",
        )
        index = row["payload_index"]
        _require(
            Path(mount["file_path"]).is_absolute()
            and Path(mount["mapping_path"]).is_absolute()
            and Path(mount["file_path"])
            == _context_file(contexts.input_directory.absolute(), index)
            and Path(mount["mapping_path"])
            == contexts.input_directory.absolute() / "image-catalog.json"
            and Path(mount["file_path"])
            .resolve()
            .is_relative_to(contexts.input_directory.resolve())
            and Path(mount["mapping_path"])
            .resolve()
            .is_relative_to(contexts.input_directory.resolve()),
            "native overlay file mapping differs from staged catalog",
        )
        original = _original_file(
            _context_file(contexts.input_directory, index), images[mount["image_id"]]
        )
        payload = original[
            row["payload_offset"] : row["payload_offset"] + row["payload_bytes"]
        ]
        _require(
            hashlib.sha256(payload).hexdigest() == row["payload_sha256"],
            "native overlay original payload differs",
        )
        base = (row["load_address"] & 0x1FFFFFFF) | 0x80000000
        _require(
            row["load_address"] == base
            and 0x80000000 <= base < 0x80200000
            and len(payload) <= 0x80200000 - base,
            "original-only overlay consumer requires canonical KSEG0 mappings",
        )
        payloads[mount["image_id"]] = payload, row["load_address"]
        boundaries.update((base, base + len(payload)))
    if copied:
        copy_start = copied_original["destination"]
        copy_end = copy_start + copied_original["bytes"]
        ranges = [
            (mount["load_address"], mount["payload_bytes"]) for mount in mounts[:-1]
        ] + [
            (member["start"], member["end"] - member["start"])
            for member in [request["member"], *request["callees"]]
        ]
        for address, count in ranges:
            if (
                not 0 <= address <= 0xFFFFFFFF
                or address & 0xE0000000 not in (0, 0x80000000, 0xA0000000)
                or address & 0x1FFFFFFF >= 0x200000
            ):
                continue
            base = (address & 0x1FFFFFFF) | 0x80000000
            _require(
                copy_end <= base or base + count <= copy_start,
                "native copied window overlaps a catalog mount or reviewed function",
            )
    visible = []
    ordered = sorted(boundaries)
    for start, end in zip(ordered, ordered[1:]):
        for mount in reversed(mounts):
            base = (mount["load_address"] & 0x1FFFFFFF) | 0x80000000
            if not base <= start < base + mount["payload_bytes"]:
                continue
            if (
                visible
                and visible[-1]["image_id"] == mount["image_id"]
                and visible[-1]["end"] == start
            ):
                visible[-1]["end"] = end
            else:
                visible.append(
                    dict(
                        start=start,
                        end=end,
                        image_id=mount["image_id"],
                        payload_offset=start - base,
                    )
                )
            break
    _require(
        context["visible_intervals"] == visible,
        "native visible overlay intervals differ",
    )
    root = request["member"]
    _require(
        _overlay_owner(context, root["start"], root["end"] - root["start"])
        == root["image_id"],
        "native selected root has mixed or replaced image ownership",
    )
    return context, payloads


def validate_contract_graph(
    configuration,
    wanted,
    request,
    expected_scope,
    payload,
    load,
    *,
    release_configuration_sha256,
    bindings=None,
    contexts=None,
    entry_observation: bool | None = None,
    copied_original=None,
    copied_payload=None,
):
    _require(
        entry_observation is None
        or (
            type(entry_observation) is bool
            and wanted.get("schema")
            in {
                "retdec118.native-request/v2",
                "retdec118.native-request/v3",
                "retdec118.native-request/v4",
                "retdec118.native-request/v5",
                "retdec118.native-request/v6",
                "retdec118.native-request/v7",
                "retdec118.native-request/v8",
            }
            and entry_observation == ("entry_observation" in wanted)
            and entry_observation
            == (
                wanted["schema"]
                in {"retdec118.native-request/v6", "retdec118.native-request/v8"}
            )
        ),
        "validated invocation entry observation context differs",
    )
    _keys(expected_scope, {"problem_sha256", "callees"}, "expected reviewed scope")
    _require(
        wanted["problem_sha256"] == expected_scope["problem_sha256"]
        and wanted["callees"] == expected_scope["callees"],
        "native request differs from staged reviewed scope",
    )
    _require(
        type(wanted["problem_sha256"]) is str
        and re.fullmatch(r"[0-9a-f]{64}", wanted["problem_sha256"]),
        "active Problem identity",
    )
    scope = _keys(
        configuration["decompParams"].get("originalCallScope"),
        set(
            "request_sha256 configuration_sha256 payload_sha256 problem_sha256 root callees".split()
        ),
        "native original call scope",
    )
    _require(
        scope
        == {
            "request_sha256": hashlib.sha256(request).hexdigest(),
            "configuration_sha256": release_configuration_sha256,
            "payload_sha256": hashlib.sha256(payload).hexdigest(),
            "problem_sha256": wanted["problem_sha256"],
            "root": wanted["member"],
            "callees": wanted["callees"],
        },
        "native invocation graph binding",
    )
    inventory = [
        scope["root"],
        *_rows(scope["callees"], 117, "reviewed callee inventory"),
    ]
    identities, entries = {}, {}
    context = active_images = None
    if wanted.get("schema") in {
        "retdec118.native-request/v5",
        "retdec118.native-request/v6",
        "retdec118.native-request/v7",
        "retdec118.native-request/v8",
    }:
        context, active_images = validate_overlay_context(
            configuration,
            wanted,
            contexts,
            copied_original=copied_original,
            copied_payload=copied_payload,
        )
    else:
        _require(
            "psxMemoryContext" not in configuration["decompParams"]
            and all(
                "originalDiscovered" not in row
                for row in configuration.get("functions", [])
            ),
            "legacy original-only request acquired an undeclared overlay context",
        )
    if copied_original is not None:
        for function in configuration.get("functions", []):
            native = function.get("returnDisposition", {})
            evidence = native.get("evidence")
            _require(type(evidence) is dict, "copied context function lacks evidence")
            start = evidence.get("start")
            if native.get("rule") == "mips-all-normal-exits/v1":
                _keys(
                    function,
                    {
                        "name",
                        "callingConvention",
                        "fncType",
                        "startAddr",
                        "endAddr",
                        "returnDisposition",
                    },
                    "unavailable native symbol",
                )
                _require(
                    type(start) is int
                    and 0 <= start < 1 << 32
                    and evidence.get("end") == start + 1
                    and proofvalues._address(function["startAddr"]) == start
                    and proofvalues._address(function["endAddr"]) == start + 1
                    and function["callingConvention"] == "unknown"
                    and function["fncType"] == "decompilerDefined"
                    and evidence.get("decodeComplete") is False
                    and evidence.get("analysisComplete") is False
                    and all(
                        evidence.get(category) == []
                        for category in (
                            "instructions",
                            "edges",
                            "definitions",
                            "exits",
                            "calls",
                            "obligations",
                        )
                    ),
                    "unavailable native symbol acquired analysis facts",
                )
                callcontracts.validate_return(
                    function,
                    {"start": start, "end": start + 1, "selection": "diagnostic"},
                )
                continue
            reasons = {
                item.get("reason")
                for item in _rows(
                    evidence.get("obligations"), 4096, "copied context obligations"
                )
                if type(item) is dict
                and item.get("pc") == start
                and item.get("domain") == "architectural"
            }
            _require(
                "observed-copy-context-is-not-universal" in reasons,
                "copied context lost its universal acceptance obligation",
            )
            if start == copied_original["entry"]:
                _require(
                    "observed-copy-entry-does-not-prove-caller-edge" in reasons,
                    "copied entry lost its caller edge obligation",
                )
            summary = function.get("originalCallSummary")
            if summary is not None:
                _require(
                    not proofvalues._complete(summary["declaration"]),
                    "copied context acquired an accepted declaration",
                )
    for member in inventory:
        _validate_reviewed_member(member, scope["root"], payload, load)
        _require(
            member["key"] not in identities and member["start"] not in entries,
            "duplicate reviewed function",
        )
        identities[member["key"]] = dict(member, root=member is scope["root"])
        entries[member["start"]] = member["key"]
    derived_count = derived_words = 0
    copied_entry_found = copied_original is None
    if active_images is not None:
        owners = {image["image_id"]: image for image in context["mounts"]}
        for function in configuration.get("functions", []):
            node = function.get("originalDiscovered")
            if node is None:
                continue
            _keys(
                node,
                set(
                    "kind key target imageId contextHash start end words remaining".split()
                ),
                "native discovered function",
            )
            _require(
                node["kind"] == "native-bounded-callee/v1"
                and node["contextHash"] == context["identity_sha256"]
                and node["imageId"] in active_images,
                "native discovered context identity",
            )
            start, end = node["start"], node["end"]
            if copied_original is not None and start == copied_original["entry"]:
                _require(
                    node["imageId"]
                    == f"observed-copy:{context['instruction_scope_sha256']}",
                    "observed copied entry differs from its original owner",
                )
                copied_entry_found = True
            _require(
                type(start) is int
                and type(end) is int
                and 0x80000000 <= start < end <= 0x80200000
                and start % 4 == end % 4 == 0
                and start < end <= start + 1024
                and start not in entries
                and node["key"] not in identities,
                "native discovered instruction bound",
            )
            _require(
                node["target"] == owners[node["imageId"]]["target"]
                and node["key"] == f"{node['target']}@{start:x}",
                "native discovered target/key differs from its owner",
            )
            words = _rows(node["words"], 256, "native discovered words")
            derived_count += 1
            derived_words += len(words)
            _require(
                derived_count <= 32 and derived_words <= 4096,
                "native discovered aggregate bound",
            )
            bytes_, base = active_images[node["imageId"]]
            observed = set()
            for word in words:
                _keys(word, {"pc", "word"}, "native discovered word")
                pc = word["pc"]
                _require(
                    type(pc) is int
                    and start <= pc < end
                    and pc % 4 == 0
                    and pc not in observed
                    and type(word["word"]) is int
                    and 0 <= word["word"] < 1 << 32
                    and 0 <= pc - base <= len(bytes_) - 4
                    and word["word"]
                    == int.from_bytes(bytes_[pc - base : pc - base + 4], "little"),
                    "native discovered original instruction",
                )
                observed.add(pc)
            _require(
                start in observed
                and observed
                and max(observed) + 4 == end
                and all(
                    word["pc"] in observed
                    for word in function["returnDisposition"]["evidence"][
                        "instructions"
                    ]
                ),
                "native discovered retained ownership",
            )
            _require(
                _overlay_owner(context, start, end - start) == node["imageId"],
                "native discovered visible ownership",
            )
            reasons = _rows(node["remaining"], 256, "native discovered obligations")
            obligations = function["returnDisposition"]["evidence"]["obligations"]
            _require(
                all(
                    type(reason) is str
                    and reason
                    and any(
                        item["pc"] == start
                        and item["domain"] == "architectural"
                        and item["reason"] == reason
                        for item in obligations
                    )
                    for reason in reasons
                ),
                "native discovered remaining facts lack architectural obligations",
            )
            identities[node["key"]] = dict(
                key=node["key"],
                target=node["target"],
                image_id=node["imageId"],
                start=start,
                end=end,
                root=False,
            )
            entries[start] = node["key"]
    _require(copied_entry_found, "observed copied entry discovery is missing")
    extents = sorted((item["start"], item["end"]) for item in identities.values())
    _require(
        all(first[1] <= second[0] for first, second in zip(extents, extents[1:])),
        "overlapping original functions",
    )
    rows = {}
    for row in configuration.get("functions", []):
        native = row.get("returnDisposition", {})
        revised = native.get("rule") == "mips-all-normal-exits/v2"
        if not revised:
            _require(
                "originalCallSummary" not in row,
                "summary without revised return evidence",
            )
            continue
        start = native["evidence"]["start"]
        _require(
            start in entries and proofvalues._address(row.get("startAddr")) == start,
            "activated function identity",
        )
        key = entries[start]
        _require(
            key not in rows
            and proofvalues._address(row.get("endAddr")) == identities[key]["end"],
            "activated function extent/uniqueness",
        )
        identity = identities[key]
        if active_images is not None:
            _require(
                0x80000000 <= identity["start"] < identity["end"] <= 0x80200000
                and _overlay_owner(
                    context, identity["start"], identity["end"] - identity["start"]
                )
                == identity["image_id"],
                "activated original function has mixed or replaced ownership",
            )
        original, base = (
            active_images[identity["image_id"]]
            if active_images is not None
            else (payload, load)
        )
        for item in native["evidence"]["instructions"]:
            pc = item["pc"]
            _require(
                identity["start"] <= pc < identity["end"]
                and pc % 4 == 0
                and 0 <= pc - base <= len(original) - 4
                and item["owner"] == row["name"]
                and item["word"]
                == int.from_bytes(original[pc - base : pc - base + 4], "little"),
                "function-owned original instruction word",
            )
        rows[key] = row
    _require(scope["root"]["key"] in rows, "selected root graph is missing")
    callcontracts.validate_functions(
        rows,
        identities,
        payload=payload,
        load=load,
        images=active_images,
        context=context,
        entry_observation=entry_observation,
    )
    for row in rows.values():
        _address_views(row)

    if bindings is not None:
        _validate_graph_links(rows, scope["root"], bindings)
    return context, active_images
