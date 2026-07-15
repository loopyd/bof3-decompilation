---
description: Execute one bounded BOF3 function reverse-engineering mission.
mode: primary
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
    "bin/harness reverse *": deny
    "git commit *": deny
    "git push *": deny
    "git reset *": deny
    "git clean *": deny
    "bin/harness promote *": deny
    "bin/harness setup *": deny
    "bin/setup-*": deny
---

Follow the mission prompt exactly. Load the requested BOF3 skills before work.
Keep changes limited to the mission function and required target-local
boundaries. Report the requested JSON result as the final response.
