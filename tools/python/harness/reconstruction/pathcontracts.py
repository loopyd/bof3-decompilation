"""Replay diagnostic call paths without changing universal call contracts."""

from __future__ import annotations

from harness.domain.mips import word_fields
from harness.reconstruction import memorycontracts
from harness.reconstruction.proofvalues import (
    _complete,
    _integer,
    _keys,
    _references,
    _require,
    _rows,
)


REMAINING = sorted(
    (
        "conditional-entry-state-unresolved",
        "declaration-unresolved",
        "normal-return-target-unresolved",
        "original-memory-effects-unresolved",
        "stack-alias-restoration-unresolved",
    )
)


def _signed(value):
    return value - (1 << 32) if value & 0x80000000 else value


def _compute(fields, registers):
    op, fn = fields.opcode, fields.funct
    left, right = registers[fields.rs], registers[fields.rt]
    destination, value = None, None
    if op == 0:
        destination = fields.rd
        if fn in {0, 2, 3} and fields.rs == 0:
            if right is not None:
                value = (
                    right << fields.shift
                    if fn == 0
                    else right >> fields.shift
                    if fn == 2
                    else _signed(right) >> fields.shift
                )
        elif fn in {4, 6, 7} and fields.shift == 0:
            if left is not None and right is not None:
                shift = left & 31
                value = (
                    right << shift
                    if fn == 4
                    else right >> shift
                    if fn == 6
                    else _signed(right) >> shift
                )
        elif fn in {33, 35, 36, 37, 38, 39, 42, 43} and fields.shift == 0:
            if left is not None and right is not None:
                value = {
                    33: lambda: left + right,
                    35: lambda: left - right,
                    36: lambda: left & right,
                    37: lambda: left | right,
                    38: lambda: left ^ right,
                    39: lambda: ~(left | right),
                    42: lambda: int(_signed(left) < _signed(right)),
                    43: lambda: int(left < right),
                }[fn]()
        else:
            _require(False, "call path unsupported integer instruction")
    elif op in {9, 10, 11, 12, 13, 14}:
        destination = fields.rt
        if left is not None:
            immediate = fields.signed_immediate
            value = {
                9: lambda: left + immediate,
                10: lambda: int(_signed(left) < immediate),
                11: lambda: int(left < (immediate & 0xFFFFFFFF)),
                12: lambda: left & fields.immediate,
                13: lambda: left | fields.immediate,
                14: lambda: left ^ fields.immediate,
            }[op]()
    elif op == 15 and fields.rs == 0:
        destination, value = fields.rt, fields.immediate << 16
    else:
        _require(False, "call path unsupported instruction")
    return destination, None if value is None else value & 0xFFFFFFFF


