# Decisions

> Canonical decision ledger. Append-only. Scribe merges from decisions/inbox/.

---

## Decision: gh-aw daily-repo-status workflow setup

**Author:** Dallas (Lead)  
**Date:** 2025-07-18

### Decision
Added a daily-repo-status agentic workflow using `gh aw` that generates daily GitHub issues summarizing repo activity.

### Key details
- Workflow file: `.github/workflows/daily-repo-status.md`
- Compiled lock file: `.github/workflows/daily-repo-status.lock.yml`
- Engine: `copilot`
- Imports Squad agent (`../agents/squad.agent.md`) for team context in reports
- Runs daily on a cron schedule (auto-scattered by gh-aw)

### Note on import paths
`gh aw compile` resolves import paths relative to the workflow file location, not the repo root. Use `../agents/` not `.github/agents/` from within `.github/workflows/`.
