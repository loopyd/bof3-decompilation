"""Read-only discovery and reviewed transactions for C source identifier names.

Arguments and local variables are lexical C entities, not map symbols. Discovery
parses already-lifted source and never opens the reverse index. A rename
transaction is contained to one implementation span, refuses a new spelling that
already occurs anywhere in the file, and is verified by a native byte-identity
gate before it can be accepted.
"""

from __future__ import annotations

import hashlib
import json
import re
from pathlib import Path
from typing import Any

from harness.common.commands import command as canonical
from harness.common.digests import digest
from harness.common.files import atomic_write
from harness.common.lease import acquire_writer
from harness.common.lexicon import iter_c_lexemes
from harness.common.process import run_bounded
from harness.domain.functions import parse_function_records, select_function_record
from harness.domain.ids import parse_function_id
from harness.domain.manifests import load_target_manifests
from harness.domain.registry import resolve_function

SCHEMA = "bof3.source-identifiers/v1"
RECEIPT_SCHEMA = "bof3.source-rename-transaction/v1"
NATIVE_SCHEMA = "bof3.source-rename-native/v1"
KINDS = ("argument", "local")

_IDENTIFIER = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")
_C_KEYWORDS = frozenset(
    "auto break case char const continue default do double else enum extern float "
    "for goto if inline int long register restrict return short signed sizeof "
    "static struct switch typedef union unsigned void volatile while _Bool".split()
)
_SCALAR_TYPES = frozenset("bool s8 s16 s32 s64 u8 u16 u32 u64 f32 f64".split())
_QUALIFIERS = frozenset("const volatile static register auto".split())
_TYPE_HEADS = frozenset("struct union enum".split())
_NATIVE_TIMEOUT = 180
_NATIVE_OUTPUT_LIMIT = 2 * 1024 * 1024


def _masked(text: str) -> str:
    """Blank comments and literals while retaining every text offset."""

    parts: list[str] = []
    start = 0
    for match in iter_c_lexemes(text):
        parts.append(text[start : match.start()])
        parts.append(re.sub(r"[^\n]", " ", match.group()))
        start = match.end()
    return "".join(parts) + text[start:]


def _tags(masked: str) -> set[str]:
    return set(re.findall(r"\b(?:struct|union|enum)\s+([A-Za-z_]\w*)", masked))


def _declaration_names(statement: str) -> tuple[str, ...]:
    """Return declared ordinary names, dropping aggregate tags and type heads."""

    from harness.domain.c_context import declaration_names

    masked = _masked(statement)
    try:
        names = declaration_names(statement)
    except (ValueError, RecursionError):
        return ()
    tags = _tags(masked)
    return tuple(name for name in names if name not in tags and name not in _C_KEYWORDS)


def _declarator_names(remainder: str) -> tuple[str, ...]:
    """Parse a declarator list whose type prefix has already been consumed."""

    if not re.match(r"\s*[*([\s]*[A-Za-z_]", remainder):
        return ()
    names: list[str] = []
    for item in re.split(r",(?![^()\[\]]*[)\]}])", remainder):
        item = item.strip()
        if not item or "->" in item or "." in item.split("=", 1)[0]:
            continue
        head = item.split("=", 1)[0]
        names.extend(_declaration_names(f"{head.strip()};"))
    return tuple(dict.fromkeys(names))


def _is_declaration(statement: str) -> bool:
    """Classify one statement as a scalar declaration without guessing types."""

    masked = _masked(statement).strip()
    if not masked.endswith(";"):
        return False
    masked = masked[:-1].strip()
    tokens = _IDENTIFIER.findall(masked)
    if (
        not tokens
        or tokens[0] in _C_KEYWORDS - _SCALAR_TYPES - _TYPE_HEADS - _QUALIFIERS
    ):
        return False
    # A target-owned typedef is still a valid type head; require a declarator.
    remainder = masked
    while True:
        match = re.match(r"\s*([A-Za-z_]\w*)", remainder)
        if match is None:
            return False
        token = match.group(1)
        if token in _QUALIFIERS:
            remainder = remainder[match.end() :]
            continue
        if token in _TYPE_HEADS:
            remainder = remainder[match.end() :]
            tag = re.match(r"\s*[A-Za-z_]\w*", remainder)
            if tag is not None:
                remainder = remainder[tag.end() :]
            continue
        remainder = remainder[match.end() :]
        break
    if remainder.lstrip().startswith("="):
        return False
    return bool(_declarator_names(remainder))