def _seeds(path, call, caller):
    row, _, instructions, definitions = caller
    evidence = row["returnDisposition"]["evidence"]
    pc = call["pc"]
    memorycontracts._entry_flow(row, use_pc=pc)
    incoming = {}
    for edge in evidence["edges"]:
        incoming.setdefault(edge["to"], []).append(edge)
    ancestors, pending = set(), [pc]
    while pending:
        point = pending.pop()
        if point in ancestors:
            continue
        ancestors.add(point)
        _require(len(ancestors) <= 4096, "call path caller prefix bound")
        pending.extend(edge["from"] for edge in incoming.get(point, []))
    _require(
        not any(
            item["pc"] in ancestors - {pc}
            and item["domain"] == "architectural"
            and item["reason"].startswith("conditional-")
            for item in evidence["obligations"]
        ),
        "call path caller has incoming conditional state",
    )

    def reaches(point, register, references):
        pending, visited, found = [(point, False)], set(), set()
        while pending:
            point, include = pending.pop()
            if (point, include) in visited:
                continue
            visited.add((point, include))
            _require(len(visited) <= 4096, "call path seed predecessor bound")
            kind, destinations = memorycontracts._memory_word(
                instructions[point]["word"]
            )
            if include and register in destinations:
                matches = {
                    ref
                    for ref in references
                    if definitions[ref]["pc"] == point
                    and definitions[ref]["register"] == register
                }
                _require(
                    kind != "load" and matches,
                    "call path seed has another reaching writer",
                )
                found.update(matches)
                continue
            _require(
                not include or kind != "call" or point == pc,
                "call path seed crosses an unresolved call",
            )
            predecessors = incoming.get(point, [])
            _require(predecessors, "call path seed reads unknown caller entry")
            for edge in predecessors:
                _require(
                    edge["kind"] != "call-continuation",
                    "call path seed crosses a call continuation",
                )
                previous = edge["from"]
                previous_kind, previous_destinations = memorycontracts._memory_word(
                    instructions[previous]["word"]
                )
                if (
                    not include
                    and previous_kind == "load"
                    and register in previous_destinations
                ):
                    pending.extend(
                        (prior["from"], True) for prior in incoming.get(previous, [])
                    )
                    _require(
                        incoming.get(previous), "call path seed pending load at entry"
                    )
                else:
                    pending.append((previous, True))
        _require(found == set(references), "call path seed reaching definitions differ")

    active, recovered = set(), {}

    def derive(reference):
        if reference in recovered:
            return recovered[reference]
        _require(
            reference in definitions and reference not in active,
            "call path seed definition cycle or absence",
        )
        active.add(reference)
        definition = definitions[reference]
        point, register = definition["pc"], definition["register"]
        _require(
            definition["width"] == 32
            and not definition["delayed"]
            and not definition["bad"]
            and not definition["formals"]
            and not definition["entries"]
            and definition["equalEntry"] is None
            and definition["origin"] == {"kind": "instruction"},
            "call path seed definition is not a clean original constant",
        )
        fields = word_fields(instructions[point]["word"])
        sources = []
        if fields.opcode == 3 and register == 31:
            value, equality = (point + 8) & 0xFFFFFFFF, {point}
        else:
            source_registers = (
                ()
                if fields.opcode == 15
                else (fields.rs, fields.rt)
                if fields.opcode == 0
                else (fields.rs,)
            )
            registers, provenance = [None] * 32, {0: set()}
            registers[0] = 0
            for source in set(source_registers) - {0}:
                refs = [
                    ref
                    for ref in definition["inputs"]
                    if definitions[ref]["register"] == source
                ]
                _require(refs, "call path seed source definition missing")
                reaches(point, source, refs)
                values = [derive(ref) for ref in refs]
                _require(
                    len({value for value, _ in values}) == 1,
                    "call path seed constants disagree",
                )
                registers[source] = values[0][0]
                provenance[source] = set().union(*(pcs for _, pcs in values))
                sources.extend(refs)
            destination, value = _compute(fields, registers)
            _require(
                destination == register and value is not None,
                "call path seed original value unresolved",
            )
            copy_source = None
            if fields.opcode == 0:
                if fields.funct in {0, 2, 3} and fields.shift == 0:
                    copy_source = fields.rt
                elif fields.funct in {33, 37, 38}:
                    copy_source = (
                        fields.rs
                        if registers[fields.rt] == 0
                        else fields.rt
                        if registers[fields.rs] == 0
                        else None
                    )
                elif fields.funct == 35 and registers[fields.rt] == 0:
                    copy_source = fields.rs
            elif fields.opcode in {9, 13, 14} and fields.immediate == 0:
                copy_source = fields.rs
            _require(
                fields.opcode in {9, 10, 11, 12, 13, 14, 15} or copy_source is not None,
                "call path seed unsupported native constant definition",
            )
            equality = {point} | (
                provenance[copy_source] if copy_source is not None else set()
            )
        _require(
            definition["inputs"] == sorted(set(sources))
            and definition["constant"] == value
            and definition["equalityEvidence"] == sorted(equality),
            "call path seed value or provenance differs from original words",
        )
        active.remove(reference)
        recovered[reference] = value, equality
        return value, equality

    bindings = _rows(path["bindings"], 31, "call path bindings")
    registers, previous = [None] * 32, 0
    registers[0] = 0
    delay = word_fields(instructions[pc + 4]["word"])
    delay_kind, delay_destinations = memorycontracts._memory_word(delay.word)
    _require(
        delay_kind
        not in {"branch", "call", "return", "indirect-branch", "unsupported", "trap"},
        "call path caller delay unsupported",
    )
    _require(delay_kind != "load" or delay.rt == 0, "call path pending caller load")
    for binding in bindings:
        _keys(
            binding,
            {"register", "constant", "definitions", "equalityEvidence"},
            "call path binding",
        )
        register = binding["register"]
        _integer(register, 31)
        _integer(binding["constant"], 0xFFFFFFFF)
        _references(binding["equalityEvidence"], 0xFFFFFFFF)
        _require(register > previous, "call path binding order or duplicate")
        previous = register
        references = _references(binding["definitions"], 4096, 1)
        _require(
            references and all(ref in definitions for ref in references),
            "call path binding definitions absent",
        )
        _require(
            all(definitions[ref]["register"] == register for ref in references),
            "call path binding definition register differs",
        )
        if register in delay_destinations or register == 31:
            point = pc + 4 if register in delay_destinations else pc
            _require(
                all(definitions[ref]["pc"] == point for ref in references),
                "call path seed ignores JAL or delay write",
            )
        else:
            reaches(pc + 4, register, references)
        values = [derive(ref) for ref in references]
        _require(
            all(value == binding["constant"] for value, _ in values)
            and binding["equalityEvidence"]
            == sorted(set().union(*(pcs for _, pcs in values))),
            "call path binding constant or provenance differs",
        )
        registers[register] = binding["constant"]
    return registers


