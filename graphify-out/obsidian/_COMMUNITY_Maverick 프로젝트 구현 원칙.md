---
type: community
cohesion: 0.40
members: 5
---

# Maverick 프로젝트 구현 원칙

**Cohesion:** 0.40 - moderately connected
**Members:** 5 nodes

## Members
- [[Character and Domain Responsibility Separation]] - rationale - AGENTS.md
- [[Maverick Project Context]] - concept - AGENTS.md
- [[Policy-First Implementation and Debugging]] - rationale - AGENTS.md
- [[Trace-First Debugging Policy]] - rationale - POLICY.md
- [[Unity Build Identifier Safety]] - rationale - POLICY.md

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/Maverick___
SORT file.name ASC
```
