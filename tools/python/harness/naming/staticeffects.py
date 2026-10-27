"""One static MIPS effect interpreter shared by naming body analyses.

The module owns the parts every complete-body analysis needs and that are
independent of the question being proved:

* the interned expression graph, so one proved value has one identity that a
  receipt can cite instead of a compressed summary;
* pending MIPS-I loads that stay readable for exactly one following instruction
  and settle at every instruction boundary, so delayed loads used as operands,
  branch operands, slot values or call arguments keep the original timing;
* the frame-cell model of entry-relative ``$sp`` writes, which is how a body's
  own stores stay distinct from the loads that read them back;
* static classification of every word and syntactic edge before execution;
* a bounded, acyclic path walk over forward branches, jumps, calls and returns.

Policies stay with the analyses that need them.  A body subclass declares the
encodings it supports, its resource bounds, its store and call policy and
whether a natural loop may be summarized; everything else here is shared.
Unsupported encodings, transfers, addresses and exhausted bounds raise
``ValueError`` through :meth:`StaticBody.refuse` with the exact unsupported
obligation.
"""

from __future__ import annotations

import json
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

LOADS = {0x21: "lh", 0x23: "lw", 0x24: "lbu", 0x25: "lhu"}
STORES = {0x28: "sb", 0x29: "sh", 0x2B: "sw"}
WIDTHS = {"lh": 2, "lw": 4, "lbu": 1, "lhu": 2, "sb": 1, "sh": 2, "sw": 4}
CALL_CLOBBERED = (1, 2, 3, 4, 5, 6, 7, *range(8, 16), 24, 25, 26, 27, 28)
RETURN_REGISTER = 31
STACK_POINTER = 29
FRAME_POINTER = 30
ZERO = 0
# Only a word or an unsigned-byte read of a recorded frame cell is forwarded: a
# narrower or signed read would need a partial-value or extension model.
FORWARDABLE = {"lw": 4, "lbu": 1}
# Fields that carry a value-node identity in every emitted projection.  Receipts
# are validated against this list so a referenced node cannot be missing from the
# ``values`` table.
NODE_FIELDS = frozenset(
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
        "a0",
        "a1",
        "a2",
        "a3",
        "relation",
        "values",
    }
)


def hex_address(value: int) -> str:
    return f"0x{value & 0xFFFFFFFF:08X}"


def register_name(register: int) -> str:
    return REGISTER_NAMES[register]


def signed_immediate(value: int, bits: int) -> int:
    value &= (1 << bits) - 1
    return value - (1 << bits) if value & (1 << (bits - 1)) else value


# ------------------------------------------------------------- static shape

_SHIFT_FUNCTS = {0x00: "sll", 0x02: "srl", 0x03: "sra"}
_LOGICAL_FUNCTS = {
    0x21: "addu",
    0x23: "subu",
    0x24: "and",
    0x25: "or",
    0x26: "xor",
    0x27: "nor",
    0x2A: "slt",
    0x2B: "sltu",
}
_JUMP_FUNCTS = {0x08: "jr"}
_IMMEDIATE_NAMES = {
    0x09: "addiu",
    0x0B: "sltiu",
    0x0C: "andi",
    0x0D: "ori",
    0x0E: "xori",
}
_REGISTERS = {
    0x01: {0x00: "bltz", 0x01: "bgez"},
    0x02: "j",
    0x03: "jal",
    0x04: "beq",
    0x05: "bne",
    0x0F: "lui",
    **LOADS,
    **STORES,
}


def word_mnemonic(fields) -> str | None:
    """Name one supported encoding, or return ``None`` when it is unsupported."""

    if fields.word == 0:
        return "nop"
    if fields.opcode == 0:
        name = _SHIFT_FUNCTS.get(fields.funct) or _LOGICAL_FUNCTS.get(fields.funct)
        return name or _JUMP_FUNCTS.get(fields.funct)
    name = _REGISTERS.get(fields.opcode)
    if isinstance(name, dict):
        return name.get(fields.rt)
    if name is not None:
        return name
    return _IMMEDIATE_NAMES.get(fields.opcode)


def reserved_fields_are_legal(fields) -> bool:
    """Check the reserved encodings a legal instruction must keep zero."""

    if fields.word == 0:
        return True
    if fields.opcode == 0:
        if fields.funct in _SHIFT_FUNCTS:
            return fields.rs == 0
        if fields.funct == 0x08:
            return fields.rt == 0 and fields.rd == 0 and fields.shift == 0
        return fields.shift == 0
    if fields.opcode == 0x0F:
        return fields.rs == 0
    return True


