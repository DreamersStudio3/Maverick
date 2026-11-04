<%*
const fallbackTitle = tp.file.folder() || "새 문서";
const title = (await tp.system.prompt("문서 제목", fallbackTitle)) ?? fallbackTitle;
const subtitle = (await tp.system.prompt("부제목", "")) ?? "";
const editor = (await tp.system.prompt("최근 수정자", "")) ?? "";
-%>
---
제목: <% JSON.stringify(title) %>
부제목: <% JSON.stringify(subtitle) %>
최근수정일: <% tp.date.now("YYYY-MM-DD") %>
최근수정자: <% JSON.stringify(editor) %>
관련문서: []
---

# <% title %>

<% tp.file.cursor() %>
