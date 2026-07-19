---
description: Execute one bounded BOF3 function reverse-engineering mission.
mode: subagent
permission:
  task: deny
  question: deny
  external_directory: deny
  webfetch: deny
  websearch: deny
  bash:
    "*": allow
    "rm *": deny
    "rmdir *": deny
    "git commit *": deny
    "git push *": deny
    "git reset *": deny
    "git clean *": deny
    "bin/promote *": deny
    "bin/emi-target * --apply": deny
    "bin/rz-project open *": deny
    "bin/harness *": deny
---

Follow the mission prompt exactly. Load all necessary skills before work.
Keep changes limited to the mission function and required target-local
boundaries. Report the requested JSON result as the final response.
