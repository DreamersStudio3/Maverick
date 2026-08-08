---
type: community
members: 2
---

# TODO 작업 진행 관리

**Members:** 2 nodes

## Members
- [[TODO-Based Task Tracking]] - rationale - AGENTS.md
- [[TODOREADME]] - document - AGENTS.md

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/TODO___
SORT file.name ASC
```
