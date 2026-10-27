"""Prove complete static effects of one reviewed byte-lookup consumer body.

One instruction-transfer owner executes the frozen original words:

* pending MIPS-I loads keep the old destination readable during the next
  instruction and settle at every instruction boundary, including branches, jumps
  and calls, so a delayed load used as an operand, a slot value or a table index
  follows the original timing;
* a call links ``ra`` with ``pc+8`` before its delay slot runs, commits a slot
  load at that boundary, and then applies the caller-saved clobbers;
* shared expression nodes give every proved value one identity, so a receipt can
  cite the instruction that produced it instead of a compressed summary;
* ordered memory events and opaque ``jal`` boundaries stay explicit, including
  the caller-saved clobbers and the distinct ``v0``/``v1`` results;
* natural loops are proved structurally and replaced by one symbolic iteration
  instead of being unrolled: every continuing path must execute the step, mask and
  comparison, a carried register is either proved as a relation on the induction
  value or havocked, and loop-written exit values are havocked unless they cannot
  depend on the iteration;
* the ``values`` table is derived from the emitted receipt itself, so every
  reference a section makes is present.

Acyclic control flow is enumerated as bounded paths: every reviewed body here has
at most three sentinel guards outside its loops, and the alternative bound refuses
anything wider.  Unsupported encodings, transfers, addresses, loop shapes, and
resource limits raise ``ValueError`` with the exact unsupported obligation, which
the byte-lookup capture turns into a named consumer gap.
"""

from __future__ import annotations

import json
import struct
from dataclasses import dataclass

from harness.domain.mips import word_fields

REGISTER_NAMES = (
    "zero",
    "at",
    "v0",
    "v1",
    "a0",
    "a1",
    "a2",
    "a3",
    "t0",
    "t1",
    "t2",
    "t3",
    "t4",
    "t5",
    "t6",
    "t7",
    "s0",
    "s1",
    "s2",
    "s3",
    "s4",
    "s5",
    "s6",
    "s7",
    "t8",
    "t9",
    "k0",
    "k1",
    "gp",
    "sp",
    "fp",
    "ra",
)

SHARED_PREMISES = (
    "Normal mapped memory completion for every observed access.",
    "Instruction bytes, address mappings and the reviewed layout stay unchanged "
    "while this body runs.",
    "Each direct call returns normally under O32: callee-saved registers, SP and "
    "the caller save cells this body writes are preserved.",
    "Wherever a proven address is treated as distinct from other memory, stack, "
    "code and data nonaliasing is a stated premise, not a proved fact.",
)

SCOPE = (
    "Complete static consumer body under the declared premises. Every original "
    "word, control edge, delayed load, memory event and direct call is accounted "
    "for, but no callee effect, called return value, object extent, safe table "
    "index, storage ownership, initializer identity or gameplay meaning is proved."
)

_LIMITS = {
    "body_words": 256,
    "loops": 2,
    "calls": 11,
    "edges": 512,
    "nodes": 4096,
    "alternatives": 64,
    "walk_steps": 4096,
}

_LOADS = {0x21: "lh", 0x23: "lw", 0x24: "lbu", 0x25: "lhu"}
_STORES = {0x28: "sb", 0x2B: "sw"}
_WIDTHS = {"lh": 2, "lw": 4, "lbu": 1, "lhu": 2, "sb": 1, "sw": 4}
_SIGNED = frozenset({"lh"})
_UNSIGNED_BYTE = "lbu"
_CALL_CLOBBERED = (1, 2, 3, 4, 5, 6, 7, *range(8, 16), 24, 25, 26, 27, 28)
_RETURN_REGISTER = 31
_STACK_POINTER = 29
_ZERO = 0


def _hex(value: int) -> str:
    return f"0x{value & 0xFFFFFFFF:08X}"


def _name(register: int) -> str:
    return REGISTER_NAMES[register]


def _branch_target(fields, address: int) -> int | None:
    """Return the absolute target of one already-extracted branch, else None."""

    if fields.opcode not in (0x04, 0x05):
        return None
    return (address + 4 + (fields.signed_immediate << 2)) & 0xFFFFFFFF


def _jump_target(fields, address: int) -> int:
    return (((address + 4) & 0xF0000000) | (fields.target26 << 2)) & 0xFFFFFFFF


_MNEMONICS = {
    0x00: "sll",
    0x03: "sra",
    0x08: "jr",
    0x21: "addu",
    0x23: "subu",
    0x2B: "sltu",
}
_OPCODES = {
    0x02: "j",
    0x03: "jal",
    0x04: "beq",
    0x05: "bne",
    0x09: "addiu",
    0x0B: "sltiu",
    0x0C: "andi",
    0x0E: "xori",
    0x0F: "lui",
    **_LOADS,
    **_STORES,
}


def _mnemonic(fields, address: int) -> str:
    """Name one supported encoding, refusing anything outside the grammar."""

    if fields.word == 0:
        return "nop"
    name = (
        _MNEMONICS.get(fields.funct)
        if fields.opcode == 0
        else _OPCODES.get(fields.opcode)
    )
    if name is None:
        raise ValueError(
            f"consumer body has an unsupported instruction at {_hex(address)}"
        )
    return name


def _alias(fields) -> str | None:
    """Name the degenerate encodings that only read as one operation."""

    if fields.opcode == 0 and fields.funct == 0x21 and fields.rt == 0:
        return "move"
    if fields.opcode == 0 and fields.funct == 0x23 and fields.rs == 0:
        return "negu"
    if fields.opcode == 0x05 and fields.rt == 0:
        return "bnez"
    if fields.opcode == 0x04 and fields.rt == 0:
        return "beqz"
    return None


def _destination(fields) -> int | None:
    """Return the written register of one instruction, ignoring ``$zero``."""

    if fields.opcode == 0:
        if fields.funct == 0x08:
            return None
        written = fields.rd
    elif fields.opcode in _STORES or fields.opcode in (0x02, 0x04, 0x05):
        written = 0
    elif fields.opcode == 0x03:
        written = _RETURN_REGISTER
    else:
        written = fields.rt
    return written or None


def _read_registers(fields) -> tuple[int, ...]:
    """Return the registers one instruction reads, without implicit operands."""

    if fields.word == 0:
        return ()
    if fields.opcode == 0:
        if fields.funct in (0x00, 0x02, 0x03):
            return (fields.rt,)
        if fields.funct == 0x08:
            return (fields.rs,)
        return (fields.rs, fields.rt)
    if fields.opcode in _STORES or fields.opcode in (0x04, 0x05):
        return (fields.rs, fields.rt)
    if fields.opcode in _LOADS:
        return (fields.rs,)
    if fields.opcode == 0x0F:
        return ()
    if fields.opcode in (0x02, 0x03):
        return ()
    return (fields.rs,)


# ------------------------------------------------------------ expression graph


