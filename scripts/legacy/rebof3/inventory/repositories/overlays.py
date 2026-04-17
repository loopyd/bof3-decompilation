from __future__ import annotations

import sqlite3
from collections import defaultdict
from pathlib import Path
from typing import Any

from ..direct_overlay_catalog import candidate_name as build_candidate_name


class OverlayRepository:
    def __init__(self, connection: sqlite3.Connection):
        self.connection = connection

    def load_candidates(self) -> list[dict[str, Any]]:
        rows = self.connection.execute(
            """
            SELECT
                entries.archive_id,
                archives.archive_name,
                entries.entry_index,
                entries.entry_name,
                entries.family,
                entries.payload_path,
                entries.load_arg,
                entries.size,
                entries.first_word,
                entries.sha256,
                archives.emi_path
            FROM emi_entries AS entries
            JOIN archives ON archives.archive_id = entries.archive_id
            WHERE entries.code_candidate = 1
            ORDER BY entries.family, entries.archive_id, entries.entry_index
            """
        ).fetchall()
        candidates: list[dict[str, Any]] = []
        duplicate_sizes = self._duplicate_group_sizes()
        for row in rows:
            load_arg = int(row["load_arg"] or 0)
            entry_index = int(row["entry_index"])
            sha256 = str(row["sha256"] or "")
            candidates.append(
                {
                    "archive_id": row["archive_id"],
                    "archive_name": row["archive_name"],
                    "candidate_name": build_candidate_name(
                        str(row["family"] or "unknown"),
                        str(row["archive_name"] or Path(str(row["archive_id"])).name),
                        entry_index,
                        load_arg,
                    ),
                    "emi_path": row["emi_path"],
                    "entry_index": entry_index,
                    "entry_name": row["entry_name"],
                    "family": row["family"],
                    "first4": row["first_word"],
                    "payload_path": row["payload_path"],
                    "ram_ptr": load_arg,
                    "ram_ptr_hex": f"0x{load_arg:08x}",
                    "sha256": sha256,
                    "size": int(row["size"]),
                    "duplicate_group_size": duplicate_sizes.get(sha256, 1),
                }
            )
        return candidates

    def _duplicate_group_sizes(self) -> dict[str, int]:
        rows = self.connection.execute(
            """
            SELECT sha256, COUNT(*) AS group_size
            FROM emi_entries
            WHERE code_candidate = 1 AND sha256 IS NOT NULL AND sha256 != ''
            GROUP BY sha256
            """
        ).fetchall()
        return {str(row["sha256"]): int(row["group_size"]) for row in rows}

    def build_clusters(self) -> dict[str, Any]:
        candidates = self.load_candidates()
        by_hash: dict[str, list[dict[str, Any]]] = defaultdict(list)
        by_region: dict[tuple[str, int], list[dict[str, Any]]] = defaultdict(list)
        for candidate in candidates:
            by_hash[candidate["sha256"]].append(candidate)
            by_region[(candidate["ram_ptr_hex"], candidate["size"])].append(candidate)

        exact_groups = []
        for sha256, members in by_hash.items():
            if not sha256 or len(members) < 2:
                continue
            members = sorted(
                members,
                key=lambda item: (
                    item["family"],
                    item["archive_id"],
                    item["entry_index"],
                ),
            )
            exact_groups.append(
                {
                    "sha256": sha256,
                    "group_size": len(members),
                    "representative": members[0]["candidate_name"],
                    "load_addresses": sorted(
                        {member["ram_ptr_hex"] for member in members}
                    ),
                    "families": sorted({member["family"] for member in members}),
                    "members": [
                        {
                            "candidate_name": member["candidate_name"],
                            "archive_id": member["archive_id"],
                            "entry_index": member["entry_index"],
                            "payload_path": member["payload_path"],
                            "ram_ptr_hex": member["ram_ptr_hex"],
                            "size": member["size"],
                        }
                        for member in members
                    ],
                }
            )
        exact_groups.sort(
            key=lambda group: (-group["group_size"], group["representative"])
        )

        region_groups = []
        for (ram_ptr_hex, size), members in by_region.items():
            if len(members) < 2:
                continue
            distinct_hashes = sorted(
                {member["sha256"] for member in members if member["sha256"]}
            )
            region_groups.append(
                {
                    "ram_ptr_hex": ram_ptr_hex,
                    "size": size,
                    "member_count": len(members),
                    "distinct_hash_count": len(distinct_hashes),
                    "families": sorted({member["family"] for member in members}),
                    "representative_candidates": sorted(
                        {member["candidate_name"] for member in members}
                    )[:16],
                }
            )
        region_groups.sort(
            key=lambda group: (
                -group["member_count"],
                group["ram_ptr_hex"],
                group["size"],
            )
        )

        return {
            "generated_from": "processed/inventory/inventory.sqlite",
            "exact_duplicate_group_count": len(exact_groups),
            "region_cluster_count": len(region_groups),
            "exact_duplicate_groups": exact_groups,
            "region_clusters": region_groups,
        }

    def build_unique_overlay_map(self) -> dict[str, Any]:
        candidates = self.load_candidates()
        by_sha: dict[str, list[dict[str, Any]]] = defaultdict(list)
        for candidate in candidates:
            by_sha[candidate["sha256"]].append(candidate)

        representatives: list[dict[str, Any]] = []
        alias_map: dict[str, str] = {}
        alias_rows: list[tuple[str, int, str, int]] = []

        for sha256, group_candidates in sorted(by_sha.items()):
            group_candidates = sorted(
                group_candidates,
                key=lambda candidate: (
                    candidate["family"],
                    candidate["archive_id"],
                    candidate["entry_index"],
                    candidate["candidate_name"],
                ),
            )
            representative = group_candidates[0]
            representatives.append(
                {
                    "representative_name": representative["candidate_name"],
                    "representative": self.member_record(representative),
                    "sha256": sha256,
                    "group_size": len(group_candidates),
                    "families": sorted(
                        {candidate["family"] for candidate in group_candidates}
                    ),
                    "load_addresses": sorted(
                        {candidate["ram_ptr_hex"] for candidate in group_candidates}
                    ),
                    "members": [
                        self.member_record(candidate) for candidate in group_candidates
                    ],
                }
            )
            for candidate in group_candidates:
                alias_map[candidate["candidate_name"]] = representative[
                    "candidate_name"
                ]
                alias_rows.append(
                    (
                        str(candidate["archive_id"]),
                        int(candidate["entry_index"]),
                        str(representative["archive_id"]),
                        int(representative["entry_index"]),
                    )
                )

        representatives.sort(
            key=lambda group: (
                -group["group_size"],
                group["representative"]["family"],
                group["representative"]["archive_id"],
                group["representative"]["entry_index"],
            )
        )

        with self.connection:
            self.connection.execute("DELETE FROM overlay_aliases")
            self.connection.executemany(
                """
                INSERT INTO overlay_aliases(
                    archive_id, entry_index, representative_archive_id, representative_entry_index
                ) VALUES (?, ?, ?, ?)
                """,
                alias_rows,
            )

        return {
            "generated_from": {
                "overlay_candidates": "processed/inventory/inventory.sqlite",
                "overlay_clusters": "processed/inventory/inventory.sqlite",
            },
            "candidate_count": len(candidates),
            "representative_count": len(representatives),
            "exact_duplicate_representative_count": sum(
                1 for group in representatives if group["group_size"] > 1
            ),
            "singleton_representative_count": sum(
                1 for group in representatives if group["group_size"] == 1
            ),
            "representatives": representatives,
            "alias_to_representative": alias_map,
            "unmapped_candidate_count": 0,
        }

    def replace_entry_tables(self, rows: list[dict[str, Any]]) -> None:
        with self.connection:
            self.connection.execute("DELETE FROM overlay_entry_points")
            self.connection.execute("DELETE FROM overlay_entry_tables")
            for row in rows:
                cursor = self.connection.execute(
                    """
                    INSERT INTO overlay_entry_tables(
                        archive_id,
                        entry_index,
                        entry_count,
                        entry_in_range_count,
                        confidence,
                        payload_path
                    ) VALUES (?, ?, ?, ?, ?, ?)
                    RETURNING id
                    """,
                    (
                        row["archive_id"],
                        int(row["entry_index"]),
                        int(row.get("entry_count") or row.get("first_word") or 0),
                        int(row.get("entry_in_range_count") or 0),
                        str(row.get("confidence") or ""),
                        row.get("payload_path"),
                    ),
                )
                table_id = int(cursor.fetchone()["id"])
                for table_index, address_text in enumerate(
                    row.get("entry_addresses", [])
                ):
                    self.connection.execute(
                        """
                        INSERT INTO overlay_entry_points(
                            table_id, table_index, address, address_hex, label_name, label_comment
                        ) VALUES (?, ?, ?, ?, ?, ?)
                        """,
                        (
                            table_id,
                            table_index,
                            int(str(address_text), 16),
                            str(address_text),
                            None,
                            None,
                        ),
                    )

    def load_entry_table_rows(self) -> list[dict[str, Any]]:
        rows = self.connection.execute(
            """
            SELECT
                tables.archive_id,
                tables.entry_index,
                entries.family,
                entries.payload_path,
                entries.load_arg,
                tables.entry_count,
                tables.entry_in_range_count,
                tables.confidence,
                archives.archive_name,
                entries.entry_name,
                entries.size,
                entries.first_word
            FROM overlay_entry_tables AS tables
            JOIN emi_entries AS entries
                ON entries.archive_id = tables.archive_id
               AND entries.entry_index = tables.entry_index
            JOIN archives ON archives.archive_id = tables.archive_id
            ORDER BY tables.archive_id, tables.entry_index
            """
        ).fetchall()
        point_rows = self.connection.execute(
            """
            SELECT
                tables.archive_id,
                tables.entry_index,
                points.table_index,
                points.address_hex
            FROM overlay_entry_points AS points
            JOIN overlay_entry_tables AS tables ON tables.id = points.table_id
            ORDER BY tables.archive_id, tables.entry_index, points.table_index
            """
        ).fetchall()
        addresses_by_key: dict[tuple[str, int], list[str]] = defaultdict(list)
        for row in point_rows:
            addresses_by_key[(str(row["archive_id"]), int(row["entry_index"]))].append(
                str(row["address_hex"])
            )

        result: list[dict[str, Any]] = []
        for row in rows:
            archive_id = str(row["archive_id"])
            entry_index = int(row["entry_index"])
            load_arg = int(row["load_arg"] or 0)
            result.append(
                {
                    "archive_id": archive_id,
                    "candidate_name": build_candidate_name(
                        str(row["family"] or "unknown"),
                        str(row["archive_name"]),
                        entry_index,
                        load_arg,
                    ),
                    "entry_index": entry_index,
                    "entry_name": row["entry_name"],
                    "payload_path": row["payload_path"],
                    "ram_ptr_hex": f"0x{load_arg:08x}",
                    "first_word": int(row["first_word"] or 0),
                    "entry_count": int(row["entry_count"] or 0),
                    "entry_in_range_count": int(row["entry_in_range_count"] or 0),
                    "entry_addresses": addresses_by_key.get(
                        (archive_id, entry_index), []
                    ),
                    "family": row["family"],
                    "size": int(row["size"] or 0),
                    "confidence": row["confidence"],
                }
            )
        return result

    @staticmethod
    def member_record(candidate: dict[str, Any]) -> dict[str, Any]:
        record = {
            "candidate_name": candidate["candidate_name"],
            "archive_id": candidate["archive_id"],
            "entry_index": candidate["entry_index"],
            "family": candidate["family"],
            "payload_path": candidate["payload_path"],
            "ram_ptr_hex": candidate["ram_ptr_hex"],
            "sha256": candidate["sha256"],
            "size": candidate["size"],
        }
        if "emi_path" in candidate:
            record["emi_path"] = candidate["emi_path"]
        return record
