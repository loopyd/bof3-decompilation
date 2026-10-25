from __future__ import annotations

from collections import deque

from harness.domain.mips import word_fields
from harness.reconstruction.proofvalues import (
    CallContractRefused,
    _complete,
    _integer,
    _keys,
    _references,
    _require,
    _rows,
)


DIMENSIONS = set(
    "objectExtent initializedObject zeroInitializedObject constQualification "
    "volatileQualification restrictQualification currentContents aliasEffects placement".split()
)

FRAME_ADMISSION_KIND = "observed-frame-call-admission/v1"
FRAME_ADMISSION_CONDITIONS = [
    "captured-entry-registers-and-settled-load-state",
    "ordinary-ram-mapping-and-cache-mode-remain-active",
    "original-instructions-remain-active",
    "accesses-and-instruction-fetches-complete-normally",
    "no-asynchronous-or-external-state-interference",
    "mips-i-load-and-branch-delays",
    "each-crossed-call-returns-to-its-recorded-continuation",
]
FRAME_ADMISSION_CHECKS = [
    "call-boundary-settled-and-direct",
    "caller-and-callee-instructions-complete",
    "closed-reachable-call-inventory",
    "callee-has-normal-exit",
    "outgoing-interval-complete",
    "reachable-writes-disjoint-from-protected-words",
    "protected-addresses-not-exposed",
    "ordinary-ram-aliases-normalized",
]
# Whole four-byte saves of entry gp, fp, s0-s7 or the incoming link are the only
# protected words; saving the stack pointer or an argument is not an admitted save.
FRAME_ENTRY_REGISTERS = frozenset({28, 30, 31}) | frozenset(range(16, 24))


def _memory_word(word):
    fields = word_fields(word)
    opcode, rs, rt = fields.opcode, fields.rs, fields.rt
    rd, fn, shift = fields.rd, fields.funct, fields.shift
    if opcode == 0:
        if fn == 8 and rt == 0 and rd == 0 and shift == 0:
            return ("return" if rs == 31 else "indirect-branch"), ()
        if fn == 9:
            return "call", ()
        if fn in {0, 2, 3, 4, 6, 7, 32, 33, 34, 35, 36, 37, 38, 39, 42, 43}:
            return "compute", (rd,)
        if fn in {24, 25, 26, 27} and rd == 0 and shift == 0:
            return "compute", (32, 33)
        if fn == 13:
            return "trap", ()
        if fn in {16, 18} and rs == 0 and rt == 0 and shift == 0:
            return "compute", (rd,)
    elif opcode == 3:
        return "call", (31,)
    elif opcode in {2, 4, 5, 6, 7} or (opcode == 1 and rt in {0, 1}):
        return "branch", ()
    elif 8 <= opcode <= 15:
        return "compute", (rt,)
    elif opcode in {32, 33, 34, 35, 36, 37, 38}:
        return "load", (rt,)
    elif opcode in {40, 41, 42, 43, 46}:
        return "store", ()
    return "unsupported", ()


def _signed16(word):
    immediate = word & 0xFFFF
    return immediate - 0x10000 if immediate & 0x8000 else immediate


def _observed_cfg(row):
    """Reconstruct all reachable original edges, including delay-slot ownership."""
    evidence = row["returnDisposition"]["evidence"]
    instructions = {item["pc"]: item for item in evidence["instructions"]}
    _require(
        evidence["decodeComplete"]
        and evidence["analysisComplete"]
        and len(instructions) == len(evidence["instructions"])
        and set(instructions) == set(range(evidence["start"], evidence["end"], 4)),
        "observed restoration requires complete original words",
    )
    for item in instructions.values():
        _require(
            item["mapped"]
            and not item["undef"]
            and item["bytes"] == 4
            and item["owner"] == row["name"],
            "observed restoration original ownership",
        )
    pending = [(evidence["start"], None)]
    contexts, successors, derived, exits = {}, {}, set(), set()
    while pending:
        pc, owner = pending.pop()
        if pc in contexts:
            _require(
                contexts[pc] == owner, "observed restoration ambiguous delay owner"
            )
            continue
        _require(
            pc in instructions and len(contexts) < 4096, "observed original CFG bounds"
        )
        contexts[pc] = owner
        word = instructions[pc]["word"]
        kind, _ = _memory_word(word)
        _require(
            kind not in {"unsupported", "trap", "indirect-branch"}
            and (kind != "call" or word >> 26 == 3),
            "observed restoration unsupported control",
        )
        following = set()
        if owner is None:
            following.add(
                (pc + 4, pc, "delay-slot")
                if kind in {"branch", "return", "call"}
                else (pc + 4, None, "fallthrough")
            )
        else:
            branch = instructions[owner]["word"]
            owner_kind, _ = _memory_word(branch)
            _require(
                pc == owner + 4 and kind not in {"branch", "return", "call"},
                "observed restoration illegal delay slot",
            )
            if owner_kind == "branch":
                target = (
                    (((owner + 4) & 0xF0000000) | ((branch & 0x03FFFFFF) << 2))
                    if branch >> 26 == 2
                    else (owner + 4 + _signed16(branch) * 4) & 0xFFFFFFFF
                )
                following.add((target, None, "branch"))
                if branch >> 26 != 2:
                    following.add((owner + 8, None, "fallthrough"))
            elif owner_kind == "call":
                following.add((owner + 8, None, "call-continuation"))
            else:
                _require(branch == 0x03E00008, "observed restoration unproved return")
                exits.add(owner)
        successors[pc] = [target for target, _, _ in following]
        for target, delay_owner, edge_kind in following:
            derived.add((pc, target, delay_owner, edge_kind))
            pending.append((target, delay_owner))
    reported = {
        (edge["from"], edge["to"], edge["delayOwner"], edge["kind"])
        for edge in evidence["edges"]
    }
    _require(
        len(reported) == len(evidence["edges"])
        and reported == derived
        and exits == {item["pc"] for item in evidence["exits"]}
        and exits,
        "observed restoration complete CFG inventory differs",
    )
    incoming = {pc: [] for pc in contexts}
    for pc, targets in successors.items():
        for target in targets:
            incoming[target].append(pc)
    degrees = {pc: len(parents) for pc, parents in incoming.items()}
    ready = deque(pc for pc, count in degrees.items() if count == 0)
    _require(list(ready) == [evidence["start"]], "observed restoration entry cycle")
    ordered = []
    while ready:
        pc = ready.popleft()
        ordered.append(pc)
        for target in successors[pc]:
            degrees[target] -= 1
            if not degrees[target]:
                ready.append(target)
    _require(len(ordered) == len(contexts), "observed restoration cyclic original CFG")
    return instructions, contexts, incoming, ordered, exits


def _join_observed_value(left, right):
    if left == right:
        return left
    if left and right and left[0] == right[0] == "saved" and left[:4] == right[:4]:
        return (*left[:4], left[4] | right[4], left[5] | right[5])
    return None