def instruction_alias(fields) -> str | None:
    """Name the degenerate encodings that only read as one operation."""

    if fields.word == 0:
        return None
    if fields.opcode == 0 and fields.funct == 0x21 and fields.rt == 0:
        return "move"
    if fields.opcode == 0 and fields.funct == 0x23 and fields.rs == 0:
        return "negu"
    if fields.opcode == 0x05 and fields.rt == 0:
        return "bnez"
    if fields.opcode == 0x04 and fields.rt == 0:
        return "beqz"
    return None


def written_register(fields) -> int | None:
    """Return the written register of one classified instruction, ignoring ``$zero``."""

    if fields.opcode == 0:
        if fields.funct == 0x08:
            return None
        written = fields.rd
    elif fields.opcode in STORES or fields.opcode in (0x01, 0x02, 0x04, 0x05):
        written = 0
    elif fields.opcode == 0x03:
        written = RETURN_REGISTER
    else:
        written = fields.rt
    return written or None


def read_registers(fields) -> tuple[int, ...]:
    """Return the registers one classified instruction reads."""

    if fields.word == 0:
        return ()
    if fields.opcode == 0:
        if fields.funct in (0x00, 0x02, 0x03):
            return (fields.rt,)
        if fields.funct == 0x08:
            return (fields.rs,)
        return (fields.rs, fields.rt)
    if fields.opcode in STORES or fields.opcode in (0x04, 0x05):
        return (fields.rs, fields.rt)
    if fields.opcode == 0x01:
        return (fields.rs,)
    if fields.opcode in LOADS:
        return (fields.rs,)
    if fields.opcode in (0x02, 0x03, 0x0F):
        return ()
    return (fields.rs,)


def branch_target(fields, address: int) -> int | None:
    """Return the absolute target of one conditional transfer, else ``None``."""

    if fields.opcode not in (0x01, 0x04, 0x05):
        return None
    return (address + 4 + (fields.signed_immediate << 2)) & 0xFFFFFFFF


def jump_target(fields, address: int) -> int:
    return (((address + 4) & 0xF0000000) | (fields.target26 << 2)) & 0xFFFFFFFF


# ------------------------------------------------------------ expression graph


