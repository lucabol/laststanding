# Routing

## Default Routing

| Signal | Route |
| --- | --- |
| Architecture, ambiguous work, reviewer decisions, multi-agent coordination | Ripley |
| `l_os.h`, `l_img.h`, new runtime/image APIs, low-level C implementation | Parker |
| `Taskfile`, `ci.ps1`, compat shims, vendored library wiring, build failures | Dallas |
| `tests/`, CI failures, regression analysis, reviewer passes/fails | Lambert |
| `README.md`, docs sections, examples, user-facing explanations | Brett |
| Decisions, orchestration logs, cross-agent summaries | Scribe |
| Backlog/status polling | Ralph |

## Notes

- Cross-cutting feature work should start with Ripley and fan out to Parker, Dallas, Lambert, and Brett as needed.
- Lambert is the default reviewer for feature implementations unless the user explicitly asks Ripley to review.