def _signature_and_body(implementation: str) -> tuple[str, str]:
    """Split one implementation into its signature and body text."""

    masked = _masked(implementation)
    parens = 0
    body_start = None
    for index, character in enumerate(masked):
        if character == "(":
            parens += 1
        elif character == ")":
            parens -= 1
        elif character == "{" and parens == 0:
            body_start = index
            break
    if body_start is None:
        raise ValueError("source identifier scan requires a function body")
    body_end = masked.rfind("}")
    if body_end < body_start:
        raise ValueError("source identifier scan requires a closed function body")
    return implementation[:body_start], implementation[body_start + 1 : body_end]


def _parameter_span(signature: str) -> tuple[int, int]:
    """Locate the parameter-list span inside one function signature."""

    masked = _masked(signature)
    depth = 0
    opening = None
    for index, character in enumerate(masked):
        if character == "(":
            if depth == 0:
                opening = index
            depth += 1
        elif character == ")":
            depth -= 1
            if depth == 0 and opening is not None:
                return opening, index
    raise ValueError("source identifier scan requires a closed parameter list")


def _parameter_declarations(signature: str) -> list[str]:
    opening, closing = _parameter_span(signature)
    span = signature[opening + 1 : closing]
    masked = _masked(span)
    if not masked.strip() or masked.strip() == "void":
        return []
    declarations = [
        item.strip() for item in re.split(r",(?![^(]*\))", span) if item.strip()
    ]
    return declarations


def _statement_declarations(body: str) -> list[str]:
    """Enumerate body statements whose leading type head is a declaration."""

    masked = _masked(body)
    declarations: list[str] = []
    start = 0
    parens = brackets = 0
    for index, character in enumerate(masked):
        if character == "(":
            parens += 1
        elif character == ")":
            parens -= 1
        elif character == "[":
            brackets += 1
        elif character == "]":
            brackets -= 1
        elif character == ";" and parens == 0 and brackets == 0:
            statement = body[start : index + 1]
            if _is_declaration(statement):
                declarations.append(statement)
            start = index + 1
    for match in re.finditer(r"\bfor\s*\(([^;{}]*);", masked):
        clause = body[match.start(1) : match.end(1)]
        if _is_declaration(clause + ";"):
            declarations.append(clause + ";")
    return declarations


def _identifiers(root, selector: str):
    """Resolve one selector to its source, record and lexical identifiers."""

    resolved = resolve_function(root, selector)
    if resolved.source is None:
        raise ValueError("selector has no authored source claim")
    source = resolved.source.resolve()
    text = source.read_text(encoding="utf-8")
    record = select_function_record(text, resolved.id.address)
    if record.kind != "function":
        raise ValueError("selector is not an ordinary function implementation")
    implementation = text[record.implementation_start : record.implementation_end]
    signature, body = _signature_and_body(implementation)
    masked = _masked(implementation)
    uses: dict[str, int] = {}
    for name in _IDENTIFIER.findall(masked):
        uses[name] = uses.get(name, 0) + 1
    return resolved, source, text, record, signature, body, uses


def collect_source_identifiers(root: Path, target: str) -> list[dict[str, Any]]:
    """Enumerate argument and local naming leads for every claimed target source."""

    from harness.domain.ids import normalize_target_id

    target = normalize_target_id(target).value
    manifests = load_target_manifests(root)
    if target not in manifests:
        raise ValueError(f"unknown target: {target}")
    manifest = manifests[target]
    sources = sorted(set(manifest.sources) | set(manifest.support_sources))
    rows: list[dict[str, Any]] = []
    for relative in sources:
        if not relative.endswith(".c"):
            continue
        path = root / relative
        if not path.is_file():
            continue
        try:
            records = parse_function_records(path.read_text(encoding="utf-8"))
        except ValueError:
            continue
        for record in records:
            if record.kind != "function":
                continue
            rows.extend(_rows_for_record(root, target, relative, record.address))
    return sorted(
        rows, key=lambda row: (row["selector"], row["kind"] != "argument", row["name"])
    )


def _rows_for_record(
    root: Path, target: str, relative: str, address: int
) -> list[dict[str, Any]]:
    selector = f"{target}@{address:08X}"
    try:
        resolved, _source, _text, record, signature, body, uses = _identifiers(
            root, selector
        )
    except (ValueError, FileNotFoundError):
        return []
    parameters = _parameter_declarations(signature)
    locals_ = _statement_declarations(body)
    rows: list[dict[str, Any]] = []
    for kind, declarations in (("argument", parameters), ("local", locals_)):
        seen: set[str] = set()
        for statement in declarations:
            names = _declaration_names(statement)
            if not names:
                continue
            for name in names:
                if name in seen or name in _C_KEYWORDS:
                    continue
                seen.add(name)
                row = {
                    "schema": SCHEMA,
                    "id": f"{selector}@{kind}:{name}",
                    "target": target,
                    "selector": selector,
                    "function": record.spelling,
                    "kind": kind,
                    "name": name,
                    "source": relative,
                    "line": record.line,
                    "uses": uses.get(name, 0),
                    "declaration": " ".join(statement.split()),
                }
                rows.append({**row, "fingerprint": digest(row)})
    return rows