def _observed_restoration(row, proof, admitted=frozenset()):
    """Replay a callee-local captured frame without trusting claimed address facts."""
    instructions, contexts, incoming, ordered, exits = _observed_cfg(row)
    evidence = row["returnDisposition"]["evidence"]
    bindings = {item["register"]: item["value"] for item in proof["entryBindings"]}
    calls = {item["pc"]: item for item in evidence["calls"]}
    frame, sp = proof["frameBytes"], proof["entrySp"]
    states, terminal = {}, []
    protected = {}

    def physical(address, width):
        if not 0 <= address <= 0xFFFFFFFF or address & 0xE0000000 not in {
            0,
            0x80000000,
            0xA0000000,
        }:
            return None
        address &= 0x1FFFFFFF
        return address if address + width <= 0x200000 else None

    def joined(parents):
        registers, pending, words, used, link = parents[0]
        registers, pending, words, used = (
            registers.copy(),
            pending.copy(),
            words.copy(),
            used.copy(),
        )
        for (
            other_registers,
            other_pending,
            other_words,
            other_used,
            other_link,
        ) in parents[1:]:
            _require(
                pending.keys() == other_pending.keys(), "observed pending-load join"
            )
            registers = [
                _join_observed_value(left, right)
                for left, right in zip(registers, other_registers, strict=True)
            ]
            pending = {
                reg: _join_observed_value(value, other_pending[reg])
                for reg, value in pending.items()
            }
            words = {
                address: value
                for address, saved in words.items()
                if (value := _join_observed_value(saved, other_words.get(address)))
            }
            used.update(other_used)
            link = _join_observed_value(link, other_link)
        _require(registers[29] is not None, "observed stack join")
        return registers, pending, words, used, link

    def entry(value):
        return value[1] if value and value[0] in {"entry", "saved"} else None

    for pc in ordered:
        if incoming[pc]:
            registers, pending, words, used, link = joined(
                [states[parent] for parent in incoming[pc]]
            )
        else:
            registers = [("entry", reg) for reg in range(34)]
            registers[0], registers[29] = ("constant", 0), ("sp", 0)
            pending, words, used, link = {}, {}, set(), None
        word = instructions[pc]["word"]
        fields = word_fields(word)
        opcode, rs, rt = fields.opcode, fields.rs, fields.rt
        kind, destinations = _memory_word(word)
        immediate = _signed16(word)
        memory = kind in {"load", "store"}
        adjustment = opcode == 9 and rs == rt == 29
        computed = None
        loaded = None
        if adjustment:
            _require(
                registers[29] and registers[29][0] == "sp", "observed stack identity"
            )
            offset = registers[29][1]
            _require(
                (pc == evidence["start"] and offset == 0 and immediate == -frame)
                or (offset == -frame and immediate == frame),
                "observed stack adjustment",
            )
            computed = ("sp", offset + immediate)
        elif kind == "compute":
            left, right = registers[rs], registers[rt]
            if opcode == 0 and fields.funct in {33, 37} and (rs == 0 or rt == 0):
                computed = registers[rs or rt]
            elif opcode == 9 and immediate == 0:
                computed = left
            elif opcode == 15:
                computed = ("constant", (word & 0xFFFF) << 16)
            elif left and left[0] == "constant" and opcode in {9, 13}:
                computed = (
                    "constant",
                    (
                        (left[1] + immediate)
                        if opcode == 9
                        else (left[1] | (word & 0xFFFF))
                    )
                    & 0xFFFFFFFF,
                )
            elif (
                opcode == 0
                and fields.funct in {33, 37}
                and left
                and right
                and left[0] == right[0] == "constant"
            ):
                computed = (
                    "constant",
                    (
                        (left[1] + right[1])
                        if fields.funct == 33
                        else (left[1] | right[1])
                    )
                    & 0xFFFFFFFF,
                )
        elif kind == "call":
            computed = ("constant", pc + 8)
        if memory:
            _require(
                opcode not in {34, 38, 42, 46}, "observed merge access unsupported"
            )
            width = 1 if opcode in {32, 36, 40} else 2 if opcode in {33, 37, 41} else 4
            base = registers[rs]
            offset, address = None, None
            if base and base[0] == "sp":
                offset = base[1] + immediate
                address = physical(sp + offset, width)
            elif base and base[0] == "constant":
                address = physical(base[1] + immediate, width)
            elif (reg := entry(base)) is not None:
                if reg in bindings:
                    address = physical(bindings[reg] + immediate, width)
                if address is not None:
                    used.add(reg)
            if kind == "store":
                _require(address is not None, "observed store address unresolved")
                words = {
                    location: saved
                    for location, saved in words.items()
                    if not (address < location + 4 and location < address + width)
                }
                saved_entry = entry(registers[rt])
                if (
                    opcode == 43
                    and offset is not None
                    and -frame <= offset <= -4
                    and address % 4 == 0
                    and saved_entry is not None
                ):
                    words[address] = (
                        "saved",
                        saved_entry,
                        offset,
                        pc,
                        frozenset(),
                        frozenset(),
                    )
            elif (
                opcode == 35
                and address is not None
                and offset is not None
                and address % 4 == 0
            ):
                saved = words.get(address)
                if saved and saved[2] == offset:
                    loaded = (*saved[:4], saved[4] | {pc}, saved[5])
        if kind == "return":
            _require(
                word == 0x03E00008 and 31 not in pending, "observed return pending link"
            )
            link = registers[31]
        immediate_destinations = set(destinations) if kind != "load" else set()
        for reg, value in pending.items():
            if reg not in immediate_destinations:
                registers[reg] = value
        pending = {}
        for reg in destinations:
            if not reg:
                continue
            _require(
                instructions[pc]["fullWidthRegisters"].count(reg) == 1
                and (reg != 29 or adjustment),
                "observed register write width or stack escape",
            )
            if kind == "load":
                pending[reg] = loaded
            else:
                registers[reg] = computed
        owner = contexts[pc]
        if owner is not None and _memory_word(instructions[owner]["word"])[0] == "call":
            call = calls.get(owner)
            _require(call is not None and not pending, "observed call pending load")
            contract = call["contract"]
            proven = _complete(contract["noMemoryWrites"])
            if not proven:
                # Only the approved preserved words may survive an unproved call; every other
                # saved value loses its relation here, exactly as the producer does.
                words = {
                    address: saved
                    for address, saved in words.items()
                    if saved[1] in FRAME_ENTRY_REGISTERS and saved[2] <= -4
                }
                registers = [
                    value
                    if not (
                        value is not None
                        and value[0] == "saved"
                        and (value[1] not in FRAME_ENTRY_REGISTERS or value[2] > -4)
                    )
                    else None
                    for value in registers
                ]
            crossing = [
                dict(
                    physicalWord=address,
                    entrySpOffset=saved[2],
                    register=saved[1],
                    storePc=saved[3],
                )
                for address, saved in sorted(words.items())
            ]
            # Only a call that crosses a preserved word needs the continuation condition; an
            # unproved call that crosses none retains no saved-memory fact and needs neither a
            # write-absence proof nor an admission record.
            conditional_call = bool(crossing) and not proven
            _require(
                conditional_call == (owner in admitted),
                "observed call carries a stale or unneeded admission",
            )
            _require(
                call["target"]
                == (
                    ((owner + 4) & 0xF0000000)
                    | ((instructions[owner]["word"] & 0x03FFFFFF) << 2)
                )
                and contract["callee"]
                and contract["delay"] == pc
                and contract["continuation"] == owner + 8
                and contract["actualIncomingLink"] == owner + 8
                and _complete(contract["identity"])
                and _complete(contract["inputs"])
                and len(contract["registerTransfers"]) == 34,
                "observed call lacks settled call binding or transfers",
            )
            if proven:
                _require(
                    all(
                        _complete(contract[dimension])
                        for dimension in ("control", "transfers", "noMemoryWrites")
                    ),
                    "proved observed call lacks complete control or transfers",
                )
            for reg, transfer in enumerate(contract["registerTransfers"]):
                settled = (
                    transfer["register"] == reg
                    and _complete(transfer["proof"])
                    and not transfer["pending"]
                )
                preserved = (
                    settled
                    and transfer["kind"] == "preserved"
                    and transfer["equalEntry"] == reg
                )
                exposes_stack = (
                    settled
                    and not preserved
                    and (transfer["equalEntry"] == 29 or 29 in transfer["entries"])
                )
                if settled and reg == 29 and not preserved:
                    raise CallContractRefused(
                        "observed call proves a contrary stack transfer"
                    )
                _require(
                    not exposes_stack,
                    "observed call proves a stack escape",
                )
                if proven:
                    _require(
                        settled
                        and transfer["mayNoPending"]
                        and not transfer["pendingDefinitions"],
                        "observed call unsettled transfer",
                    )
                if not preserved:
                    # Unsettled or non-preserving transfers keep no relation. The stack pointer
                    # relation is retained separately as the conditional frame base.
                    if reg != 29:
                        registers[reg] = None
            if conditional_call:
                protected[owner] = crossing
            words = {
                address: (*saved[:5], saved[5] | {owner})
                for address, saved in words.items()
            }
        state = registers, pending, words, used, link
        states[pc] = state
        if owner in exits:
            _require(
                registers[29] == ("sp", 0)
                and link
                and link[0] == "saved"
                and link[1] == 31
                and link[4]
                and not pending,
                "observed exit lacks settled original stack/link restoration",
            )
            terminal.append(state)
    registers, pending, _, used, _ = joined(terminal)
    restored = []
    for reg, value in enumerate(registers[:32]):
        if (
            reg
            and value
            and value[0] == "saved"
            and value[1] == reg
            and value[4]
            and reg not in pending
        ):
            restored.append(
                dict(
                    register=reg,
                    entrySpOffset=value[2],
                    physicalWord=physical(sp + value[2], 4),
                    storePc=value[3],
                    loadPcs=sorted(value[4]),
                    callPcs=sorted(value[5]),
                    exitPcs=sorted(exits),
                )
            )
    _require(used == bindings.keys(), "observed entry binding inventory differs")
    _require(
        restored == proof["registers"],
        "observed restoration differs from original replay",
    )
    _require(admitted == set(protected), "admitted call inventory differs")
    return {"registers": restored, "protected": protected}


def _admission_physical(address, width):
    if type(address) is not int or not 0 <= address <= 0xFFFFFFFF:
        return None
    if address & 0xE0000000 not in {0, 0x80000000, 0xA0000000}:
        return None
    physical = address & 0x1FFFFFFF
    return physical if width > 0 and physical + width <= 0x200000 else None


def _admission_offset(base, add):
    if base is None:
        return None
    if base[0] == "stack":
        return ("stack", base[1] + add)
    if base[0] == "entry":
        return ("entry", base[1], base[2] + add)
    if base[0] == "constant":
        return ("constant", (base[1] + add) & 0xFFFFFFFF)
    return None


def _admission_step(pc, word, state):
    """Register effects of one census word; unsupported effects refuse admission."""
    fields = word_fields(word)
    opcode, rs, rt, rd = fields.opcode, fields.rs, fields.rt, fields.rd
    fn, shift = fields.funct, fields.shift
    immediate = _signed16(word)
    kind, _ = _memory_word(word)
    _require(
        opcode not in {34, 38, 42, 46}
        and kind not in {"unsupported", "trap", "indirect-branch"}
        and not (opcode == 0 and fn == 9)
        and not (opcode == 0 and fn == 8 and rs != 31),
        "admission census unsupported original word",
    )

    def assign(reg, value):
        if reg:
            state[reg] = value

    if opcode == 3:
        assign(31, ("constant", (pc + 8) & 0xFFFFFFFF))
        return
    if opcode == 0:
        if fn == 8:
            return
        if fn == 0 and shift == 0:
            assign(rd, state[rt])
            return
        if fn in {33, 37}:
            value = None
            if rs == 0:
                value = state[rt]
            elif rt == 0:
                value = state[rs]
            elif (
                state[rs] is not None
                and state[rt] is not None
                and state[rs][0] == state[rt][0] == "constant"
            ):
                left, right = state[rs][1], state[rt][1]
                value = (
                    "constant",
                    ((left + right) & 0xFFFFFFFF) if fn == 33 else left | right,
                )
            assign(rd, value)
            return
        assign(rd, None)
        return
    if opcode == 15:
        assign(rt, ("constant", (word & 0xFFFF) << 16))
        return
    if opcode in {8, 9}:
        assign(rt, _admission_offset(state[rs], immediate))
        return
    if opcode == 13:
        value = None
        if state[rs] is not None and state[rs][0] == "constant":
            value = ("constant", state[rs][1] | (word & 0xFFFF))
        assign(rt, value)
        return
    if 8 <= opcode <= 14:
        assign(rt, None)
        return
    if kind == "load":
        assign(rt, None)


