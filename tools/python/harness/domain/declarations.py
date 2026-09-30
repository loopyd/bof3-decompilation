"""Namespace-qualified C declaration entities and provenance reconciliation."""

from __future__ import annotations

import re
from collections.abc import Iterable
from dataclasses import dataclass

_TOKEN = re.compile(
    r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|[A-Za-z_]\w*|'
    r"(?:\d[\w.]*|\.\d[\w.]*)|>>=|<<=|\.\.\.|->|\+\+|--|&&|\|\||"
    r"<<|>>|<=|>=|==|!=|[+*/%&|^!-]=|[^\s]"
)
_AGGREGATES = frozenset({"struct", "union", "enum"})


@dataclass(frozen=True)
class CField:
    """One lexical member; offset and width remain separate analysis facts."""

    name: str
    type_name: str
    array_extent: str | None = None
    qualifiers: str = ""


@dataclass(frozen=True)
class CDeclarator:
    """A named ordinary declarator with its unexpanded type expression."""

    name: str
    type_expression: str
    kind: str
    relationship: str = "identity"
    alias_namespace: str | None = None
    alias_name: str | None = None
    diagnostic: str | None = None


@dataclass(frozen=True)
class CDeclaration:
    """One source statement, potentially introducing several C entities."""

    names: tuple[str, ...]
    kind: str
    canonical: str
    tag_name: str | None = None
    fields: tuple[CField, ...] = ()
    diagnostic: str | None = None
    complete: bool = False
    definition: str | None = None
    declarators: tuple[CDeclarator, ...] = ()
    statement: str = ""


@dataclass(frozen=True)
class DeclarationKey:
    """Named scope/namespace identity, or an anonymous occurrence identity."""

    scope: str
    namespace: str
    name: str
    origin: str = ""
    ordinal: int = -1


@dataclass(frozen=True)
class CDeclarationOccurrence:
    """One entity's contribution from one source statement (zero-based ordinal)."""

    key: DeclarationKey
    origin: str
    ordinal: int
    canonical: str
    statement: str
    kind: str
    type_expression: str
    complete: bool
    definition: str | None
    fields: tuple[CField, ...] = ()
    alias_target: DeclarationKey | None = None
    alias_kind: str | None = None
    relationship: str | None = None
    diagnostic: str | None = None


@dataclass(frozen=True)
class CDeclarationEntity:
    """Reconciled facts with all evidence and explicit definition ownership."""

    key: DeclarationKey
    kind: str
    canonical: str
    type_expression: str
    complete: bool
    definition: str | None
    fields: tuple[CField, ...]
    alias_target: DeclarationKey | None
    alias_kind: str | None
    relationship: str | None
    representative: CDeclarationOccurrence
    selected_definition: CDeclarationOccurrence | None
    occurrences: tuple[CDeclarationOccurrence, ...]
    diagnostic: str | None


def tokenize_declaration(text: str) -> tuple[str, ...]:
    """Compare lexical tokens without changing literal contents or operators."""

    return tuple(_TOKEN.findall(text))


def build_declaration_occurrences(
    records: Iterable[CDeclaration], *, origin: str, scope: str = ""
) -> tuple[CDeclarationOccurrence, ...]:
    """Expand parser records without parsing again or copying alias layouts.

    Supply one call per origin, in source order. Scope is the caller's visibility
    domain; collecting headers alone does not establish translation-unit scope.
    Anonymous names are empty and their keys retain source origin and ordinal.
    Ordinary completeness means the declarator is known, not its type's layout.
    """

    occurrences: list[CDeclarationOccurrence] = []
    for ordinal, record in enumerate(records):
        common = dict(
            origin=origin,
            ordinal=ordinal,
            canonical=record.canonical,
            statement=record.statement or record.canonical,
        )
        tag_key = None
        # A prototype that merely returns or accepts an aggregate references the
        # tag; it never declares it, so emitting a tag occurrence would falsely
        # conflict with the real definition.
        declares_tag = not (
            record.declarators
            and all(item.relationship == "function" for item in record.declarators)
        )
        if record.kind in _AGGREGATES and declares_tag:
            tag_key = DeclarationKey(
                scope,
                "tag",
                record.tag_name or "",
                "" if record.tag_name else origin,
                -1 if record.tag_name else ordinal,
            )
            occurrences.append(
                CDeclarationOccurrence(
                    **common,
                    key=tag_key,
                    kind=record.kind,
                    type_expression=f"{record.kind} {record.tag_name or ''}".strip(),
                    complete=record.complete,
                    definition=record.definition,
                    fields=record.fields if record.complete else (),
                    diagnostic=record.diagnostic,
                )
            )
        for declarator in record.declarators:
            alias_target = None
            alias_kind = None
            if declarator.kind == "typedef":
                if declarator.alias_namespace == "tag":
                    alias_target = (
                        DeclarationKey(scope, "tag", declarator.alias_name)
                        if declarator.alias_name
                        else tag_key
                    )
                elif declarator.alias_namespace and declarator.alias_name:
                    alias_target = DeclarationKey(
                        scope, declarator.alias_namespace, declarator.alias_name
                    )
                alias_kind = (
                    "identity" if declarator.relationship == "identity" else "derived"
                )
            occurrences.append(
                CDeclarationOccurrence(
                    **common,
                    key=DeclarationKey(scope, "ordinary", declarator.name),
                    kind=declarator.kind,
                    type_expression=declarator.type_expression,
                    complete=declarator.diagnostic is None
                    and record.diagnostic is None,
                    definition=declarator.type_expression,
                    alias_target=alias_target,
                    alias_kind=alias_kind,
                    relationship=declarator.relationship,
                    diagnostic=declarator.diagnostic or record.diagnostic,
                )
            )
        if record.kind == "enum" and record.complete:
            for field in record.fields:
                occurrences.append(
                    CDeclarationOccurrence(
                        **common,
                        key=DeclarationKey(scope, "ordinary", field.name),
                        kind="enumerator",
                        type_expression=field.type_name,
                        complete=record.diagnostic is None,
                        definition=record.definition,
                        alias_target=tag_key,
                        relationship="enumerator",
                        diagnostic=record.diagnostic,
                    )
                )
        if tag_key is None and not record.declarators:
            occurrences.append(
                CDeclarationOccurrence(
                    **common,
                    key=DeclarationKey(scope, "ordinary", "", origin, ordinal),
                    kind=record.kind,
                    type_expression=record.canonical,
                    complete=False,
                    definition=None,
                    diagnostic=record.diagnostic or "unsupported declaration",
                )
            )
    return tuple(occurrences)