class Graph:
    """Interned expression nodes shared by every state of one frozen body."""

    def __init__(self, limit: int, label: str) -> None:
        self.limit = limit
        self.label = label
        self.nodes: list[dict] = []
        self._ids: dict[str, int] = {}

    def add(self, node: dict) -> int:
        key = json.dumps(node, sort_keys=True)
        known = self._ids.get(key)
        if known is not None:
            return known
        if len(self.nodes) >= self.limit:
            raise ValueError(
                f"{self.label} exceeds {self.limit} shared expression nodes"
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
            "width": WIDTHS[opcode],
            "signed": opcode == "lh",
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


def project_address(graph: Graph, node: int, offset: int) -> dict:
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


def render_address(address: dict) -> dict:
    rendered = {"kind": address["kind"]}
    if address["kind"] == "absolute":
        rendered["address"] = hex_address(address["address"])
    elif address["kind"] == "indexed":
        rendered["fixed_base"] = hex_address(address["fixed_base"])
        rendered["index"] = address["index"]
    elif address["kind"] == "entry_relative":
        rendered["register"] = register_name(address["register"])
        rendered["offset"] = address["offset"]
    else:
        rendered["source"] = address["source"]
        rendered["offset"] = address["offset"]
    return rendered


def render_node(item: dict) -> dict:
    rendered = dict(item)
    if isinstance(rendered.get("register"), int):
        rendered["register"] = register_name(rendered["register"])
    if isinstance(rendered.get("address"), dict):
        rendered["address"] = render_address(rendered["address"])
    return rendered


def node_role(graph: Graph, node: int) -> dict:
    """Describe one value node by the kind of value it holds."""

    item = graph.get(node)
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
        right = graph.get(item["right"])
        if right["kind"] == "constant":
            reads = [
                graph.get(member)["site"]
                for member in value_cone(graph, item["left"])
                if graph.get(member)["kind"] == "memory_read"
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


def value_cone(graph: Graph, root: int) -> set[int]:
    """Return every node one value is built from."""

    seen: set[int] = set()
    stack = [root]
    while stack:
        node = stack.pop()
        if node in seen:
            continue
        seen.add(node)
        item = graph.get(node)
        if item["kind"] == "expression":
            stack.extend((item["left"], item["right"]))
        elif item["kind"] == "indexed_base":
            stack.append(item["index"])
    return seen


# ---------------------------------------------------------------- execution


@dataclass(frozen=True)
class FrameCell:
    """One recorded entry-relative ``$sp`` cell this body stored and may reread."""

    base: int
    offset: int
    width: int
    node: int
    site: str
    preserved: bool = False


@dataclass(frozen=True)
class MachineState:
    """One path's committed registers, delayed load and recorded frame cells."""

    registers: tuple[int, ...]
    pending: tuple[int, int] | None = None
    cells: tuple[FrameCell, ...] = ()


@dataclass(frozen=True)
class Outcome:
    """One bounded-walk result, with the state sampled at its transfer."""

    label: str
    index: int
    state: MachineState
    guards: tuple[str, ...]
    path: int
    sampled: MachineState | None = None


def retire(state: MachineState) -> MachineState:
    """Retire one pending delayed load at an instruction boundary."""

    if state.pending is None:
        return state
    registers = list(state.registers)
    registers[state.pending[0]] = state.pending[1]
    return MachineState(tuple(registers), None, state.cells)


def cell_register_value(
    state: MachineState, register: int, offset: int, width: int
) -> tuple[int, str] | None:
    """Return the value and store site of one recorded frame cell."""

    for cell in state.cells:
        if (cell.base, cell.offset, cell.width) == (register, offset, width):
            return cell.node, cell.site
    return None


def cells_overlapping(
    cells: tuple[FrameCell, ...], offset: int, width: int
) -> tuple[FrameCell, ...]:
    """Return every cell on the same base whose bytes intersect one stored range."""

    return tuple(
        cell
        for cell in cells
        if cell.offset < offset + width and offset < cell.offset + cell.width
    )


def save_cell(
    cells: tuple[FrameCell, ...],
    base: int,
    offset: int,
    width: int,
    node: int,
    site: str,
    *,
    preserved: bool = False,
) -> tuple[FrameCell, ...]:
    """Record one frame cell, dropping every overlapping earlier write."""

    kept = [
        cell
        for cell in cells
        if not (
            cell.base == base
            and cell.offset < offset + width
            and offset < cell.offset + cell.width
        )
    ]
    kept.append(FrameCell(base, offset, width, node, site, preserved))
    return tuple(sorted(kept, key=lambda cell: (cell.base, cell.offset, cell.width)))


class StaticBody:
    """One frozen original body and the static facts its policy supports."""

    #: Noun for refusals about the whole body: "<label> has an unsupported
    #: instruction at ...".  A policy names what its owner proves.
    label = "body"
    #: Noun for refusals about one instruction, transfer or loop:
    #: "<subject> call at ... is not word aligned".
    subject = "body"
    supported: frozenset[str] = frozenset()
    max_words = 256
    max_edges = 512
    max_calls = 11
    max_nodes = 4096
    max_walk_steps = 4096
    max_pending = 64
    max_completed: int | None = None

    def __init__(self, start: int, words: list[int]) -> None:
        if start % 4:
            raise ValueError(f"{self.label} does not start on a word boundary")
        if not 1 <= len(words) <= self.max_words:
            raise ValueError(
                f"{self.label} is not a finite aligned body of at most "
                f"{self.max_words} words"
            )
        self.start = start
        self.words = words
        self.fields = [word_fields(word) for word in words]
        self.graph = Graph(self.max_nodes, self.label)
        self.records: list[dict] = []
        self.events: list[dict] = []
        self.call_sites: dict[str, dict] = {}
        self.guards: dict[str, dict] = {}
        self.returns: list[dict] = []
        self.covered: set[int] = set()
        self.delay_slots: frozenset[int] = frozenset()
        self.summarized_loops: dict[int, int] = {}
        self.value_sites: dict[int, str] = {}
        self.path_calls: dict[int, list[dict]] = {}
        self.next_path_number = 1
        self.walk_steps = 0

    # -- bookkeeping ------------------------------------------------------

    def refuse(self, message: str) -> ValueError:
        """Return the body-level refusal for one unsupported obligation."""

        return ValueError(f"{self.label} {message}")

    def refuse_at(self, message: str) -> ValueError:
        """Return the instruction-level refusal for one unsupported obligation."""

        return ValueError(f"{self.subject} {message}")

    def address(self, index: int) -> int:
        return self.start + index * 4

    def _value(self, index: int, node: int) -> int:
        self.value_sites.setdefault(node, hex_address(self.address(index)))
        return node

    def index_of(self, target: int) -> int:
        offset = target - self.start
        if offset % 4:
            raise self.refuse_at(f"target {hex_address(target)} is not word aligned")
        return offset // 4

    def new_path(self) -> int:
        number = self.next_path_number
        self.next_path_number += 1
        return number

    def entry_state(self) -> MachineState:
        """Return the entry state every path of one body starts from."""

        return MachineState(
            tuple(
                self.graph.constant(0)
                if register == ZERO
                else self.graph.entry(register)
                for register in range(32)
            )
        )

    # -- static shape -----------------------------------------------------

    def mnemonic(self, fields, address: int) -> str:
        """Resolve one word to a supported mnemonic or refuse the encoding."""

        name = word_mnemonic(fields)
        if (
            name is None
            or name not in self.supported
            or not reserved_fields_are_legal(fields)
        ):
            raise self.refuse(
                f"has an unsupported instruction at {hex_address(address)}"
            )
        return name

    def describe(self) -> None:
        """Classify every word and its declared successors before execution."""

        count = len(self.words)
        slots: set[int] = set()
        transfers: set[int] = set()
        for index, fields in enumerate(self.fields):
            name = self.mnemonic(fields, self.address(index))
            control = fields.opcode in (0x01, 0x02, 0x03, 0x04, 0x05) or name == "jr"
            if not control:
                continue
            transfers.add(index)
            if index + 1 >= count:
                raise self.refuse_at(
                    f"transfer at {hex_address(self.address(index))} has no "
                    "ordinary delay slot"
                )
            slots.add(index + 1)
        for index in sorted(slots):
            if index in transfers:
                raise self.refuse_at(
                    f"delay slot at {hex_address(self.address(index))} is itself a "
                    "transfer"
                )
        self.delay_slots = frozenset(slots)
        edges = 0
        for index, fields in enumerate(self.fields):
            mnemonic = self.mnemonic(fields, self.address(index))
            record = {
                "address": hex_address(self.address(index)),
                "opcode": mnemonic,
                "reads": [
                    register_name(register) for register in read_registers(fields)
                ],
                "writes": [
                    register_name(register)
                    for register in [written_register(fields)]
                    if register is not None
                ],
                "successors": [],
            }
            alias = instruction_alias(fields)
            if alias is not None and alias != mnemonic:
                record["alias"] = alias
            if index in slots:
                record["role"] = "delay_slot"
            elif index in transfers:
                record["role"] = "transfer"
                record["successors"] = self.successors(index, fields)
            else:
                record["successors"] = [
                    {"to": hex_address(self.address(index + 1)), "kind": "fall_through"}
                ]
            edges += len(record["successors"])
            self.records.append(record)
        if edges > self.max_edges:
            raise self.refuse(f"exceeds {self.max_edges} control-flow edges")

    def successors(self, index: int, fields) -> list[dict]:
        address = self.address(index)
        for target in (
            branch_target(fields, address),
            jump_target(fields, address) if fields.opcode == 0x02 else None,
        ):
            if target is None:
                continue
            if self.index_of(target) in self.delay_slots:
                raise self.refuse_at(
                    f"transfer at {hex_address(address)} targets the delay slot at "
                    f"{hex_address(target)}"
                )
            self.forward_edge(index, target)
        if fields.opcode == 0 and fields.funct == 0x08:
            if fields.rs != RETURN_REGISTER:
                raise self.refuse(f"has an indirect jump at {hex_address(address)}")
            return [{"to": "entry_ra", "kind": "return"}]
        if fields.opcode == 0x02:
            return [{"to": hex_address(jump_target(fields, address)), "kind": "jump"}]
        if fields.opcode == 0x03:
            return [{"to": hex_address(address + 8), "kind": "call_return"}]
        target = branch_target(fields, address)
        assert target is not None
        return [
            {"to": hex_address(target), "kind": "branch_taken"},
            {"to": hex_address(address + 8), "kind": "branch_not_taken"},
        ]

    # -- policy hooks -----------------------------------------------------

    def memory_address(self, node: int, offset: int) -> dict:
        """Project an address under the owning analysis policy."""
        return project_address(self.graph, node, offset)

    def sltiu_immediate(self, fields) -> int:
        """Keep the lookup policy's retained projection unless overridden."""
        return fields.immediate

    def forward_edge(self, index: int, target: int) -> None:
        """Refuse one control edge the owner policy cannot follow."""

    def check_store(self, index: int, address: dict, opcode: str) -> None:
        """Refuse one store the owner policy cannot account for."""

    def store_cells(
        self, state: MachineState, index: int, address: dict, opcode: str
    ) -> tuple[FrameCell, ...]:
        """Return the recorded frame cells after one store."""

        return state.cells

    def forward(
        self, state: MachineState, address: dict, opcode: str
    ) -> tuple[int, str] | None:
        """Return the value and store site of one recorded frame cell read."""

        if (
            address["kind"] != "entry_relative"
            or address["register"] != STACK_POINTER
            or address["offset"] >= 0
        ):
            return None
        if FORWARDABLE.get(opcode) is None:
            return None
        return cell_register_value(
            state, address["register"], address["offset"], FORWARDABLE[opcode]
        )

    def loop_state(
        self, index: int, state: MachineState, guards: tuple[str, ...]
    ) -> MachineState:
        """Summarize or refuse one natural loop entered at ``index``."""

        raise self.refuse(f"has a loop at {hex_address(self.address(index))}")

    def observe_call(self, record: dict, guards: tuple[str, ...], path: int) -> None:
        """Observe one recorded call site with the path that reached it."""

    def observe_return(self, record: dict, path: int) -> None:
        """Observe one recorded return with the path that reached it."""

    def call_effect(
        self,
        index: int,
        address: int,
        callee: int,
        state: MachineState,
        guards: tuple[str, ...],
        path: int,
    ) -> None:
        """Observe one call after its delay slot settled and before it clobbers."""

    # -- transfer ---------------------------------------------------------

    def transfer(
        self,
        state: MachineState,
        index: int,
        guards: tuple[str, ...],
        loop: str | None = None,
        path: int = 0,
    ) -> MachineState:
        """Apply one ordinary instruction, keeping its delayed load timing."""

        if not 0 <= index < len(self.words):
            raise self.refuse("ends without a complete delay slot")
        fields = self.fields[index]
        self.covered.add(index)
        address = self.address(index)
        graph = self.graph
        record = self.records[index]
        value: int | None = None
        destination = written_register(fields)
        delayed = False
        cells = state.cells
        opcode = fields.opcode
        if fields.word == 0:
            pass
        elif opcode == 0x0F and fields.rs == ZERO:
            value = graph.constant(fields.immediate << 16)
        elif opcode == 0x09:
            value = graph.add_immediate(
                state.registers[fields.rs], fields.signed_immediate
            )
        elif opcode == 0x0C:
            value = graph.operation(
                "and", state.registers[fields.rs], graph.constant(fields.immediate)
            )
        elif opcode == 0x0D:
            value = graph.operation(
                "or", state.registers[fields.rs], graph.constant(fields.immediate)
            )
        elif opcode == 0x0E:
            value = graph.operation(
                "xor", state.registers[fields.rs], graph.constant(fields.immediate)
            )
        elif opcode == 0x0B:
            value = graph.operation(
                "less_than_unsigned",
                state.registers[fields.rs],
                graph.constant(self.sltiu_immediate(fields)),
            )
        elif opcode == 0 and fields.funct == 0x00:
            value = graph.operation(
                "shift_left", state.registers[fields.rt], graph.constant(fields.shift)
            )
        elif opcode == 0 and fields.funct == 0x02:
            value = graph.operation(
                "shift_right_logical",
                state.registers[fields.rt],
                graph.constant(fields.shift),
            )
        elif opcode == 0 and fields.funct == 0x03:
            value = graph.operation(
                "shift_right_arithmetic",
                state.registers[fields.rt],
                graph.constant(fields.shift),
            )
        elif opcode == 0 and fields.funct in _LOGICAL_FUNCTS:
            name = {
                0x21: "add",
                0x23: "sub_unsigned",
                0x24: "and",
                0x25: "or",
                0x26: "xor",
                0x27: "nor",
                0x2A: "less_than_signed",
                0x2B: "less_than_unsigned",
            }[fields.funct]
            value = graph.operation(
                name, state.registers[fields.rs], state.registers[fields.rt]
            )
        elif opcode in LOADS:
            name = LOADS[opcode]
            decoded = self.memory_address(
                state.registers[fields.rs], fields.signed_immediate
            )
            forwarded = self.forward(state, decoded, name)
            node = graph.read(
                hex_address(address),
                name,
                decoded,
                loop,
                forwarded[1] if forwarded else None,
            )
            record["memory"] = self.record_event(
                index,
                {
                    "kind": "read",
                    "opcode": name,
                    "address": decoded,
                    "width": WIDTHS[name],
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
        elif opcode in STORES:
            name = STORES[opcode]
            decoded = self.memory_address(
                state.registers[fields.rs], fields.signed_immediate
            )
            self.check_store(index, decoded, name)
            record["memory"] = self.record_event(
                index,
                {
                    "kind": "write",
                    "opcode": name,
                    "address": decoded,
                    "width": WIDTHS[name],
                    "value": state.registers[fields.rt],
                    "loop": loop,
                },
                guards,
            )
            cells = self.store_cells(state, index, decoded, name)
        else:
            raise self.refuse(
                f"has an unsupported instruction at {hex_address(address)}"
            )
        if destination is not None and state.pending is not None:
            if state.pending[0] == destination:
                raise self.refuse_at(
                    "load result is overwritten in its delay slot at "
                    f"{hex_address(address)}"
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
            raise self.refuse_at(
                f"instruction at {hex_address(address)} has no proved value"
            )
        registers = list(state.registers)
        if state.pending is not None:
            registers[state.pending[0]] = state.pending[1]
        pending: tuple[int, int] | None = None
        if delayed and destination is not None:
            pending = (destination, value)
        elif destination is not None:
            registers[destination] = self._value(index, value)
        return MachineState(tuple(registers), pending, cells)

    # -- control ----------------------------------------------------------

    def guard(
        self,
        index: int,
        fields,
        left: int,
        right: int,
        taken: bool,
        loop: str | None,
    ) -> str:
        target = branch_target(fields, self.address(index))
        assert target is not None
        site = hex_address(self.address(index))
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
            "site": site,
            "opcode": word_mnemonic(fields),
            "outcome": outcome,
            "left": left,
            "right": right,
            "target": hex_address(target),
            "delay_slot": hex_address(self.address(index + 1)),
            "loop": loop,
        }
        return name

    def branch(
        self,
        state: MachineState,
        index: int,
        guards: tuple[str, ...],
        loop: str | None = None,
        path: int = 0,
    ) -> list[tuple[int, MachineState, str, MachineState]]:
        fields = self.fields[index]
        address = self.address(index)
        target = branch_target(fields, address)
        assert target is not None
        target_index = self.index_of(target)
        if not 0 <= target_index < len(self.words):
            raise self.refuse_at(
                f"branch at {hex_address(address)} leaves the reviewed body"
            )
        right_register = ZERO if fields.opcode == 0x01 else fields.rt
        left, right = (
            state.registers[fields.rs],
            state.registers[right_register],
        )
        decided = self.decide(fields, left, right)
        # Operands are sampled before the slot, and the load that was issued by the
        # previous instruction retires at this branch boundary so the slot sees it.
        settled = retire(state)
        results = []
        for taken in (False, True):
            if decided is not None and taken != decided:
                continue
            decision = self.guard(index, fields, left, right, taken, loop)
            after = self.transfer(settled, index + 1, guards + (decision,), loop, path)
            results.append(
                (target_index if taken else index + 2, after, decision, settled)
            )
        return results

    def decide(self, fields, left: int, right: int) -> bool | None:
        first, second = self.graph.get(left), self.graph.get(right)
        if first["kind"] != "constant" or second["kind"] != "constant":
            return None
        if fields.opcode == 0x01:
            return signed_immediate(first["value"], 32) >= 0
        equal = first["value"] == second["value"]
        return equal if fields.opcode == 0x04 else not equal

    def call(
        self,
        state: MachineState,
        index: int,
        guards: tuple[str, ...],
        loop: str | None = None,
        path: int = 0,
    ) -> MachineState:
        fields = self.fields[index]
        address = self.address(index)
        callee = jump_target(fields, address)
        if callee % 4:
            raise self.refuse_at(
                f"call at {hex_address(address)} does not target a word boundary"
            )
        if (
            hex_address(address) not in self.call_sites
            and len(self.call_sites) >= self.max_calls
        ):
            raise self.refuse(f"exceeds {self.max_calls} call sites")
        slot = index + 1
        if written_register(self.fields[slot]) == RETURN_REGISTER:
            raise self.refuse_at(
                f"call at {hex_address(address)} has a delay slot that rewrites the "
                "return address"
            )
        if state.pending is not None and state.pending[0] == RETURN_REGISTER:
            raise self.refuse_at(
                f"call at {hex_address(address)} is preceded by a delayed load into "
                "the return address"
            )
        # The architectural link value exists before the slot runs, and the load the
        # previous instruction issued retires at this call boundary.
        linked = list(retire(state).registers)
        linked[RETURN_REGISTER] = self.graph.constant(address + 8)
        state = self.transfer(
            MachineState(tuple(linked), None, state.cells), slot, guards, loop, path
        )
        state = self.settle_slot_load(state)
        self.call_effect(index, address, callee, state, guards, path)
        registers = list(state.registers)
        for register in CALL_CLOBBERED:
            registers[register] = self.graph.call_clobber(
                hex_address(address), hex_address(callee), register, loop
            )
        for register in (2, 3):
            registers[register] = self.graph.call_result(
                hex_address(address), hex_address(callee), register, loop
            )
        record = {
            "site": hex_address(address),
            "callee": hex_address(callee),
            "delay_slot": hex_address(address + 4),
            "return_address": hex_address(address + 8),
            "architectural_ra": "pc_plus_8",
            "clobbered": [register_name(register) for register in CALL_CLOBBERED],
            "outputs": {"v0": registers[2], "v1": registers[3]},
            "loop": loop,
            "premise": "normal return under the O32 convention",
        }
        self.call_sites[hex_address(address)] = record
        self.path_calls.setdefault(path, []).append(record)
        self.observe_call(record, guards, path)
        return MachineState(tuple(registers), None, state.cells)

    def settle_slot_load(self, state: MachineState) -> MachineState:
        """Commit a load issued by a call delay slot before callee clobbers."""

        if state.pending is None:
            return state
        registers = list(state.registers)
        registers[state.pending[0]] = state.pending[1]
        # The loaded value exists at the call boundary: the callee sees it as an
        # argument, and CLOBBER then invalidates it exactly like any other
        # caller-saved value.
        return MachineState(tuple(registers), None, state.cells)

    def record_return(
        self,
        index: int,
        jr: int,
        state: MachineState,
        target: int,
        guards: tuple[str, ...],
        loop: str | None,
        path: int = 0,
    ) -> None:
        """Record one terminal return from the sampled jump target and slot state."""

        if state.pending is not None:
            raise self.refuse_at(
                f"load result is unresolved at the return {hex_address(self.address(jr))}"
            )
        if state.registers[STACK_POINTER] != self.graph.entry(STACK_POINTER):
            raise self.refuse_at(
                f"return at {hex_address(self.address(jr))} does not restore the "
                "entry stack pointer"
            )
        if target != self.graph.entry(RETURN_REGISTER):
            raise self.refuse_at(
                f"return at {hex_address(self.address(jr))} does not use the entry "
                "return address"
            )
        record = {
            "site": hex_address(self.address(jr)),
            "delay_slot": hex_address(self.address(index)),
            "guards": list(guards),
            "loop": loop,
            "value": state.registers[2],
            "stack_pointer": "restored_entry_sp",
            "return_address": "entry_ra",
        }
        if self.max_completed is not None and len(self.returns) >= self.max_completed:
            raise self.refuse(f"exceeds {self.max_completed} completed alternatives")
        self.returns.append(record)
        self.observe_return(record, path)

    def walk(
        self,
        start: int,
        state: MachineState,
        guards: tuple[str, ...],
        *,
        cut: dict[int, str] | None = None,
        loop: str | None = None,
        owner: int | None = None,
    ) -> list[Outcome]:
        """Enumerate bounded paths, substituting each summarized loop once."""

        cut = cut or {}
        outcomes: list[Outcome] = []
        work: list[tuple[int, MachineState, tuple[str, ...], bool, int]] = [
            (start, state, guards, False, 0)
        ]
        steps = 0
        while work:
            index, state, guards, entered, path = work.pop()
            while True:
                steps += 1
                if steps > self.max_walk_steps:
                    raise self.refuse(
                        f"exceeds the bounded walk of {self.max_walk_steps} steps"
                    )
                if entered and index in cut:
                    outcomes.append(Outcome(cut[index], index, state, guards, path))
                    break
                entered = True
                if not 0 <= index <= len(self.words):
                    raise self.refuse_at("control flow leaves the reviewed body")
                if index == len(self.words):
                    raise self.refuse_at("control flow runs past the reviewed body")
                if index in self.summarized_loops and index != owner:
                    if loop is not None:
                        raise self.refuse(
                            f"has a loop nested inside the loop at {loop}"
                        )
                    state = self.loop_state(index, state, guards)
                    index = self.summarized_loops[index]
                    continue
                fields = self.fields[index]
                if fields.word == 0x03E00008:
                    self.covered.add(index)
                    # The jump target is sampled before its own delay slot runs, and
                    # the previous load retires at this transfer boundary.
                    target = state.registers[RETURN_REGISTER]
                    state = self.transfer(retire(state), index + 1, guards, loop, path)
                    self.record_return(
                        index + 1, index, state, target, guards, loop, path
                    )
                    break
                if fields.opcode == 0x03:
                    self.covered.add(index)
                    state = self.call(state, index, guards, loop, path)
                    index += 2
                    continue
                if fields.opcode == 0x02:
                    self.covered.add(index)
                    target = self.index_of(jump_target(fields, self.address(index)))
                    state = self.transfer(retire(state), index + 1, guards, loop, path)
                    index = target
                    continue
                if fields.opcode in (0x01, 0x04, 0x05):
                    self.covered.add(index)
                    for outcome_index, after, decision, sampled in self.branch(
                        state, index, guards, loop, path
                    ):
                        if outcome_index in cut:
                            outcomes.append(
                                Outcome(
                                    cut[outcome_index],
                                    outcome_index,
                                    after,
                                    guards + (decision,),
                                    path,
                                    sampled,
                                )
                            )
                            continue
                        work.append(
                            (
                                outcome_index,
                                after,
                                guards + (decision,),
                                True,
                                self.new_path(),
                            )
                        )
                    if len(work) > self.max_pending:
                        raise self.refuse(
                            f"exceeds {self.max_pending} path alternatives"
                        )
                    if (
                        self.max_completed is not None
                        and len(outcomes) > self.max_completed
                    ):
                        raise self.refuse(
                            f"exceeds {self.max_completed} completed alternatives"
                        )
                    break
                state = self.transfer(state, index, guards, loop, path)
                index += 1
        self.walk_steps += steps
        return outcomes

    def record_event(self, index: int, event: dict, guards: tuple[str, ...]) -> int:
        event["site"] = hex_address(self.address(index))
        event["guards"] = list(guards)
        self.events.append(event)
        return len(self.events) - 1

    # -- rendering --------------------------------------------------------

    def render_event(self, event: dict) -> dict:
        rendered = {
            "site": event["site"],
            "kind": event["kind"],
            "opcode": event["opcode"],
            "address": render_address(event["address"]),
            "width": event["width"],
            "guards": event["guards"],
            "loop": event["loop"],
        }
        for key in ("value", "node", "forwarded_from"):
            if key in event:
                rendered[key] = event[key]
        return rendered

    def referenced(self, item: object, found: set[int]) -> None:
        """Collect every node id one emitted projection references."""

        if isinstance(item, dict):
            for key, value in item.items():
                if key in NODE_FIELDS and isinstance(value, int):
                    found.add(value)
                else:
                    self.referenced(value, found)
        elif isinstance(item, list | tuple):
            for value in item:
                self.referenced(value, found)

    def values_table(self, sections: dict) -> dict:
        """Emit every node the receipt references, or refuse an incomplete table."""

        roots: set[int] = set()
        self.referenced(sections, roots)
        reachable: set[int] = set()
        stack = list(roots)
        while stack:
            node = stack.pop()
            if node in reachable:
                continue
            if not 0 <= node < len(self.graph.nodes):
                raise self.refuse(f"references an unknown value node {node}")
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
        return {
            str(node): render_node(self.graph.get(node)) for node in sorted(reachable)
        }

    def finished_covered(self) -> None:
        """Require every word of the body to be explained by one reached path."""

        missing = [
            index for index in range(len(self.words)) if index not in self.covered
        ]
        if missing:
            raise self.refuse(
                "contains unexplained or unreachable instructions at "
                + ", ".join(hex_address(self.address(index)) for index in missing[:4])
            )
