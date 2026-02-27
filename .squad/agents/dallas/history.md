# Dallas — History

## Core Context
- **Project:** laststanding — freestanding C runtime, no libc, direct syscalls
- **Stack:** C (freestanding), inline asm, GCC/Clang/MinGW, Win32 API
- **User:** Luca Bolognese
- **Key file:** l_os.h (single header, ~900 lines)
- **Tests:** test/*.c → bin/*, run via Taskfile or test_all.bat

## Learnings
- **gh-aw workflow imports**: Import paths in `.github/workflows/*.md` frontmatter are resolved relative to the workflow file, not the repo root. Use `../agents/squad.agent.md` not `.github/agents/squad.agent.md`.
- **gh-aw compile**: After creating/editing a `.md` workflow, run `gh aw compile` to generate the `.lock.yml` file. Both files must be committed.
- **Daily repo status workflow**: Lives at `.github/workflows/daily-repo-status.md`, uses `engine: copilot`, imports Squad agent, runs daily via cron.
