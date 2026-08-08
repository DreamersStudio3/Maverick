---
type: community
cohesion: 0.22
members: 9
---

# 문서·Graphify 거버넌스

**Cohesion:** 0.22 - loosely connected
**Members:** 9 nodes

## Members
- [[Build and Impact Verification]] - rationale - .github/pull_request_template.md
- [[Commit-Time Incremental Update]] - rationale - docs/wiki/Documentation-Workflow.md
- [[Graphify Doxygen Extraction Gap]] - rationale - docs/wiki/Header-Documentation.md
- [[Graphify Generated Read Models]] - rationale - docs/wiki/Documentation-Workflow.md
- [[Information Hierarchy]] - rationale - docs/wiki/Documentation-Workflow.md
- [[Local Contract and Canonical Design Split]] - rationale - docs/wiki/Header-Documentation.md
- [[Pre-Push Wrap-Up Gate]] - rationale - docs/wiki/Documentation-Workflow.md
- [[Prompt-Time Query-First]] - rationale - docs/wiki/Documentation-Workflow.md
- [[Pull Request Checklist]] - document - .github/pull_request_template.md

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/Graphify_
SORT file.name ASC
```
