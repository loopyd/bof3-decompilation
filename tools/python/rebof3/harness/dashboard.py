from __future__ import annotations

from html import escape
from pathlib import Path
from typing import Any


SECTIONS = (
    "overview",
    "artifacts",
    "modules",
    "queue",
    "active claims",
    "blockers",
    "target detail",
    "tool health",
)


def _table(rows: list[dict[str, Any]], columns: list[str]) -> str:
    if not rows:
        return '<p class="muted">No rows.</p>'
    header = "".join(f"<th>{escape(column)}</th>" for column in columns)
    body = []
    for row in rows:
        cells = "".join(
            f"<td>{escape(str(row.get(column, '')))}</td>" for column in columns
        )
        body.append(f"<tr>{cells}</tr>")
    return (
        f"<table><thead><tr>{header}</tr></thead><tbody>{''.join(body)}</tbody></table>"
    )


def render_dashboard(report: dict[str, Any]) -> str:
    status_cards = "".join(
        f'<div class="metric"><span>{escape(status)}</span><strong>{count}</strong></div>'
        for status, count in report["counts_by_status"].items()
    )
    type_rows = [
        {"type": type_name, "count": count}
        for type_name, count in report["counts_by_type"].items()
    ]
    health_rows = report["tool_health"]
    return f"""<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>rebof3 harness</title>
  <style>
    :root {{
      color-scheme: light;
      --ink: #20242a;
      --muted: #667085;
      --line: #d9dee7;
      --band: #f5f7fa;
      --accent: #0f766e;
      --warn: #9f580a;
    }}
    * {{ box-sizing: border-box; }}
    body {{
      margin: 0;
      font: 14px/1.45 system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
      color: var(--ink);
      background: #ffffff;
    }}
    header {{
      border-bottom: 1px solid var(--line);
      padding: 20px 28px 16px;
    }}
    h1 {{ margin: 0 0 4px; font-size: 24px; font-weight: 650; }}
    h2 {{ margin: 0 0 12px; font-size: 16px; font-weight: 650; }}
    main {{ display: grid; grid-template-columns: 220px 1fr; min-height: calc(100vh - 74px); }}
    nav {{ border-right: 1px solid var(--line); background: var(--band); padding: 18px; }}
    nav a {{ display: block; color: var(--ink); text-decoration: none; padding: 5px 0; }}
    section {{ padding: 22px 28px; border-bottom: 1px solid var(--line); }}
    .muted {{ color: var(--muted); }}
    .metrics {{ display: flex; flex-wrap: wrap; gap: 10px; }}
    .metric {{ border: 1px solid var(--line); border-radius: 6px; padding: 10px 12px; min-width: 120px; }}
    .metric span {{ display: block; color: var(--muted); font-size: 12px; }}
    .metric strong {{ font-size: 22px; color: var(--accent); }}
    table {{ width: 100%; border-collapse: collapse; }}
    th, td {{ border-bottom: 1px solid var(--line); padding: 7px 8px; text-align: left; vertical-align: top; }}
    th {{ color: var(--muted); font-size: 12px; font-weight: 650; }}
    @media (max-width: 760px) {{
      main {{ display: block; }}
      nav {{ border-right: 0; border-bottom: 1px solid var(--line); }}
    }}
  </style>
</head>
<body>
  <header>
    <h1>rebof3 harness</h1>
    <div class="muted">{escape(str(report.get("database", "")))}</div>
  </header>
  <main>
    <nav>
      {"".join(f'<a href="#{escape(section.replace(" ", "-"))}">{escape(section.title())}</a>' for section in SECTIONS)}
    </nav>
    <div>
      <section id="overview">
        <h2>Overview</h2>
        <div class="metrics">{status_cards}</div>
      </section>
      <section id="artifacts">
        <h2>Artifacts</h2>
        {_table(type_rows, ["type", "count"])}
      </section>
      <section id="modules">
        <h2>Modules</h2>
        {_table(type_rows, ["type", "count"])}
      </section>
      <section id="queue">
        <h2>Queue</h2>
        {_table(report["next_targets"], ["id", "type", "priority", "summary", "source_hint"])}
      </section>
      <section id="active-claims">
        <h2>Active Claims</h2>
        {_table(report["active_claims"], ["target_id", "owner", "expires_at", "summary"])}
      </section>
      <section id="blockers">
        <h2>Blockers</h2>
        {_table(report["blockers"], ["id", "type", "summary", "source_hint"])}
      </section>
      <section id="target-detail">
        <h2>Target Detail</h2>
        {_table(report["next_targets"][:5], ["id", "program_path", "entry_hex", "summary"])}
      </section>
      <section id="tool-health">
        <h2>Tool Health</h2>
        {_table(health_rows, ["name", "status", "detail"])}
      </section>
    </div>
  </main>
</body>
</html>
"""


def write_dashboard(report: dict[str, Any], output_dir: Path) -> Path:
    output_dir.mkdir(parents=True, exist_ok=True)
    path = output_dir / "index.html"
    path.write_text(render_dashboard(report), encoding="utf-8")
    return path
