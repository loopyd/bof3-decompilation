from __future__ import annotations

import unittest

from scripts.rebof3.match import import_backlog as MODULE


class MatchImportBacklogTests(unittest.TestCase):
    def test_representative_is_queued_and_group_member_is_deferred(self) -> None:
        payload = MODULE.build_import_backlog_payload(
            {
                "generated_at": "2026-01-01T00:00:00+00:00",
                "inventory_db": "inventory.sqlite",
                "match_root": "tmp/matching",
                "source_root": "bof3",
                "summary": {"blocking_issues": []},
                "entries": [
                    {
                        "archive_id": "BIN/BATTLE/BATTLE",
                        "entry_index": 3,
                        "family": "BATTLE",
                        "payload_path": "build/extracted/BIN/BATTLE/BATTLE.EMI#3",
                        "load_arg": 0x801D0C00,
                        "sha256": "sha-rep",
                        "duplicate_group_key": "BIN/BATTLE/BATTLE#3",
                        "duplicate_group_size": 42,
                        "entry_table_confidence": "high",
                        "imported_program_count": 0,
                        "entry_state": "candidate_missing_program",
                    },
                    {
                        "archive_id": "BIN/BOSS/BOSS001",
                        "entry_index": 0,
                        "family": "BOSS",
                        "payload_path": "build/extracted/BIN/BOSS/BOSS001.EMI#0",
                        "load_arg": 0x801D0C00,
                        "sha256": "sha-member",
                        "duplicate_group_key": "BIN/BATTLE/BATTLE#3",
                        "duplicate_group_size": 42,
                        "entry_table_confidence": None,
                        "imported_program_count": 0,
                        "entry_state": "candidate_missing_program",
                    },
                ],
            }
        )

        self.assertEqual(len(payload["items"]), 1)
        self.assertEqual(payload["items"][0]["archive_id"], "BIN/BATTLE/BATTLE")
        self.assertEqual(
            payload["items"][0]["recommended_action"], "import_representative"
        )
        self.assertEqual(len(payload["deferred_items"]), 1)
        self.assertEqual(payload["deferred_items"][0]["archive_id"], "BIN/BOSS/BOSS001")
        self.assertEqual(
            payload["deferred_items"][0]["item_state"], "deferred_group_member"
        )

    def test_seeded_group_member_is_queued_as_followup(self) -> None:
        payload = MODULE.build_import_backlog_payload(
            {
                "generated_at": "2026-01-01T00:00:00+00:00",
                "inventory_db": "inventory.sqlite",
                "match_root": "tmp/matching",
                "source_root": "bof3",
                "summary": {"blocking_issues": []},
                "entries": [
                    {
                        "archive_id": "BIN/BATTLE/BATTLE",
                        "entry_index": 3,
                        "family": "BATTLE",
                        "payload_path": "build/extracted/BIN/BATTLE/BATTLE.EMI#3",
                        "load_arg": 0x801D0C00,
                        "sha256": "sha-rep",
                        "duplicate_group_key": "BIN/BATTLE/BATTLE#3",
                        "duplicate_group_size": 42,
                        "entry_table_confidence": "high",
                        "imported_program_count": 1,
                        "entry_state": "candidate_imported",
                    },
                    {
                        "archive_id": "BIN/BOSS/BOSS001",
                        "entry_index": 0,
                        "family": "BOSS",
                        "payload_path": "build/extracted/BIN/BOSS/BOSS001.EMI#0",
                        "load_arg": 0x801D0C00,
                        "sha256": "sha-member",
                        "duplicate_group_key": "BIN/BATTLE/BATTLE#3",
                        "duplicate_group_size": 42,
                        "entry_table_confidence": None,
                        "imported_program_count": 0,
                        "entry_state": "candidate_missing_program",
                    },
                ],
            }
        )

        self.assertEqual(len(payload["items"]), 1)
        self.assertEqual(payload["items"][0]["archive_id"], "BIN/BOSS/BOSS001")
        self.assertEqual(payload["items"][0]["recommended_action"], "import_member")
        self.assertEqual(payload["summary"]["queued_items"], 1)

    def test_manual_review_archive_is_deferred(self) -> None:
        payload = MODULE.build_import_backlog_payload(
            {
                "generated_at": "2026-01-01T00:00:00+00:00",
                "inventory_db": "inventory.sqlite",
                "match_root": "tmp/matching",
                "source_root": "bof3",
                "summary": {"blocking_issues": []},
                "entries": [
                    {
                        "archive_id": "BIN/ETC/FIRST",
                        "entry_index": 11,
                        "family": "ETC",
                        "payload_path": "build/extracted/BIN/ETC/FIRST.EMI#11",
                        "load_arg": 0x8001A000,
                        "sha256": "sha-first",
                        "duplicate_group_key": "BIN/ETC/FIRST#11",
                        "duplicate_group_size": 1,
                        "entry_table_confidence": None,
                        "imported_program_count": 0,
                        "entry_state": "candidate_missing_program",
                    }
                ],
            }
        )

        self.assertEqual(payload["items"], [])
        self.assertEqual(len(payload["deferred_items"]), 1)
        self.assertEqual(payload["deferred_items"][0]["item_state"], "manual_review")
        self.assertEqual(payload["summary"]["manual_review_items"], 1)


if __name__ == "__main__":
    unittest.main()