def describe_source_identifier(
    root: Path,
    target: str,
    identifier: str,
    *,
    expected_fingerprint: str | None = None,
) -> dict[str, Any]:
    """Require exact current membership of one source-identifier lead."""

    matches = [
        row
        for row in collect_source_identifiers(root, target)
        if row["id"] == identifier
    ]
    if len(matches) != 1:
        raise ValueError("unknown source identifier for target")
    row = matches[0]
    if expected_fingerprint is not None and row["fingerprint"] != expected_fingerprint:
        raise ValueError("source identifier fingerprint drifted")
    return row


def _occurrences(text: str, name: str) -> list[int]:
    return [match.start() for match in re.finditer(rf"\b{re.escape(name)}\b", text)]


def _code_occurrences(text: str, name: str) -> list[int]:
    """Occurrences in code only; comments and literals never bind an identifier."""

    return _occurrences(_masked(text), name)


def _validate_rename(
    text: str,
    record,
    *,
    kind: str,
    old_name: str,
    new_name: str,
) -> None:
    if kind not in KINDS:
        raise ValueError("source rename kind must be argument or local")
    if not _IDENTIFIER.fullmatch(new_name) or new_name in _C_KEYWORDS:
        raise ValueError("source rename target must be a non-keyword C identifier")
    if new_name == old_name:
        raise ValueError("source rename target already matches the current name")
    start, end = record.implementation_start, record.implementation_end
    occurrences = _code_occurrences(text, old_name)
    escapes = [index for index in occurrences if not start <= index < end]
    if escapes:
        raise ValueError(
            "source rename scope escape: identifier occurs outside function"
        )
    if not occurrences or not start <= occurrences[0] < end:
        raise ValueError("source rename identifier is absent from the function")
    if _code_occurrences(text, new_name):
        raise ValueError("source rename target already occurs in the file")


def _receipt_path(root: Path, value: Path) -> Path:
    path = (root / value).resolve() if not value.is_absolute() else value.resolve()
    try:
        path.relative_to(root)
    except ValueError as error:
        raise ValueError("source rename receipt escapes repository root") from error
    return path


def _load_receipt(path: Path) -> dict[str, Any]:
    payload = json.loads(path.read_text(encoding="utf-8"))
    if payload.get("schema") != RECEIPT_SCHEMA:
        raise ValueError("unexpected source rename receipt schema")
    return payload


def prepare_source_transaction(
    root: Path,
    target: str,
    selector: str,
    *,
    kind: str,
    old_name: str,
    new_name: str,
    output: Path,
) -> dict[str, Any]:
    """Bind one reviewed argument/local rename to exact PRE source state."""

    function = parse_function_id(selector)
    if function.target.value != target:
        raise ValueError("source rename selector does not match target")
    resolved, source, text, record, _signature, _body, _uses = _identifiers(
        root, selector
    )
    row = describe_source_identifier(root, target, f"{selector}@{kind}:{old_name}")
    _validate_rename(text, record, kind=kind, old_name=old_name, new_name=new_name)
    path = _receipt_path(root, output)
    backup = path.with_name(path.name + ".pre")
    payload = {
        "schema": RECEIPT_SCHEMA,
        "target": target,
        "selector": selector,
        "function": record.spelling,
        "kind": kind,
        "old_name": old_name,
        "new_name": new_name,
        "source": source.relative_to(root).as_posix(),
        "implementation": {
            "start": record.implementation_start,
            "end": record.implementation_end,
            "line": record.line,
        },
        "pre_sha256": hashlib.sha256(text.encode("utf-8")).hexdigest(),
        "post_sha256": None,
        "applied": False,
        "fingerprint": row["fingerprint"],
        "backup": backup.relative_to(root).as_posix(),
    }
    payload["receipt_sha256"] = digest(
        {key: value for key, value in payload.items() if key != "receipt_sha256"}
    )
    path.parent.mkdir(parents=True, exist_ok=True)
    with acquire_writer(root):
        atomic_write(root, backup.relative_to(root).as_posix(), text.encode("utf-8"))
        atomic_write(
            root,
            path.relative_to(root).as_posix(),
            json.dumps(payload, indent=2, sort_keys=True).encode("utf-8") + b"\n",
        )
    return payload


