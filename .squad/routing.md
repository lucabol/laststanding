# Routing

## Rules

| Pattern | Route To | Notes |
|---------|----------|-------|
| Architecture, design, scope decisions | Dallas | Lead reviews all major changes |
| Code review, PR review | Dallas | Lead gates merges |
| C implementation, syscalls, porting, platform code | Parker | Core dev for all l_os.h work |
| New l_ functions, inline asm, Windows API | Parker | Primary implementer |
| Tests, test cases, edge cases | Lambert | Tester writes and reviews tests |
| Cross-platform verification, build verification | Lambert | Tester validates builds |
| Bug fixes (code) | Parker | Then Lambert verifies |
| Bug fixes (tests) | Lambert | Then Dallas reviews |
| Documentation, README | Dallas | Lead owns docs |
| Build scripts, Taskfile, batch files | Parker | Build tooling |

## Fallback

If unclear, route to Dallas for triage.