def _reject_occurrences(
    previous: CDeclarationOccurrence, incoming: CDeclarationOccurrence, reason: str
) -> None:
    raise ValueError(
        f"conflicting declarations for {incoming.key.name or '<anonymous>'} "
        f"({incoming.key.namespace}, scope={incoming.key.scope!r}): {reason}; "
        f"{previous.origin}:{previous.ordinal}: {previous.canonical} != "
        f"{incoming.origin}:{incoming.ordinal}: {incoming.canonical}"
    )


def _reject_alias_cycles(entities: Iterable[CDeclarationEntity]) -> None:
    aliases = {
        entity.key: entity
        for entity in entities
        if entity.key.namespace == "ordinary"
        and entity.kind == "typedef"
        and entity.alias_target is not None
    }
    resolved: set[DeclarationKey] = set()
    for key in aliases:
        positions: dict[DeclarationKey, int] = {}
        chain: list[CDeclarationEntity] = []
        while key in aliases and key not in resolved:
            if key in positions:
                cycle = chain[positions[key] :]
                evidence = " -> ".join(
                    f"{entity.key.name} (scope={entity.key.scope!r}) at "
                    f"{entity.representative.origin}:{entity.representative.ordinal}: "
                    f"{entity.canonical}"
                    for entity in cycle
                )
                raise ValueError(f"cyclic typedef aliases: {evidence} -> {key.name}")
            positions[key] = len(chain)
            entity = aliases[key]
            chain.append(entity)
            key = entity.alias_target
        resolved.update(positions)


def reconcile_declarations(
    occurrences: Iterable[CDeclarationOccurrence],
) -> tuple[CDeclarationEntity, ...]:
    """Merge supported evidence by exact identity, rejecting unresolved equivalence.

    Equivalent definitions are evidence, not permission to emit both definitions
    into one C translation unit. Diagnostics never establish compatibility.
    """

    grouped: dict[DeclarationKey, list[CDeclarationOccurrence]] = {}
    for occurrence in occurrences:
        grouped.setdefault(occurrence.key, []).append(occurrence)
    entities: list[CDeclarationEntity] = []
    for key, members in grouped.items():
        representative = members[0]
        definition = None
        for member in members:
            if member.kind != representative.kind:
                _reject_occurrences(representative, member, "kind mismatch")
            if member is not representative and (
                member.diagnostic or representative.diagnostic
            ):
                _reject_occurrences(
                    representative, member, "unsupported declaration equivalence"
                )
            if key.namespace == "ordinary" and (
                tokenize_declaration(member.type_expression)
                != tokenize_declaration(representative.type_expression)
                or member.alias_target != representative.alias_target
                or member.alias_kind != representative.alias_kind
            ):
                _reject_occurrences(representative, member, "type expression mismatch")
            if member.complete:
                if member.definition is None:
                    _reject_occurrences(member, member, "missing explicit definition")
                if definition and tokenize_declaration(
                    definition.definition or ""
                ) != tokenize_declaration(member.definition or ""):
                    _reject_occurrences(
                        definition, member, "complete definition mismatch"
                    )
                if definition is None:
                    definition = member
        if definition is not None:
            representative = definition
        entities.append(
            CDeclarationEntity(
                key=key,
                kind=representative.kind,
                canonical=representative.canonical,
                type_expression=representative.type_expression,
                complete=definition is not None,
                definition=definition.definition if definition else None,
                fields=definition.fields if definition else (),
                alias_target=representative.alias_target,
                alias_kind=representative.alias_kind,
                relationship=representative.relationship,
                representative=representative,
                selected_definition=definition,
                occurrences=tuple(members),
                diagnostic=representative.diagnostic,
            )
        )
    _reject_alias_cycles(entities)
    return tuple(entities)