def _admission_join(states):
    joined = list(states[0])
    for other in states[1:]:
        joined = [
            left if left == right else None
            for left, right in zip(joined, other, strict=True)
        ]
    return joined


def _admission_preserved(evidence):
    """Registers whose call-transfer proof is a settled preservation, per call site."""
    preserved = {}
    for call in evidence["calls"]:
        mask = 0
        contract = call.get("contract")
        if (
            type(contract) is dict
            and len(contract.get("registerTransfers") or ()) == 34
        ):
            for reg, transfer in enumerate(contract["registerTransfers"]):
                if (
                    1 <= reg < 32
                    and transfer["register"] == reg
                    and _complete(transfer["proof"])
                    and transfer["kind"] == "preserved"
                    and not transfer["pending"]
                    and transfer["mayNoPending"]
                    and transfer["equalEntry"] == reg
                ):
                    mask |= 1 << reg
        preserved[call["pc"]] = mask
    return preserved


def _admission_replay(
    row, entry_sp_offset, call_sp_physical, entry, identities, captured
):
    """Replay one census function from original words and its inherited registers."""
    instructions, contexts, incoming, ordered, exits = _observed_cfg(row)
    evidence = row["returnDisposition"]["evidence"]
    _require(bool(exits), "admission census callee normal exit")
    preserved = _admission_preserved(evidence)
    states, posts, entries = {}, {}, {}
    for pc in ordered:
        owner = contexts[pc]
        parents = incoming[pc]
        pre = (
            list(entry)
            if not parents
            else _admission_join([posts[parent] for parent in parents])
        )
        post = list(pre)
        _admission_step(pc, instructions[pc]["word"], post)
        if owner is not None and _memory_word(instructions[owner]["word"])[0] == "call":
            _require(
                instructions[owner]["word"] >> 26 == 3,
                "admission census indirect call",
            )
            _require(
                post[29] is not None and post[29][0] == "stack",
                "admission census call frame base",
            )
            _require(
                _memory_word(instructions[pc]["word"])[0] != "load",
                "admission census call delay load",
            )
            # The callee receives the registers actually present after the delay slot;
            # only the caller continuation sees the return-side clobbers.
            entries[pc] = list(post)
            entries[pc][31] = ("constant", (owner + 8) & 0xFFFFFFFF)
            post[31] = ("constant", (owner + 8) & 0xFFFFFFFF)
            # A register survives the call only when the callee proves it preserved;
            # every other caller relation is invalidated, as the producer does.
            kept = preserved.get(owner, 0)
            for reg in range(1, 32):
                if reg not in {29, 31} and not kept & (1 << reg):
                    post[reg] = None
        states[pc] = pre
        posts[pc] = post
    _require(len(states) == len(instructions), "admission census reachable inventory")
    rows, calls = [], []
    for pc in ordered:
        pre = states[pc]
        word = instructions[pc]["word"]
        kind, _ = _memory_word(word)
        fields = word_fields(word)
        if kind in {"load", "store"}:
            _require(
                pre[fields.rs] is not None,
                "admission census unresolved access base",
            )
            base = pre[fields.rs]
            immediate = _signed16(word)
            opcode = fields.opcode
            width = 1 if opcode in {32, 36, 40} else 2 if opcode in {33, 37, 41} else 4
            access = {"pc": pc, "write": kind == "store", "bytes": width}
            if base[0] == "stack":
                access["stack"] = True
                access["offset"] = entry_sp_offset + base[1] + immediate
                # Every supported derivation resolves into the call-time frame coordinate,
                # so a constant or entry-derived alias of the frame still contributes.
                access["frameOffset"] = access["offset"]
            elif base[0] == "constant":
                physical = _admission_physical(base[1] + immediate, width)
                _require(physical is not None, "admission census constant address")
                access["stack"] = False
                access["physical"] = physical
                access["frameOffset"] = physical - call_sp_physical
            else:
                _require(base[1] in captured, "admission census unbound entry address")
                physical = _admission_physical(
                    captured[base[1]] + base[2] + immediate, width
                )
                _require(physical is not None, "admission census entry address")
                access["stack"] = False
                access["physical"] = physical
                access["frameOffset"] = physical - call_sp_physical
            if access["write"]:
                value = pre[fields.rt]
                access["valueKnown"] = False
                access["valueFrameOffset"] = 0
                if value is not None and value[0] == "stack":
                    access["valueKnown"] = True
                    access["valueFrameOffset"] = entry_sp_offset + value[1]
                elif value is not None and value[0] == "constant":
                    resolved = _admission_physical(value[1], 1)
                    if resolved is not None:
                        access["valueKnown"] = True
                        access["valueFrameOffset"] = resolved - call_sp_physical
                elif value is not None and value[0] == "entry":
                    if value[1] in captured:
                        resolved = _admission_physical(captured[value[1]] + value[2], 1)
                        if resolved is not None:
                            access["valueKnown"] = True
                            access["valueFrameOffset"] = resolved - call_sp_physical
            rows.append(access)
        owner = contexts[pc]
        if owner is not None and _memory_word(instructions[owner]["word"])[0] == "call":
            owner_word = instructions[owner]["word"]
            target = ((owner + 4) & 0xF0000000) | ((owner_word & 0x03FFFFFF) << 2)
            sp_offset = entries[pc][29][1]
            calls.append(
                {
                    "pc": owner,
                    "target": target,
                    "callee": identities.get(target),
                    "entrySpOffset": entry_sp_offset + sp_offset,
                    "entry": [
                        ("stack", value[1] - sp_offset)
                        if value is not None and value[0] == "stack"
                        else value
                        for value in entries[pc]
                    ],
                }
            )
    rows.sort(key=lambda item: item["pc"])
    return {
        "function": None,
        "start": evidence["start"],
        "end": evidence["end"],
        "entrySpOffset": entry_sp_offset,
        "instructions": len(instructions),
        "calls": len(calls),
        "rows": rows,
    }, calls