class _Graph:
    """Interned expression nodes shared by every state of one consumer body."""

    def __init__(self) -> None:
        self.nodes: list[dict] = []
        self._ids: dict[str, int] = {}

    def add(self, node: dict) -> int:
        key = json.dumps(node, sort_keys=True)
        known = self._ids.get(key)
        if known is not None:
            return known
        if len(self.nodes) >= _LIMITS["nodes"]:
            raise ValueError(
                f"consumer body exceeds {_LIMITS['nodes']} shared expression nodes"
            )
        self._ids[key] = len(self.nodes)
        self.nodes.append(node)
        return len(self.nodes) - 1

    def get(self, node: int) -> dict:
        return self.nodes[node]

    def constant(self, value: int) -> int:
        return self.add({"kind": "constant", "value": value & 0xFFFFFFFF})

    def entry(self, register: int) -> int:
        return self.add({"kind": "entry_register", "register": register})

    def entry_offset(self, register: int, offset: int) -> int:
        if offset == 0:
            return self.entry(register)
        return self.add(
            {"kind": "entry_offset", "register": register, "offset": offset}
        )

    def base(self, constant: int, index: int) -> int:
        return self.add(
            {"kind": "indexed_base", "constant": constant & 0xFFFFFFFF, "index": index}
        )

    def unknown(self, reason: str) -> int:
        return self.add({"kind": "unknown", "reason": reason})

    def carried(self, loop: str, register: int) -> int:
        return self.add({"kind": "loop_carried", "loop": loop, "register": register})

    def induction(self, loop: str, register: int, bound: int) -> int:
        return self.add(
            {"kind": "induction", "loop": loop, "register": register, "bound": bound}
        )

    def read(
        self,
        site: str,
        opcode: str,
        address: dict,
        loop: str | None,
        forwarded_from: str | None = None,
    ) -> int:
        node = {
            "kind": "memory_read",
            "site": site,
            "opcode": opcode,
            "address": address,
            "width": _WIDTHS[opcode],
            "signed": opcode in _SIGNED,
            "loop": loop,
        }
        if forwarded_from is not None:
            node["forwarded_from"] = forwarded_from
            node["premise"] = (
                "protected caller save cell across normal-return calls, with stack "
                "and materialized data addresses treated as distinct"
            )
        return self.add(node)

    def call_result(
        self, site: str, callee: str, register: int, loop: str | None
    ) -> int:
        return self.add(
            {
                "kind": "call_result",
                "site": site,
                "callee": callee,
                "register": register,
                "loop": loop,
            }
        )

    def call_clobber(
        self, site: str, callee: str, register: int, loop: str | None
    ) -> int:
        return self.add(
            {
                "kind": "call_clobber",
                "site": site,
                "callee": callee,
                "register": register,
                "loop": loop,
            }
        )

    def operation(self, name: str, left: int, right: int) -> int:
        if name == "add":
            return self.add_values(left, right)
        return self.add(
            {"kind": "expression", "operation": name, "left": left, "right": right}
        )

    def add_immediate(self, left: int, immediate: int) -> int:
        item = self.get(left)
        kind = item["kind"]
        if kind == "constant":
            return self.constant(item["value"] + immediate)
        if kind == "indexed_base":
            return self.base(item["constant"] + immediate, item["index"])
        if kind == "entry_register":
            return self.entry_offset(item["register"], immediate)
        if kind == "entry_offset":
            return self.entry_offset(item["register"], item["offset"] + immediate)
        return self.add(
            {
                "kind": "expression",
                "operation": "add",
                "left": left,
                "right": self.constant(immediate),
            }
        )

    def add_values(self, left: int, right: int) -> int:
        """Combine one materialized constant with one dynamic operand."""

        first, second = self.get(left), self.get(right)
        if first["kind"] == "constant" and second["kind"] == "constant":
            return self.constant(first["value"] + second["value"])
        if second["kind"] == "constant" and second["value"] == 0:
            return left
        if first["kind"] == "constant" and first["value"] == 0:
            return right
        if second["kind"] == "constant":
            return self.offset_value(left, second["value"])
        if first["kind"] == "constant":
            return self.offset_value(right, first["value"])
        return self.add(
            {"kind": "expression", "operation": "add", "left": left, "right": right}
        )

    def offset_value(self, left: int, offset: int) -> int:
        """Materialize one constant plus a dynamic operand as an address base."""

        item = self.get(left)
        if item["kind"] == "indexed_base":
            return self.base(item["constant"] + offset, item["index"])
        return self.base(offset, left)


def _address(graph: _Graph, node: int, offset: int) -> dict:
    """Project one value onto the memory address it is used with."""

    item = graph.get(node)
    kind = item["kind"]
    if kind == "constant":
        return {"kind": "absolute", "address": (item["value"] + offset) & 0xFFFFFFFF}
    if kind == "indexed_base":
        return {
            "kind": "indexed",
            "fixed_base": (item["constant"] + offset) & 0xFFFFFFFF,
            "index": item["index"],
        }
    if kind == "entry_register":
        return {
            "kind": "entry_relative",
            "register": item["register"],
            "offset": offset,
        }
    if kind == "entry_offset":
        return {
            "kind": "entry_relative",
            "register": item["register"],
            "offset": item["offset"] + offset,
        }
    return {"kind": "symbolic", "source": node, "offset": offset}


def _targets_selected(address: dict, selected: int) -> bool:
    """Prove that one decoded address reaches the selected object."""

    if address["kind"] == "absolute":
        return address["address"] == selected
    if address["kind"] == "indexed":
        return address["fixed_base"] == selected
    return False


def _render_address(address: dict) -> dict:
    rendered = {"kind": address["kind"]}
    if address["kind"] == "absolute":
        rendered["address"] = _hex(address["address"])
    elif address["kind"] == "indexed":
        rendered["fixed_base"] = _hex(address["fixed_base"])
        rendered["index"] = address["index"]
    elif address["kind"] == "entry_relative":
        rendered["register"] = _name(address["register"])
        rendered["offset"] = address["offset"]
    else:
        rendered["source"] = address["source"]
        rendered["offset"] = address["offset"]
    return rendered


# ------------------------------------------------------------------ execution


@dataclass(frozen=True)
class _State:
    """One path's committed registers, delayed load and protected save cells."""

    registers: tuple[int, ...]
    pending: tuple[int, int] | None = None
    saved: tuple[tuple[int, int, int, str], ...] = ()


@dataclass(frozen=True)
class _Outcome:
    """One bounded-walk result, with the state sampled at its transfer."""

    label: str
    index: int
    state: _State
    guards: tuple[str, ...]
    sampled: _State | None = None


def _retire(state: _State) -> _State:
    """Retire one pending delayed load at an instruction boundary."""

    if state.pending is None:
        return state
    registers = list(state.registers)
    registers[state.pending[0]] = state.pending[1]
    return _State(tuple(registers), None, state.saved)


_FORWARDABLE = {"lw": 4, "lbu": 1}
# Fields that carry a value-node identity in every emitted projection.  The receipts
# are validated against this list so a referenced node can never be missing from the
# ``values`` table.
_NODE_FIELDS = frozenset(
    {
        "node",
        "left",
        "right",
        "source",
        "value",
        "operand",
        "exit_value",
        "bound",
        "v0",
        "v1",
        "relation",
        "values",
    }
)


def _cell(
    state: _State, register: int, offset: int, width: int
) -> tuple[int, str] | None:
    """Return the value and store site of one protected caller save cell."""

    for item_register, item_offset, item_width, node, site in state.saved:
        if (item_register, item_offset, item_width) == (register, offset, width):
            return node, site
    return None


def _overlaps(item: tuple[int, int, int, int, str], offset: int, width: int) -> bool:
    """Decide whether one stored byte range intersects another on the same base."""

    return item[1] < offset + width and offset < item[1] + item[2]


def _save(
    saved: tuple[tuple[int, int, int, int, str], ...],
    register: int,
    offset: int,
    width: int,
    node: int,
    site: str,
) -> tuple[tuple[int, int, int, int, str], ...]:
    """Record one protected save cell, dropping every overlapping earlier write."""

    kept = [item for item in saved if not _overlaps(item, offset, width)]
    kept.append((register, offset, width, node, site))
    return tuple(sorted(kept))


