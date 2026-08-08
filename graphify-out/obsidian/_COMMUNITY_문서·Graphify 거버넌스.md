---
type: community
cohesion: 0.24
members: 11
---

# 문서·Graphify 거버넌스

**Cohesion:** 0.24 - loosely connected
**Members:** 11 nodes

## Members
- [[Build and Impact Verification]] - rationale - .github/pull_request_template.md
- [[Commit-Time Incremental Graphify Update]] - rationale - docs/wiki/Documentation-Workflow.md
- [[Documentation Information Hierarchy]] - rationale - docs/wiki/Documentation-Workflow.md
- [[Documentation and Graph Governance]] - rationale - POLICY.md
- [[Graphify Generated Read Models]] - rationale - docs/wiki/Documentation-Workflow.md
- [[Graphify Navigation and Freshness Lifecycle]] - rationale - AGENTS.md
- [[Graphify Pre-Push Freshness Gate]] - rationale - POLICY.md
- [[Pre-Push Graphify Wrap-Up Gate]] - rationale - docs/wiki/Documentation-Workflow.md
- [[Prompt-Time Query-First Workflow]] - rationale - docs/wiki/Documentation-Workflow.md
- [[Pull Request Checklist]] - document - .github/pull_request_template.md
- [[Work and Documentation Lifecycle]] - rationale - AGENTS.md

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/Graphify_
SORT file.name ASC
```