def apply_source_transaction(root: Path, receipt_path: Path) -> dict[str, Any]:
    """Apply one prepared rename inside its implementation span only."""

    path = _receipt_path(root, receipt_path)
    with acquire_writer(root):
        payload = _load_receipt(path)
        source = root / payload["source"]
        text = source.read_text(encoding="utf-8")
        if hashlib.sha256(text.encode("utf-8")).hexdigest() != payload["pre_sha256"]:
            raise ValueError("source changed after prepare; transaction refused")
        record = select_function_record(
            text, int(payload["selector"].split("@")[1], 16)
        )
        _validate_rename(
            text,
            record,
            kind=payload["kind"],
            old_name=payload["old_name"],
            new_name=payload["new_name"],
        )
        start, end = record.implementation_start, record.implementation_end
        updated = (
            text[:start]
            + re.sub(
                rf"\b{re.escape(payload['old_name'])}\b",
                payload["new_name"],
                text[start:end],
            )
            + text[end:]
        )
        atomic_write(
            root,
            payload["source"],
            updated.encode("utf-8"),
            expected=text.encode("utf-8"),
        )
        payload["post_sha256"] = hashlib.sha256(updated.encode("utf-8")).hexdigest()
        payload["applied"] = True
        payload["receipt_sha256"] = digest(
            {key: value for key, value in payload.items() if key != "receipt_sha256"}
        )
        atomic_write(
            root,
            path.relative_to(root).as_posix(),
            json.dumps(payload, indent=2, sort_keys=True).encode("utf-8") + b"\n",
        )
    return payload


def native_byte_identity(root: Path, selector: str) -> dict[str, Any]:
    """Require a byte-identical recompilation for one renamed function."""

    commands = (
        canonical("asm-diff", selector, "--json", "--detail", "normal"),
        canonical("byte-match", selector, "--json"),
    )
    records = []
    for argv in commands:
        result = run_bounded(
            root,
            argv,
            timeout=_NATIVE_TIMEOUT,
            output_limit=_NATIVE_OUTPUT_LIMIT,
        )
        if result["exit_code"] != 0 or result["failure"] is not None:
            raise ValueError(
                "source rename native gate failed: " + " ".join(str(a) for a in argv)
            )
        records.append(
            {
                "command": " ".join(str(a) for a in argv),
                "exit_code": result["exit_code"],
            }
        )
    return {"schema": NATIVE_SCHEMA, "selector": selector, "commands": records}


def verify_source_transaction(
    root: Path, receipt_path: Path, *, native: bool = True
) -> dict[str, Any]:
    """Prove the applied rename is the only change and remains byte-identical."""

    path = _receipt_path(root, receipt_path)
    payload = _load_receipt(path)
    if not payload["applied"]:
        raise ValueError("source rename transaction is not applied")
    source = root / payload["source"]
    text = source.read_text(encoding="utf-8")
    if hashlib.sha256(text.encode("utf-8")).hexdigest() != payload["post_sha256"]:
        raise ValueError("applied source no longer matches the transaction")
    start, end = payload["implementation"]["start"], payload["implementation"]["end"]
    expected = (
        text[:start]
        + re.sub(
            rf"\b{re.escape(payload['new_name'])}\b",
            payload["old_name"],
            text[start:end],
        )
        + text[end:]
    )
    original = (root / payload["backup"]).read_text(encoding="utf-8")
    if expected != original:
        raise ValueError("applied rename changed text outside the renamed identifier")
    if _code_occurrences(text, payload["old_name"]):
        raise ValueError("applied source still contains the old identifier")
    verified = {
        "schema": RECEIPT_SCHEMA,
        "target": payload["target"],
        "selector": payload["selector"],
        "kind": payload["kind"],
        "old_name": payload["old_name"],
        "new_name": payload["new_name"],
        "source": payload["source"],
        "post_sha256": payload["post_sha256"],
        "verified": True,
    }
    if native:
        verified["native"] = native_byte_identity(root, payload["selector"])
    return verified


def rollback_source_transaction(root: Path, receipt_path: Path) -> dict[str, Any]:
    """Restore the exact PRE bytes after an applied rename."""

    path = _receipt_path(root, receipt_path)
    with acquire_writer(root):
        payload = _load_receipt(path)
        source = root / payload["source"]
        backup = root / payload["backup"]
        current = source.read_bytes()
        if hashlib.sha256(current).hexdigest() != payload["post_sha256"]:
            raise ValueError(
                "source differs from the applied transaction; rollback refused"
            )
        atomic_write(
            root,
            payload["source"],
            backup.read_bytes(),
            expected=current,
        )
        payload["applied"] = False
        payload["post_sha256"] = None
        payload["receipt_sha256"] = digest(
            {key: value for key, value in payload.items() if key != "receipt_sha256"}
        )
        atomic_write(
            root,
            path.relative_to(root).as_posix(),
            json.dumps(payload, indent=2, sort_keys=True).encode("utf-8") + b"\n",
        )
    return payload
