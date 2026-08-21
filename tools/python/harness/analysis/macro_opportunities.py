"""Read-only, evidence-blocked macro and repetitive-source opportunities."""

from __future__ import annotations

import hashlib
import re
import sqlite3
from collections import defaultdict
from pathlib import Path
from typing import Any

from ..discovery import file_sha256
from .macro_exact_groups import exact_group_opportunities

_GENERATED = re.compile(
    r"(?:generated\s+by|auto[- ]?generated|do\s+not\s+edit|@generated)", re.IGNORECASE
)
_TOKEN = re.compile(
    r"(?P<space>\s+)|(?P<comment>/\*.*?\*/|//[^\n]*)|"
    r"(?P<string>\"(?:\\.|[^\"\\])*\"|'(?:\\.|[^'\\])*')|"
    r"(?P<number>0[xX][0-9A-Fa-f]+(?:[uUlL]+)?|\d+(?:[uUlL]+)?)|"
    r"(?P<identifier>[A-Za-z_]\w*)|"
    r"(?P<operator>->|<<=|>>=|\+\+|--|==|!=|<=|>=|&&|\|\||"
    r"[+\-*/%&|^~!<>=?:.,;(){}\[\]])",
    re.DOTALL,
)
_GUARD_NAMES = (
    "evaluation_count",
    "side_effects",
    "integer_promotions",
    "precedence",
    "lvalue",
    "volatile",
    "aliasing",
    "control_flow",
)
_TRIVIAL_LITERALS = {"0", "0U", "0u", "1", "1U", "1u"}
_CONTROL = {"break", "continue", "goto", "return", "case", "default"}
_ASSIGNMENTS = {"=", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>="}


def _without_directives(text: str) -> str:
    rows = text.splitlines(keepends=True)
    masked: list[str] = []
    skipping = False
    for row in rows:
        directive = skipping or row.lstrip().startswith("#")
        skipping = directive and row.rstrip("\r\n").rstrip().endswith("\\")
        masked.append(re.sub(r"[^\n]", " ", row) if directive else row)
    return "".join(masked)


def _tokens(text: str) -> list[tuple[str, str, int]]:
    result: list[tuple[str, str, int]] = []
    source = _without_directives(text)
    for match in _TOKEN.finditer(source):
        kind = match.lastgroup or ""
        if kind in {"space", "comment", "string"}:
            continue
        result.append((match.group(), kind, source.count("\n", 0, match.start()) + 1))
    return result


def _guards(**overrides: tuple[str, str]) -> dict[str, dict[str, str]]:
    guards = {
        name: {
            "status": "unproven",
            "evidence": "lexical repetition is not semantic proof",
        }
        for name in _GUARD_NAMES
    }
    for name, (status, evidence) in overrides.items():
        guards[name] = {"status": status, "evidence": evidence}
    return guards


def _candidate_id(kind: str, key: str) -> str:
    digest = hashlib.sha256(f"{kind}\0{key}".encode()).hexdigest()[:16]
    return f"{kind}:{digest}"


def _literal_context(tokens: list[tuple[str, str, int]], index: int) -> str:
    previous = tokens[index - 1][0] if index else ""
    following = tokens[index + 1][0] if index + 1 < len(tokens) else ""
    if previous == "case":
        return "case_label"
    if previous in {"==", "!=", "<", ">", "<=", ">="} or following in {
        "==",
        "!=",
        "<",
        ">",
        "<=",
        ">=",
    }:
        return "comparison"
    if previous == "[" or following == "]":
        return "extent_or_index"
    if previous in _ASSIGNMENTS:
        return "assignment_or_initializer"
    if previous in {"(", ","}:
        return "argument_or_expression"
    return "arithmetic_or_expression"


def _literal_type(token: str) -> str:
    suffix = re.search(r"[uUlL]+$", token)
    return "unsuffixed_int" if suffix is None else suffix.group().lower()


def _source_inputs(
    connection: sqlite3.Connection, root: Path, target: str | None
) -> list[tuple[str, str, Path, str]]:
    clauses = "WHERE m.owner_target = m.target_id"
    params: list[object] = []
    if target:
        clauses += " AND m.target_id = ?"
        params.append(target)
    rows = connection.execute(
        "SELECT m.target_id, m.source_path, m.sha256, m.input_kind "
        f"FROM macro_input_fingerprints m {clauses} "
        "ORDER BY m.target_id, m.source_path",
        params,
    ).fetchall()
    result: list[tuple[str, str, Path, str]] = []
    root_resolved = root.resolve()
    for target_id, source_path, digest, input_kind in rows:
        path = (root / source_path).resolve()
        if not path.is_relative_to(root_resolved) or not path.is_file():
            raise ValueError(
                f"stale macro opportunity source: {target_id}:{source_path}"
            )
        if file_sha256(path) != digest:
            raise ValueError(
                f"stale macro opportunity source: {target_id}:{source_path}"
            )
        text = path.read_text(encoding="utf-8", errors="replace")
        if not _GENERATED.search(text):
            result.append((target_id, source_path, path, text))
    return result


def _constant_opportunities(
    sources: list[tuple[str, str, Path, str]],
) -> list[dict[str, Any]]:
    occurrences: dict[tuple[str, str, str, str], list[dict[str, Any]]] = defaultdict(
        list
    )
    contexts: dict[tuple[str, str], set[str]] = defaultdict(set)
    for target, source_path, _path, text in sources:
        tokens = _tokens(text)
        for index, (token, kind, line) in enumerate(tokens):
            if kind != "number" or token in _TRIVIAL_LITERALS:
                continue
            context = _literal_context(tokens, index)
            type_context = _literal_type(token)
            contexts[(target, token)].add(context)
            occurrences[(target, token, context, type_context)].append(
                {"target": target, "source_path": source_path, "line": line}
            )
    candidates: list[dict[str, Any]] = []
    for (target, literal, context, type_context), members in occurrences.items():
        if len(members) < 3:
            continue
        other_contexts = sorted(contexts[(target, literal)] - {context})
        key = f"{target}:{literal}:{context}:{type_context}"
        candidates.append(
            {
                "id": _candidate_id("constant", key),
                "kind": "constant",
                "status": "blocked",
                "rank": len(members),
                "target_scope": target,
                "pattern": literal,
                "members": members,
                "evidence": {
                    "support": len(members),
                    "semantic_context": context,
                    "type_context": type_context,
                    "source_fingerprints": sorted(
                        {
                            file_sha256(path)
                            for t, _s, path, _text in sources
                            if t == target
                        }
                    ),
                },
                "counterexamples": [
                    {"kind": "different_semantic_context", "context": value}
                    for value in other_contexts
                ],
                "semantic_guards": _guards(
                    evaluation_count=(
                        "not_applicable",
                        "object-like constant has no operand",
                    ),
                    side_effects=("not_applicable", "literal has no side effects"),
                    precedence=(
                        "not_applicable",
                        "named constant replacement has no operator",
                    ),
                    lvalue=("not_applicable", "literal is not an lvalue"),
                    volatile=("not_applicable", "literal performs no access"),
                    aliasing=("not_applicable", "literal performs no access"),
                    control_flow=("not_applicable", "literal has no control flow"),
                ),
                "blockers": [
                    "semantic_name_unreviewed",
                    "integer_promotion_equivalence_unproven",
                    "read_only_analysis_only",
                ],
            }
        )
    return candidates


def _accessor_opportunities(
    sources: list[tuple[str, str, Path, str]],
) -> list[dict[str, Any]]:
    uses: dict[tuple[str, str, str, str], list[dict[str, Any]]] = defaultdict(list)
    unsafe: dict[tuple[str, str, str], list[dict[str, Any]]] = defaultdict(list)
    for target, source_path, _path, text in sources:
        tokens = _tokens(text)
        for index in range(1, len(tokens) - 1):
            operator = tokens[index][0]
            if operator not in {"->", "."} or tokens[index + 1][1] != "identifier":
                continue
            field = tokens[index + 1][0]
            previous = tokens[index - 1]
            key = (target, operator, field)
            if previous[1] != "identifier":
                unsafe[key].append(
                    {"target": target, "source_path": source_path, "line": previous[2]}
                )
                continue
            following = tokens[index + 2][0] if index + 2 < len(tokens) else ""
            use_kind = (
                "write"
                if following in _ASSIGNMENTS or following in {"++", "--"}
                else "read"
            )
            uses[(target, operator, field, use_kind)].append(
                {
                    "target": target,
                    "source_path": source_path,
                    "line": previous[2],
                    "receiver": previous[0],
                }
            )
    candidates: list[dict[str, Any]] = []
    for (target, operator, field, use_kind), members in uses.items():
        if len(members) < 3:
            continue
        key = f"{target}:{operator}:{field}:{use_kind}"
        counterexamples = [
            {**row, "kind": "non_identifier_receiver"}
            for row in unsafe[(target, operator, field)]
        ]
        candidates.append(
            {
                "id": _candidate_id("expression_accessor", key),
                "kind": "expression_accessor",
                "status": "blocked",
                "rank": len(members) * 2,
                "target_scope": target,
                "pattern": f"receiver{operator}{field}",
                "members": members,
                "evidence": {
                    "support": len(members),
                    "receiver_shape": "single identifier",
                    "use_kind": use_kind,
                },
                "counterexamples": counterexamples,
                "semantic_guards": _guards(
                    evaluation_count=(
                        "proven",
                        "each observed receiver is one identifier token",
                    ),
                    side_effects=(
                        "proven",
                        "identifier receiver has no lexical side effect",
                    ),
                    integer_promotions=(
                        "not_applicable",
                        "member access performs no arithmetic",
                    ),
                    precedence=(
                        "proven",
                        "a proposed ((receiver)->field) form can preserve precedence",
                    ),
                    lvalue=(
                        "proven",
                        "parenthesized member access preserves observed read/write category",
                    ),
                    control_flow=(
                        "not_applicable",
                        "member access has no control-flow operator",
                    ),
                ),
                "blockers": [
                    "receiver_type_unproven",
                    "volatile_qualification_unproven",
                    "alias_equivalence_unproven",
                    "read_only_analysis_only",
                ],
            }
        )
    return candidates


def _statement_opportunities(
    sources: list[tuple[str, str, Path, str]],
) -> list[dict[str, Any]]:
    windows: dict[tuple[str, tuple[str, ...]], list[dict[str, Any]]] = defaultdict(list)
    for target, source_path, _path, text in sources:
        tokens = _tokens(text)
        statements: list[list[tuple[str, str, int]]] = []
        current: list[tuple[str, str, int]] = []
        for token in tokens:
            current.append(token)
            if token[0] in {"{", "}"}:
                current = []
                statements = []
            elif token[0] == ";":
                if current:
                    statements.append(current)
                current = []
                if len(statements) >= 3:
                    window = statements[-3:]
                    normalized = tuple(
                        item[0] for statement in window for item in statement
                    )
                    if len(normalized) >= 12:
                        windows[(target, normalized)].append(
                            {
                                "target": target,
                                "source_path": source_path,
                                "start_line": window[0][0][2],
                                "end_line": window[-1][-1][2],
                            }
                        )
    candidates: list[dict[str, Any]] = []
    for (target, normalized), raw_members in windows.items():
        members = list(
            {
                (m["source_path"], m["start_line"], m["end_line"]): m
                for m in raw_members
            }.values()
        )
        if len(members) < 2:
            continue
        key = f"{target}:{' '.join(normalized)}"
        control = sorted(set(normalized) & _CONTROL)
        side_effect_tokens = sorted(set(normalized) & (_ASSIGNMENTS | {"++", "--"}))
        candidates.append(
            {
                "id": _candidate_id("statement_window", key),
                "kind": "statement_window",
                "status": "blocked",
                "rank": len(normalized) * len(members),
                "target_scope": target,
                "pattern": " ".join(normalized),
                "members": sorted(
                    members, key=lambda row: (row["source_path"], row["start_line"])
                ),
                "evidence": {
                    "support": len(members),
                    "statement_count": 3,
                    "token_count": len(normalized),
                    "normalization": "comments/whitespace/directives removed; tokens otherwise exact",
                    "control_tokens": control,
                    "side_effect_tokens": side_effect_tokens,
                },
                "counterexamples": [],
                "semantic_guards": _guards(
                    precedence=(
                        "proven",
                        "candidate retains the exact observed token stream",
                    ),
                    control_flow=(
                        "unproven" if control else "not_applicable",
                        f"control tokens: {control}"
                        if control
                        else "no control-transfer token observed",
                    ),
                ),
                "blockers": [
                    "evaluation_order_unproven",
                    "side_effect_equivalence_unproven",
                    "integer_promotion_equivalence_unproven",
                    "lvalue_equivalence_unproven",
                    "volatile_qualification_unproven",
                    "alias_equivalence_unproven",
                    "control_flow_equivalence_unproven",
                    "read_only_analysis_only",
                ],
            }
        )
    return candidates


def macro_opportunities_payload(
    connection: sqlite3.Connection,
    root: Path,
    *,
    target: str | None,
    kind: str | None,
    limit: int,
) -> list[dict[str, Any]]:
    """Return ranked blocked leads; never mutate source or the index."""

    sources = _source_inputs(connection, root, target)
    payload = (
        _constant_opportunities(sources)
        + _accessor_opportunities(sources)
        + _statement_opportunities(sources)
        + exact_group_opportunities(connection, target, _candidate_id, _guards)
    )
    if kind:
        payload = [row for row in payload if row["kind"] == kind]
    payload.sort(key=lambda row: (-row["rank"], row["kind"], row["id"]))
    return payload if limit == 0 else payload[:limit]


__all__ = ["macro_opportunities_payload"]