def validate_entry_path(call, caller, graph, *, context, entry_observation):
    path = _keys(
        call["entryPath"],
        {
            "kind",
            "callee",
            "contextHash",
            "bindings",
            "instructions",
            "edges",
            "branches",
            "terminalPc",
            "remaining",
        },
        "bound call path",
    )
    contract = call["contract"]
    _integer(path["terminalPc"], 0xFFFFFFFF)
    for row in _rows(path["instructions"], 256, "call path instructions"):
        _keys(row, {"pc", "word"}, "call path instruction")
        _integer(row["pc"], 0xFFFFFFFF)
        _integer(row["word"], 0xFFFFFFFF)
    for row in _rows(path["edges"], 256, "call path edges"):
        _keys(row, {"from", "to", "delayOwner", "kind"}, "call path edge")
        for field in ("from", "to"):
            _integer(row[field], 0xFFFFFFFF)
        _integer(row["delayOwner"], 0xFFFFFFFF, optional=True)
        _require(type(row["kind"]) is str, "call path edge kind")
    for row in _rows(path["branches"], 256, "call path branches"):
        _keys(row, {"pc", "taken", "operands"}, "call path branch")
        _integer(row["pc"], 0xFFFFFFFF)
        _require(type(row["taken"]) is bool, "call path branch decision")
        for operand in _rows(row["operands"], 2, "call path branch operands"):
            _keys(operand, {"register", "constant"}, "call path branch operand")
            _integer(operand["register"], 31)
            _integer(operand["constant"], 0xFFFFFFFF)
    caller_row = caller[0]
    _require(
        context is not None
        and not context.get("instruction_scope_sha256")
        and entry_observation is not True
        and "entryContext" not in caller[1]
        and path["kind"] == "mips-bound-call-path/v1"
        and path["contextHash"] == context["identity_sha256"]
        and path["callee"] == contract["callee"]
        and path["callee"] in graph
        and contract["kind"] == "partial"
        and call["qualified"] is False
        and caller_row["returnDisposition"]["kind"] == "unknown"
        and not _complete(caller[1]["declaration"])
        and path["remaining"] == REMAINING,
        "call path is not an unresolved universal-context diagnostic",
    )
    word = caller[2][call["pc"]]["word"]
    callee, summary, instructions, _ = graph[path["callee"]]
    evidence = callee["returnDisposition"]["evidence"]
    _require(
        word >> 26 == 3
        and call["target"] == evidence["start"]
        and "entryContext" not in summary
        and evidence["analysisComplete"] is True
        and 0 < len(instructions) <= 256
        and evidence["start"] < evidence["end"] <= evidence["start"] + 1024,
        "call path lacks a bounded original direct callee",
    )
    registers = _seeds(path, call, caller)
    words, edges, branches, visited = [], [], [], set()
    point, transfer, pending = evidence["start"], None, None
    terminal = None
    while True:
        _require(
            point in instructions and point not in visited and len(visited) < 256,
            "call path missing instruction or loop",
        )
        visited.add(point)
        instruction = instructions[point]
        _require(
            instruction["mapped"] is True
            and instruction["undef"] is False
            and instruction["bytes"] == 4
            and instruction["owner"] == callee["name"],
            "call path original instruction ownership differs",
        )
        fields = word_fields(instruction["word"])
        words.append({"pc": point, "word": fields.word})
        op = fields.opcode
        branch = op in {2, 4, 5, 6, 7} or (op == 1 and fields.rt in {0, 1})
        returning = fields.word == 0x03E00008
        destination = value = load = None
        outgoing = None
        if branch or returning:
            _require(transfer is None, "call path control transfer in delay slot")
            if returning:
                terminal = point
                outgoing = (None, "return")
            elif op == 2:
                outgoing = (
                    (((point + 4) & 0xF0000000) | (fields.target26 << 2)),
                    "branch",
                )
            else:
                _require(
                    op not in {6, 7} or fields.rt == 0,
                    "call path malformed signed branch",
                )
                operands = [fields.rs, fields.rt] if op in {4, 5} else [fields.rs]
                _require(
                    all(registers[reg] is not None for reg in operands),
                    "call path unknown branch operand",
                )
                left = registers[fields.rs]
                taken = (
                    left == registers[fields.rt]
                    if op == 4
                    else left != registers[fields.rt]
                    if op == 5
                    else _signed(left) <= 0
                    if op == 6
                    else _signed(left) > 0
                    if op == 7
                    else _signed(left) < 0
                    if fields.rt == 0
                    else _signed(left) >= 0
                )
                branches.append(
                    dict(
                        pc=point,
                        taken=taken,
                        operands=[
                            dict(register=reg, constant=registers[reg])
                            for reg in operands
                        ],
                    )
                )
                target = point + 4 + fields.signed_immediate * 4
                _require(
                    0 <= target <= 0xFFFFFFFF,
                    "call path branch target exceeds address space",
                )
                outgoing = (
                    target if taken else point + 8,
                    "branch" if taken else "fallthrough",
                )
        elif op in {32, 33, 35, 36, 37}:
            load = fields.rt or None
        elif op in {40, 41, 43}:
            pass
        else:
            destination, value = _compute(fields, registers)
        if pending is not None:
            registers[pending] = None
        if destination:
            registers[destination] = value
        pending = load
        registers[0] = 0
        if transfer is not None:
            target, kind = transfer
            if target is None:
                _require(pending is None, "call path unsettled terminal load")
                break
            edges.append(
                dict(**{"from": point, "to": target}, delayOwner=None, kind=kind)
            )
            point, transfer = target, None
        elif outgoing is not None:
            edges.append(
                dict(
                    **{"from": point, "to": point + 4},
                    delayOwner=point,
                    kind="delay-slot",
                )
            )
            point, transfer = point + 4, outgoing
        else:
            edges.append(
                dict(
                    **{"from": point, "to": point + 4},
                    delayOwner=None,
                    kind="fallthrough",
                )
            )
            point += 4
    _require(
        path["instructions"] == words
        and path["edges"] == edges
        and path["branches"] == branches
        and path["terminalPc"] == terminal,
        "call path replay differs from original instructions and actual bindings",
    )
