"""Current target-qualified source relationships without grouping decisions."""

from __future__ import annotations

from collections import defaultdict
from contextlib import closing
from pathlib import Path

from harness.analysis.index import connect
from harness.macros.queries import macro_uses_payload
from harness.types.queries import type_usages_payload


def relationship_rows(root: Path, target: str) -> dict[int, dict]:
    with closing(connect(root)) as connection:
        connection.execute("PRAGMA query_only = ON")
        functions = list(
            connection.execute(
                "SELECT id,address,source FROM functions WHERE target_id=? ORDER BY address",
                (target,),
            )
        )
        by_id = {row[0]: row[1] for row in functions}
        members = defaultdict(list)
        for identity, _address, source in functions:
            if source:
                members[source].append(identity)
        facts = {
            address: {
                "indexed_selector": identity,
                "source_members": members[source],
                "callees": [],
                "callers": [],
                "macros": [],
                "types": [],
                "data_references": [],
            }
            for identity, address, source in functions
        }
        for caller, callee, site in connection.execute(
            "SELECT caller,callee,callsite FROM calls ORDER BY caller,callee,callsite"
        ):
            if caller in by_id:
                facts[by_id[caller]]["callees"].append(
                    {"function": callee, "callsite": f"0x{site:08X}"}
                )
            if callee in by_id:
                facts[by_id[callee]]["callers"].append(
                    {"function": caller, "callsite": f"0x{site:08X}"}
                )
        for row in macro_uses_payload(connection, target=target, pattern=None, limit=0):
            if row["function_id"] in by_id:
                facts[by_id[row["function_id"]]]["macros"].append(
                    {
                        key: row[key]
                        for key in (
                            "definition_id",
                            "name",
                            "source_path",
                            "source_line",
                        )
                    }
                )
        for row in type_usages_payload(
            connection, target=target, pattern=None, limit=0
        ):
            if row["function_id"] in by_id:
                facts[by_id[row["function_id"]]]["types"].append(
                    {
                        key: row[key]
                        for key in (
                            "type_name",
                            "source_path",
                            "use_kind",
                            "storage_kind",
                            "subject",
                        )
                    }
                )
        for identity, source, address, symbol, access, opcode in connection.execute(
            "SELECT function_id,source,address,symbol,access_kind,opcode "
            "FROM data_references WHERE target_id=? ORDER BY function_id,source,address",
            (target,),
        ):
            if identity in by_id:
                facts[by_id[identity]]["data_references"].append(
                    {
                        "source": f"0x{source:08X}",
                        "address": f"0x{address:08X}",
                        "symbol": symbol,
                        "access_kind": access,
                        "opcode": opcode,
                    }
                )
        return facts
