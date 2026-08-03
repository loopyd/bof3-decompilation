from __future__ import annotations

from pathlib import Path

import yaml


ROOT = Path(__file__).resolve().parents[3]
WORKFLOW = ROOT / ".github" / "workflows" / "moderate-blocked-users.yml"


def test_blocked_user_moderation_workflow_covers_all_comment_surfaces() -> None:
    workflow = yaml.safe_load(WORKFLOW.read_text(encoding="utf-8"))

    assert workflow[True] == {
        "pull_request_target": {"types": ["opened", "reopened", "edited", "synchronize"]},
        "issues": {"types": ["opened", "reopened", "edited"]},
        "issue_comment": {"types": ["created", "edited"]},
        "pull_request_review": {"types": ["submitted", "edited"]},
        "pull_request_review_comment": {"types": ["created", "edited"]},
        "commit_comment": {"types": ["created"]},
        "discussion": {"types": ["created", "edited"]},
        "discussion_comment": {"types": ["created", "edited"]},
    }
    assert workflow["permissions"] == {
        "discussions": "write",
        "issues": "write",
        "pull-requests": "write",
    }

    job = workflow["jobs"]["moderate"]
    assert "19838093" in job["if"]
    assert ".user.login" not in job["if"]
    script = job["steps"][0]["run"]
    for route in (
        "/issues/$number",
        "/issues/$number/lock",
        "/issues/comments/$comment_id",
        "/pulls/$number/reviews/$review_id/dismissals",
        "/pulls/comments/$comment_id",
        "/comments/$comment_id",
        "deleteDiscussion",
        "deleteDiscussionComment",
    ):
        assert route in script
    assert "actions/checkout" not in WORKFLOW.read_text(encoding="utf-8")
    assert "contents:" not in workflow["permissions"]