class _Body:
    """One frozen consumer body and every effect its instructions prove."""

    def __init__(self, start: int, words: list[int], selected: int) -> None:
        self.start = start
        self.words = words
        self.selected = selected
        self.fields = [word_fields(word) for word in words]
        self.graph = _Graph()
        self.records: list[dict] = []
        self.events: list[dict] = []
        self.call_sites: dict[str, dict] = {}
        self.guards: dict[str, dict] = {}
        self.loops: list[dict] = []
        self.loop_ranges: list[tuple[int, int]] = []
        self.loop_headers: dict[int, dict] = {}
        self.loop_exits: dict[int, tuple[_State, _State]] = {}
        self.returns: list[dict] = []
        self.covered: set[int] = set()
        self.delay_slots: frozenset[int] = frozenset()
        self.value_sites: dict[int, str] = {}

    # -- bookkeeping ------------------------------------------------------

    def address(self, index: int) -> int:
        return self.start + index * 4

    def _value(self, index: int, node: int) -> int:
        self.value_sites.setdefault(node, _hex(self.address(index)))
        return node

    def _index(self, target: int) -> int:
        offset = target - self.start
        if offset % 4:
            raise ValueError(f"consumer target {_hex(target)} is not word aligned")
        return offset // 4

    # -- static shape -----------------------------------------------------

    def _describe(self) -> None:
        """Classify every word and its declared successors before execution."""

        count = len(self.words)
        slots: set[int] = set()
        transfers: set[int] = set()
        for index, fields in enumerate(self.fields):
            _mnemonic(fields, self.address(index))
            control = fields.opcode in (0x02, 0x03, 0x04, 0x05) or (
                fields.opcode == 0 and fields.funct == 0x08
            )
            if not control:
                continue
            transfers.add(index)
            if index + 1 >= count:
                raise ValueError(
                    f"consumer transfer at {_hex(self.address(index))} has no "
                    "ordinary delay slot"
                )
            slots.add(index + 1)
        for index in sorted(slots):
            if index in transfers:
                raise ValueError(
                    f"consumer delay slot at {_hex(self.address(index))} is itself a "
                    "transfer"
                )
        self.delay_slots = frozenset(slots)
        edges = 0
        for index, fields in enumerate(self.fields):
            mnemonic = _mnemonic(fields, self.address(index))
            record = {
                "address": _hex(self.address(index)),
                "opcode": mnemonic,
                "reads": [_name(register) for register in _read_registers(fields)],
                "writes": [
                    _name(register)
                    for register in [_destination(fields)]
                    if register is not None
                ],
                "successors": [],
            }
            alias = _alias(fields)
            if alias is not None and alias != mnemonic:
                record["alias"] = alias
            if index in slots:
                record["role"] = "delay_slot"
            elif index in transfers:
                record["role"] = "transfer"
                record["successors"] = self._successors(index, fields)
            else:
                record["successors"] = [
                    {"to": _hex(self.address(index + 1)), "kind": "fall_through"}
                ]
            edges += len(record["successors"])
            self.records.append(record)
        if edges > _LIMITS["edges"]:
            raise ValueError(
                f"consumer body exceeds {_LIMITS['edges']} control-flow edges"
            )

    def _successors(self, index: int, fields) -> list[dict]:
        address = self.address(index)
        for target in (
            _branch_target(fields, address),
            _jump_target(fields, address) if fields.opcode == 0x02 else None,
        ):
            if target is None:
                continue
            if self._index(target) in self.delay_slots:
                raise ValueError(
                    f"consumer transfer at {_hex(address)} targets the delay slot at "
                    f"{_hex(target)}"
                )
        if fields.opcode == 0 and fields.funct == 0x08:
            if fields.rs != _RETURN_REGISTER:
                raise ValueError(
                    f"consumer body has an indirect jump at {_hex(address)}"
                )
            return [{"to": "entry_ra", "kind": "return"}]
        if fields.opcode == 0x02:
            return [{"to": _hex(_jump_target(fields, address)), "kind": "jump"}]
        if fields.opcode == 0x03:
            return [{"to": _hex(address + 8), "kind": "call_return"}]
        target = _branch_target(fields, address)
        assert target is not None
        return [
            {"to": _hex(target), "kind": "branch_taken"},
            {"to": _hex(address + 8), "kind": "branch_not_taken"},
        ]

    # -- loops ------------------------------------------------------------

    def _find_loops(self) -> None:
        found: list[tuple[int, int]] = []
        for index, fields in enumerate(self.fields):
            target = _branch_target(fields, self.address(index))
            if target is None or target > self.address(index):
                continue
            found.append((self._index(target), index))
        if len(found) > _LIMITS["loops"]:
            raise ValueError(
                f"consumer body exceeds {_LIMITS['loops']} nonnested natural loops"
            )
        for position, (header, latch) in enumerate(sorted(found)):
            if position and header <= self.loop_ranges[-1][1]:
                raise ValueError("consumer body has nested or overlapping loops")
            self.loop_ranges.append((header, latch + 1))
            self.loop_headers[header] = {"header": header, "latch": latch}

    def _definition_before(self, register: int, index: int, low: int) -> int:
        """Return the reaching definition index of one register inside a loop."""

        for candidate in range(index - 1, low - 1, -1):
            fields = self.fields[candidate]
            if fields.opcode == 0x03 and register in _CALL_CLOBBERED:
                raise ValueError(
                    f"consumer loop reads {_name(register)} at "
                    f"{_hex(self.address(index))} after a call that may clobber it"
                )
            if _destination(fields) == register:
                return candidate
        raise ValueError(
            f"consumer loop reads {_name(register)} at "
            f"{_hex(self.address(index))} without a defining write inside the loop"
        )

    def _prove_loop(
        self, header: int, entry_state: _State, guards: tuple[str, ...]
    ) -> dict:
        latch = self.loop_headers[header]["latch"]
        slot = latch + 1
        key = _hex(self.address(header))
        for index, fields in enumerate(self.fields):
            if header <= index <= slot:
                continue
            target = _branch_target(fields, self.address(index))
            if target is None and fields.opcode == 0x02:
                target = _jump_target(fields, self.address(index))
            if target is not None and header <= self._index(target) <= slot:
                raise ValueError(
                    f"consumer loop at {key} has a second control entry at "
                    f"{_hex(self.address(index))}"
                )
        for index in range(header, latch + 1):
            fields = self.fields[index]
            target = _branch_target(fields, self.address(index))
            if target is None:
                continue
            target_index = self._index(target)
            if not header <= target_index <= slot:
                raise ValueError(
                    f"consumer loop at {key} has an exit branch at "
                    f"{_hex(self.address(index))} instead of the latch"
                )
        latch_fields = self.fields[latch]
        if latch_fields.opcode != 0x05 or latch_fields.rt != _ZERO:
            raise ValueError(
                f"consumer loop latch at {_hex(self.address(latch))} is not a bnez "
                "on the proved induction comparison"
            )
        condition = latch_fields.rs
        compare = self._definition_before(condition, latch, header)
        compare_fields = self.fields[compare]
        if compare_fields.opcode != 0x0B and (
            compare_fields.opcode != 0 or compare_fields.funct != 0x2B
        ):
            raise ValueError(
                f"consumer loop latch at {key} does not test an unsigned comparison"
            )
        masked = compare_fields.rs
        mask = self._definition_before(masked, compare, header)
        mask_fields = self.fields[mask]
        if (
            mask_fields.opcode != 0x0C
            or mask_fields.rt != masked
            or mask_fields.immediate not in (0xFF, 0xFFFF)
        ):
            raise ValueError(
                f"consumer loop at {key} does not mask the induction register before "
                "its latch comparison"
            )
        induction = mask_fields.rs
        step = self._definition_before(induction, mask, header)
        step_fields = self.fields[step]
        if (
            step_fields.opcode != 0x09
            or step_fields.rs != induction
            or step_fields.rt != induction
            or step_fields.signed_immediate != 1
        ):
            raise ValueError(
                f"consumer loop at {key} has no proved unit induction step"
            )
        writers = [
            index
            for index in range(header, slot + 1)
            if _destination(self.fields[index]) == induction
        ]
        if writers != [step]:
            raise ValueError(
                f"consumer loop at {key} writes {_name(induction)} outside its single "
                "unit step"
            )
        initialize = self._definition_before(induction, header, 0)
        initial_fields = self.fields[initialize]
        zero_initialized = (
            initial_fields.opcode == 0
            and initial_fields.funct == 0x21
            and initial_fields.rs == _ZERO
            and initial_fields.rt == _ZERO
            and initial_fields.rd == induction
        ) or (
            initial_fields.opcode == 0x09
            and initial_fields.rs == _ZERO
            and initial_fields.rt == induction
            and initial_fields.signed_immediate == 0
        )
        if not zero_initialized:
            raise ValueError(
                f"consumer loop at {key} does not zero-initialize {_name(induction)}"
            )
        if entry_state.registers[induction] != self.graph.constant(0):
            raise ValueError(
                f"consumer loop at {key} is entered with a nonzero {_name(induction)}"
            )
        if compare_fields.opcode == 0x0B:
            bound = self.graph.constant(compare_fields.immediate)
            bound_kind = "immediate"
            iterations = compare_fields.immediate
            if not 1 <= iterations <= 255:
                raise ValueError(
                    f"consumer loop at {key} has an unusable immediate iteration bound"
                )
        else:
            bound = entry_state.registers[compare_fields.rt]
            node = self.graph.get(bound)
            if node["kind"] != "memory_read" or node["opcode"] != _UNSIGNED_BYTE:
                raise ValueError(
                    f"consumer loop at {key} compares against a value that is not a "
                    "cached unsigned byte count"
                )
            if not self._proves_nonzero(guards, bound):
                raise ValueError(
                    f"consumer loop at {key} has no guard proving its cached byte "
                    "count is nonzero at the entry"
                )
            bound_kind = "cached_unsigned_byte"
            iterations = 255
        return {
            "key": key,
            "header": header,
            "latch": latch,
            "slot": slot,
            "condition": condition,
            "compare": compare,
            "masked": masked,
            "mask": mask,
            "mask_immediate": mask_fields.immediate,
            "induction": induction,
            "step": step,
            "initialize": initialize,
            "bound": bound,
            "bound_kind": bound_kind,
            "iterations": iterations,
        }

    def _proves_nonzero(self, guards: tuple[str, ...], value: int) -> bool:
        """Decide whether one entered guard proves a value is not zero."""

        zero = self.graph.constant(0)
        for name in guards:
            guard = self.guards.get(name)
            if guard is None:
                continue
            if guard["left"] == value:
                other = guard["right"]
            elif guard["right"] == value:
                other = guard["left"]
            else:
                continue
            if other != zero:
                continue
            if guard["opcode"] == "beq" and guard["outcome"] == "not_taken":
                return True
            if guard["opcode"] == "bne" and guard["outcome"] == "taken":
                return True
        return False

    def _loop_effects(self, header: int, slot: int) -> tuple[set[int], set[int]]:
        """Return the registers a loop iteration writes and reads, slots included."""

        written: set[int] = set()
        read: set[int] = set()
        for index in range(header, slot + 1):
            fields = self.fields[index]
            read |= set(_read_registers(fields))
            destination = _destination(fields)
            if destination is not None:
                written.add(destination)
            if fields.opcode == 0x03:
                written |= set(_CALL_CLOBBERED)
        return written, read

    def _checkpoint(self) -> tuple:
        return (
            len(self.events),
            dict(self.guards),
            [dict(record) for record in self.records],
            set(self.covered),
            dict(self.value_sites),
            list(self.returns),
            dict(self.call_sites),
        )

    def _restore(self, checkpoint: tuple) -> None:
        events, guards, records, covered, value_sites, returns, call_sites = checkpoint
        del self.events[events:]
        self.guards = guards
        self.records = records
        self.covered = covered
        self.value_sites = value_sites
        self.returns = returns
        self.call_sites = call_sites

    def _probe_relations(
        self, proof: dict, entry_state: _State, carried: list[int]
    ) -> dict[int, int]:
        """Prove what each carried register holds when an iteration starts.

        A register whose value at the backedge is a function of the induction value
        alone carries that relation into the next iteration.  Replacing the stepped
        value by the induction value gives the entry relation, which must also hold
        for the first iteration.
        """

        if not carried:
            return {}
        key = proof["key"]
        induction_node = self.graph.induction(key, proof["induction"], proof["bound"])
        step_node = self.graph.add_immediate(induction_node, 1)
        registers = list(entry_state.registers)
        for register in carried:
            registers[register] = self.graph.carried(key, register)
        registers[proof["induction"]] = induction_node
        probe = _State(tuple(registers), None, entry_state.saved)
        checkpoint = self._checkpoint()
        try:
            outcomes = self._walk(
                proof["header"],
                probe,
                (),
                cut={
                    proof["header"]: "iteration_continues",
                    proof["latch"] + 2: "loop_exit",
                },
                loop=key,
                owner=proof["header"],
            )
        except ValueError:
            return {}
        finally:
            self._restore(checkpoint)
        continues = [
            outcome for outcome in outcomes if outcome.label == "iteration_continues"
        ]
        if not continues:
            return {}
        relations: dict[int, int] = {}
        for register in carried:
            values = {outcome.state.registers[register] for outcome in continues}
            if len(values) != 1:
                continue
            value = values.pop()
            if not self._depends_only_on(induction_node, value):
                continue
            relation = self._substitute(value, {step_node: induction_node})
            at_zero = self._substitute(
                relation, {induction_node: self.graph.constant(0)}
            )
            if at_zero != entry_state.registers[register]:
                continue
            relations[register] = relation
        return relations

    def _depends_only_on(self, allowed: int, value: int) -> bool:
        """Decide whether every leaf of a value is one node or a constant."""

        stack = [value]
        seen: set[int] = set()
        while stack:
            node = stack.pop()
            if node in seen:
                continue
            seen.add(node)
            if node == allowed:
                continue
            item = self.graph.get(node)
            kind = item["kind"]
            if kind == "constant":
                continue
            if kind == "expression":
                stack.extend((item["left"], item["right"]))
                continue
            if kind == "indexed_base":
                stack.append(item["index"])
                continue
            return False
        return True

    def _substitute(self, value: int, replacements: dict[int, int]) -> int:
        """Rebuild one expression with nodes replaced, interning the result."""

        memo: dict[int, int] = {}

        def rebuild(node: int) -> int:
            if node in replacements:
                return replacements[node]
            known = memo.get(node)
            if known is not None:
                return known
            item = self.graph.get(node)
            kind = item["kind"]
            if kind == "expression":
                result = self.graph.operation(
                    item["operation"], rebuild(item["left"]), rebuild(item["right"])
                )
            elif kind == "indexed_base":
                result = self.graph.base(item["constant"], rebuild(item["index"]))
            else:
                result = node
            memo[node] = result
            return result

        return rebuild(value)

    def _iteration_independent(self, key: str, value: int) -> bool:
        """Decide whether a value cannot depend on the arbitrary iteration."""

        stack = [value]
        seen: set[int] = set()
        while stack:
            node = stack.pop()
            if node in seen:
                continue
            seen.add(node)
            item = self.graph.get(node)
            kind = item["kind"]
            if kind == "constant":
                continue
            if kind == "unknown":
                return False
            if kind in ("induction", "loop_carried") and item["loop"] == key:
                return False
            if kind in ("call_result", "call_clobber") and item["loop"] == key:
                return False
            if kind == "memory_read":
                if item["loop"] == key:
                    return False
                address = item["address"]
                for field in ("index", "source"):
                    if isinstance(address.get(field), int):
                        stack.append(address[field])
                continue
            if kind == "expression":
                stack.extend((item["left"], item["right"]))
            elif kind == "indexed_base":
                stack.append(item["index"])
        return True

    def _summarize_loop(
        self, header: int, entry_state: _State, guards: tuple[str, ...] = ()
    ) -> _State:
        cached = self.loop_exits.get(header)
        if cached is not None:
            if cached[0] != entry_state:
                raise ValueError(
                    f"consumer loop at {_hex(self.address(header))} is entered from "
                    "more than one state"
                )
            return cached[1]
        proof = self._prove_loop(header, entry_state, guards)
        latch, induction, key = proof["latch"], proof["induction"], proof["key"]
        if entry_state.pending is not None:
            raise ValueError(
                f"consumer loop at {key} is entered with an unresolved delayed load"
            )
        written, read = self._loop_effects(header, proof["slot"])
        carried = sorted((written & read) - {induction})
        relations = self._probe_relations(proof, entry_state, carried)
        induction_node = self.graph.induction(key, induction, proof["bound"])
        registers = list(entry_state.registers)
        for register in carried:
            registers[register] = relations.get(
                register, self.graph.carried(key, register)
            )
        registers[induction] = induction_node
        iteration_state = _State(tuple(registers), None, entry_state.saved)
        outcomes = self._walk(
            header,
            iteration_state,
            (),
            cut={header: "iteration_continues", latch + 2: "loop_exit"},
            loop=key,
            owner=header,
        )
        exits = [outcome for outcome in outcomes if outcome.label == "loop_exit"]
        continues = [
            outcome for outcome in outcomes if outcome.label == "iteration_continues"
        ]
        if not exits:
            raise ValueError(f"consumer loop at {key} has no proved exit")
        if not continues:
            raise ValueError(f"consumer loop at {key} has no executed backedge")
        step_node = self.graph.add_immediate(induction_node, 1)
        mask_node = self.graph.operation(
            "and", step_node, self.graph.constant(proof["mask_immediate"])
        )
        compare_node = self.graph.operation(
            "less_than_unsigned", mask_node, proof["bound"]
        )
        for outcome in continues:
            sampled = outcome.sampled or outcome.state
            if sampled.registers[induction] != step_node:
                raise ValueError(
                    f"consumer loop at {key} reaches its backedge at "
                    f"{_hex(self.address(outcome.index if outcome.index < len(self.words) else latch))} "
                    "without executing the unit step"
                )
            if (
                proof["masked"] != proof["condition"]
                and sampled.registers[proof["masked"]] != mask_node
            ):
                raise ValueError(
                    f"consumer loop at {key} reaches its backedge without masking the "
                    "stepped induction"
                )
            if sampled.registers[proof["condition"]] != compare_node:
                raise ValueError(
                    f"consumer loop at {key} reaches its backedge without comparing the "
                    "masked stepped induction"
                )
        cells = {
            (event["address"]["register"], event["address"]["offset"])
            for event in self.events
            if event.get("loop") == key
            and event["kind"] == "write"
            and event["address"]["kind"] == "entry_relative"
        }
        exited = self._merge_loop_state(
            exits, key, induction, proof["bound"], written, cells
        )
        self.loops.append(
            self._loop_record(proof, carried, relations, exits, continues)
        )
        self.loop_exits[header] = (entry_state, exited)
        return exited

    def _merge_loop_state(
        self,
        exits: list[_Outcome],
        key: str,
        induction: int,
        bound: int,
        written: set[int],
        cells: set[tuple[int, int]],
    ) -> _State:
        """Merge the exits, havocking every value that depends on the iteration."""

        merged = list(exits[0].state.registers)
        for register in range(len(merged)):
            if register == induction or register not in written:
                continue
            value = exits[0].state.registers[register]
            agrees = all(
                outcome.state.registers[register] == value for outcome in exits[1:]
            )
            if agrees and self._iteration_independent(key, value):
                merged[register] = value
            else:
                merged[register] = self.graph.unknown(
                    f"written by an iteration of the loop at {key}"
                )
        for outcome in exits:
            if outcome.state.pending is not None:
                raise ValueError(
                    f"consumer loop at {key} leaves an unresolved delayed load"
                )
        merged[induction] = bound
        common = [
            cell
            for cell in exits[0].state.saved
            if (cell[0], cell[1]) not in cells
            and self._iteration_independent(key, cell[3])
            and all(
                _cell(outcome.state, cell[0], cell[1], cell[2]) == (cell[3], cell[4])
                for outcome in exits[1:]
            )
        ]
        return _State(tuple(merged), None, tuple(common))

    def _loop_record(
        self,
        proof: dict,
        carried: list[int],
        relations: dict[int, int],
        exits: list,
        continues: list,
    ) -> dict:
        key = proof["key"]
        record = {
            "header": key,
            "latch": _hex(self.address(proof["latch"])),
            "delay_slot": _hex(self.address(proof["slot"])),
            "exit": _hex(self.address(proof["latch"] + 2)),
            "entry": _hex(self.address(proof["header"] - 1)),
            "words": proof["latch"] - proof["header"] + 2,
            "backedge": {
                "site": _hex(self.address(proof["latch"])),
                "opcode": "bnez",
                "condition": _name(proof["condition"]),
                "target": key,
                "delay_slot": _hex(self.address(proof["slot"])),
            },
            "induction": {
                "register": _name(proof["induction"]),
                "initialized": _hex(self.address(proof["initialize"])),
                "entry_value": 0,
                "step": _hex(self.address(proof["step"])),
                "step_immediate": 1,
                "mask": {
                    "site": _hex(self.address(proof["mask"])),
                    "immediate": proof["mask_immediate"],
                },
                "compare": {
                    "site": _hex(self.address(proof["compare"])),
                    "operation": "less_than_unsigned",
                },
                "exit_value": proof["bound"],
                "positions": {"min": 0, "max": proof["iterations"] - 1},
            },
            "bound": {
                "kind": proof["bound_kind"],
                "node": proof["bound"],
                "maximum_iterations": proof["iterations"],
            },
            "carried": [
                {"register": _name(register), "relation": relations.get(register)}
                for register in carried
            ],
            "havoc": [
                _name(register) for register in carried if register not in relations
            ],
            "iteration": {
                "exits": len(exits),
                "continues": len(continues),
                "memory_events": sum(
                    1 for event in self.events if event.get("loop") == key
                ),
            },
            "premise": (
                "One arbitrary iteration with an induction value in "
                f"0..{proof['iterations'] - 1} stands for every iteration; values "
                "written by an earlier iteration are not assumed equal."
            ),
        }
        recurrences = self._recurrences(proof)
        if recurrences:
            record["recurrences"] = recurrences
        return record

    def _recurrences(self, proof: dict) -> list[dict]:
        """Report a loop-carried memory cell the proved iteration updates."""

        reads = [
            event
            for event in self.events
            if event.get("loop") == proof["key"] and event["kind"] == "read"
        ]
        writes = [
            event
            for event in self.events
            if event.get("loop") == proof["key"] and event["kind"] == "write"
        ]
        found: list[dict] = []
        for write in writes:
            if write["address"]["kind"] != "absolute":
                continue
            cell = write["address"]["address"]
            for read in reads:
                if (
                    read["address"]["kind"] != "absolute"
                    or read["address"]["address"] != cell
                ):
                    continue
                if read["node"] not in self._cone(write["value"]):
                    continue
                stored = self.graph.get(write["value"])
                found.append(
                    {
                        "cell": _hex(cell),
                        "read_site": read["site"],
                        "write_site": write["site"],
                        "opcode": read["opcode"],
                        "stored_width": write["width"],
                        "stored_operation": stored.get("operation"),
                        "initializer": self._initializer(cell),
                        "recurrence": (
                            "the bytes one iteration stores are the bytes the next "
                            "iteration reads from the same address"
                        ),
                        "premise": (
                            "this body's own store is the only writer of that cell "
                            "and no asynchronous writer changes it"
                        ),
                    }
                )
        return found

    def _initializer(self, cell: int) -> dict | None:
        for event in self.events:
            if (
                event["kind"] == "write"
                and event["address"]["kind"] == "absolute"
                and event["address"]["address"] == cell
                and event.get("loop") is None
            ):
                node = self.graph.get(event["value"])
                return {
                    "site": event["site"],
                    "kind": node["kind"],
                    "constant": node.get("value"),
                }
        return None

    def _cone(self, root: int) -> set[int]:
        seen: set[int] = set()
        stack = [root]
        while stack:
            node = stack.pop()
            if node in seen:
                continue
            seen.add(node)
            item = self.graph.get(node)
            if item["kind"] == "expression":
                stack.extend((item["left"], item["right"]))
            elif item["kind"] == "indexed_base":
                stack.append(item["index"])
        return seen

    # -- transfer ---------------------------------------------------------

    def _transfer(
        self, state: _State, index: int, guards: tuple[str, ...], loop: str | None
    ) -> _State:
        """Apply one ordinary instruction, keeping its delayed load timing."""

        if not 0 <= index < len(self.words):
            raise ValueError("consumer body ends without a complete delay slot")
        fields = self.fields[index]
        self.covered.add(index)
        address = self.address(index)
        graph = self.graph
        record = self.records[index]
        value: int | None = None
        destination = _destination(fields)
        delayed = False
        saved = state.saved
        opcode = fields.opcode
        if fields.word == 0:
            pass
        elif opcode == 0x0F and fields.rs == _ZERO:
            value = graph.constant(fields.immediate << 16)
        elif opcode == 0x09:
            value = graph.add_immediate(
                state.registers[fields.rs], fields.signed_immediate
            )
        elif opcode == 0x0C:
            value = graph.operation(
                "and", state.registers[fields.rs], graph.constant(fields.immediate)
            )
        elif opcode == 0x0E:
            value = graph.operation(
                "xor", state.registers[fields.rs], graph.constant(fields.immediate)
            )
        elif opcode == 0x0B:
            value = graph.operation(
                "less_than_unsigned",
                state.registers[fields.rs],
                graph.constant(fields.immediate),
            )
        elif opcode == 0 and fields.funct == 0x00:
            value = graph.operation(
                "shift_left", state.registers[fields.rt], graph.constant(fields.shift)
            )
        elif opcode == 0 and fields.funct == 0x03:
            value = graph.operation(
                "shift_right_arithmetic",
                state.registers[fields.rt],
                graph.constant(fields.shift),
            )
        elif opcode == 0 and fields.funct in (0x21, 0x23, 0x2B):
            name = {0x21: "add", 0x23: "sub_unsigned", 0x2B: "less_than_unsigned"}[
                fields.funct
            ]
            value = graph.operation(
                name, state.registers[fields.rs], state.registers[fields.rt]
            )
        elif opcode in _LOADS:
            name = _LOADS[opcode]
            decoded = _address(
                graph, state.registers[fields.rs], fields.signed_immediate
            )
            forwarded = self._forward(state, decoded, name)
            node = graph.read(
                _hex(address),
                name,
                decoded,
                loop,
                forwarded[1] if forwarded else None,
            )
            record["memory"] = self._record_event(
                index,
                {
                    "kind": "read",
                    "opcode": name,
                    "address": decoded,
                    "width": _WIDTHS[name],
                    "node": node,
                    "loop": loop,
                    **({"forwarded_from": forwarded[1]} if forwarded else {}),
                },
                guards,
            )
            if forwarded is not None:
                value = forwarded[0]
            else:
                value = node
            delayed = True
        elif opcode in _STORES:
            name = _STORES[opcode]
            decoded = _address(
                graph, state.registers[fields.rs], fields.signed_immediate
            )
            self._check_store(index, decoded, name)
            record["memory"] = self._record_event(
                index,
                {
                    "kind": "write",
                    "opcode": name,
                    "address": decoded,
                    "width": _WIDTHS[name],
                    "value": state.registers[fields.rt],
                    "loop": loop,
                },
                guards,
            )
            if (
                decoded["kind"] == "entry_relative"
                and decoded["register"] == _STACK_POINTER
                and decoded["offset"] < 0
            ):
                saved = _save(
                    saved,
                    decoded["register"],
                    decoded["offset"],
                    _WIDTHS[name],
                    state.registers[fields.rt],
                    _hex(address),
                )
            elif decoded["kind"] in ("entry_relative", "symbolic"):
                # A store through an unproved or differently based pointer may alias
                # the entry frame, so no save cell from before it survives.
                saved = ()
        else:
            raise ValueError(
                f"consumer body has an unsupported instruction at {_hex(address)}"
            )
        if destination is not None and state.pending is not None:
            if state.pending[0] == destination:
                raise ValueError(
                    "consumer load result is overwritten in its delay slot at "
                    f"{_hex(address)}"
                )
        if value is not None:
            node = self._value(index, value)
            produced = record.get("values")
            if produced is None:
                record["value"] = node
                record["values"] = [node]
            elif node not in produced:
                # A path-dependent instruction record names every produced value
                # instead of presenting the last path's value as its only effect.
                produced.append(node)
                produced.sort()
                record["value"] = None
        if destination is not None and value is None:
            raise ValueError(
                f"consumer instruction at {_hex(address)} has no proved value"
            )
        registers = list(state.registers)
        if state.pending is not None:
            registers[state.pending[0]] = state.pending[1]
        pending: tuple[int, int] | None = None
        if delayed and destination is not None:
            pending = (destination, value)
        elif destination is not None:
            registers[destination] = self._value(index, value)
        return _State(tuple(registers), pending, saved)

    def _forward(
        self, state: _State, decoded: dict, opcode: str
    ) -> tuple[int, str] | None:
        """Return the value and store site of one protected entry-frame SP cell."""

        if (
            decoded["kind"] != "entry_relative"
            or decoded["register"] != _STACK_POINTER
            or decoded["offset"] >= 0
        ):
            return None
        # Only a word or unsigned-byte read is forwarded: a narrower or signed read
        # of the same cell would need an extension this model does not track.
        width = _FORWARDABLE.get(opcode)
        if width is None:
            return None
        return _cell(state, decoded["register"], decoded["offset"], width)

    def _check_store(self, index: int, decoded: dict, opcode: str) -> None:
        if _targets_selected(decoded, self.selected):
            raise ValueError(
                f"consumer writes the selected object at {_hex(self.address(index))}"
            )
        if decoded["kind"] != "absolute":
            return
        width = _WIDTHS[opcode]
        absolute = decoded["address"]
        if absolute % width:
            raise ValueError(
                f"consumer store at {_hex(self.address(index))} is not {width}-byte "
                "aligned"
            )
        end = self.start + len(self.words) * 4
        if absolute < end and absolute + width > self.start:
            raise ValueError(
                f"consumer store at {_hex(self.address(index))} overlaps the captured "
                "instruction bytes"
            )

    # -- control ----------------------------------------------------------

    def _guard(
        self, index: int, fields, left: int, right: int, taken: bool, loop: str | None
    ) -> str:
        target = _branch_target(fields, self.address(index))
        assert target is not None
        site = _hex(self.address(index))
        outcome = "taken" if taken else "not_taken"
        for known, guard in self.guards.items():
            if (
                guard["site"] == site
                and guard["outcome"] == outcome
                and guard["loop"] == loop
                and guard["left"] == left
                and guard["right"] == right
            ):
                return known
        name = f"g{len(self.guards) + 1}"
        self.guards[name] = {
            "site": _hex(self.address(index)),
            "opcode": "beq" if fields.opcode == 0x04 else "bne",
            "outcome": outcome,
            "left": left,
            "right": right,
            "target": _hex(target),
            "delay_slot": _hex(self.address(index + 1)),
            "loop": loop,
        }
        return name

    def _branch(
        self, state: _State, index: int, guards: tuple[str, ...], loop: str | None
    ) -> list[tuple[int, _State, str, _State]]:
        fields = self.fields[index]
        address = self.address(index)
        target = _branch_target(fields, address)
        assert target is not None
        target_index = self._index(target)
        if not 0 <= target_index < len(self.words):
            raise ValueError(
                f"consumer branch at {_hex(address)} leaves the reviewed body"
            )
        left, right = state.registers[fields.rs], state.registers[fields.rt]
        decided = self._decide(fields, left, right)
        # Operands are sampled before the slot, and the load that was issued by the
        # previous instruction retires at this branch boundary so the slot sees it.
        settled = _retire(state)
        results = []
        for taken in (False, True):
            if decided is not None and taken != decided:
                continue
            decision = self._guard(index, fields, left, right, taken, loop)
            after = self._transfer(settled, index + 1, guards + (decision,), loop)
            results.append(
                (target_index if taken else index + 2, after, decision, settled)
            )
        return results

    def _decide(self, fields, left: int, right: int) -> bool | None:
        first, second = self.graph.get(left), self.graph.get(right)
        if first["kind"] != "constant" or second["kind"] != "constant":
            return None
        equal = first["value"] == second["value"]
        return equal if fields.opcode == 0x04 else not equal

    def _call(
        self, state: _State, index: int, guards: tuple[str, ...], loop: str | None
    ) -> _State:
        fields = self.fields[index]
        address = self.address(index)
        callee = _jump_target(fields, address)
        if callee % 4:
            raise ValueError(
                f"consumer call at {_hex(address)} does not target a word boundary"
            )
        if (
            _hex(address) not in self.call_sites
            and len(self.call_sites) >= _LIMITS["calls"]
        ):
            raise ValueError(f"consumer body exceeds {_LIMITS['calls']} call sites")
        slot = index + 1
        if _destination(self.fields[slot]) == _RETURN_REGISTER:
            raise ValueError(
                f"consumer call at {_hex(address)} has a delay slot that rewrites the "
                "return address"
            )
        if state.pending is not None and state.pending[0] == _RETURN_REGISTER:
            raise ValueError(
                f"consumer call at {_hex(address)} is preceded by a delayed load into "
                "the return address"
            )
        # The architectural link value exists before the slot runs, and the load the
        # previous instruction issued retires at this call boundary.
        linked = list(_retire(state).registers)
        linked[_RETURN_REGISTER] = self.graph.constant(address + 8)
        state = self._transfer(
            _State(tuple(linked), None, state.saved), slot, guards, loop
        )
        state = self._settle_slot_load(state, address)
        registers = list(state.registers)
        for register in _CALL_CLOBBERED:
            registers[register] = self.graph.call_clobber(
                _hex(address), _hex(callee), register, loop
            )
        for register in (2, 3):
            registers[register] = self.graph.call_result(
                _hex(address), _hex(callee), register, loop
            )
        self.call_sites[_hex(address)] = {
            "site": _hex(address),
            "callee": _hex(callee),
            "delay_slot": _hex(address + 4),
            "return_address": _hex(address + 8),
            "architectural_ra": "pc_plus_8",
            "clobbered": [_name(register) for register in _CALL_CLOBBERED],
            "outputs": {"v0": registers[2], "v1": registers[3]},
            "loop": loop,
            "premise": "normal return under the O32 convention",
        }
        return _State(tuple(registers), None, state.saved)

    def _settle_slot_load(self, state: _State, address: int) -> _State:
        """Commit a load issued by a call delay slot before callee clobbers."""

        if state.pending is None:
            return state
        register = state.pending[0]
        registers = list(state.registers)
        registers[register] = state.pending[1]
        # The loaded value exists at the call boundary: the callee sees it as an
        # argument, and CLOBBER then invalidates it exactly like any other
        # caller-saved value.
        return _State(tuple(registers), None, state.saved)

    def _return(
        self,
        index: int,
        jr: int,
        state: _State,
        target: int,
        guards: tuple[str, ...],
        loop: str | None,
    ) -> None:
        """Record one terminal return from the sampled jump target and slot state."""

        if state.pending is not None:
            raise ValueError(
                "consumer load result is unresolved at the return "
                f"{_hex(self.address(jr))}"
            )
        if state.registers[_STACK_POINTER] != self.graph.entry(_STACK_POINTER):
            raise ValueError(
                f"consumer return at {_hex(self.address(jr))} does not restore the "
                "entry stack pointer"
            )
        if target != self.graph.entry(_RETURN_REGISTER):
            raise ValueError(
                f"consumer return at {_hex(self.address(jr))} does not use the entry "
                "return address"
            )
        self.returns.append(
            {
                "site": _hex(self.address(jr)),
                "delay_slot": _hex(self.address(index)),
                "guards": list(guards),
                "loop": loop,
                "value": state.registers[2],
                "stack_pointer": "restored_entry_sp",
                "return_address": "entry_ra",
            }
        )

    def _walk(
        self,
        start: int,
        state: _State,
        guards: tuple[str, ...],
        *,
        cut: dict[int, str] | None = None,
        loop: str | None = None,
        owner: int | None = None,
    ) -> list[_Outcome]:
        """Enumerate bounded paths, substituting each natural loop once."""

        cut = cut or {}
        outcomes: list[_Outcome] = []
        work: list[tuple[int, _State, tuple[str, ...], bool]] = [
            (start, state, guards, False)
        ]
        steps = 0
        while work:
            index, state, guards, entered = work.pop()
            while True:
                steps += 1
                if steps > _LIMITS["walk_steps"]:
                    raise ValueError(
                        f"consumer body exceeds the bounded walk of "
                        f"{_LIMITS['walk_steps']} steps"
                    )
                if entered and index in cut:
                    outcomes.append(_Outcome(cut[index], index, state, guards))
                    break
                entered = True
                if not 0 <= index <= len(self.words):
                    raise ValueError("consumer control flow leaves the reviewed body")
                if index == len(self.words):
                    raise ValueError(
                        "consumer control flow runs past the reviewed body"
                    )
                if index in self.loop_headers and index != owner:
                    if loop is not None:
                        raise ValueError(
                            f"consumer body has a loop nested inside the loop at {loop}"
                        )
                    state = self._summarize_loop(index, state, guards)
                    index = self.loop_headers[index]["latch"] + 2
                    continue
                fields = self.fields[index]
                if fields.opcode == 0 and fields.funct == 0x08:
                    self.covered.add(index)
                    # The jump target is sampled before its own delay slot runs, and
                    # the previous load retires at this transfer boundary.
                    target = state.registers[_RETURN_REGISTER]
                    state = self._transfer(_retire(state), index + 1, guards, loop)
                    self._return(index + 1, index, state, target, guards, loop)
                    break
                if fields.opcode == 0x03:
                    self.covered.add(index)
                    state = self._call(state, index, guards, loop)
                    index += 2
                    continue
                if fields.opcode == 0x02:
                    self.covered.add(index)
                    target = self._index(_jump_target(fields, self.address(index)))
                    state = self._transfer(_retire(state), index + 1, guards, loop)
                    index = target
                    continue
                if fields.opcode in (0x04, 0x05):
                    self.covered.add(index)
                    for outcome_index, after, decision, sampled in self._branch(
                        state, index, guards, loop
                    ):
                        if outcome_index in cut:
                            outcomes.append(
                                _Outcome(
                                    cut[outcome_index],
                                    outcome_index,
                                    after,
                                    guards + (decision,),
                                    sampled,
                                )
                            )
                            continue
                        work.append((outcome_index, after, guards + (decision,), True))
                    if len(work) > _LIMITS["alternatives"]:
                        raise ValueError(
                            f"consumer body exceeds {_LIMITS['alternatives']} path "
                            "alternatives"
                        )
                    break
                state = self._transfer(state, index, guards, loop)
                index += 1
        return outcomes

    def _record_event(self, index: int, event: dict, guards: tuple[str, ...]) -> int:
        event["site"] = _hex(self.address(index))
        event["guards"] = list(guards)
        self.events.append(event)
        return len(self.events) - 1

    # -- report -----------------------------------------------------------

    def run(self) -> dict:
        if not 1 <= len(self.words) <= _LIMITS["body_words"]:
            raise ValueError(
                "consumer body is not a finite aligned body of at most "
                f"{_LIMITS['body_words']} words"
            )
        self._describe()
        self._find_loops()
        entry = _State(
            tuple(
                self.graph.constant(0)
                if register == _ZERO
                else self.graph.entry(register)
                for register in range(32)
            )
        )
        self._walk(0, entry, ())
        missing = [
            index for index in range(len(self.words)) if index not in self.covered
        ]
        if missing:
            raise ValueError(
                "consumer body contains unexplained or unreachable instructions at "
                + ", ".join(_hex(self.address(index)) for index in missing[:4])
            )
        if not self.returns:
            raise ValueError("consumer body has no proved terminal return")
        return self._report()

    def _report(self) -> dict:
        selected_reads = [
            event
            for event in self.events
            if event["kind"] == "read"
            and _targets_selected(event["address"], self.selected)
        ]
        if not selected_reads:
            raise ValueError(
                "consumer body does not prove a read of the selected object"
            )
        table_reads = [self._table_read(event) for event in selected_reads]
        body = {
            "kind": "complete_static_consumer_body",
            "words": len(self.words),
            "table_reads": table_reads,
            "sums": self._sums(selected_reads),
            "masks": self._masks(),
            "comparisons": self._comparisons(),
            "sentinels": self._sentinels(table_reads),
            "loops": self.loops,
            "calls": [self.call_sites[site] for site in sorted(self.call_sites)],
            "returns": self._returns(),
            "instructions": self.records,
            "memory": [self._render_event(event) for event in self.events],
            "guards": self.guards,
            "coverage": {
                "instructions": len(self.words),
                "executed": len(self.covered),
                "edges": sum(len(record["successors"]) for record in self.records),
                "calls": len(self.call_sites),
                "loops": len(self.loops),
                "returns": len(self.returns),
            },
            "premises": list(SHARED_PREMISES),
            "scope": SCOPE,
        }
        # The value table is derived from the emitted projections themselves, so a
        # reference added to a section can never escape the receipt.
        body["values"] = self._values(body)
        return body

    def _table_read(self, event: dict) -> dict:
        if event["address"]["kind"] == "absolute":
            index = {"kind": "constant_element"}
            fixed = event["address"]["address"]
        else:
            node = self.graph.get(event["address"]["index"])
            if node["kind"] != "memory_read" or node["opcode"] != _UNSIGNED_BYTE:
                raise ValueError(
                    f"selected table read at {event['site']} does not use one settled "
                    "unsigned byte index"
                )
            index = {
                "kind": "loaded_byte",
                "node": event["address"]["index"],
                "site": node["site"],
                "opcode": node["opcode"],
                "range": {"min": 0, "max": 255},
                "loop": node["loop"],
            }
            fixed = event["address"]["fixed_base"]
        return {
            "site": event["site"],
            "opcode": event["opcode"],
            "width": event["width"],
            "fixed_base": _hex(fixed),
            "index": index,
            "guards": event["guards"],
            "loop": event["loop"],
        }

    def _render_event(self, event: dict) -> dict:
        rendered = {
            "site": event["site"],
            "kind": event["kind"],
            "opcode": event["opcode"],
            "address": _render_address(event["address"]),
            "width": event["width"],
            "guards": event["guards"],
            "loop": event["loop"],
        }
        for key in ("value", "node", "forwarded_from"):
            if key in event:
                rendered[key] = event[key]
        return rendered

    def _sums(self, selected_reads: list[dict]) -> list[dict]:
        selected = {event["node"] for event in selected_reads}
        sums: list[dict] = []
        for index, record in enumerate(self.records):
            for node in record.get("values") or ():
                self._sum(sums, index, node, selected)
        return sums

    def _sum(self, sums: list[dict], index: int, node: int, selected: set) -> None:
        """Append one recorded add that consumes a selected-object read."""

        item = self.graph.get(node)
        if item["kind"] != "expression" or item["operation"] != "add":
            return
        inside = self._cone(node) & selected
        if not inside:
            return
        sums.append(
            {
                "site": _hex(self.address(index)),
                "left": item["left"],
                "right": item["right"],
                "selected_reads": sorted(
                    self.graph.get(member)["site"] for member in inside
                ),
            }
        )

    def _masks(self) -> list[dict]:
        masks: list[dict] = []
        for index, record in enumerate(self.records):
            for node in record.get("values") or ():
                item = self.graph.get(node)
                if item["kind"] != "expression" or item["operation"] != "and":
                    continue
                right = self.graph.get(item["right"])
                if right["kind"] != "constant":
                    continue
                masks.append(
                    {
                        "site": _hex(self.address(index)),
                        "immediate": right["value"],
                        "source": item["left"],
                    }
                )
        return masks

    def _comparisons(self) -> list[dict]:
        comparisons: list[dict] = []
        for index, record in enumerate(self.records):
            for node in record.get("values") or ():
                item = self.graph.get(node)
                if item["kind"] != "expression":
                    continue
                if item["operation"] != "less_than_unsigned":
                    continue
                comparisons.append(
                    {
                        "site": _hex(self.address(index)),
                        "left": self._role(item["left"]),
                        "right": self._role(item["right"]),
                    }
                )
        return comparisons

    def _role(self, node: int) -> dict:
        item = self.graph.get(node)
        role: dict = {"node": node}
        if item["kind"] == "memory_read":
            role["role"] = {
                "lhu": "unsigned_halfword_read",
                "lh": "signed_halfword_read",
                "lbu": "unsigned_byte_read",
                "lw": "word_read",
            }[item["opcode"]]
            role["site"] = item["site"]
            role["opcode"] = item["opcode"]
            return role
        if item["kind"] == "expression" and item["operation"] == "and":
            right = self.graph.get(item["right"])
            if right["kind"] == "constant":
                cone = self._cone(item["left"])
                reads = [
                    self.graph.get(member)["site"]
                    for member in cone
                    if self.graph.get(member)["kind"] == "memory_read"
                ]
                if reads:
                    role["role"] = (
                        "low_byte_of_accumulated_reads"
                        if right["value"] == 0xFF
                        else "masked_accumulation"
                    )
                    role["mask"] = right["value"]
                    role["sources"] = sorted(reads)
                    return role
        role["role"] = item["kind"]
        if item["kind"] == "constant":
            role["constant"] = item["value"]
        elif item["kind"] in ("call_result", "call_clobber"):
            role["site"] = item["site"]
            role["callee"] = item["callee"]
        elif item["kind"] == "unknown":
            role["reason"] = item["reason"]
        return role

    def _inversion(self, node: int) -> dict | None:
        item = self.graph.get(node)
        if item["kind"] != "expression" or item["operation"] != "xor":
            return None
        right = self.graph.get(item["right"])
        if right["kind"] != "constant" or right["value"] != 1:
            return None
        return {
            "operand": item["left"],
            "immediate": 1,
            "effect": "returns the complementary comparison result",
        }

    def _returns(self) -> list[dict]:
        rendered = []
        for entry in sorted(
            self.returns, key=lambda item: (item["site"], item["guards"])
        ):
            rendered.append(
                {
                    **entry,
                    "value_role": self._role(entry["value"]),
                    "inversion": self._inversion(entry["value"]),
                }
            )
        return rendered

    def _sentinels(self, table_reads: list[dict]) -> list[dict]:
        """Pair every constant-guarded branch with what each outcome reads or skips."""

        latches = {loop["latch"] for loop in self.loops}
        sentinels = []
        for site in sorted({guard["site"] for guard in self.guards.values()}):
            if site in latches:
                continue
            decisions = {
                guard["outcome"]: key
                for key, guard in self.guards.items()
                if guard["site"] == site
            }
            first = self.guards[next(iter(decisions.values()))]
            constants = [
                self.graph.get(node)["value"]
                for node in (first["left"], first["right"])
                if self.graph.get(node)["kind"] == "constant"
            ]
            if len(constants) != 1:
                continue
            index = (int(site, 16) - self.start) // 4
            target = self._index(int(first["target"], 16))
            entry = {
                "site": site,
                "opcode": first["opcode"],
                "sentinel": constants[0],
                "taken": [
                    read["site"]
                    for read in table_reads
                    if "taken" in decisions and decisions["taken"] in read["guards"]
                ],
                "not_taken": [
                    read["site"]
                    for read in table_reads
                    if "not_taken" in decisions
                    and decisions["not_taken"] in read["guards"]
                ],
                "delay_slot": self._slot_effect(first["delay_slot"]),
            }
            skipped = [
                _hex(self.address(header))
                for header in sorted(self.loop_headers)
                if index < header < target
            ]
            if skipped:
                entry["skips_loops"] = skipped
            sentinels.append(entry)
        return sentinels

    def _slot_effect(self, site: str) -> dict:
        for event in self.events:
            if event["site"] == site:
                return {
                    "site": site,
                    "kind": event["kind"],
                    "opcode": event["opcode"],
                    "address": _render_address(event["address"]),
                }
        return {"site": site, "kind": "integer"}

    def _referenced(self, item: object, found: set[int]) -> None:
        """Collect every node id one emitted projection references."""

        if isinstance(item, dict):
            for key, value in item.items():
                if key in _NODE_FIELDS and isinstance(value, int):
                    found.add(value)
                else:
                    self._referenced(value, found)
        elif isinstance(item, list | tuple):
            for value in item:
                self._referenced(value, found)

    def _values(self, sections: dict) -> dict:
        """Emit every node the receipt references, or refuse an incomplete table."""

        roots: set[int] = set()
        self._referenced(sections, roots)
        known = range(len(self.graph.nodes))
        reachable: set[int] = set()
        stack = list(roots)
        while stack:
            node = stack.pop()
            if node in reachable:
                continue
            if not 0 <= node < len(self.graph.nodes):
                raise ValueError(
                    f"consumer body references an unknown value node {node}"
                )
            reachable.add(node)
            item = self.graph.get(node)
            if item["kind"] == "expression":
                stack.extend((item["left"], item["right"]))
            elif item["kind"] == "indexed_base":
                stack.append(item["index"])
            address = item.get("address")
            if isinstance(address, dict):
                for field in ("index", "source"):
                    if isinstance(address.get(field), int):
                        stack.append(address[field])
        del known
        return {
            str(node): self._render_node(self.graph.get(node))
            for node in sorted(reachable)
        }

    def _render_node(self, item: dict) -> dict:
        rendered = dict(item)
        if isinstance(rendered.get("register"), int):
            rendered["register"] = _name(rendered["register"])
        if isinstance(rendered.get("address"), dict):
            rendered["address"] = _render_address(rendered["address"])
        return rendered


def decode_consumer_body(code: bytes, start: int, selected: int) -> dict:
    """Explain every word of one reviewed consumer body, or refuse it."""

    if not code or len(code) % 4:
        raise ValueError("byte lookup consumer is not a finite aligned body")
    if start % 4:
        raise ValueError("byte lookup consumer does not start on a word boundary")
    words = list(struct.unpack_from(f"<{len(code) // 4}I", code, 0))
    return _Body(start, words, selected).run()
