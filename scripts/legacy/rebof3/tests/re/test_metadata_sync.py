from __future__ import annotations

import json
import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from scripts.rebof3.inventory.db.connection import connect_inventory_database
from scripts.rebof3.inventory.db.migrations import ensure_inventory_schema
from scripts.rebof3.inventory.repositories.metadata import MetadataRepository
from scripts.rebof3.inventory.repositories.programs import ProgramRepository
from scripts.rebof3.models.inventory import InventoryProgramRow
from scripts.rebof3.re import metadata
from scripts.rebof3.re.services import metadata as metadata_service
from scripts.rebof3.re.services.metadata import service as metadata_service_impl


class MetadataSyncTests(unittest.TestCase):
    def seed_db(self, db_path: Path) -> None:
        connection = connect_inventory_database(db_path)
        ensure_inventory_schema(connection)
        programs = ProgramRepository(connection)
        metadata = MetadataRepository(connection)
        programs.upsert_program(
            InventoryProgramRow(
                program_slug="bins_bin_battle_battle_3_bin",
                program_name="3.bin",
                program_path="/bins/BIN/BATTLE/BATTLE/3.bin",
                folder="/bins/BIN/BATTLE/BATTLE",
                source_hint="build/extracted/BIN/BATTLE/BATTLE.EMI#3",
            )
        )
        connection.execute(
            "INSERT INTO archives(archive_id, archive_name, family, emi_path) VALUES (?, ?, ?, ?)",
            (
                "BIN/BATTLE/BATTLE",
                "BATTLE",
                "BATTLE",
                "build/extracted/BIN/BATTLE/BATTLE.EMI",
            ),
        )
        connection.execute(
            "INSERT INTO emi_entries(archive_id, entry_index, entry_name, type_id, load_arg, size, family, payload_path, code_candidate) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)",
            (
                "BIN/BATTLE/BATTLE",
                3,
                "3.bin",
                0,
                0x801D0C00,
                4096,
                "BATTLE",
                "build/extracted/BIN/BATTLE/BATTLE.EMI#3",
                1,
            ),
        )
        metadata.upsert_row(
            row_key="/bins/BIN/BATTLE/BATTLE/3.bin|structure|BattleState",
            program_path="/bins/BIN/BATTLE/BATTLE/3.bin",
            kind="structure",
            address_key=None,
            address=None,
            entry_text=None,
            path="/Battle/BattleState",
            name="BattleState",
            comment=None,
            repeatable_comment=None,
            type_spec="struct BattleState",
            source="analysis",
            confidence="high",
            tags=[],
            extra={
                "length": 8,
                "components": [
                    {"offset": 0, "length": 4, "field_name": "hp", "type_spec": "int"}
                ],
            },
        )
        metadata.upsert_row(
            row_key="/bins/BIN/BATTLE/BATTLE/3.bin|data|801d0c2c",
            program_path="/bins/BIN/BATTLE/BATTLE/3.bin",
            kind="data",
            address_key="801d0c2c",
            address=0x801D0C2C,
            entry_text="801d0c2c",
            path=None,
            name="switchdataD_801d0c2c",
            comment=None,
            repeatable_comment=None,
            type_spec="[14] PTR_BattleRoundAdvanceCaseHandlers",
            source="analysis",
            confidence="medium",
            tags=[],
            extra={},
        )
        metadata.upsert_row(
            row_key="/bins/BIN/BATTLE/BATTLE/3.bin|label|801d5054",
            program_path="/bins/BIN/BATTLE/BATTLE/3.bin",
            kind="label",
            address_key="801d5054",
            address=0x801D5054,
            entry_text="801d5054",
            path=None,
            name="switchD_801d5054",
            comment=None,
            repeatable_comment=None,
            type_spec="undefined label",
            source="analysis",
            confidence="medium",
            tags=[],
            extra={},
        )
        metadata.upsert_row(
            row_key="/bins/BIN/BATTLE/BATTLE/3.bin|function|801d5024",
            program_path="/bins/BIN/BATTLE/BATTLE/3.bin",
            kind="function",
            address_key="801d5024",
            address=0x801D5024,
            entry_text="801d5024",
            path=None,
            name="battle_dispatch_step",
            comment=None,
            repeatable_comment=None,
            type_spec="int battle_dispatch_step(void (*handler)(int))",
            source="analysis",
            confidence="medium",
            tags=[],
            extra={},
        )
        connection.close()

    def test_build_plan_classifies_rows(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            db_path = Path(tmp_dir) / "inventory.sqlite"
            self.seed_db(db_path)

            plan = metadata_service.build_plan(
                db_path=db_path,
                selectors=("/bins/BIN/BATTLE/BATTLE/3.bin",),
                kind="all",
                mode="preflight",
            )

        by_key = {row.row_key: row for row in plan.row_plans}
        self.assertEqual(
            plan.program_selectors["/bins/BIN/BATTLE/BATTLE/3.bin"],
            "/bins/BIN/BATTLE/BATTLE/BATTLE_e03_801d0c00.bin",
        )
        self.assertEqual(
            by_key["/bins/BIN/BATTLE/BATTLE/3.bin|data|801d0c2c"].classification,
            "blocked_missing_dependency",
        )
        self.assertEqual(
            by_key["/bins/BIN/BATTLE/BATTLE/3.bin|label|801d5054"].classification,
            "skip_pseudo_type",
        )
        self.assertEqual(
            by_key["/bins/BIN/BATTLE/BATTLE/3.bin|function|801d5024"].classification,
            "apply_direct",
        )
        self.assertIn(
            "/bins/BIN/BATTLE/BATTLE/3.bin|typedef|HandlerCallback",
            by_key,
        )
        self.assertEqual(
            by_key[
                "/bins/BIN/BATTLE/BATTLE/3.bin|typedef|HandlerCallback"
            ].classification,
            "apply_direct",
        )
        self.assertEqual(
            by_key["/bins/BIN/BATTLE/BATTLE/3.bin|function|801d5024"].row["type_spec"],
            "int battle_dispatch_step(HandlerCallback handler)",
        )

    def test_build_plan_skips_default_named_type_less_data_rows(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            db_path = Path(tmp_dir) / "inventory.sqlite"
            self.seed_db(db_path)
            connection = connect_inventory_database(db_path)
            repository = MetadataRepository(connection)
            repository.upsert_row(
                row_key="/bins/BIN/BATTLE/BATTLE/3.bin|data|80097eb8",
                program_path="/bins/BIN/BATTLE/BATTLE/3.bin",
                kind="data",
                address_key="80097eb8",
                address=0x80097EB8,
                entry_text="80097eb8",
                path=None,
                name="SUB_80097eb8",
                comment=None,
                repeatable_comment=None,
                type_spec=None,
                source=None,
                confidence=None,
                tags=[],
                extra={"name_source": "DEFAULT"},
            )
            connection.close()

            plan = metadata_service.build_plan(
                db_path=db_path,
                selectors=("/bins/BIN/BATTLE/BATTLE/3.bin",),
                kind="all",
                mode="preflight",
            )

        by_key = {row.row_key: row for row in plan.row_plans}
        self.assertEqual(
            by_key["/bins/BIN/BATTLE/BATTLE/3.bin|data|80097eb8"].classification,
            "apply_direct",
        )

    def test_build_plan_skips_analysis_internal_label_data_rows(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            db_path = Path(tmp_dir) / "inventory.sqlite"
            self.seed_db(db_path)
            connection = connect_inventory_database(db_path)
            repository = MetadataRepository(connection)
            repository.upsert_row(
                row_key="/bins/BIN/BATTLE/BATTLE/3.bin|data|801d505c",
                program_path="/bins/BIN/BATTLE/BATTLE/3.bin",
                kind="data",
                address_key="801d505c",
                address=0x801D505C,
                entry_text="801d505c",
                path=None,
                name="caseD_0",
                comment=None,
                repeatable_comment=None,
                type_spec=None,
                source=None,
                confidence=None,
                tags=[],
                extra={"name_source": "ANALYSIS"},
            )
            connection.close()

            plan = metadata_service.build_plan(
                db_path=db_path,
                selectors=("/bins/BIN/BATTLE/BATTLE/3.bin",),
                kind="all",
                mode="preflight",
            )

        by_key = {row.row_key: row for row in plan.row_plans}
        self.assertEqual(
            by_key["/bins/BIN/BATTLE/BATTLE/3.bin|data|801d505c"].classification,
            "apply_direct",
        )

    def test_build_plan_applies_named_type_less_user_defined_data_rows(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            db_path = Path(tmp_dir) / "inventory.sqlite"
            self.seed_db(db_path)
            connection = connect_inventory_database(db_path)
            repository = MetadataRepository(connection)
            repository.upsert_row(
                row_key="/bins/BIN/BATTLE/BATTLE/3.bin|data|800a9820",
                program_path="/bins/BIN/BATTLE/BATTLE/3.bin",
                kind="data",
                address_key="800a9820",
                address=0x800A9820,
                entry_text="800a9820",
                path=None,
                name="battle_dispatch_selected_target",
                comment=None,
                repeatable_comment=None,
                type_spec=None,
                source=None,
                confidence=None,
                tags=[],
                extra={"name_source": "USER_DEFINED"},
            )
            connection.close()

            plan = metadata_service.build_plan(
                db_path=db_path,
                selectors=("/bins/BIN/BATTLE/BATTLE/3.bin",),
                kind="all",
                mode="preflight",
            )

        by_key = {row.row_key: row for row in plan.row_plans}
        self.assertEqual(
            by_key["/bins/BIN/BATTLE/BATTLE/3.bin|data|800a9820"].classification,
            "apply_direct",
        )

    def test_ghidra_payload_uses_resolved_program_selector(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            db_path = Path(tmp_dir) / "inventory.sqlite"
            self.seed_db(db_path)

            plan = metadata_service.build_plan(
                db_path=db_path,
                selectors=("/bins/BIN/BATTLE/BATTLE/3.bin",),
                kind="all",
                mode="preflight",
            )

        payload = metadata_service._ghidra_metadata_payload(plan)
        program_paths = {str(row.get("program_path")) for row in payload["rows"]}
        self.assertEqual(
            program_paths,
            {"/bins/BIN/BATTLE/BATTLE/BATTLE_e03_801d0c00.bin"},
        )

    def test_build_plan_uses_known_type_names_to_satisfy_dependencies(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            db_path = Path(tmp_dir) / "inventory.sqlite"
            self.seed_db(db_path)

            plan = metadata_service.build_plan(
                db_path=db_path,
                selectors=("/bins/BIN/BATTLE/BATTLE/3.bin",),
                kind="all",
                mode="preflight",
                known_type_names=("PTR_BattleRoundAdvanceCaseHandlers",),
            )

        by_key = {row.row_key: row for row in plan.row_plans}
        self.assertEqual(
            by_key["/bins/BIN/BATTLE/BATTLE/3.bin|data|801d0c2c"].classification,
            "apply_normalized",
        )

    def test_load_known_type_names_reads_ghidra_payload(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            db_path = Path(tmp_dir) / "inventory.sqlite"
            self.seed_db(db_path)
            output_path = Path(tmp_dir) / "known_types.json"

            def fake_run_command(command, cwd, env):
                self.assertIn("known-types", command)
                self.assertIn(
                    "/bins/BIN/BATTLE/BATTLE/BATTLE_e03_801d0c00.bin", command
                )
                output_path.write_text(
                    json.dumps(
                        {
                            "schema": "bof3.metadata.known_types/v1",
                            "programs": [
                                {
                                    "program_path": "/bins/BIN/BATTLE/BATTLE/3.bin",
                                    "type_names": [
                                        "BattleState",
                                        "PTR_BattleRoundAdvanceCaseHandlers",
                                    ],
                                }
                            ],
                        }
                    ),
                    encoding="utf-8",
                )
                return subprocess.CompletedProcess(command, 0, stdout="", stderr="")

            with patch.object(
                metadata_service, "run_command", side_effect=fake_run_command
            ):
                known_type_names = metadata_service.load_known_type_names(
                    db_path=db_path,
                    selectors=("/bins/BIN/BATTLE/BATTLE/3.bin",),
                    project_dir=Path(tmp_dir) / "project",
                    project_name="bof3_main",
                    output_path=output_path,
                )

        self.assertIn("BattleState", known_type_names)
        self.assertIn("PTR_BattleRoundAdvanceCaseHandlers", known_type_names)

    def test_execute_preflight_returns_nonzero_when_blocked_rows_remain(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            db_path = Path(tmp_dir) / "inventory.sqlite"
            self.seed_db(db_path)
            output_path = Path(tmp_dir) / "preflight.json"
            with patch.object(
                metadata_service,
                "run_ghidra_metadata",
                return_value=(0, {"rows": []}, "", ""),
            ):
                status = metadata.execute(
                    metadata.parse_args(
                        [
                            "sync",
                            "to",
                            "--mode",
                            "preflight",
                            "--db",
                            str(db_path),
                            "--program",
                            "/bins/BIN/BATTLE/BATTLE/3.bin",
                            "--output",
                            str(output_path),
                            "--json",
                        ]
                    )
                )

        self.assertEqual(status, 1)

    def test_execute_apply_returns_nonzero_on_ghidra_failures(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            db_path = Path(tmp_dir) / "inventory.sqlite"
            self.seed_db(db_path)
            output_path = Path(tmp_dir) / "apply.json"
            with (
                patch.object(
                    metadata_service_impl,
                    "build_plan",
                    wraps=metadata_service.build_plan,
                ),
                patch.object(
                    metadata_service_impl,
                    "run_ghidra_metadata",
                    return_value=(
                        0,
                        {"rows": [{"status": "type_not_found", "address": "801d0c2c"}]},
                        "",
                        "",
                    ),
                ),
            ):
                status = metadata.execute(
                    metadata.parse_args(
                        [
                            "sync",
                            "to",
                            "--mode",
                            "apply",
                            "--db",
                            str(db_path),
                            "--program",
                            "/bins/BIN/BATTLE/BATTLE/3.bin",
                            "--kind",
                            "label",
                            "--output",
                            str(output_path),
                        ]
                    )
                )

        self.assertEqual(status, 1)

    def test_execute_writes_combined_payload_to_output_when_requested(self) -> None:
        payload = None
        with tempfile.TemporaryDirectory() as tmp_dir:
            db_path = Path(tmp_dir) / "inventory.sqlite"
            self.seed_db(db_path)
            output_path = Path(tmp_dir) / "preflight.json"
            with patch.object(
                metadata_service,
                "run_ghidra_metadata",
                return_value=(0, {"rows": []}, "", ""),
            ):
                metadata.execute(
                    metadata.parse_args(
                        [
                            "sync",
                            "to",
                            "--mode",
                            "preflight",
                            "--db",
                            str(db_path),
                            "--program",
                            "/bins/BIN/BATTLE/BATTLE/3.bin",
                            "--output",
                            str(output_path),
                        ]
                    )
                )
            payload = json.loads(output_path.read_text(encoding="utf-8"))

        assert payload is not None
        self.assertIn("plan", payload)
        self.assertIn("blocked_rows", payload)

    def test_sync_from_capture_defaults_include_default(self) -> None:
        args = metadata.parse_args(["sync", "from"])
        self.assertTrue(args.include_default)

    def test_sync_from_preflight_does_not_persist_rows(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            db_path = Path(tmp_dir) / "inventory.sqlite"
            self.seed_db(db_path)

            def fake_run_command(command, cwd, env):
                output_path = Path(command[command.index("--output") + 1])
                output_path.write_text(
                    json.dumps(
                        {
                            "schema": "bof3.metadata/v1",
                            "rows": [
                                {
                                    "kind": "function",
                                    "program_path": "/bins/BIN/BATTLE/BATTLE/BATTLE_e03_801d0c00.bin",
                                    "address": "801d5024",
                                    "name": "battle_dispatch_step",
                                    "type_spec": "int battle_dispatch_step(void)",
                                }
                            ],
                        }
                    ),
                    encoding="utf-8",
                )
                return subprocess.CompletedProcess(command, 0, stdout="", stderr="")

            with patch.object(
                metadata_service, "run_command", side_effect=fake_run_command
            ):
                report = metadata.execute(
                    metadata.parse_args(
                        [
                            "sync",
                            "from",
                            "--mode",
                            "preflight",
                            "--db",
                            str(db_path),
                            "--program",
                            "/bins/BIN/BATTLE/BATTLE/3.bin",
                            "--json",
                        ]
                    )
                )

            self.assertEqual(report, 0)
            connection = connect_inventory_database(db_path)
            try:
                rows = connection.execute(
                    "SELECT COUNT(*) AS count FROM metadata_rows WHERE row_key = ?",
                    ("/bins/BIN/BATTLE/BATTLE/3.bin|function|801d5024",),
                ).fetchone()
            finally:
                connection.close()
        self.assertEqual(rows["count"], 1)

    def test_sync_from_normalizes_ghidra_overlay_program_paths(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            db_path = Path(tmp_dir) / "inventory.sqlite"
            self.seed_db(db_path)

            def fake_run_command(command, cwd, env):
                self.assertIn("capture", command)
                output_path = Path(command[command.index("--output") + 1])
                output_path.write_text(
                    json.dumps(
                        {
                            "schema": "bof3.metadata/v1",
                            "rows": [
                                {
                                    "kind": "function",
                                    "program_path": "/bins/BIN/BATTLE/BATTLE/BATTLE_e03_801d0c00.bin",
                                    "address": "801d5024",
                                    "name": "battle_dispatch_step",
                                    "comment": "dispatch helper",
                                    "repeatable_comment": None,
                                    "type_spec": "int battle_dispatch_step(void)",
                                    "name_source": "USER_DEFINED",
                                    "namespace": "Battle",
                                    "is_thunk": False,
                                    "body_min": "801d5024",
                                    "body_max": "801d50ff",
                                }
                            ],
                        }
                    ),
                    encoding="utf-8",
                )
                return subprocess.CompletedProcess(command, 0, stdout="", stderr="")

            with patch.object(
                metadata_service, "run_command", side_effect=fake_run_command
            ):
                report = metadata_service.capture_into_inventory(
                    db_path=db_path,
                    selectors=("/bins/BIN/BATTLE/BATTLE/3.bin",),
                    kind="function",
                    project_dir=Path(tmp_dir) / "project",
                    project_name="bof3_main",
                )

            self.assertEqual(report["canonical_program_count"], 1)
            self.assertEqual(report["row_count"], 1)
            connection = connect_inventory_database(db_path)
            try:
                row = connection.execute(
                    "SELECT program_path, row_key FROM metadata_rows ORDER BY id DESC LIMIT 1"
                ).fetchone()
                function_row = connection.execute(
                    """
                    SELECT programs.program_path AS program_path, functions.body_min, functions.body_max
                    FROM functions
                    JOIN programs ON programs.id = functions.program_id
                    ORDER BY functions.id DESC LIMIT 1
                    """
                ).fetchone()
            finally:
                connection.close()
        self.assertEqual(row["program_path"], "/bins/BIN/BATTLE/BATTLE/3.bin")
        self.assertEqual(
            row["row_key"],
            "/bins/BIN/BATTLE/BATTLE/3.bin|function|801d5024",
        )
        self.assertEqual(function_row["program_path"], "/bins/BIN/BATTLE/BATTLE/3.bin")
        self.assertEqual(function_row["body_min"], 0x801D5024)
        self.assertEqual(function_row["body_max"], 0x801D50FF)

    def test_sync_from_overlay_path_fallback_strips_bins_prefix(self) -> None:
        with tempfile.TemporaryDirectory() as tmp_dir:
            db_path = Path(tmp_dir) / "inventory.sqlite"
            connection = connect_inventory_database(db_path)
            ensure_inventory_schema(connection)
            connection.close()

            def fake_run_command(command, cwd, env):
                self.assertIn("capture", command)
                output_path = Path(command[command.index("--output") + 1])
                output_path.write_text(
                    json.dumps(
                        {
                            "schema": "bof3.metadata/v1",
                            "rows": [
                                {
                                    "kind": "function",
                                    "program_path": "/bins/BIN/BATTLE/BATTLE/BATTLE_e03_801d0c00.bin",
                                    "address": "801d5024",
                                    "name": "battle_dispatch_step",
                                    "comment": None,
                                    "repeatable_comment": None,
                                    "type_spec": "int battle_dispatch_step(void)",
                                    "name_source": "USER_DEFINED",
                                    "namespace": "Battle",
                                    "is_thunk": False,
                                    "body_min": "801d5024",
                                    "body_max": "801d50ff",
                                }
                            ],
                        }
                    ),
                    encoding="utf-8",
                )
                return subprocess.CompletedProcess(command, 0, stdout="", stderr="")

            with patch.object(
                metadata_service, "run_command", side_effect=fake_run_command
            ):
                report = metadata_service.capture_into_inventory(
                    db_path=db_path,
                    selectors=("/bins/BIN/BATTLE/BATTLE/3.bin",),
                    kind="function",
                    project_dir=Path(tmp_dir) / "project",
                    project_name="bof3_main",
                )

            self.assertEqual(report["canonical_program_count"], 1)
            connection = connect_inventory_database(db_path)
            try:
                program_row = connection.execute(
                    "SELECT program_path, folder, source_hint FROM programs ORDER BY id DESC LIMIT 1"
                ).fetchone()
            finally:
                connection.close()
        self.assertEqual(program_row["program_path"], "/bins/BIN/BATTLE/BATTLE/3.bin")
        self.assertEqual(program_row["folder"], "/bins/BIN/BATTLE/BATTLE")
        self.assertEqual(
            program_row["source_hint"], "build/extracted/BIN/BATTLE/BATTLE.EMI#3"
        )