def frame_admission_census(row, call, closure, *, frame, entry_sp, captured, protected):
    """Re-derive one crossed call's census from original words and authenticated inputs."""
    evidence = row["returnDisposition"]["evidence"]
    _require(
        evidence["decodeComplete"] and evidence["analysisComplete"],
        "admission census caller coverage",
    )
    _require(
        0 < frame <= 32768 and frame % 4 == 0 and 0 <= entry_sp - frame <= 0xFFFFFFFF,
        "admission census frame base",
    )
    instructions = {item["pc"]: item for item in evidence["instructions"]}
    _require(
        call["pc"] in instructions and call["pc"] + 4 in instructions,
        "admission census call words",
    )
    word = instructions[call["pc"]]["word"]
    _require(word >> 26 == 3, "admission census direct call")
    target = ((call["pc"] + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2)
    _require(
        call["target"] == target
        and _memory_word(instructions[call["pc"] + 4]["word"])[0] != "load",
        "admission census settled direct call",
    )
    identities = {start: key for start, (key, _) in closure.items()}
    call_sp_physical = (entry_sp - frame) & 0x1FFFFFFF
    entry = [None] * 34
    entry[0] = ("constant", 0)
    entry[29] = ("stack", 0)
    for reg in range(1, 32):
        if reg != 29:
            entry[reg] = ("entry", reg, 0)
    caller, caller_calls = _admission_replay(
        row, frame, call_sp_physical, entry, identities, captured
    )
    crossed = [item for item in caller_calls if item["pc"] == call["pc"]]
    _require(
        len(crossed) == 1
        and crossed[0]["target"] == target
        and crossed[0]["callee"] is not None
        and crossed[0]["entrySpOffset"] == 0,
        "admission census crossed call binding",
    )
    crossed = crossed[0]
    edges = {}
    pending = [crossed["target"]]
    seen = set()
    while pending:
        start = pending.pop()
        if start in seen:
            continue
        _require(start in closure, "admission census unresolved callee")
        _require(len(seen) < 32, "admission census closure bound")
        seen.add(start)
        member_row = closure[start][1]
        member_evidence = member_row["returnDisposition"]["evidence"]
        _require(
            member_evidence["start"] == start
            and member_evidence["decodeComplete"]
            and member_evidence["analysisComplete"]
            and bool(member_evidence["exits"]),
            "admission census callee coverage",
        )
        edges[start] = []
        for item in member_evidence["instructions"]:
            member_word = item["word"]
            _require(
                not (member_word >> 26 == 0 and member_word & 63 == 9),
                "admission census indirect callee call",
            )
            if member_word >> 26 != 3:
                continue
            callee = ((item["pc"] + 4) & 0xF0000000) | ((member_word & 0x03FFFFFF) << 2)
            _require(callee in closure, "admission census external callee")
            edges[start].append(callee)
            pending.append(callee)
    colors = {}

    def visit(start):
        if colors.get(start) == 2:
            return True
        if colors.get(start) == 1:
            return False
        colors[start] = 1
        if not all(visit(target) for target in edges[start]):
            return False
        colors[start] = 2
        return True

    _require(visit(crossed["target"]), "admission census recursive closure")
    known = {crossed["target"]: (crossed["entry"], crossed["entrySpOffset"])}
    pending = [crossed]
    members = {}
    while pending:
        current = pending.pop(0)
        if current["target"] in members:
            continue
        member, nested = _admission_replay(
            closure[current["target"]][1],
            current["entrySpOffset"],
            call_sp_physical,
            current["entry"],
            identities,
            captured,
        )
        for item in nested:
            _require(
                item["callee"] is not None, "admission census unresolved callee call"
            )
            if item["target"] in known:
                _require(
                    known[item["target"]] == (item["entry"], item["entrySpOffset"]),
                    "admission census repeated callee entry",
                )
                continue
            known[item["target"]] = (item["entry"], item["entrySpOffset"])
            pending.append(item)
        members[current["target"]] = member
    physical_words, physical_set = {}, set()
    for word in protected:
        offset = word["entrySpOffset"] + frame
        _require(
            0 <= offset <= frame - 4 and word["physicalWord"] not in physical_set,
            "admission census protected word placement",
        )
        physical_set.add(word["physicalWord"])
        physical_words[offset] = word
    _require(physical_words, "admission census lacks protected words")
    outgoing = 16
    outgoing_pcs = set()
    rows = [(access, True) for access in caller["rows"]]
    for member in members.values():
        rows.extend((access, False) for access in member["rows"])
    for access, owner in rows:
        # A stack-pointer access belongs to a frame by construction. An ordinary-RAM access
        # belongs to the caller frame only when its resolved interval actually lies inside it;
        # a disjoint object above or below S is neither a stack argument nor a frame access.
        offset = access["frameOffset"]
        inside = 0 <= offset <= frame - access["bytes"]
        _require(
            not (owner and access["stack"] and not inside),
            "admission census caller frame access",
        )
        saved = owner and access["bytes"] == 4 and inside and offset in physical_words
        if access["write"] and not saved:
            for protected in physical_words:
                _require(
                    not (
                        offset < protected + 4 and protected < offset + access["bytes"]
                    ),
                    "admission census reachable write overlaps a protected word",
                )
        if saved or offset < 0:
            continue
        if not access["stack"] and not inside:
            continue
        outgoing_pcs.add(access["pc"])
        outgoing = max(outgoing, offset + access["bytes"])
    outgoing = (outgoing + 3) & ~3
    _require(
        outgoing <= frame and len(outgoing_pcs) <= 4096,
        "admission census outgoing interval",
    )
    for offset, word in physical_words.items():
        _require(
            offset >= outgoing
            and offset % 4 == 0
            and ((entry_sp - frame + offset) & 0x1FFFFFFF) == word["physicalWord"],
            "admission census protected word interval",
        )
    for access in caller["rows"]:
        if not access["write"] or not access["valueKnown"]:
            continue
        if access["pc"] >= call["pc"]:
            continue
        _require(
            not (
                0 <= access["valueFrameOffset"] < frame
                and int(access["valueFrameOffset"]) in physical_words
            ),
            "admission census protected address exposed",
        )
    for reg in range(1, 32):
        if reg == 29 or crossed["entry"][reg] is None:
            continue
        if crossed["entry"][reg][0] != "stack":
            continue
        _require(
            crossed["entry"][reg][1] not in physical_words,
            "admission census protected address passed to the callee",
        )
    return {
        "outgoingBytes": outgoing,
        "outgoingPcs": sorted(outgoing_pcs),
        "protectedWords": [
            dict(
                physicalWord=physical_words[offset]["physicalWord"],
                entrySpOffset=offset - frame,
                register=physical_words[offset]["register"],
                storePc=physical_words[offset]["storePc"],
            )
            for offset in sorted(physical_words)
        ],
        "closure": [
            dict(
                function=closure[start][0],
                start=member["start"],
                end=member["end"],
                entrySpOffset=member["entrySpOffset"],
                instructions=member["instructions"],
                calls=member["calls"],
                accesses=len(member["rows"]),
                writes=sum(1 for access in member["rows"] if access["write"]),
            )
            for start, member in sorted(members.items())
        ],
    }


def _memory_prefix(row, access):
    evidence = row["returnDisposition"]["evidence"]
    summary = row["originalCallSummary"]
    instructions = {item["pc"]: item for item in evidence["instructions"]}
    first, last, stop = evidence["start"], evidence["end"], access["pc"]
    _require(
        evidence["decodeComplete"]
        and evidence["analysisComplete"]
        and _complete(summary["control"])
        and summary["pendingEntry"] == "requires-no-pending-gpr-loads"
        and set(instructions) == set(range(first, last, 4)),
        "memory prefix requires complete original words/control",
    )
    exits = {item["pc"]: item for item in summary["exits"]}
    prefix = set(range(first, stop + 4, 4))
    for pc, instruction in instructions.items():
        word = instruction["word"]
        kind, _ = _memory_word(word)
        _require(
            instruction["bytes"] == 4
            and instruction["mapped"]
            and not instruction["undef"]
            and instruction["owner"] == row["name"]
            and kind not in {"unsupported", "call", "trap", "indirect-branch"},
            "memory prefix unsupported original control/word",
        )
        if kind == "return":
            exit = exits.get(pc)
            _require(
                word == 0x03E00008
                and exit is not None
                and _complete(exit["incomingLink"])
                and exit["sampledEntry"] == 31,
                "memory prefix unproved original return",
            )
        if pc <= stop:
            _require(
                kind not in {"branch", "return"},
                "memory prefix contains earlier control",
            )
        if kind == "branch":
            target = (
                ((pc + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2)
                if word >> 26 == 2
                else (pc + 4 + _signed16(word) * 4) & 0xFFFFFFFF
            )
            successors = {target}
            if word >> 26 != 2:
                successors.add((pc + 8) & 0xFFFFFFFF)
            _require(not successors & prefix, "original control enters memory prefix")
    incoming = []
    for edge in evidence["edges"]:
        if edge["to"] in prefix:
            _require(
                edge["to"] != first
                and edge["from"] == edge["to"] - 4
                and edge["from"] in prefix
                and edge["kind"] == "fallthrough"
                and edge["delayOwner"] is None,
                "memory prefix has alternative incoming edge",
            )
            incoming.append((edge["from"], edge["to"]))
        if edge["from"] in prefix - {stop}:
            _require(
                edge["to"] == edge["from"] + 4
                and edge["kind"] == "fallthrough"
                and edge["delayOwner"] is None,
                "memory prefix has non-fallthrough edge",
            )
    _require(
        len(incoming) == len(prefix) - 1
        and set(incoming) == {(pc, pc + 4) for pc in prefix - {stop}},
        "memory prefix edge coverage",
    )
    return instructions


def _memory_prefix_loads(instructions, stop, protected_registers):
    protected_registers.discard(0)
    for pc, instruction in instructions.items():
        if pc >= stop:
            continue
        kind, destinations = _memory_word(instruction["word"])
        _require(
            kind != "load" or not protected_registers.intersection(destinations),
            "memory prefix load modifies address chain",
        )
        _require(
            all(
                instruction["fullWidthRegisters"].count(register) == 1
                for register in destinations
                if register
            ),
            "memory prefix has unmapped full-width destination",
        )


def _memory_prefix_address(row, access, entry_flow=None, *, first_use=False):
    evidence = row["returnDisposition"]["evidence"]
    if entry_flow is None:
        instructions = _memory_prefix(row, access)
    else:
        summary = row["originalCallSummary"]
        instructions = {item["pc"]: item for item in evidence["instructions"]}
        _require(
            evidence["analysisComplete"]
            and summary["pendingEntry"] == "requires-no-pending-gpr-loads"
            and (
                first_use
                or (
                    evidence["decodeComplete"]
                    and set(instructions)
                    == set(range(evidence["start"], evidence["end"], 4))
                    and all(
                        _memory_word(item["word"])[0]
                        not in {"unsupported", "trap", "indirect-branch"}
                        for item in instructions.values()
                    )
                )
            ),
            "fixed memory flow requires complete original words",
        )
    first, stop = evidence["start"], access["pc"]
    prefix = set(range(first, stop + 4, 4))
    definitions = {}
    by_id = {definition["id"]: definition for definition in evidence["definitions"]}
    for definition in evidence["definitions"]:
        key = (definition["pc"], definition["register"])
        definitions.setdefault(key, []).append(definition)
    if entry_flow is not None:
        _require(
            len(by_id) == len(evidence["definitions"])
            and all(len(matches) == 1 for matches in definitions.values()),
            "fixed memory flow has duplicate original definitions",
        )
    base_register = (instructions[stop]["word"] >> 21) & 31
    references, visited = access["addressDefinitions"], set()
    _require(
        entry_flow is None or base_register != 0, "fixed flow requires a defined base"
    )
    chain, before, register = [], stop, base_register
    while register:
        _require(len(chain) < 128, "memory constant chain bound")
        if entry_flow is None:
            candidates = [
                pc
                for pc in prefix
                if pc < before and register in _memory_word(instructions[pc]["word"])[1]
            ]
            _require(candidates, "memory constant reads unknown entry")
            pc = max(candidates)
        else:
            _require(
                len(references) == 1 and references[0] in by_id,
                "fixed flow requires one original reaching definition",
            )
            pc = by_id[references[0]]["pc"]
            entry_flow[1](before, register, pc)
        matches = definitions.get((pc, register), [])
        _require(len(matches) == 1, "memory constant definition ambiguity")
        definition = matches[0]
        _require(
            definition["id"] not in visited
            and (entry_flow is None or references == [definition["id"]]),
            "memory constant reaching definition differs",
        )
        visited.add(definition["id"])
        word, opcode = instructions[pc]["word"], instructions[pc]["word"] >> 26
        _require(
            definition["width"] == 32
            and not definition["delayed"]
            and not definition["bad"]
            and not definition["formals"]
            and not definition["entries"]
            and definition["equalEntry"] is None
            and definition["origin"] == {"kind": "instruction"}
            and instructions[pc]["fullWidthRegisters"].count(register) == 1
            and ((word >> 16) & 31) == register
            and opcode in {9, 12, 13, 14, 15}
            and definition["operation"]
            == ("lui" if opcode == 15 else "integer-immediate")
            and (opcode != 15 or ((word >> 21) & 31) == 0),
            "memory constant has unsupported original definition",
        )
        if entry_flow is not None:
            entry_flow[1](pc, register, pc, after=True, destination=register)
        chain.append((definition, word))
        if opcode == 15:
            break
        references = definition["inputs"]
        register, before = (word >> 21) & 31, pc
    value, previous, pcs, stored_pcs = 0, None, [], []
    for definition, word in reversed(chain):
        opcode, source = word >> 26, (word >> 21) & 31
        _require(
            definition["inputs"] == ([] if opcode == 15 or source == 0 else [previous]),
            "memory constant original dependency differs",
        )
        if opcode == 15:
            value = (word & 0xFFFF) << 16
        elif opcode == 9:
            value = (value + _signed16(word)) & 0xFFFFFFFF
        elif opcode == 12:
            value &= word & 0xFFFF
        elif opcode == 13:
            value |= word & 0xFFFF
        else:
            value ^= word & 0xFFFF
        pcs.append(definition["pc"])
        if (word & 0xFFFF) or opcode not in {9, 13, 14}:
            stored_pcs.clear()
        stored_pcs.append(definition["pc"])
        _require(
            definition["constant"] == value
            and definition["equalityEvidence"]
            == (stored_pcs if entry_flow is None else sorted(stored_pcs)),
            "memory constant/equality evidence differs from original words",
        )
        previous = definition["id"]
    _require(
        not access["addressEntries"]
        and access["addressDefinitions"] == ([] if base_register == 0 else [previous]),
        "memory access latest base definition differs",
    )
    protected_registers = {base_register}
    for _, word in chain:
        protected_registers.update(_memory_word(word)[1])
        if word >> 26 != 15:
            protected_registers.add((word >> 21) & 31)
    if entry_flow is None:
        _memory_prefix_loads(instructions, stop, protected_registers)
    return (
        value,
        (value + _signed16(instructions[stop]["word"])) & 0xFFFFFFFF,
        sorted(pcs),
    )


def _entry_relative_prefix_address(row, access):
    evidence = row["returnDisposition"]["evidence"]
    instructions = _memory_prefix(row, access)
    stop = access["pc"]
    definitions = {}
    for definition in evidence["definitions"]:
        key = (definition["pc"], definition["register"])
        definitions.setdefault(key, []).append(definition)
    base_register = (instructions[stop]["word"] >> 21) & 31
    _require(base_register != 0, "memory relative base is architectural zero")
    chain, before, register = [], stop, base_register
    while True:
        candidates = [
            pc
            for pc, instruction in instructions.items()
            if pc < before and register in _memory_word(instruction["word"])[1]
        ]
        if not candidates:
            break
        _require(len(chain) < 128, "memory relative chain bound")
        pc = max(candidates)
        matches = definitions.get((pc, register), [])
        _require(len(matches) == 1, "memory relative definition missing or ambiguous")
        definition = matches[0]
        word = instructions[pc]["word"]
        fields = word_fields(word)
        copy = (
            fields.opcode == 0
            and fields.funct == 33
            and fields.shift == 0
            and bool(fields.rs) != bool(fields.rt)
        )
        source = fields.rs if fields.opcode == 9 or fields.rs else fields.rt
        destination = fields.rt if fields.opcode == 9 else fields.rd
        _require(
            (fields.opcode == 9 or copy)
            and source != 0
            and destination == register
            and definition["width"] == 32
            and not definition["delayed"]
            and definition["origin"] == {"kind": "instruction"}
            and definition["constant"] is None
            and definition["operation"]
            == ("integer-immediate" if fields.opcode == 9 else "integer-binary")
            and instructions[pc]["fullWidthRegisters"].count(destination) == 1,
            "memory relative has unsupported original definition",
        )
        chain.append((definition, word, source))
        before, register = pc, source
    entries = [register]
    formals = entries if 4 <= register <= 7 else []
    bad = (
        ["incoming-v0"]
        if register == 2
        else [f"unproved-entry-register-{register}"]
        if register in {1, 3} or 8 <= register <= 30
        else []
    )
    offset, previous, pcs, identity_pcs, equal_entry = 0, None, [], [], register
    protected_registers = {base_register, register}
    for definition, word, source in reversed(chain):
        _require(
            definition["inputs"] == ([] if previous is None else [previous])
            and definition["entries"] == entries
            and definition["formals"] == formals
            and definition["bad"] == bad,
            "memory relative original dependency/provenance differs",
        )
        if word >> 26 == 9 and word & 0xFFFF:
            offset = (offset + _signed16(word)) & 0xFFFFFFFF
            equal_entry = None
            identity_pcs.clear()
        else:
            identity_pcs.append(definition["pc"])
        if offset == 0 and definition["equalEntry"] == register:
            equal_entry = register
            identity_pcs = [*pcs, definition["pc"]]
        _require(
            definition["equalEntry"] == equal_entry
            and definition["equalityEvidence"] == identity_pcs,
            "memory relative original identity evidence differs",
        )
        pcs.append(definition["pc"])
        protected_registers.update((source, definition["register"]))
        previous = definition["id"]
    _require(
        access["addressEntries"] == entries
        and access["addressDefinitions"] == ([] if previous is None else [previous]),
        "memory relative access latest base provenance differs",
    )
    _memory_prefix_loads(instructions, stop, protected_registers)
    return register, (offset + _signed16(instructions[stop]["word"])) & 0xFFFFFFFF, pcs


def _conditional_frame_flow(evidence):
    if evidence["decodeComplete"]:
        return True
    architectural = [
        item for item in evidence["obligations"] if item["domain"] == "architectural"
    ]
    return any(
        item["reason"] == "conditional-symbolic-frame-restoration-unresolved"
        for item in architectural
    ) and all(
        item["reason"]
        in {
            "conditional-symbolic-frame-restoration-unresolved",
            "all-path-v0-definition-unresolved",
        }
        for item in architectural
    )


def _entry_flow(row, *, frame_uses=None, use_pc=None):
    evidence = row["returnDisposition"]["evidence"]
    instructions = {item["pc"]: item for item in evidence["instructions"]}
    definitions = {item["id"]: item for item in evidence["definitions"]}

    def adjustment(pc):
        word = instructions[pc]["word"]
        return word >> 26 == 9 and word & 0xFFFF

    claims = [
        definition
        for definition in definitions.values()
        if definition["equalEntry"] is not None
        and (
            adjustment(definition["pc"])
            or any(adjustment(pc) for pc in definition["equalityEvidence"])
        )
    ]
    flow_claims = any(
        type(access) is dict
        and type(access.get("address")) is dict
        and access["address"].get("kind")
        in {
            "entry-register-relative-flow/v1",
            "entry-register-bounded-flow/v1",
            "instruction-fixed-flow/v1",
        }
        for access in row["originalCallSummary"].get("accesses", [])
    )
    if use_pc is None:
        if not claims and not flow_claims:
            return None
        _require(
            _conditional_frame_flow(evidence) and evidence["analysisComplete"],
            "arithmetic identity requires complete original decode",
        )
    else:
        _require(
            evidence["analysisComplete"]
            and row["originalCallSummary"]["pendingEntry"]
            == "requires-no-pending-gpr-loads"
            and use_pc in instructions,
            "first-use address requires original entry and operand analysis",
        )

    def original_word(pc):
        instruction = instructions.get(pc)
        _require(
            instruction is not None
            and evidence["start"] <= pc < evidence["end"]
            and pc % 4 == 0
            and instruction["bytes"] == 4
            and instruction["mapped"]
            and not instruction["undef"]
            and instruction["owner"] == row["name"],
            "arithmetic identity original word/ownership differs",
        )
        return instruction["word"]

    calls = {call["pc"]: call for call in evidence["calls"]}
    _require(len(calls) == len(evidence["calls"]), "duplicate arithmetic identity call")

    def call_preserves(pc, register):
        _integer(register, 33)
        call = calls.get(pc)
        word = original_word(pc)
        if pc > 0xFFFFFFFF - 8 or call is None or word >> 26 != 3:
            return False
        contract = call["contract"]
        binding = (frame_uses or {}).get(pc)
        if binding and register in binding["registers"]:
            return (
                binding["call"] is call
                and call["target"] == binding["entryPc"]
                and binding["continuation"] == pc + 8
                and all(
                    register not in _memory_word(original_word(point))[1]
                    for point in (pc, pc + 4)
                )
            )
        transfers = _rows(contract["registerTransfers"], 34, "call register transfers")
        if (
            call["target"] != (((pc + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2))
            or not contract["callee"]
            or (use_pc is not None and contract["kind"] != "complete")
            or not all(
                _complete(contract[dimension])
                for dimension in ("identity", "inputs", "control", "transfers")
            )
            or contract["delay"] != pc + 4
            or contract["continuation"] != pc + 8
            or contract["actualIncomingLink"] != pc + 8
            or len(transfers) != 34
        ):
            return False
        for index, transfer in enumerate(transfers):
            _keys(
                transfer,
                set(
                    "register kind proof definitions entries bad equalEntry constant equalityEvidence pending mayNoPending pendingDefinitions".split()
                ),
                "call register transfer",
            )
            _integer(transfer["register"], 33)
            _integer(transfer["equalEntry"], 33, optional=True)
            if (
                transfer["register"] != index
                or not _complete(transfer["proof"])
                or transfer["pending"] is not False
                or transfer["mayNoPending"] is not True
                or _references(transfer["pendingDefinitions"], 4096, 1)
            ):
                return False
        transfer = transfers[register]
        return (
            transfer["kind"] == "preserved"
            and transfer["equalEntry"] == register
            and all(
                register not in _memory_word(original_word(point))[1]
                for point in (pc, pc + 4)
            )
        )

    pending, nodes, contexts, derived = [(evidence["start"], None)], set(), {}, set()
    crossed, outgoing = set(), {}
    for edge in evidence["edges"]:
        outgoing.setdefault(edge["from"], []).append(edge)
    while pending:
        pc, owner = pending.pop()
        if (pc, owner) in nodes:
            continue
        nodes.add((pc, owner))
        _require(len(nodes) <= 4096, "arithmetic identity original CFG bound")
        contexts.setdefault(pc, set()).add(owner)
        _require(
            use_pc is None or len(contexts[pc]) == 1,
            "first-use address ambiguous delay context",
        )
        word = original_word(pc)
        kind, _ = _memory_word(word)
        if pc == use_pc:
            _require(
                owner is None
                or (
                    pc == owner + 4
                    and _memory_word(original_word(owner))[0] in {"branch", "call"}
                    and kind not in {"branch", "return", "call"}
                ),
                "first-use address operand delay owner differs",
            )
            continue
        _require(
            use_pc is None
            or (kind != "return" and (kind != "call" or word >> 26 == 3)),
            "first-use address crosses unproved control",
        )
        _require(
            kind not in {"trap", "unsupported", "indirect-branch"},
            "arithmetic identity unsupported control",
        )
        if owner is None:
            successors = (
                {(pc + 4, pc, "delay-slot")}
                if kind in {"branch", "return", "call"}
                else {(pc + 4, None, "fallthrough")}
            )
        else:
            branch = original_word(owner)
            fields = word_fields(branch)
            owner_kind, _ = _memory_word(branch)
            _require(
                pc == owner + 4
                and owner_kind in {"branch", "return", "call"}
                and kind not in {"branch", "return", "call"},
                "arithmetic identity delay owner differs",
            )
            successors = set()
            if owner_kind == "branch":
                target = (
                    (((owner + 4) & 0xF0000000) | ((branch & 0x03FFFFFF) << 2))
                    if fields.opcode == 2
                    else (owner + 4 + _signed16(branch) * 4) & 0xFFFFFFFF
                )
                successors.add((target, None, "branch"))
                if fields.opcode != 2:
                    successors.add((owner + 8, None, "fallthrough"))
            elif owner_kind == "call":
                if use_pc is not None:
                    _require(
                        call_preserves(
                            owner, (instructions[use_pc]["word"] >> 21) & 31
                        ),
                        "first-use address crosses unbound call",
                    )
                    crossed.add(owner)
                successors.add((owner + 8, None, "call-continuation"))
        if use_pc is not None:
            actual = {
                (edge["to"], edge["delayOwner"], edge["kind"])
                for edge in outgoing.get(pc, [])
            }
            _require(
                len(actual) == len(outgoing.get(pc, [])) and actual == successors,
                "first-use address participating CFG inventory differs",
            )
        for target, delay_owner, edge_kind in successors:
            derived.add((pc, target, delay_owner, edge_kind))
            _require(
                len(derived) <= (4096 if use_pc is None else 8192),
                "arithmetic identity original edge bound",
            )
            pending.append((target, delay_owner))
    reported = {
        (edge["from"], edge["to"], edge["delayOwner"], edge["kind"])
        for edge in evidence["edges"]
    }
    if use_pc is None:
        _require(
            len(reported) == len(evidence["edges"]) and reported == derived,
            "arithmetic identity original CFG inventory differs",
        )
    else:
        _require(use_pc in contexts, "first-use address operand unreachable")
        indegrees, successors = {pc: 0 for pc in contexts}, {}
        for source, target, _, _ in derived:
            indegrees[target] += 1
            successors.setdefault(source, []).append(target)
        ready = [pc for pc, count in indegrees.items() if count == 0]
        _require(ready == [evidence["start"]], "first-use address entry cycle")
        consumed = 0
        while ready:
            pc = ready.pop()
            consumed += 1
            for target in successors.get(pc, []):
                indegrees[target] -= 1
                if indegrees[target] == 0:
                    ready.append(target)
        _require(consumed == len(contexts), "first-use address cyclic prefix")
    incoming = {}
    for source, target, _, _ in derived:
        incoming.setdefault(target, []).append(source)

    def reaches(pc, register, writer, *, after=False, destination=None):
        if destination is not None:
            for previous in incoming.get(pc, []):
                kind, destinations = _memory_word(original_word(previous))
                _require(
                    kind != "load" or destination not in destinations,
                    "arithmetic identity pending destination",
                )
        pending, visited, terminals, references = [(pc, after)], set(), 0, 0
        while pending:
            pc, include = pending.pop()
            if include and pc in visited:
                continue
            if include:
                visited.add(pc)
            _require(len(visited) <= 4096, "arithmetic identity predecessor bound")
            owners = contexts.get(pc, set())
            _require(len(owners) == 1, "arithmetic identity ambiguous delay context")
            owner = next(iter(owners))
            kind, destinations = _memory_word(original_word(pc))
            _require(
                (use_pc is not None or kind != "call" or call_preserves(pc, register))
                and (
                    owner is None
                    or _memory_word(instructions[owner]["word"])[0] != "call"
                    or (use_pc is not None and not include)
                    or (
                        (use_pc is None or owner in crossed)
                        and call_preserves(owner, register)
                    )
                ),
                "arithmetic identity crosses call",
            )
            predecessors = incoming.get(pc, [])
            if include and register in destinations:
                _require(
                    kind != "load"
                    and pc == writer
                    and instructions[pc]["fullWidthRegisters"].count(register) == 1,
                    "arithmetic identity alternative/pending writer",
                )
                terminals += 1
                continue
            if pc == evidence["start"]:
                _require(writer is None, "arithmetic identity entry bypass")
                terminals += 1
            _require(
                predecessors or pc == evidence["start"],
                "arithmetic identity missing predecessor",
            )
            references += len(predecessors)
            _require(references <= 8192, "arithmetic identity predecessor references")
            pending.extend((previous, True) for previous in predecessors)
        _require(terminals, "arithmetic identity has no reaching root")

    def entry_offset(reference):
        chain, visited, offset, pcs, adjusted = [], set(), 0, set(), False
        _require(reference in definitions, "arithmetic identity missing definition")
        definition = definitions[reference]
        while True:
            reference = definition["id"]
            _require(
                reference not in visited and len(visited) < 128,
                "arithmetic identity cyclic/deep definitions",
            )
            visited.add(reference)
            word = original_word(definition["pc"])
            fields = word_fields(word)
            copy = (
                fields.opcode == 0
                and fields.funct == 33
                and fields.shift == 0
                and bool(fields.rs) != bool(fields.rt)
            )
            source = fields.rs if fields.opcode == 9 or fields.rs else fields.rt
            destination = fields.rt if fields.opcode == 9 else fields.rd
            _require(
                (fields.opcode == 9 or copy)
                and source != 0
                and destination == definition["register"]
                and definition["width"] == 32
                and not definition["delayed"]
                and definition["origin"] == {"kind": "instruction"}
                and definition["constant"] is None
                and definition["operation"]
                == ("integer-immediate" if fields.opcode == 9 else "integer-binary")
                and instructions[definition["pc"]]["fullWidthRegisters"].count(
                    destination
                )
                == 1
                and len(definition["inputs"]) <= 1,
                "arithmetic identity unsupported original definition",
            )
            _require(
                not definition["inputs"] or definition["inputs"][0] in definitions,
                "arithmetic identity missing input definition",
            )
            input = (
                definitions[definition["inputs"][0]] if definition["inputs"] else None
            )
            _require(
                input is None or input["register"] == source,
                "arithmetic identity source ownership",
            )
            reaches(
                definition["pc"],
                source,
                None if input is None else input["pc"],
                destination=destination,
            )
            if fields.opcode == 9:
                offset = (offset + _signed16(word)) & 0xFFFFFFFF
                adjusted |= bool(word & 0xFFFF)
            chain.append(definition)
            pcs.add(definition["pc"])
            if input is None:
                entry = source
                break
            definition = input
        formals = [entry] if 4 <= entry <= 7 else []
        bad = (
            ["incoming-v0"]
            if entry == 2
            else [f"unproved-entry-register-{entry}"]
            if entry in {1, 3} or 8 <= entry <= 30
            else []
        )
        _require(
            all(
                member["entries"] == [entry]
                and member["formals"] == formals
                and member["bad"] == bad
                for member in chain
            ),
            "arithmetic identity original provenance differs",
        )
        return entry, offset, pcs, adjusted

    return entry_offset, reaches, claims if use_pc is None else sorted(crossed)


def _entry_relative_flow_address(row, access, entry_flow, *, first_use=False):
    evidence = row["returnDisposition"]["evidence"]
    summary = row["originalCallSummary"]
    instructions = {item["pc"]: item for item in evidence["instructions"]}
    definitions = {item["id"]: item for item in evidence["definitions"]}
    _require(
        entry_flow is not None
        and (first_use or _conditional_frame_flow(evidence))
        and evidence["analysisComplete"]
        and summary["pendingEntry"] == "requires-no-pending-gpr-loads"
        and (
            first_use
            or set(instructions) == set(range(evidence["start"], evidence["end"], 4))
        ),
        "memory flow requires complete original words and analysis",
    )
    entry_offset, reaches, _ = entry_flow
    word = instructions[access["pc"]]["word"]
    register = (word >> 21) & 31
    references = access["addressDefinitions"]
    _require(
        register != 0 and len(references) <= 1,
        "memory flow ambiguous base definition",
    )
    if references:
        _require(references[0] in definitions, "memory flow missing base definition")
        definition = definitions[references[0]]
        _require(
            definition["register"] == register,
            "memory flow base definition ownership",
        )
        entry, offset, pcs, _ = entry_offset(references[0])
        writer = definition["pc"]
    else:
        entry, offset, pcs, writer = register, 0, set(), None
    _require(
        access["addressEntries"] == [entry],
        "memory flow access entry provenance differs",
    )
    reaches(access["pc"], register, writer)
    return entry, (offset + _signed16(word)) & 0xFFFFFFFF, sorted(pcs)


def _bounded_entry_flow_address(row, access, entry_flow):
    evidence, summary = row["returnDisposition"]["evidence"], row["originalCallSummary"]
    instructions = {item["pc"]: item for item in evidence["instructions"]}
    definitions = {item["id"]: item for item in evidence["definitions"]}
    _require(
        entry_flow is not None
        and evidence["decodeComplete"]
        and evidence["analysisComplete"]
        and _complete(summary["control"])
        and summary["exits"]
        and not evidence["calls"]
        and summary["pendingEntry"] == "requires-no-pending-gpr-loads"
        and set(instructions) == set(range(evidence["start"], evidence["end"], 4)),
        "bounded memory flow requires complete original words/control",
    )
    entry_offset, reaches, _ = entry_flow
    edges = {
        (e["from"], e["to"], e["delayOwner"], e["kind"]) for e in evidence["edges"]
    }

    def word(pc):
        return instructions.get(pc, {}).get("word", 0xFFFFFFFF)

    def definition(pc, register):
        found = [
            d
            for d in definitions.values()
            if d["pc"] == pc and d["register"] == register
        ]
        return found[0] if len(found) == 1 else None

    def latest(before, register):
        pcs = [
            pc
            for pc in instructions
            if pc < before and register in _memory_word(word(pc))[1]
        ]
        return definition(max(pcs), register) if pcs else None

    def closed(begin, end, expected):
        observed = {e for e in edges if begin <= e[0] <= end or begin <= e[1] <= end}
        if observed != expected or len(observed) > 8192:
            return False
        for pc in range(begin, end + 4, 4):
            instruction = instructions.get(pc)
            kind, destinations = _memory_word(word(pc))
            if (
                instruction is None
                or not instruction["mapped"]
                or instruction["undef"]
                or instruction["bytes"] != 4
                or pc % 4
                or instruction["owner"] != row["name"]
                or kind in {"call", "trap", "unsupported", "indirect-branch"}
                or word(pc) >> 26 in {34, 38, 42, 46}
                or any(
                    reg and instruction["fullWidthRegisters"].count(reg) != 1
                    for reg in destinations
                )
            ):
                return False
        return True

    def address(pc, pointer, offset, count, pcs, writers):
        if access["pc"] != pc:
            return None
        _require(
            access["width"] == 8
            and word(pc) & 0xFFFF == 0
            and (word(pc) >> 21) & 31 == pointer
            and access["addressEntries"] == [29]
            and access["addressDefinitions"] == sorted(writers)
            and pcs
            and len(pcs) <= 8192,
            "bounded memory address inventory differs",
        )
        return {
            "kind": "entry-register-bounded-flow/v1",
            "entryRegister": 29,
            "offset": offset,
            "indexLower": 0,
            "indexUpper": count - 1,
            "boundEvidence": sorted(pcs),
        }

    for h in sorted(instructions):
        if access["pc"] not in {h, h + 48, h + 68}:
            continue
        load, compare, increment, mask = (
            word(h),
            word(h + 20),
            word(h + 28),
            word(h + 12),
        )
        pointer, loaded = (load >> 21) & 31, (load >> 16) & 31
        endpoint, comparison = (compare >> 16) & 31, (compare >> 11) & 31
        counter, count_register = (increment >> 21) & 31, (mask >> 16) & 31
        registers = {pointer, loaded, endpoint, counter, count_register}
        if (
            h < evidence["start"] + 4
            or load >> 26 != 36
            or load & 0xFFFF
            or len(registers) != 5
            or 0 in registers
            or comparison != loaded
            or word(h + 4) != 0
            or word(h + 8) != (4 << 26 | loaded << 21 | 6)
            or word(h + 16) != (9 << 26 | pointer << 21 | pointer << 16 | 1)
            or compare != (pointer << 21 | endpoint << 16 | comparison << 11 | 42)
            or word(h + 24) != (5 << 26 | comparison << 21 | 0xFFF9)
            or increment != (9 << 26 | counter << 21 | counter << 16 | 1)
            or mask >> 26 != 12
            or (mask >> 21) & 31 != counter
            or word(h + 32) != mask
        ):
            continue
        seed, counter_seed = latest(h, pointer), latest(h, counter)
        recurrence = definition(h + 16, pointer)
        endpoint_seed = latest(h + 20, endpoint)
        if (
            seed is None
            or counter_seed is None
            or recurrence is None
            or endpoint_seed is None
            or counter_seed["pc"] >= h - 4
            or word(counter_seed["pc"]) != (counter << 11 | 33)
        ):
            continue
        _require(
            all(
                _memory_word(word(previous))[0] != "load"
                or counter not in _memory_word(word(previous))[1]
                for previous, target, _, _ in edges
                if target == counter_seed["pc"]
            ),
            "bounded counter initializer has pending load",
        )
        reaches(h - 4, pointer, seed["pc"], after=True)
        reaches(h - 4, counter, counter_seed["pc"], after=True)
        start_entry, offset, pcs, _ = entry_offset(seed["id"])
        end_entry, end_offset, end_pcs, _ = entry_offset(endpoint_seed["id"])
        reaches(h + 20, endpoint, endpoint_seed["pc"])
        count, mask_bits = (end_offset - offset) & 0xFFFFFFFF, mask & 0xFFFF
        if (
            start_entry != 29
            or end_entry != 29
            or not 0 < count <= 4096
            or count > mask_bits
            or mask_bits & (mask_bits + 1)
        ):
            continue
        scan_edges = {
            (h - 4, h, None, "fallthrough"),
            (h, h + 4, None, "fallthrough"),
            (h + 4, h + 8, None, "fallthrough"),
            (h + 8, h + 12, h + 8, "delay-slot"),
            (h + 12, h + 16, None, "fallthrough"),
            (h + 12, h + 36, None, "branch"),
            (h + 16, h + 20, None, "fallthrough"),
            (h + 20, h + 24, None, "fallthrough"),
            (h + 24, h + 28, h + 24, "delay-slot"),
            (h + 28, h, None, "branch"),
            (h + 28, h + 32, None, "fallthrough"),
            (h + 32, h + 36, None, "fallthrough"),
        }
        if not closed(h, h + 32, scan_edges):
            continue
        pcs |= end_pcs | {counter_seed["pc"], h - 4} | set(range(h, h + 36, 4))
        found = address(h, pointer, offset, count, pcs, {seed["id"], recurrence["id"]})
        if found is not None:
            return found
        guard, body = h + 36, h + 48
        body_load, body_compare = word(body), word(body + 24)
        index, body_pointer = (word(guard + 4) >> 11) & 31, (body_load >> 21) & 31
        body_value, body_comparison = (body_load >> 16) & 31, (body_compare >> 11) & 31
        body_registers = {index, body_pointer, count_register, body_comparison}
        if (
            len(body_registers) != 4
            or 0 in body_registers
            or body_value != body_comparison
            or word(guard) != (6 << 26 | count_register << 21 | 11)
            or word(guard + 4) != (index << 11 | 33)
            or body_load != (36 << 26 | body_pointer << 21 | body_value << 16)
            or word(body + 16) != (9 << 26 | index << 21 | index << 16 | 1)
            or word(body + 20) != (40 << 26 | body_pointer << 21 | body_value << 16)
            or body_compare
            != (index << 21 | count_register << 16 | body_comparison << 11 | 42)
            or word(body + 28) != (5 << 26 | body_comparison << 21 | 0xFFF8)
            or word(body + 32)
            != (9 << 26 | body_pointer << 21 | body_pointer << 16 | 1)
        ):
            continue
        body_seed, body_recurrence = (
            definition(guard + 8, body_pointer),
            definition(body + 32, body_pointer),
        )
        if body_seed is None or body_recurrence is None:
            continue
        base_entry, base_offset, base_pcs, _ = entry_offset(body_seed["id"])
        if base_entry != 29 or base_offset != offset:
            continue
        if any(
            _memory_word(word(pc))[0] not in {"load", "store", "compute"}
            or set(_memory_word(word(pc))[1]) & {body_pointer, index, count_register}
            for pc in range(body + 4, body + 16, 4)
        ):
            continue
        body_edges = {
            (h + 12, guard, None, "branch"),
            (h + 32, guard, None, "fallthrough"),
            (guard, guard + 4, guard, "delay-slot"),
            (guard + 4, guard + 8, None, "fallthrough"),
            (guard + 4, body + 36, None, "branch"),
            (guard + 8, body, None, "fallthrough"),
            (body + 28, body + 32, body + 28, "delay-slot"),
            (body + 32, body, None, "branch"),
            (body + 32, body + 36, None, "fallthrough"),
        } | {(pc, pc + 4, None, "fallthrough") for pc in range(body, body + 28, 4)}
        if not closed(guard, body + 32, body_edges):
            continue
        pcs |= base_pcs | set(range(guard, body + 36, 4))
        for pc in (body, body + 20):
            found = address(
                pc,
                body_pointer,
                offset,
                count,
                pcs,
                {body_seed["id"], body_recurrence["id"]},
            )
            if found is not None:
                return found
    _require(False, "bounded memory has no closed original scan/count loop")


def validate_access(
    row,
    access,
    payload,
    load,
    entry_flow=None,
    *,
    context=None,
    images=None,
    frame_uses=None,
):
    for dimension in DIMENSIONS:
        _require(
            not _complete(access[dimension]), "unsupported whole-object memory closure"
        )
    address = access["address"]
    _require(
        type(address) is dict and type(address.get("kind")) is str,
        "memory address shape",
    )
    if address["kind"] in {"unknown", "partial", "complete"}:
        _require(not _complete(address), "unknown memory address has complete proof")
        return
    opcode = next(
        instruction["word"] >> 26
        for instruction in row["returnDisposition"]["evidence"]["instructions"]
        if instruction["pc"] == access["pc"]
    )
    _require(
        opcode not in {34, 38, 42, 46},
        "merge memory has no ordinary fixed-address proof",
    )
    if address["kind"] == "entry-register-relative-first-use/v1":
        _keys(
            address,
            set(
                "kind entryRegister offset equalityEvidence entryPc usePc "
                "operandPhase callPcs conditions".split()
            ),
            "first-use entry-relative original memory address",
        )
        _integer(address["entryRegister"], 31)
        _require(address["entryRegister"] != 0, "memory relative entry register")
        for field in ("offset", "entryPc", "usePc"):
            _integer(address[field], 0xFFFFFFFF)
        _references(address["equalityEvidence"], 0xFFFFFFFF)
        calls = _references(address["callPcs"], 0xFFFFFFFF - 8)
        evidence = row["returnDisposition"]["evidence"]
        _require(
            address["entryPc"] == evidence["start"]
            and address["usePc"] == access["pc"]
            and address["operandPhase"] == "before-instruction"
            and address["conditions"]
            == ["first-reach-from-original-entry-before-unproved-control-crossing"]
            and calls == sorted(calls)
            and all(pc % 4 == 0 for pc in calls)
            and not _complete(row["originalCallSummary"]["effects"])
            and not _complete(row["originalCallSummary"]["declaration"])
            and not _complete(
                row["originalCallSummary"].get(
                    "noMemoryWrites", {"kind": "partial", "remaining": ["unresolved"]}
                )
            ),
            "first-use address domain or ordinary authority differs",
        )
        prefix = _entry_flow(row, frame_uses=frame_uses, use_pc=access["pc"])
        _require(
            prefix[2] == calls
            and (
                address["entryRegister"],
                address["offset"],
                address["equalityEvidence"],
            )
            == _entry_relative_flow_address(row, access, prefix, first_use=True),
            "reported first-use address differs from closed original prefix",
        )
        return
    if address["kind"] == "entry-register-bounded-flow/v1":
        _keys(
            address,
            {
                "kind",
                "entryRegister",
                "offset",
                "indexLower",
                "indexUpper",
                "boundEvidence",
            },
            "bounded entry-relative original memory address",
        )
        _integer(address["entryRegister"], 31)
        for field in ("offset", "indexLower", "indexUpper"):
            _integer(address[field], 0xFFFFFFFF)
        _references(address["boundEvidence"], 0xFFFFFFFF)
        _require(
            address == _bounded_entry_flow_address(row, access, entry_flow),
            "reported bounded address differs from closed original flow",
        )
        return
    if address["kind"] in {
        "entry-register-relative-prefix/v1",
        "entry-register-relative-flow/v1",
    }:
        _keys(
            address,
            {"kind", "entryRegister", "offset", "equalityEvidence"},
            "entry-relative original memory address",
        )
        _integer(address["entryRegister"], 31)
        _require(address["entryRegister"] != 0, "memory relative entry register")
        _integer(address["offset"], 0xFFFFFFFF)
        _references(address["equalityEvidence"], 0xFFFFFFFF)
        _require(
            (
                address["entryRegister"],
                address["offset"],
                address["equalityEvidence"],
            )
            == (
                _entry_relative_prefix_address(row, access)
                if address["kind"] == "entry-register-relative-prefix/v1"
                else _entry_relative_flow_address(row, access, entry_flow)
            ),
            "reported relative address differs from closed original prefix"
            if address["kind"] == "entry-register-relative-prefix/v1"
            else "reported relative address differs from closed original flow",
        )
        return
    first_use = address["kind"] == "instruction-fixed-first-use/v1"
    fields = {"kind", "base", "value", "equalityEvidence", "alignment", "fileBytes"}
    if first_use:
        fields.update({"entryPc", "usePc", "operandPhase", "callPcs", "conditions"})
    _keys(address, fields, "fixed original memory address")
    _require(
        address["kind"]
        in {
            "entry-fallthrough-prefix/v1",
            "instruction-fixed-flow/v1",
            "instruction-fixed-first-use/v1",
        },
        "original address rule",
    )
    if first_use:
        evidence = row["returnDisposition"]["evidence"]
        for field in ("entryPc", "usePc"):
            _integer(address[field], 0xFFFFFFFF)
        _require(
            address["entryPc"] == evidence["start"]
            and address["usePc"] == access["pc"]
            and address["operandPhase"] == "before-instruction"
            and address["callPcs"] == []
            and address["conditions"]
            == ["first-reach-from-original-entry-before-unproved-control-crossing"]
            and not _complete(row["originalCallSummary"]["effects"])
            and not _complete(row["originalCallSummary"]["declaration"])
            and not _complete(row["originalCallSummary"]["noMemoryWrites"])
            and type(address["fileBytes"]) is dict
            and address["fileBytes"].get("kind") in {"partial", "unknown"}
            and not _complete(address["fileBytes"]),
            "fixed first-use domain or ordinary authority differs",
        )
        entry_flow = _entry_flow(row, frame_uses=frame_uses, use_pc=access["pc"])
        _require(not entry_flow[2], "fixed first-use address crosses a call")
    for field in ("base", "value"):
        _integer(address[field], 0xFFFFFFFF)
    _references(address["equalityEvidence"], 0xFFFFFFFF)
    if address["kind"] == "instruction-fixed-flow/v1":
        _require(entry_flow is not None, "fixed address lacks complete original flow")
    base, value, pcs = _memory_prefix_address(
        row,
        access,
        entry_flow
        if first_use or address["kind"] == "instruction-fixed-flow/v1"
        else None,
        first_use=first_use,
    )
    _require(
        (address["base"], address["value"], address["equalityEvidence"])
        == (base, value, pcs),
        "reported address differs from closed original prefix",
    )
    size = access["width"] // 8
    aligned = _complete(address["alignment"])
    _require(aligned == (value % size == 0), "original access alignment differs")
    backing = address["fileBytes"]
    _require(
        type(backing) is dict and type(backing.get("kind")) is str,
        "memory file-byte shape",
    )
    if backing["kind"] in {"unknown", "partial", "complete"}:
        _require(not _complete(backing), "unavailable backing has complete proof")
        return
    _keys(
        backing,
        {"kind", "mappingStart", "mappingBytes", "payloadOffset", "bytes"},
        "original file-backed bytes",
    )
    _require(backing["kind"] == "file-backed-initial-bytes", "initial-byte rule")
    _integer(backing["mappingStart"], 0xFFFFFFFF)
    _integer(backing["mappingBytes"], 16 << 20)
    _integer(backing["payloadOffset"], 16 << 20)
    actual = _rows(backing["bytes"], 4, "initial bytes")
    for byte in actual:
        _integer(byte, 255)
    if context is not None:
        _require(images is not None, "mounted backing lacks verified images")
        canonical = (value & 0x1FFFFFFF) | 0x80000000
        interval = next(
            (
                span
                for span in context["visible_intervals"]
                if span["start"] <= canonical < span["end"]
            ),
            None,
        )
        _require(
            value & 0xE0000000 in {0, 0x80000000, 0xA0000000}
            and canonical < 0x80200000
            and interval is not None
            and size <= interval["end"] - canonical,
            "memory backing lacks one complete visible RAM span",
        )
        payload, load = images[interval["image_id"]]
        offset = interval["payload_offset"] + canonical - interval["start"]
    else:
        offset = value - load
    _require(
        aligned
        and value + size <= 1 << 32
        and backing["mappingStart"] == load
        and backing["mappingBytes"] == len(payload)
        and backing["payloadOffset"] == offset
        and 0 <= offset <= len(payload) - size
        and len(actual) == size
        and bytes(actual) == payload[offset : offset + size],
        "memory span/initial bytes differ from complete original payload",
    )
