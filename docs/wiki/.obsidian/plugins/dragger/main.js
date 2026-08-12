var __create = Object.create;
var __defProp = Object.defineProperty;
var __getOwnPropDesc = Object.getOwnPropertyDescriptor;
var __getOwnPropNames = Object.getOwnPropertyNames;
var __getProtoOf = Object.getPrototypeOf;
var __hasOwnProp = Object.prototype.hasOwnProperty;
var __export = (target, all) => {
  for (var name in all)
    __defProp(target, name, { get: all[name], enumerable: true });
};
var __copyProps = (to, from, except, desc) => {
  if (from && typeof from === "object" || typeof from === "function") {
    for (let key of __getOwnPropNames(from))
      if (!__hasOwnProp.call(to, key) && key !== except)
        __defProp(to, key, { get: () => from[key], enumerable: !(desc = __getOwnPropDesc(from, key)) || desc.enumerable });
  }
  return to;
};
var __toESM = (mod, isNodeMode, target) => (target = mod != null ? __create(__getProtoOf(mod)) : {}, __copyProps(
  // If the importer is in node compatibility mode or this is not an ESM
  // file that has been converted to a CommonJS file using a Babel-
  // compatible transform (i.e. "__esModule" has not been set), then set
  // "default" to the CommonJS "module.exports" for node compatibility.
  isNodeMode || !mod || !mod.__esModule ? __defProp(target, "default", { value: mod, enumerable: true }) : target,
  mod
));
var __toCommonJS = (mod) => __copyProps(__defProp({}, "__esModule", { value: true }), mod);

// src/plugin/main.ts
var main_exports = {};
__export(main_exports, {
  default: () => DragNDropPlugin2
});
module.exports = __toCommonJS(main_exports);
var import_obsidian4 = require("obsidian");

// src/platform/codemirror/obsidian-dragger.ts
var import_state6 = require("@codemirror/state");
var import_view7 = require("@codemirror/view");

// node_modules/.pnpm/md-dragger@2.0.1_@codemirror+state@6.7.1_@codemirror+view@6.43.7/node_modules/md-dragger/dist/npm/adapter/codemirror.mjs
var import_view = require("@codemirror/view");
var import_state = require("@codemirror/state");
var import_view2 = require("@codemirror/view");
var import_state2 = require("@codemirror/state");
var import_view3 = require("@codemirror/view");
var import_state3 = require("@codemirror/state");
var import_view4 = require("@codemirror/view");
var import_state4 = require("@codemirror/state");
var HANDLE_CLASS = "md-dragger-handle";
var EDITOR_CLASS = "md-dragger-editor";
function resolveConfig(config) {
  const raw = typeof config === "function" ? config() : config;
  if (!(raw.tabSize > 0)) {
    throw new Error(`mdDragger: config.tabSize must be positive, got ${String(raw.tabSize)}`);
  }
  if (!(raw.listIndentUnit > 0)) {
    throw new Error(`mdDragger: config.listIndentUnit must be positive, got ${String(raw.listIndentUnit)}`);
  }
  return raw;
}
function resolveLocateOptions(locate, view) {
  if (!locate) return void 0;
  return typeof locate === "function" ? locate(view) : locate;
}
function isDraggerEnabled(options, view) {
  return options.enabled ? options.enabled(view) : true;
}
function resolveTabSize(options) {
  return resolveConfig(options.config).tabSize;
}
function resolveListIndentUnit(options) {
  return resolveConfig(options.config).listIndentUnit;
}
function resolveListIndentWidthPx(options, view) {
  const raw = typeof options.listIndentWidthPx === "function" ? options.listIndentWidthPx(view) : options.listIndentWidthPx;
  if (!Number.isFinite(raw) || raw < 0) {
    throw new Error(`mdDragger: listIndentWidthPx must be a finite non-negative number, got ${String(raw)}`);
  }
  return raw;
}
function isHorizontalRuleLine(text) {
  if (!text) return false;
  const trimmed = text.trim();
  if (trimmed.length < 3) return false;
  return /^([-*_])(?:\s*\1){2,}$/.test(trimmed);
}
function isBlockquoteLine(text) {
  if (!text) return false;
  return /^(> ?)+/.test(text.trimStart());
}
function isCalloutLine(text) {
  if (!text) return false;
  return /^(\s*> ?)+\s*\[!/.test(text.trimStart());
}
function isTableLine(text) {
  if (!text) return false;
  return text.trimStart().startsWith("|");
}
function isMathFenceLine(text) {
  if (!text) return false;
  return text.trimStart().startsWith("$$");
}
function isCodeFenceLine(text) {
  if (!text) return false;
  return text.trimStart().startsWith("```");
}
var fenceLazyScanCache = /* @__PURE__ */ new WeakMap();
function isSingleLineMathFence(lineText) {
  const trimmed = lineText.trimStart();
  if (!trimmed.startsWith("$$")) return false;
  return trimmed.slice(2).includes("$$");
}
function assignFenceRangeByLine(rangeByLine, startLine, endLine) {
  const range = { startLine, endLine };
  for (let i = startLine; i <= endLine; i++) {
    rangeByLine.set(i, range);
  }
}
function createFenceLazyScanState() {
  return {
    scannedUntilLine: 0,
    openCodeStartLine: 0,
    openMathStartLine: 0,
    fullyScanned: false,
    codeRangeByLine: /* @__PURE__ */ new Map(),
    mathRangeByLine: /* @__PURE__ */ new Map()
  };
}
function getFenceLazyScanState(doc) {
  const cached = fenceLazyScanCache.get(doc);
  if (cached) return cached;
  const created = createFenceLazyScanState();
  fenceLazyScanCache.set(doc, created);
  return created;
}
function scanFenceLine(state, lineNumber, text) {
  if (state.openCodeStartLine !== 0) {
    if (isCodeFenceLine(text)) {
      assignFenceRangeByLine(state.codeRangeByLine, state.openCodeStartLine, lineNumber);
      state.openCodeStartLine = 0;
    }
    return;
  }
  if (state.openMathStartLine !== 0) {
    if (isMathFenceLine(text)) {
      assignFenceRangeByLine(state.mathRangeByLine, state.openMathStartLine, lineNumber);
      state.openMathStartLine = 0;
    }
    return;
  }
  if (isCodeFenceLine(text)) {
    state.openCodeStartLine = lineNumber;
    return;
  }
  if (isMathFenceLine(text)) {
    if (isSingleLineMathFence(text)) {
      assignFenceRangeByLine(state.mathRangeByLine, lineNumber, lineNumber);
    } else {
      state.openMathStartLine = lineNumber;
    }
  }
}
function finalizeFenceStateAtDocEnd(state) {
  if (state.openCodeStartLine !== 0) {
    assignFenceRangeByLine(state.codeRangeByLine, state.openCodeStartLine, state.openCodeStartLine);
    state.openCodeStartLine = 0;
  }
  state.openMathStartLine = 0;
  state.fullyScanned = true;
}
function ensureFenceScanComplete(doc) {
  const state = getFenceLazyScanState(doc);
  if (state.fullyScanned) return state;
  let cursor = state.scannedUntilLine + 1;
  while (cursor <= doc.lines) {
    scanFenceLine(state, cursor, doc.line(cursor).text);
    cursor++;
  }
  state.scannedUntilLine = Math.max(state.scannedUntilLine, cursor - 1);
  finalizeFenceStateAtDocEnd(state);
  return state;
}
function findMathBlockRange(doc, lineNumber) {
  var _a;
  if (lineNumber < 1 || lineNumber > doc.lines) return null;
  const state = ensureFenceScanComplete(doc);
  return (_a = state.mathRangeByLine.get(lineNumber)) != null ? _a : null;
}
function findCodeBlockRange(doc, lineNumber) {
  var _a;
  if (lineNumber < 1 || lineNumber > doc.lines) return null;
  const state = ensureFenceScanComplete(doc);
  return (_a = state.codeRangeByLine.get(lineNumber)) != null ? _a : null;
}
function indentWidth(raw, tabSize) {
  let width = 0;
  for (const ch of raw) {
    width += ch === "	" ? tabSize : 1;
  }
  return width;
}
function splitQuote(line) {
  const match = line.match(/^(\s*> ?)+/);
  if (!match) return { prefix: "", depth: 0, rest: line };
  const prefix = match[0];
  return {
    prefix,
    depth: (prefix.match(/>/g) || []).length,
    rest: line.slice(prefix.length)
  };
}
function parseMarkerAndBody(rest) {
  var _a;
  const indentMatch = rest.match(/^(\s*)/);
  const indentRaw = (_a = indentMatch == null ? void 0 : indentMatch[1]) != null ? _a : "";
  const afterIndent = rest.slice(indentRaw.length);
  const headingMatch = afterIndent.match(/^(#{1,6})\s+/);
  if (headingMatch) {
    const text = headingMatch[0];
    const level = headingMatch[1].length;
    return {
      indent: { raw: indentRaw, width: 0 },
      marker: { kind: "heading", text, level },
      body: afterIndent.slice(text.length)
    };
  }
  if (isHorizontalRuleLine(afterIndent)) {
    return {
      indent: { raw: indentRaw, width: 0 },
      marker: { kind: "hr", text: afterIndent },
      body: ""
    };
  }
  if (isCodeFenceLine(afterIndent)) {
    const info = afterIndent.replace(/^```\s*/, "").trim() || void 0;
    return {
      indent: { raw: indentRaw, width: 0 },
      marker: { kind: "fence", text: afterIndent, fence: "code", info },
      body: ""
    };
  }
  if (isMathFenceLine(afterIndent)) {
    return {
      indent: { raw: indentRaw, width: 0 },
      marker: { kind: "fence", text: afterIndent, fence: "math" },
      body: ""
    };
  }
  if (isTableLine(afterIndent)) {
    return {
      indent: { raw: indentRaw, width: 0 },
      marker: { kind: "table-row", text: afterIndent },
      body: ""
    };
  }
  if (isCalloutLine(afterIndent) || /^\[![^\]]+\]/.test(afterIndent)) {
    const m = afterIndent.match(/^\[!([^\]]+)\]\s*/);
    if (m) {
      return {
        indent: { raw: indentRaw, width: 0 },
        marker: { kind: "callout", text: m[0], calloutType: m[1] },
        body: afterIndent.slice(m[0].length)
      };
    }
  }
  const taskMatch = afterIndent.match(/^([-*+])\s\[([ xX])\]\s+/);
  if (taskMatch) {
    const text = taskMatch[0];
    const checked = taskMatch[2] !== " ";
    return {
      indent: { raw: indentRaw, width: 0 },
      marker: { kind: "list", text, markerType: "task", checked },
      body: afterIndent.slice(text.length)
    };
  }
  const unorderedMatch = afterIndent.match(/^([-*+])\s+/);
  if (unorderedMatch) {
    const text = unorderedMatch[0];
    return {
      indent: { raw: indentRaw, width: 0 },
      marker: { kind: "list", text, markerType: "unordered" },
      body: afterIndent.slice(text.length)
    };
  }
  const orderedMatch = afterIndent.match(/^(\d+)[.)]\s+/);
  if (orderedMatch) {
    const text = orderedMatch[0];
    return {
      indent: { raw: indentRaw, width: 0 },
      marker: { kind: "list", text, markerType: "ordered" },
      body: afterIndent.slice(text.length)
    };
  }
  return {
    indent: { raw: indentRaw, width: 0 },
    marker: null,
    body: afterIndent
  };
}
function parseLine(text, tabSize) {
  const { prefix, depth, rest } = splitQuote(text);
  const { indent, marker, body } = parseMarkerAndBody(rest);
  return {
    raw: text,
    quote: { depth, prefix },
    indent: {
      raw: indent.raw,
      width: indentWidth(indent.raw, tabSize)
    },
    marker,
    body
  };
}
function isListLine(p) {
  var _a;
  return ((_a = p.marker) == null ? void 0 : _a.kind) === "list";
}
function listMarkerText(p) {
  var _a;
  return ((_a = p.marker) == null ? void 0 : _a.kind) === "list" ? p.marker.text : "";
}
function listMarkerType(p) {
  var _a;
  return ((_a = p.marker) == null ? void 0 : _a.kind) === "list" ? p.marker.markerType : null;
}
function formatIndent(width, tabSize, sample = " ") {
  const safeWidth = Math.max(0, width);
  if (safeWidth === 0) return "";
  if (sample.includes("	")) {
    const tabs = Math.floor(safeWidth / tabSize);
    const spaces = safeWidth - tabs * tabSize;
    return "	".repeat(tabs) + " ".repeat(spaces);
  }
  return " ".repeat(safeWidth);
}
var lineMapCache = /* @__PURE__ */ new WeakMap();
var EMPTY_LINE_META = {
  isEmpty: true,
  isList: false,
  isQuote: false,
  isCallout: false,
  isTable: false,
  isHr: false,
  indentWidth: 0,
  quoteDepth: 0
};
function createLineMetaFromText(text, tabSize) {
  const parsed = parseLine(text, tabSize);
  const isEmpty = text.trim().length === 0;
  return {
    isEmpty,
    isList: isListLine(parsed),
    isQuote: parsed.quote.depth > 0,
    isCallout: isCalloutLine(text),
    isTable: text.trimStart().startsWith("|"),
    isHr: isHorizontalRuleLine(text),
    indentWidth: parsed.indent.width,
    quoteDepth: parsed.quote.depth
  };
}
function createLineMetaArray(doc, tabSize) {
  var _a;
  const lineMeta = Array(doc.lines + 1);
  lineMeta[0] = EMPTY_LINE_META;
  for (let i = 1; i <= doc.lines; i++) {
    lineMeta[i] = createLineMetaFromText((_a = doc.line(i).text) != null ? _a : "", tabSize);
  }
  return lineMeta;
}
function buildLineMapIndexes(lineMeta, totalLines) {
  var _a, _b, _c;
  const prevNonEmpty2 = new Int32Array(totalLines + 2);
  const nextNonEmpty2 = new Int32Array(totalLines + 2);
  const prevListLine = new Int32Array(totalLines + 2);
  const listParentLine = new Int32Array(totalLines + 2);
  const listSubtreeEndLine = new Int32Array(totalLines + 2);
  let previous = 0;
  let previousList = 0;
  const listStack = [];
  for (let i = 1; i <= totalLines; i++) {
    const meta = (_a = lineMeta[i]) != null ? _a : EMPTY_LINE_META;
    if (!meta.isEmpty) {
      previous = i;
    }
    prevNonEmpty2[i] = previous;
    if (meta.isEmpty) {
      prevListLine[i] = previousList;
      continue;
    }
    while (listStack.length > 0) {
      const topLine = listStack[listStack.length - 1];
      const topMeta = (_b = lineMeta[topLine]) != null ? _b : EMPTY_LINE_META;
      if (meta.indentWidth > topMeta.indentWidth) {
        break;
      }
      listStack.pop();
    }
    for (const ancestorLine of listStack) {
      listSubtreeEndLine[ancestorLine] = i;
    }
    prevListLine[i] = previousList;
    if (!meta.isList) {
      continue;
    }
    listParentLine[i] = listStack.length > 0 ? listStack[listStack.length - 1] : 0;
    listSubtreeEndLine[i] = i;
    listStack.push(i);
    previousList = i;
  }
  let next = 0;
  for (let i = totalLines; i >= 1; i--) {
    const meta = (_c = lineMeta[i]) != null ? _c : EMPTY_LINE_META;
    if (!meta.isEmpty) {
      next = i;
    }
    nextNonEmpty2[i] = next;
  }
  return {
    prevNonEmpty: prevNonEmpty2,
    nextNonEmpty: nextNonEmpty2,
    prevListLine,
    listParentLine,
    listSubtreeEndLine
  };
}
function createLineMapFromMeta(doc, tabSize, lineMeta) {
  const indexes = buildLineMapIndexes(lineMeta, doc.lines);
  return {
    doc,
    lineMeta,
    prevNonEmpty: indexes.prevNonEmpty,
    nextNonEmpty: indexes.nextNonEmpty,
    prevListLine: indexes.prevListLine,
    listParentLine: indexes.listParentLine,
    listSubtreeEndLine: indexes.listSubtreeEndLine,
    tabSize
  };
}
function buildLineMap(doc, options) {
  const tabSize = options.tabSize;
  const lineMeta = createLineMetaArray(doc, tabSize);
  return createLineMapFromMeta(doc, tabSize, lineMeta);
}
function getCachedLineMapForDoc(doc, tabSize) {
  var _a, _b;
  if (!doc || typeof doc !== "object") return null;
  return (_b = (_a = lineMapCache.get(doc)) == null ? void 0 : _a.get(tabSize)) != null ? _b : null;
}
function setCachedLineMapForDoc(doc, tabSize, lineMap) {
  const byTabSize = lineMapCache.get(doc);
  if (byTabSize) {
    byTabSize.set(tabSize, lineMap);
    return;
  }
  lineMapCache.set(doc, /* @__PURE__ */ new Map([[tabSize, lineMap]]));
}
function getLineMap(doc, options) {
  const tabSize = options.tabSize;
  if (!doc || typeof doc !== "object") {
    return buildLineMap(doc, { tabSize });
  }
  const cached = getCachedLineMapForDoc(doc, tabSize);
  if (cached) {
    return cached;
  }
  const built = buildLineMap(doc, { tabSize });
  setCachedLineMapForDoc(doc, tabSize, built);
  return built;
}
function peekCachedLineMap(doc, options) {
  const tabSize = options.tabSize;
  if (!doc || typeof doc !== "object") return null;
  return getCachedLineMapForDoc(doc, tabSize);
}
function getLineMetaAt(lineMap, lineNumber) {
  var _a;
  if (lineNumber < 1 || lineNumber >= lineMap.lineMeta.length) return null;
  return (_a = lineMap.lineMeta[lineNumber]) != null ? _a : null;
}
function listLineAtOrAbove(lineMap, lineNumber) {
  if (lineMap.doc.lines <= 0) return null;
  const clamped = Math.max(1, Math.min(lineMap.doc.lines, lineNumber));
  const meta = getLineMetaAt(lineMap, clamped);
  if (meta == null ? void 0 : meta.isList) return clamped;
  const prevListLine = lineMap.prevListLine[clamped];
  return prevListLine > 0 ? prevListLine : null;
}
var BlockType = /* @__PURE__ */ ((BlockType22) => {
  BlockType22["Paragraph"] = "paragraph";
  BlockType22["Heading"] = "heading";
  BlockType22["ListItem"] = "list-item";
  BlockType22["CodeBlock"] = "code-block";
  BlockType22["Blockquote"] = "blockquote";
  BlockType22["Table"] = "table";
  BlockType22["MathBlock"] = "math-block";
  BlockType22["Callout"] = "callout";
  BlockType22["HorizontalRule"] = "hr";
  BlockType22["Unknown"] = "unknown";
  return BlockType22;
})(BlockType || {});
function detectBlockType(lineText, tabSize) {
  var _a, _b, _c, _d, _e, _f;
  const p = parseLine(lineText, tabSize);
  if (((_a = p.marker) == null ? void 0 : _a.kind) === "heading") return "heading";
  if (((_b = p.marker) == null ? void 0 : _b.kind) === "hr") return "hr";
  if (((_c = p.marker) == null ? void 0 : _c.kind) === "list") return "list-item";
  if (((_d = p.marker) == null ? void 0 : _d.kind) === "fence") {
    return p.marker.fence === "code" ? "code-block" : "math-block";
  }
  if (((_e = p.marker) == null ? void 0 : _e.kind) === "table-row") return "table";
  if (((_f = p.marker) == null ? void 0 : _f.kind) === "callout") return "callout";
  if (p.quote.depth > 0) return "blockquote";
  if (p.body.trim().length === 0 && !p.marker) return "unknown";
  return "paragraph";
}
function isCalloutHeaderLine(text, tabSize) {
  var _a;
  return ((_a = parseLine(text, tabSize).marker) == null ? void 0 : _a.kind) === "callout";
}
function isInsideCalloutContainer(doc, lineNumber, depth, tabSize) {
  var _a;
  for (let i = lineNumber; i >= 1; i--) {
    const text = doc.line(i).text;
    const p = parseLine(text, tabSize);
    if (p.quote.depth === 0 || p.quote.depth < depth) break;
    if (((_a = p.marker) == null ? void 0 : _a.kind) === "callout" || isCalloutHeaderLine(text, tabSize)) return true;
  }
  return false;
}
function getBlockquoteContainerRange(doc, lineNumber, depth, tabSize) {
  let startLine = lineNumber;
  for (let i = lineNumber - 1; i >= 1; i--) {
    const d = parseLine(doc.line(i).text, tabSize).quote.depth;
    if (d === 0 || d < depth) break;
    startLine = i;
  }
  let endLine = lineNumber;
  for (let i = lineNumber + 1; i <= doc.lines; i++) {
    const d = parseLine(doc.line(i).text, tabSize).quote.depth;
    if (d === 0 || d < depth) break;
    endLine = i;
  }
  return { startLine, endLine };
}
function getListItemSubtreeRange(doc, lineNumber, tabSize) {
  const current = parseLine(doc.line(lineNumber).text, tabSize);
  const currentIndent = current.indent.width;
  let endLine = lineNumber;
  for (let i = lineNumber + 1; i <= doc.lines; i++) {
    const nextText = doc.line(i).text;
    if (nextText.trim().length === 0) {
      const lookahead = findNextNonEmptyLine(doc, i + 1, tabSize);
      if (!lookahead || lookahead.isList && lookahead.indentWidth <= currentIndent || lookahead.indentWidth <= currentIndent) {
        break;
      }
      endLine = i;
      continue;
    }
    const next = parseLine(nextText, tabSize);
    if (isListLine(next) && next.indent.width <= currentIndent) {
      break;
    }
    if (isListLine(next) || next.indent.width > currentIndent) {
      endLine = i;
      continue;
    }
    break;
  }
  return { startLine: lineNumber, endLine };
}
function findNextNonEmptyLine(doc, fromLine, tabSize) {
  for (let i = fromLine; i <= doc.lines; i++) {
    const text = doc.line(i).text;
    if (text.trim().length === 0) continue;
    const p = parseLine(text, tabSize);
    return { isList: isListLine(p), indentWidth: p.indent.width };
  }
  return null;
}
var blockDetectionCache = /* @__PURE__ */ new WeakMap();
var LINE_MAP_EAGER_MAX = 3e4;
var YAML_FENCE_RE = /^-{3}\s*$/;
var yamlEndCache = /* @__PURE__ */ new WeakMap();
function yamlEndLine(doc) {
  const cached = yamlEndCache.get(doc);
  if (cached !== void 0) return cached;
  let endLine = 0;
  if (doc.lines >= 2 && YAML_FENCE_RE.test(doc.line(1).text)) {
    for (let i = 2; i <= doc.lines; i++) {
      if (YAML_FENCE_RE.test(doc.line(i).text)) {
        endLine = i;
        break;
      }
    }
  }
  yamlEndCache.set(doc, endLine);
  return endLine;
}
function inYamlFrontmatter(doc, lineNumber) {
  const endLine = yamlEndLine(doc);
  return endLine > 0 && lineNumber >= 1 && lineNumber <= endLine;
}
function detectBlockUncached(doc, lineNumber, tabSize) {
  if (lineNumber < 1 || lineNumber > doc.lines) {
    return null;
  }
  if (inYamlFrontmatter(doc, lineNumber)) {
    return null;
  }
  const lineText = doc.line(lineNumber).text;
  let blockType = detectBlockType(lineText, tabSize);
  const codeRange = findCodeBlockRange(doc, lineNumber);
  const mathRange = findMathBlockRange(doc, lineNumber);
  if (codeRange) {
    blockType = "code-block";
  }
  if (mathRange) {
    blockType = "math-block";
  }
  if (blockType === "unknown") {
    return null;
  }
  let startLine = lineNumber;
  let endLine = lineNumber;
  if (blockType === "code-block" && codeRange) {
    startLine = codeRange.startLine;
    endLine = codeRange.endLine;
  }
  if (blockType === "math-block" && mathRange) {
    startLine = mathRange.startLine;
    endLine = mathRange.endLine;
  }
  if (blockType === "list-item") {
    let lineMap = peekCachedLineMap(doc, { tabSize });
    if (!lineMap && doc.lines <= LINE_MAP_EAGER_MAX) {
      lineMap = getLineMap(doc, { tabSize });
    }
    const lineMeta = lineMap ? getLineMetaAt(lineMap, lineNumber) : null;
    const subtreeEndLine = (lineMeta == null ? void 0 : lineMeta.isList) && lineMap ? lineMap.listSubtreeEndLine[lineNumber] : 0;
    if (subtreeEndLine >= lineNumber) {
      endLine = subtreeEndLine;
    } else {
      endLine = getListItemSubtreeRange(doc, lineNumber, tabSize).endLine;
    }
  }
  if (blockType === "blockquote" || blockType === "callout") {
    const quoteDepth = parseLine(lineText, tabSize).quote.depth;
    const inCallout = blockType === "callout" || isInsideCalloutContainer(doc, lineNumber, quoteDepth, tabSize);
    if (inCallout) {
      const range = getBlockquoteContainerRange(doc, lineNumber, quoteDepth, tabSize);
      startLine = range.startLine;
      endLine = range.endLine;
      blockType = "callout";
    } else {
      startLine = lineNumber;
      endLine = lineNumber;
      blockType = "blockquote";
    }
  }
  if (blockType === "table") {
    for (let i = lineNumber - 1; i >= 1; i--) {
      if (isTableLine(doc.line(i).text)) startLine = i;
      else break;
    }
    for (let i = lineNumber + 1; i <= doc.lines; i++) {
      if (isTableLine(doc.line(i).text)) endLine = i;
      else break;
    }
  }
  return {
    type: blockType,
    lines: { startLine, endLine }
  };
}
function detectBlock(doc, lineNumber, options) {
  var _a;
  const tabSize = options.tabSize;
  let cacheByTabSize = blockDetectionCache.get(doc);
  if (!cacheByTabSize) {
    cacheByTabSize = /* @__PURE__ */ new Map();
    blockDetectionCache.set(doc, cacheByTabSize);
  }
  let perDocCache = cacheByTabSize.get(tabSize);
  if (!perDocCache) {
    perDocCache = /* @__PURE__ */ new Map();
    cacheByTabSize.set(tabSize, perDocCache);
  }
  if (perDocCache.has(lineNumber)) {
    return (_a = perDocCache.get(lineNumber)) != null ? _a : null;
  }
  const block = detectBlockUncached(doc, lineNumber, tabSize);
  if (block) {
    perDocCache.set(block.lines.startLine, block);
    for (let n = block.lines.startLine + 1; n <= block.lines.endLine; n++) {
      if (isListLine(parseLine(doc.line(n).text, tabSize))) {
        continue;
      }
      perDocCache.set(n, block);
    }
  } else {
    perDocCache.set(lineNumber, null);
  }
  return block;
}
function normalizeLineRange(docLines, startLine, endLine) {
  if (docLines <= 0) {
    return { startLine: 1, endLine: 1 };
  }
  const safeStart = Math.max(1, Math.min(docLines, Math.min(startLine, endLine)));
  const safeEnd = Math.max(1, Math.min(docLines, Math.max(startLine, endLine)));
  return { startLine: safeStart, endLine: safeEnd };
}
function mergeLineRanges(docLines, ranges) {
  const normalized = ranges.map((range) => normalizeLineRange(docLines, range.startLine, range.endLine)).sort((a, b) => a.startLine - b.startLine || a.endLine - b.endLine);
  const merged = [];
  for (const range of normalized) {
    const last = merged[merged.length - 1];
    if (!last || range.startLine > last.endLine + 1) {
      merged.push({ ...range });
      continue;
    }
    if (range.endLine > last.endLine) {
      last.endLine = range.endLine;
    }
  }
  return merged;
}
function isLineNumberInRanges(line, ranges) {
  for (const range of ranges) {
    if (line >= range.startLine && line <= range.endLine) return true;
  }
  return false;
}
function blockKey(block) {
  return `${block.lines.startLine}:${block.lines.endLine}`;
}
function selectOne(block) {
  return { blocks: [block] };
}
function selectBlocks(blocks) {
  const sorted = [...blocks].sort(
    (a, b) => a.lines.startLine - b.lines.startLine || a.lines.endLine - b.lines.endLine
  );
  return { blocks: sorted };
}
function addBlocks(selection, blocks) {
  const map = new Map(selection.blocks.map((b) => [blockKey(b), b]));
  for (const block of blocks) {
    map.set(blockKey(block), block);
  }
  return selectBlocks([...map.values()]);
}
function removeBlocks(selection, blocks) {
  const remove = new Set(blocks.map(blockKey));
  return selectBlocks(selection.blocks.filter((b) => !remove.has(blockKey(b))));
}
function hasBlock(selection, block) {
  const key = blockKey(block);
  return selection.blocks.some((b) => blockKey(b) === key);
}
function selectionLineRanges(docLines, selection) {
  return mergeLineRanges(
    docLines,
    selection.blocks.map((block) => block.lines)
  );
}
function locateDropPosition(input) {
  var _a, _b;
  const { doc, selection, hitLine, belowMid: belowMid2, sourceIndentWidth, targetIndentWidth, tabSize, indentUnit } = input;
  const line = Math.max(1, Math.min(doc.lines + 1, belowMid2 ? hitLine + 1 : hitLine));
  if (indentUnit <= 0 || line <= 1) {
    return { doc, line, parent: null };
  }
  const want = Math.max(
    quantizeIndent(targetIndentWidth, indentUnit),
    quantizeIndent(sourceIndentWidth, indentUnit) - indentUnit
  );
  if (want <= 0) {
    return { doc, line, parent: null };
  }
  const lineMap = getLineMap(doc, { tabSize });
  const above = line - 1;
  let parentLine = listLineAtOrAbove(lineMap, above);
  if (parentLine === null || lineMap.listSubtreeEndLine[parentLine] < above) {
    return { doc, line, parent: null };
  }
  const desiredParentIndent = want - indentUnit;
  const sourceLines = selectionLineRanges(doc.lines, selection);
  while (parentLine > 0) {
    const meta = getLineMetaAt(lineMap, parentLine);
    if (!(meta == null ? void 0 : meta.isList)) {
      parentLine = 0;
      break;
    }
    if (isLineNumberInRanges(parentLine, sourceLines)) {
      parentLine = (_a = lineMap.listParentLine[parentLine]) != null ? _a : 0;
      continue;
    }
    if (meta.indentWidth > desiredParentIndent) {
      parentLine = (_b = lineMap.listParentLine[parentLine]) != null ? _b : 0;
      continue;
    }
    break;
  }
  if (parentLine <= 0) {
    return { doc, line, parent: null };
  }
  const parent = listItemAt(doc, parentLine, tabSize);
  return { doc, line, parent };
}
function quantizeIndent(width, indentUnit) {
  if (!(width > 0) || !(indentUnit > 0)) return 0;
  return Math.max(0, Math.round(width / indentUnit) * indentUnit);
}
function listItemAt(doc, listHeadLine, tabSize) {
  const block = detectBlock(doc, listHeadLine, { tabSize });
  if (!block || block.type !== "list-item") return null;
  return block;
}
function dropIndentWidth(position, options) {
  var _a, _b;
  if (((_a = position.parent) == null ? void 0 : _a.type) === "list-item") {
    const lineMap = getLineMap(position.doc, { tabSize: options.tabSize });
    const meta = getLineMetaAt(lineMap, position.parent.lines.startLine);
    const base = (_b = meta == null ? void 0 : meta.indentWidth) != null ? _b : 0;
    return base + options.indentUnit;
  }
  return 0;
}
var ALL_TYPES = Object.values(BlockType);
function rejectEntries(types, slot, reason) {
  return types.map((t2) => [`${t2}|${slot}`, reason]);
}
var REJECT_RULES = new Map([
  ...rejectEntries(ALL_TYPES, "inside_code_block", "inside_code_block"),
  ...rejectEntries(ALL_TYPES, "inside_math_block", "inside_math_block"),
  ...rejectEntries(
    ALL_TYPES.filter(
      (t2) => t2 !== "list-item"
      /* ListItem */
    ),
    "inside_list",
    "inside_list"
  ),
  ...rejectEntries(
    ALL_TYPES.filter(
      (t2) => t2 !== "blockquote"
      /* Blockquote */
    ),
    "inside_quote_run",
    "inside_quote_run"
  ),
  ...rejectEntries([
    "callout"
    /* Callout */
  ], "quote_before", "quote_boundary"),
  ...rejectEntries(
    ALL_TYPES.filter(
      (t2) => t2 !== "blockquote"
      /* Blockquote */
    ),
    "quote_after",
    "quote_boundary"
  ),
  ...rejectEntries(ALL_TYPES, "callout_after", "callout_after"),
  ...rejectEntries(ALL_TYPES, "table_before", "table_before"),
  ...rejectEntries(ALL_TYPES, "hr_before", "hr_before")
]);
function resolveInsertionRule(input) {
  var _a;
  const key = `${input.sourceType}|${input.slotContext}`;
  const rejectReason = (_a = REJECT_RULES.get(key)) != null ? _a : null;
  return {
    allowDrop: rejectReason === null,
    rejectReason
  };
}
function getImmediateLineText(doc, lineNumber) {
  if (lineNumber < 1 || lineNumber > doc.lines) return null;
  return doc.line(lineNumber).text;
}
function getActiveLineMap(doc, options) {
  var _a;
  return (_a = options.lineMap) != null ? _a : getLineMap(doc, { tabSize: options.tabSize });
}
function prevNonEmpty(doc, lineNumber, lineMap) {
  if (doc.lines <= 0) return null;
  const clampedLine = Math.max(1, Math.min(doc.lines, lineNumber));
  const prev = lineMap.prevNonEmpty[clampedLine];
  return prev > 0 ? prev : null;
}
function nextNonEmpty(doc, lineNumber, lineMap) {
  if (doc.lines <= 0) return null;
  const clampedLine = Math.max(1, Math.min(doc.lines, lineNumber));
  const next = lineMap.nextNonEmpty[clampedLine];
  return next > 0 ? next : null;
}
function findEnclosingListBlock(doc, lineNumber, options) {
  if (lineNumber < 1 || lineNumber > doc.lines) return null;
  const lineMap = getActiveLineMap(doc, options);
  const radius = 8;
  const minLine = Math.max(1, lineNumber - radius);
  const maxLine = Math.min(doc.lines, lineNumber + radius);
  let best = null;
  for (let ln = minLine; ln <= maxLine; ln++) {
    const meta = getLineMetaAt(lineMap, ln);
    if (meta && !meta.isList) continue;
    const block = detectBlock(doc, ln, { tabSize: lineMap.tabSize });
    if (!block || block.type !== "list-item") continue;
    const blockStart = block.lines.startLine;
    const blockEnd = block.lines.endLine;
    if (lineNumber < blockStart || lineNumber > blockEnd) continue;
    if (!best || block.lines.endLine - block.lines.startLine > best.lines.endLine - best.lines.startLine) {
      best = block;
    }
  }
  return best;
}
function isTableBlockStartAtLine(doc, lineNumber, options) {
  if (lineNumber < 1 || lineNumber > doc.lines) return false;
  const block = detectBlock(doc, lineNumber, options);
  return !!block && block.type === "table" && block.lines.startLine === lineNumber;
}
function isHorizontalRuleAtLine(doc, lineNumber, options) {
  if (lineNumber < 1 || lineNumber > doc.lines) return false;
  const block = detectBlock(doc, lineNumber, options);
  if (block) {
    return block.type === "hr" && block.lines.startLine === lineNumber;
  }
  return isHorizontalRuleLine(doc.line(lineNumber).text);
}
function isCalloutAfterBoundary(doc, prevImmediateLine, nextIsQuoteLike, options) {
  if (prevImmediateLine < 1 || prevImmediateLine > doc.lines) return false;
  if (nextIsQuoteLike) return false;
  const prevBlock = detectBlock(doc, prevImmediateLine, options);
  return !!prevBlock && prevBlock.type === "callout" && prevBlock.lines.endLine === prevImmediateLine;
}
function listSlotAt(doc, targetLineNumber, options) {
  if (doc.lines <= 0) return null;
  const lineMap = getActiveLineMap(doc, options);
  const candidates = [
    targetLineNumber - 1,
    targetLineNumber,
    targetLineNumber + 1,
    prevNonEmpty(doc, targetLineNumber - 1, lineMap),
    nextNonEmpty(doc, targetLineNumber, lineMap)
  ].filter((v) => typeof v === "number" && v >= 1 && v <= doc.lines);
  const seen = /* @__PURE__ */ new Set();
  let best = null;
  for (const line of candidates) {
    if (seen.has(line)) continue;
    seen.add(line);
    const lineMeta = getLineMetaAt(lineMap, line);
    if (lineMeta && !lineMeta.isList) continue;
    const block = findEnclosingListBlock(doc, line, {
      lineMap,
      tabSize: options.tabSize
    });
    if (!block) continue;
    const blockTopBoundary = block.lines.startLine;
    const blockBottomBoundary = block.lines.endLine + 1;
    const isInsideContainer = targetLineNumber > blockTopBoundary && targetLineNumber < blockBottomBoundary;
    if (!isInsideContainer) continue;
    if (!best || block.lines.endLine - block.lines.startLine > best.lines.endLine - best.lines.startLine) {
      best = block;
    }
  }
  if (!best) return null;
  return { type: "list-item", block: best };
}
function slotAt(doc, targetLineNumber, options) {
  const lineMap = getActiveLineMap(doc, options);
  const clampedTarget = Math.max(1, Math.min(doc.lines + 1, targetLineNumber));
  const prevImmediateLine = clampedTarget - 1;
  const nextImmediateLine = clampedTarget <= doc.lines ? clampedTarget : null;
  const prevMeta = getLineMetaAt(lineMap, prevImmediateLine);
  const nextMeta = nextImmediateLine === null ? null : getLineMetaAt(lineMap, nextImmediateLine);
  const prevImmediateText = prevMeta ? null : getImmediateLineText(doc, prevImmediateLine);
  const nextImmediateText = nextMeta || nextImmediateLine === null ? null : getImmediateLineText(doc, nextImmediateLine);
  const prevIsQuoteLike = prevMeta ? prevMeta.isQuote : isBlockquoteLine(prevImmediateText);
  const nextIsQuoteLike = nextMeta ? nextMeta.isQuote : isBlockquoteLine(nextImmediateText);
  const detectOptions = { tabSize: options.tabSize };
  const targetBlock = detectBlock(doc, clampedTarget, detectOptions);
  if (targetBlock && (targetBlock.type === "code-block" || targetBlock.type === "math-block") && clampedTarget > targetBlock.lines.startLine && clampedTarget <= targetBlock.lines.endLine) {
    return targetBlock.type === "math-block" ? "inside_math_block" : "inside_code_block";
  }
  if (isCalloutAfterBoundary(doc, prevImmediateLine, nextIsQuoteLike, detectOptions)) {
    return "callout_after";
  }
  if (nextImmediateLine !== null && isTableBlockStartAtLine(doc, nextImmediateLine, detectOptions)) {
    return "table_before";
  }
  if (nextImmediateLine !== null && isHorizontalRuleAtLine(doc, nextImmediateLine, detectOptions)) {
    return "hr_before";
  }
  if (prevIsQuoteLike && nextIsQuoteLike) {
    return "inside_quote_run";
  }
  if (!prevIsQuoteLike && nextIsQuoteLike) {
    return "quote_before";
  }
  if (prevIsQuoteLike && !nextIsQuoteLike) {
    return "quote_after";
  }
  const listContext = listSlotAt(doc, clampedTarget, { lineMap, tabSize: options.tabSize });
  if (listContext) {
    return "inside_list";
  }
  return "outside";
}
function canDropAt(doc, sourceBlock, targetLineNumber, options) {
  var _a;
  const lineMap = (_a = options.lineMap) != null ? _a : getLineMap(doc, { tabSize: options.tabSize });
  const slotContext = slotAt(doc, targetLineNumber, { lineMap, tabSize: options.tabSize });
  const decision = resolveInsertionRule({
    sourceType: sourceBlock.type,
    slotContext
  });
  return { slotContext, decision };
}
function isListSelection(params) {
  var _a;
  const { doc, source, parse, ranges } = params;
  if (((_a = source.blocks[0]) == null ? void 0 : _a.type) !== "list-item") return false;
  for (const range of ranges) {
    let foundContent = false;
    for (let lineNumber = range.startLine; lineNumber <= range.endLine; lineNumber++) {
      const text = doc.line(lineNumber).text;
      if (text.trim().length === 0) continue;
      foundContent = true;
      if (!isListLine(parse(text))) return false;
    }
    if (!foundContent) return false;
  }
  return true;
}
function selfDrop(params) {
  var _a;
  const { doc, source, targetLineNumber, parseLineWithQuote: parse, lineMap, position, tabSize, indentUnit } = params;
  const sourceBlock = source.blocks[0];
  if (!sourceBlock) {
    return { inSelfRange: false, allowInPlaceIndentChange: false, rejectReason: "self_range_blocked" };
  }
  const sourceRanges = selectionLineRanges(doc.lines, source);
  if (sourceRanges.length === 0) {
    return { inSelfRange: false, allowInPlaceIndentChange: false };
  }
  const effectiveSourceRange = {
    startLine: sourceRanges[0].startLine,
    endLine: sourceRanges[sourceRanges.length - 1].endLine
  };
  const inSelectedRange = isLineNumberInRanges(targetLineNumber, sourceRanges);
  const inSelfRange = inSelectedRange || targetLineNumber === effectiveSourceRange.endLine + 1;
  if (!inSelfRange) {
    return { inSelfRange: false, allowInPlaceIndentChange: false };
  }
  if (!position) {
    return {
      inSelfRange: true,
      allowInPlaceIndentChange: false,
      rejectReason: "self_range_blocked"
    };
  }
  const targetIndentWidth = dropIndentWidth(position, { tabSize, indentUnit });
  const hasListIntent = ((_a = position.parent) == null ? void 0 : _a.type) === "list-item" || sourceBlock.type === "list-item";
  if (!hasListIntent) {
    return {
      inSelfRange: true,
      allowInPlaceIndentChange: false,
      rejectReason: "self_range_blocked"
    };
  }
  if (!isListSelection({ doc, source, parse, ranges: sourceRanges })) {
    return {
      inSelfRange: true,
      allowInPlaceIndentChange: false,
      rejectReason: "self_range_blocked"
    };
  }
  const sourceLineNumber = effectiveSourceRange.startLine;
  const sourceLineMeta = lineMap ? getLineMetaAt(lineMap, sourceLineNumber) : null;
  if (sourceLineMeta && !sourceLineMeta.isList) {
    return {
      inSelfRange: true,
      allowInPlaceIndentChange: false,
      rejectReason: "self_range_blocked"
    };
  }
  const sourceParsed = parse(doc.line(sourceLineNumber).text);
  if (!isListLine(sourceParsed)) {
    return {
      inSelfRange: true,
      allowInPlaceIndentChange: false,
      rejectReason: "self_range_blocked"
    };
  }
  const sourceIndent = sourceParsed.indent.width;
  const isAfterSelf = targetLineNumber === effectiveSourceRange.endLine + 1;
  const isSameLine = targetLineNumber === effectiveSourceRange.startLine;
  if (isAfterSelf && position.parent && isLineNumberInRanges(position.parent.lines.startLine, sourceRanges) && targetIndentWidth > sourceIndent) {
    return {
      inSelfRange: true,
      allowInPlaceIndentChange: false,
      rejectReason: "self_embedding",
      targetIndentWidth
    };
  }
  const allowInPlaceIndentChange = isAfterSelf && targetIndentWidth !== sourceIndent || isSameLine && targetIndentWidth !== sourceIndent || !isAfterSelf && targetIndentWidth < sourceIndent;
  if (!allowInPlaceIndentChange) {
    return {
      inSelfRange: true,
      allowInPlaceIndentChange: false,
      rejectReason: "self_range_blocked",
      targetIndentWidth
    };
  }
  return {
    inSelfRange: true,
    allowInPlaceIndentChange: true,
    targetIndentWidth
  };
}
function resolveInsertionChange(doc, targetLineNumber, insertText, options) {
  var _a;
  if (targetLineNumber <= doc.lines) {
    return {
      pos: doc.line(targetLineNumber).from,
      text: insertText
    };
  }
  const normalized = insertText.endsWith("\n") ? insertText.slice(0, -1) : insertText;
  if (!normalized.length) {
    return { pos: doc.length, text: normalized };
  }
  const lengthAfterDelete = (_a = options == null ? void 0 : options.lengthAfterDelete) != null ? _a : doc.length;
  if (lengthAfterDelete <= 0) {
    return { pos: 0, text: normalized };
  }
  return {
    pos: doc.length,
    text: `
${normalized}`
  };
}
function resolveDeleteRange(doc, sourceFrom, sourceTo) {
  if (sourceTo < doc.length) {
    return {
      from: sourceFrom,
      to: Math.min(sourceTo + 1, doc.length)
    };
  }
  if (sourceFrom > 0) {
    return {
      from: sourceFrom - 1,
      to: sourceTo
    };
  }
  return {
    from: sourceFrom,
    to: sourceTo
  };
}
function getSourceListBase(lines, parse) {
  for (const line of lines) {
    const parsed = parse(line);
    if (isListLine(parsed)) {
      return { indentWidth: parsed.indent.width, indentRaw: parsed.indent.raw };
    }
  }
  return null;
}
function relevelListText(params) {
  const { sourceContent, parse, formatIndentFn, targetIndentWidth } = params;
  const lines = sourceContent.split("\n");
  const sourceBase = getSourceListBase(lines, parse);
  if (!sourceBase) return sourceContent;
  const delta = targetIndentWidth - sourceBase.indentWidth;
  if (delta === 0) return sourceContent;
  return lines.map((line) => {
    if (line.trim().length === 0) return line;
    const parsed = parse(line);
    const markerText = parsed.marker && parsed.marker.kind === "list" ? parsed.marker.text : "";
    const afterIndent = markerText + parsed.body;
    if (!isListLine(parsed)) {
      if (parsed.indent.width >= sourceBase.indentWidth) {
        const newIndent2 = formatIndentFn(sourceBase.indentRaw, Math.max(0, parsed.indent.width + delta));
        return `${parsed.quote.prefix}${newIndent2}${afterIndent}`;
      }
      return line;
    }
    const newIndent = formatIndentFn(sourceBase.indentRaw, Math.max(0, parsed.indent.width + delta));
    return `${parsed.quote.prefix}${newIndent}${markerText}${parsed.body}`;
  }).join("\n");
}
function insertTextForMove(params) {
  var _a;
  const { sourceBlock, sourceContent, position, tabSize, indentUnit } = params;
  const parse = (line) => parseLine(line, tabSize);
  let text = sourceContent;
  const nestList = sourceBlock.type === "list-item" || ((_a = position.parent) == null ? void 0 : _a.type) === "list-item";
  if (sourceBlock.type !== "blockquote" && nestList) {
    const targetIndentWidth = dropIndentWidth(position, { tabSize, indentUnit });
    text = relevelListText({
      sourceContent: text,
      parse,
      formatIndentFn: (sample, width) => formatIndent(width, tabSize, sample),
      targetIndentWidth
    });
  }
  return text.endsWith("\n") ? text : `${text}
`;
}
function reject(reason) {
  return { type: "reject", reason };
}
function renumberList(doc, parse, line) {
  if (line < 1 || line > doc.lines) return [];
  const at = (n2) => {
    const p = parse(doc.line(n2).text);
    if (!isListLine(p) || listMarkerType(p) !== "ordered") return null;
    return { indent: p.indent.width, quote: p.quote.depth, p };
  };
  let seed = at(line);
  if (!seed && line > 1) seed = at(line - 1);
  if (!seed && line < doc.lines) seed = at(line + 1);
  if (!seed) return [];
  let start = line;
  while (start > 1) {
    const prev = at(start - 1);
    if (!prev || prev.indent !== seed.indent || prev.quote !== seed.quote) break;
    start -= 1;
  }
  let end = line;
  while (end < doc.lines) {
    const next = at(end + 1);
    if (!next || next.indent !== seed.indent || next.quote !== seed.quote) break;
    end += 1;
  }
  const changes = [];
  let n = 1;
  for (let i = start; i <= end; i++) {
    const row = at(i);
    if (!row) continue;
    const lineObj = doc.line(i);
    const marker = listMarkerText(row.p);
    const from = lineObj.from + row.p.quote.prefix.length + row.p.indent.raw.length;
    const to = from + marker.length;
    const insert = `${n}. `;
    if (marker !== insert) {
      changes.push({ from, to, insert });
    }
    n += 1;
  }
  return changes;
}
function renumberRunsNear(doc, parse, anchors) {
  const changes = [];
  const seen = /* @__PURE__ */ new Set();
  for (const anchor of anchors) {
    for (const c of renumberList(doc, parse, anchor)) {
      const key = `${c.from}:${c.to}:${c.insert}`;
      if (seen.has(key)) continue;
      seen.add(key);
      changes.push(c);
    }
  }
  return changes;
}
function buildPosMapper(changes, originalLength) {
  const sorted = [...changes].sort((a, b) => a.from - b.from);
  const segments = [];
  let m = 0;
  let o = 0;
  for (const c of sorted) {
    if (c.from > o) {
      segments.push({ mStart: m, oStart: o, len: c.from - o, insert: false });
      m += c.from - o;
    }
    if (c.insert.length > 0) {
      segments.push({ mStart: m, oStart: c.from, len: c.insert.length, insert: true });
      m += c.insert.length;
    }
    o = c.to;
  }
  if (o < originalLength) {
    const len = originalLength - o;
    segments.push({ mStart: m, oStart: o, len, insert: false });
    m += len;
  }
  const deleted = sorted.filter((c) => c.to > c.from);
  const findSegment = (mPos) => {
    for (let i = 0; i < segments.length; i++) {
      const s = segments[i];
      if (mPos >= s.mStart && mPos < s.mStart + s.len) return i;
    }
    return -1;
  };
  return {
    forward: (pos) => {
      for (const c of deleted) {
        if (pos >= c.from && pos < c.to) return null;
      }
      for (const s of segments) {
        if (s.insert) continue;
        if (pos >= s.oStart && pos < s.oStart + s.len) return s.mStart + (pos - s.oStart);
      }
      return m;
    },
    backward: (pos) => {
      const i = findSegment(pos);
      if (i >= 0) {
        const s = segments[i];
        return s.insert ? "insert" : s.oStart + (pos - s.mStart);
      }
      const last = segments[segments.length - 1];
      return last ? last.insert ? "insert" : last.oStart + last.len : pos;
    }
  };
}
function stringDoc(text) {
  const parts = text.length === 0 ? [""] : text.split("\n");
  const starts = [0];
  for (let i = 0; i < parts.length - 1; i++) {
    starts.push(starts[i] + parts[i].length + 1);
  }
  const lineCount = parts.length;
  const line = (n) => {
    if (n < 1 || n > lineCount) {
      throw new Error(`stringDoc.line: ${n} not in 1..${lineCount}`);
    }
    const from = starts[n - 1];
    const to = n < lineCount ? starts[n] - 1 : text.length;
    return { text: text.slice(from, to), from, to };
  };
  return {
    lines: lineCount,
    length: text.length,
    line,
    lineAt: (pos) => {
      const p = Math.max(0, Math.min(text.length, pos));
      for (let i = lineCount; i >= 1; i--) {
        if (starts[i - 1] <= p) return { number: i };
      }
      return { number: 1 };
    },
    sliceString: (from, to) => text.slice(from, to)
  };
}
function captureMoveSource(doc, selection) {
  const ranges = selectionLineRanges(doc.lines, selection);
  if (ranges.length === 0) return null;
  const segments = ranges.map((range) => {
    const start = doc.line(range.startLine);
    const end = doc.line(range.endLine);
    const deleteRange = resolveDeleteRange(doc, start.from, end.to);
    return {
      lines: range,
      from: start.from,
      to: end.to,
      deleteFrom: deleteRange.from,
      deleteTo: deleteRange.to
    };
  });
  const content = segments.map((s) => doc.sliceString(s.from, s.to)).join("\n");
  const first = ranges[0];
  const last = ranges[ranges.length - 1];
  return {
    block: {
      type: selection.blocks[0].type,
      lines: { startLine: first.startLine, endLine: last.endLine }
    },
    payload: { content, ranges, segments }
  };
}
function moveTx(params) {
  const { sourceDoc, plan } = params;
  const targetDoc = plan.position.doc;
  const parse = (text) => parseLine(text, plan.tabSize);
  const insertText = insertTextForMove({
    doc: targetDoc,
    sourceBlock: plan.captured.block,
    targetLineNumber: plan.position.line,
    sourceContent: plan.captured.payload.content,
    position: plan.position,
    tabSize: plan.tabSize,
    indentUnit: plan.indentUnit
  });
  if (!insertText.length) return reject("no_insert_text");
  if (sourceDoc !== targetDoc) {
    const insert = geometryInsert(targetDoc, plan.position.line, insertText);
    const del = geometryDelete(plan.captured.payload);
    return [compileDocEdit(targetDoc, insert, parse), compileDocEdit(sourceDoc, del, parse)];
  }
  const geometry = geometrySameDoc({
    doc: targetDoc,
    payload: plan.captured.payload,
    targetLine: plan.position.line,
    insertText,
    allowInPlace: plan.allowIndent
  });
  if ("type" in geometry) return geometry;
  return [compileDocEdit(targetDoc, geometry, parse)];
}
function compileDocEdit(doc, geometry, parse) {
  if (geometry.length === 0) {
    return { doc, changes: [] };
  }
  const changes = sortChanges(geometry);
  const edited = stringDoc(applyChanges(doc, changes));
  const anchors = renumberAnchors(doc, changes, edited, parse);
  const renumber = renumberRunsNear(edited, parse, anchors);
  if (renumber.length === 0) {
    return { doc, changes };
  }
  return { doc, changes: composeOnOriginal(doc, changes, renumber) };
}
function renumberAnchors(doc, geometry, edited, parse) {
  const sorted = [...geometry].sort((a, b) => a.from - b.from);
  const mapper = buildPosMapper(sorted, doc.length);
  const anchors = /* @__PURE__ */ new Set();
  const addRow = (mPos) => {
    const p = Math.max(0, Math.min(edited.length, mPos));
    anchors.add(edited.lineAt(p).number);
  };
  for (const c of sorted) {
    if (c.insert.length > 0) {
      const start = c.from + editedDeltaBefore(sorted, c.from);
      const end = start + c.insert.length;
      const first = firstContentLine(c.insert);
      const last = lastContentLine(c.insert);
      if (start > 0 && isOrderedListItem(parse(first))) {
        const above = parse(lineTextAt(edited, start - 1));
        if (sameListRun(parse(first), above)) addRow(start - 1);
      }
      if (end < edited.length && isOrderedListItem(parse(last))) {
        const below = parse(lineTextAt(edited, end));
        if (sameListRun(parse(last), below)) addRow(end);
      }
    }
    if (c.to > c.from) {
      const after = mapper.forward(c.to);
      if (c.from > 0) {
        const before = mapper.forward(c.from - 1);
        if (before !== null) {
          const r1 = parse(lineTextAt(edited, before));
          const lastDeleted = parse(lineTextAt(doc, c.to - 1));
          if (isOrderedListItem(lastDeleted) && sameListRun(r1, lastDeleted)) addRow(before);
          if (after !== null && sameListRun(r1, parse(lineTextAt(edited, after)))) addRow(before);
        }
      }
      if (after !== null) {
        const r2 = parse(lineTextAt(edited, after));
        const firstDeleted = parse(lineTextAt(doc, c.from));
        if (isOrderedListItem(firstDeleted) && sameListRun(firstDeleted, r2)) addRow(after);
      }
    }
  }
  return [...anchors];
}
function lineTextAt(doc, pos) {
  return doc.line(doc.lineAt(pos).number).text;
}
function composeOnOriginal(doc, geometry, renumber) {
  var _a;
  const sorted = [...geometry].sort((a, b) => a.from - b.from);
  const mapper = buildPosMapper(sorted, doc.length);
  let insert = null;
  let insertStart = 0;
  let insertEnd = 0;
  for (const c of sorted) {
    if (c.insert.length > 0) {
      insert = c;
      insertStart = c.from + editedDeltaBefore(sorted, c.from);
      insertEnd = insert.from;
      break;
    }
  }
  let insertText = (_a = insert == null ? void 0 : insert.insert) != null ? _a : "";
  const mapped = [];
  for (const r of renumber) {
    const from = mapper.backward(r.from);
    const to = mapper.backward(r.to);
    if (insert && from === "insert" && to === "insert") {
      const offA = r.from - insertStart;
      const offB = r.to - insertStart;
      insertText = insertText.slice(0, offA) + r.insert + insertText.slice(offB);
    } else if (typeof from === "number" && typeof to === "number") {
      if (insert && insert.from === insert.to && from === insert.from) {
        insertText = insertText + r.insert;
        insertEnd = Math.max(insertEnd, to);
      } else {
        mapped.push({ from, to, insert: r.insert });
      }
    }
  }
  const out = [];
  for (const c of sorted) {
    if (c === insert) {
      out.push({ from: insert.from, to: insertEnd, insert: insertText });
    } else {
      out.push(c);
    }
  }
  return sortChanges([...out, ...mapped]);
}
function editedDeltaBefore(sorted, from) {
  let delta = 0;
  for (const c of sorted) {
    if (c.from >= from) break;
    delta += c.insert.length - (c.to - c.from);
  }
  return delta;
}
function firstContentLine(text) {
  for (const line of text.split("\n")) {
    if (line.trim().length > 0) return line;
  }
  return "";
}
function lastContentLine(text) {
  const lines = text.split("\n");
  for (let i = lines.length - 1; i >= 0; i--) {
    if (lines[i].trim().length > 0) return lines[i];
  }
  return "";
}
function isOrderedListItem(p) {
  return isListLine(p) && listMarkerType(p) === "ordered";
}
function sameListRun(a, b) {
  return isListLine(a) && isListLine(b) && a.indent.width === b.indent.width && a.quote.depth === b.quote.depth;
}
function applyChanges(doc, changes) {
  let out = "";
  let pos = 0;
  for (const c of [...changes].sort((a, b) => a.from - b.from)) {
    out += doc.sliceString(pos, c.from) + c.insert;
    pos = c.to;
  }
  return out + doc.sliceString(pos, doc.length);
}
function geometryInsert(doc, targetLine, insertText) {
  const insertion = resolveInsertionChange(doc, targetLine, insertText, {
    lengthAfterDelete: doc.length
  });
  return [{ from: insertion.pos, to: insertion.pos, insert: insertion.text }];
}
function geometryDelete(payload) {
  return payload.segments.map((s) => ({
    from: s.deleteFrom,
    to: s.deleteTo,
    insert: ""
  }));
}
function geometrySameDoc(params) {
  const { doc, payload, targetLine, insertText, allowInPlace } = params;
  const deletedLen = payload.segments.reduce((sum, s) => sum + (s.deleteTo - s.deleteFrom), 0);
  const insertion = resolveInsertionChange(doc, targetLine, insertText, {
    lengthAfterDelete: doc.length - deletedLen
  });
  if (payload.segments.some((s) => insertion.pos > s.deleteFrom && insertion.pos < s.deleteTo)) {
    return reject("insertion_inside_deleted_range");
  }
  const first = payload.segments[0];
  if (allowInPlace && insertion.pos === first.deleteFrom) {
    return [
      {
        from: first.deleteFrom,
        to: first.deleteTo,
        insert: insertion.text
      }
    ];
  }
  return [{ from: insertion.pos, to: insertion.pos, insert: insertion.text }, ...geometryDelete(payload)];
}
function sortChanges(changes) {
  const key = (c) => `${c.from}:${c.to}:${c.insert}`;
  const seen = /* @__PURE__ */ new Set();
  const out = [];
  for (const c of [...changes].sort((a, b) => b.from - a.from || b.to - a.to)) {
    const k = key(c);
    if (seen.has(k)) continue;
    seen.add(k);
    out.push(c);
  }
  return out;
}
function planMove(input) {
  var _a, _b, _c;
  const targetDoc = input.position.doc;
  const captured = (_a = input.captured) != null ? _a : captureMoveSource(input.sourceDoc, input.selection);
  if (!captured) return { type: "reject", reason: "empty_selection" };
  const line = Math.max(1, Math.min(targetDoc.lines + 1, input.position.line));
  const position = {
    doc: targetDoc,
    line,
    parent: input.position.parent
  };
  const lineMap = getLineMap(targetDoc, { tabSize: input.tabSize });
  const slot = canDropAt(targetDoc, captured.block, line, {
    lineMap,
    tabSize: input.tabSize
  });
  if (!slot.decision.allowDrop) {
    return {
      type: "reject",
      reason: (_b = slot.decision.rejectReason) != null ? _b : "container_policy"
    };
  }
  let allowIndent = false;
  if (input.sourceDoc === targetDoc) {
    const parse = (text) => parseLine(text, input.tabSize);
    const self = selfDrop({
      doc: targetDoc,
      source: selectOne(captured.block),
      targetLineNumber: line,
      parseLineWithQuote: parse,
      lineMap,
      position,
      tabSize: input.tabSize,
      indentUnit: input.indentUnit
    });
    if (self.inSelfRange && !self.allowInPlaceIndentChange) {
      return {
        type: "reject",
        reason: (_c = self.rejectReason) != null ? _c : "self_range_blocked"
      };
    }
    allowIndent = self.allowInPlaceIndentChange;
  }
  return {
    type: "ok",
    value: {
      position,
      captured,
      allowIndent,
      tabSize: input.tabSize,
      indentUnit: input.indentUnit
    }
  };
}
function selectionFromOutputs(outputs) {
  let selection = null;
  for (const output of outputs) {
    if (output.type === "selection_changed" || output.type === "drag_source_changed" || output.type === "drag_over") {
      selection = output.selection;
    } else if (output.type === "cancelled" || output.type === "terminal" || output.type === "dropped") {
      selection = null;
    }
  }
  return selection;
}
function dropSeamState(outputs, doc) {
  let position = null;
  let invalid = false;
  for (const output of outputs) {
    if (output.type === "drag_over") {
      const onView = output.drop.position && output.drop.position.doc === doc ? output.drop.position : null;
      position = onView;
      invalid = onView !== null && output.drop.rejectReason != null;
    } else if (output.type === "dropped" || output.type === "cancelled" || output.type === "terminal") {
      position = null;
    }
  }
  return { position, invalid };
}
function dragSelectionDoc(outputs) {
  let doc = null;
  for (const output of outputs) {
    if (output.type === "drag_source_changed") {
      doc = output.sourceDoc;
    } else if (output.type === "drag_over") {
      doc = output.sourceDoc;
    } else if (output.type === "cancelled" || output.type === "terminal" || output.type === "dropped") {
      doc = null;
    }
  }
  return doc;
}
function samePointer(a, b) {
  return a.id === b.id;
}
var DEFAULT_GESTURE_CONFIG = {
  dragArmMs: 0,
  multiSelectMs: 500,
  dragStartMoveThresholdPx: 4,
  dragCancelMoveThresholdPx: 12,
  multiSelectEnabled: false
};
function notifyModules(modules, hook, ctx, result) {
  var _a, _b, _c, _d;
  for (const module2 of modules) {
    if (hook === "onDragEnd") {
      (_a = module2.onDragEnd) == null ? void 0 : _a.call(module2, ctx, result);
      continue;
    }
    if (hook === "onDragStart") (_b = module2.onDragStart) == null ? void 0 : _b.call(module2, ctx);
    else if (hook === "onDragMove") (_c = module2.onDragMove) == null ? void 0 : _c.call(module2, ctx);
    else if (hook === "onCancel") (_d = module2.onCancel) == null ? void 0 : _d.call(module2, ctx);
  }
}
var DefaultUx = class {
  constructor(deps) {
    this.deps = deps;
    this.disposables = [];
    this.pressSession = null;
    var _a;
    this.modules = (_a = deps.modules) != null ? _a : [];
  }
  mount() {
    const input = this.deps.input;
    this.disposables.push(input.onPress((e) => this.handlePress(e)));
    this.disposables.push(input.onMove((e) => this.handleMove(e)));
    this.disposables.push(input.onRelease((e) => this.handleRelease(e)));
    if (input.onCancel) {
      this.disposables.push(input.onCancel((e) => this.handleCancel(e.pointer, e.releaseCapture)));
    }
    if (input.onEscape) {
      this.disposables.push(input.onEscape(() => this.runtime().clearSelectionOrCancel("keyboard_escape")));
    }
  }
  destroy() {
    var _a, _b, _c;
    this.clearTimers();
    (_b = (_a = this.pressSession) == null ? void 0 : _a.releaseCapture) == null ? void 0 : _b.call(_a);
    this.pressSession = null;
    for (const module2 of this.modules) (_c = module2.destroy) == null ? void 0 : _c.call(module2);
    for (const dispose of this.disposables) dispose();
    this.disposables.length = 0;
  }
  runtime() {
    return this.deps.runtime;
  }
  cfg() {
    return this.deps.gestureConfig();
  }
  handlePress(input) {
    var _a;
    if (input.button !== void 0 && input.button !== 0) return;
    const lineNumber = this.deps.sourceLineFromInput(input);
    if (lineNumber === null) {
      this.runtime().clearSelectionOrCancel();
      return;
    }
    const block = detectBlock(this.deps.getDoc(), lineNumber, { tabSize: this.deps.tabSize });
    if (!block) return;
    (_a = input.capture) == null ? void 0 : _a.call(input);
    if (this.runtime().isGestureActive()) {
      this.cancelPress("session_interrupted", input.pointer.type);
    } else {
      this.clearPress();
    }
    const cfg = this.cfg();
    const sessionId = this.runtime().createSessionId();
    const selection = selectOne(block);
    const existing = this.currentSelection();
    const inSelecting = cfg.multiSelectEnabled && this.runtime().state.type === "selecting";
    const selectedDragCandidate = inSelecting && existing !== null && hasBlock(existing, block);
    if (inSelecting) {
      const groupArmMs = Math.max(cfg.dragArmMs, cfg.multiSelectMs);
      if (selectedDragCandidate) {
        const timer = groupArmMs > 0 ? this.deps.scheduler.setTimer(
          () => this.markSelectedDragReady(sessionId, input.pointer),
          groupArmMs
        ) : null;
        this.pressSession = this.makeSession({
          sessionId,
          pointer: input.pointer,
          start: input.point,
          anchorBlock: block,
          selection: existing,
          baseSelection: existing,
          ready: false,
          selectedDragCandidate: true,
          selectedDragReady: groupArmMs <= 0,
          armTimer: timer,
          multiSelectTimer: null,
          releaseCapture: input.releaseCapture
        });
        return;
      }
      this.pressSession = this.makeSession({
        sessionId,
        pointer: input.pointer,
        start: input.point,
        anchorBlock: block,
        selection: existing != null ? existing : selection,
        baseSelection: existing != null ? existing : { blocks: [] },
        ready: false,
        selectedDragCandidate: false,
        selectedDragReady: false,
        armTimer: null,
        multiSelectTimer: null,
        releaseCapture: input.releaseCapture
      });
      this.startToggleSweep(this.pressSession);
      return;
    }
    this.runtime().beginHold(sessionId, selection, input.pointer.type);
    if (cfg.multiSelectEnabled) {
      const multiMs = Math.max(0, cfg.multiSelectMs);
      const armMs2 = Math.max(0, cfg.dragArmMs);
      const multiSelectTimer = multiMs > 0 ? this.deps.scheduler.setTimer(
        () => this.startRangeSweepIfCurrent(sessionId, input.pointer),
        multiMs
      ) : null;
      const armTimer2 = armMs2 > 0 ? this.deps.scheduler.setTimer(() => this.markReady(sessionId, input.pointer), armMs2) : null;
      this.pressSession = this.makeSession({
        sessionId,
        pointer: input.pointer,
        start: input.point,
        anchorBlock: block,
        selection,
        baseSelection: { blocks: [] },
        ready: armMs2 <= 0,
        selectedDragCandidate: false,
        selectedDragReady: false,
        armTimer: armTimer2,
        multiSelectTimer,
        releaseCapture: input.releaseCapture
      });
      if (armMs2 <= 0) this.runtime().markHoldReady(sessionId, input.pointer.type);
      if (multiMs <= 0) this.startRangeSweep(this.pressSession);
      return;
    }
    const armMs = Math.max(0, cfg.dragArmMs);
    const armTimer = armMs > 0 ? this.deps.scheduler.setTimer(() => this.markReady(sessionId, input.pointer), armMs) : null;
    this.pressSession = this.makeSession({
      sessionId,
      pointer: input.pointer,
      start: input.point,
      anchorBlock: block,
      selection,
      baseSelection: { blocks: [] },
      ready: armMs <= 0,
      selectedDragCandidate: false,
      selectedDragReady: false,
      armTimer,
      multiSelectTimer: null,
      releaseCapture: input.releaseCapture
    });
    if (armMs <= 0) this.markReady(sessionId, input.pointer);
  }
  handleMove(input) {
    var _a, _b;
    const session = this.pressSession;
    if (!session || !samePointer(session.pointer, input.pointer)) return;
    if (this.runtime().isGestureActive()) {
      this.runtime().moveDrag(session.sessionId, input.point, input.pointer, input.pointer.type);
      this.emitModule("onDragMove", session, input.point, input.pointer);
      return;
    }
    const distance = distanceBetween(session.start, input.point);
    const cfg = this.cfg();
    if (session.selectedDragReady) {
      if (distance < cfg.dragStartMoveThresholdPx) return;
      const state = this.runtime().state;
      if (state.type !== "selecting" || state.selection.selection.blocks.length === 0) return;
      (_a = input.claim) == null ? void 0 : _a.call(input);
      this.clearTimers();
      this.startDrag(session, state.selection.selection, input.point, input.pointer);
      return;
    }
    if (session.selectedDragCandidate && !session.toggleSweep && !session.rangeActive) {
      if (distance < cfg.dragStartMoveThresholdPx) return;
      this.startToggleSweep(session);
    }
    if (session.toggleSweep) {
      this.updateToggleSelection(session, input.point);
      return;
    }
    if (session.rangeActive) {
      this.updateRangeSelection(session, input.point);
      return;
    }
    if (!session.ready) {
      if (distance > cfg.dragCancelMoveThresholdPx) {
        this.cancelPress("press_cancelled", input.pointer.type);
      }
      return;
    }
    if (distance < cfg.dragStartMoveThresholdPx) return;
    (_b = input.claim) == null ? void 0 : _b.call(input);
    this.clearTimers();
    if (this.runtime().state.type === "holding") {
      this.runtime().markHoldReady(session.sessionId, input.pointer.type);
    }
    this.startDrag(session, session.selection, input.point, input.pointer);
  }
  handleRelease(input) {
    const session = this.pressSession;
    if (this.runtime().isGestureActive() && session && samePointer(session.pointer, input.pointer)) {
      const result = this.runtime().commitDrop(session.sessionId, input.point, input.pointer, input.pointer.type);
      this.emitModule("onDragEnd", session, input.point, input.pointer, result != null ? result : { kind: "rejected" });
      this.pressSession = null;
      return;
    }
    if (!(session && samePointer(session.pointer, input.pointer))) return;
    if (session.rangeActive || session.toggleSweep) {
      this.clearPress();
      return;
    }
    if (session.selectedDragCandidate && !session.selectedDragReady) {
      const next = removeBlocks(session.baseSelection, [session.anchorBlock]);
      if (next.blocks.length === 0) this.runtime().clearSelection();
      else this.runtime().setSelection(next);
      this.clearPress();
      return;
    }
    if (session.selectedDragCandidate) {
      this.clearPress();
      return;
    }
    this.cancelPress("press_cancelled", input.pointer.type);
  }
  handleCancel(pointer, releaseCapture) {
    const session = this.pressSession;
    releaseCapture == null ? void 0 : releaseCapture();
    if (this.runtime().isGestureActive()) {
      if (session) {
        this.emitModule("onCancel", session, session.start, pointer);
      }
      this.runtime().cancel("pointer_cancelled", pointer.type);
      this.pressSession = null;
    } else if (session && samePointer(session.pointer, pointer)) {
      this.cancelPress("pointer_cancelled", pointer.type);
    }
  }
  startDrag(session, selection, point, pointer) {
    session.selection = selection;
    session.dragActive = true;
    this.runtime().beginDrag(session.sessionId, selection, point, pointer, pointer.type, session.releaseCapture);
    session.releaseCapture = void 0;
    this.emitModule("onDragStart", session, point, pointer);
  }
  emitModule(hook, session, point, pointer, result) {
    if (this.modules.length === 0) return;
    const ctx = {
      selection: session.selection,
      point,
      pointer
    };
    notifyModules(this.modules, hook, ctx, result);
  }
  markReady(sessionId, pointer) {
    const session = this.pressSession;
    if (!session || session.sessionId !== sessionId || !samePointer(session.pointer, pointer)) return;
    session.ready = true;
    if (session.armTimer !== null) {
      this.deps.scheduler.clearTimer(session.armTimer);
      session.armTimer = null;
    }
    this.runtime().markHoldReady(sessionId, pointer.type);
  }
  markSelectedDragReady(sessionId, pointer) {
    const session = this.pressSession;
    if (!session || session.sessionId !== sessionId || !samePointer(session.pointer, pointer)) return;
    if (!session.selectedDragCandidate) return;
    if (session.armTimer !== null) {
      this.deps.scheduler.clearTimer(session.armTimer);
      session.armTimer = null;
    }
    session.selectedDragReady = true;
  }
  startRangeSweepIfCurrent(sessionId, pointer) {
    const session = this.pressSession;
    if (!session || session.sessionId !== sessionId || !samePointer(session.pointer, pointer)) return;
    this.startRangeSweep(session);
  }
  startRangeSweep(session) {
    if (session.rangeActive || session.toggleSweep) return;
    this.clearTimers();
    session.rangeActive = true;
    session.selectedDragReady = false;
    session.ready = false;
    if (this.runtime().state.type === "holding" || this.runtime().state.type === "ready_to_drag") {
      this.runtime().cancel("session_interrupted", session.pointer.type);
    }
    this.runtime().setSelection(selectOne(session.anchorBlock));
    session.selection = selectOne(session.anchorBlock);
    session.baseSelection = { blocks: [] };
  }
  startToggleSweep(session) {
    if (session.toggleSweep || session.rangeActive) return;
    this.clearTimers();
    session.toggleSweep = true;
    session.selectedDragCandidate = false;
    session.selectedDragReady = false;
    session.ready = false;
    this.applyToggleRange(session, session.anchorBlock);
  }
  updateToggleSelection(session, point) {
    const lineNumber = this.deps.lineFromPoint(point);
    if (lineNumber === null) return;
    const focus = detectBlock(this.deps.getDoc(), lineNumber, { tabSize: this.deps.tabSize });
    if (!focus) return;
    this.applyToggleRange(session, focus);
  }
  applyToggleRange(session, focus) {
    const range = blocksBetween(this.deps.getDoc(), this.deps.tabSize, session.anchorBlock, focus);
    const next = xorSelection(session.baseSelection, range);
    session.selection = next;
    if (next.blocks.length === 0) this.runtime().clearSelection();
    else this.runtime().setSelection(next);
  }
  updateRangeSelection(session, point) {
    const lineNumber = this.deps.lineFromPoint(point);
    if (lineNumber === null) return;
    const doc = this.deps.getDoc();
    const focus = detectBlock(doc, lineNumber, { tabSize: this.deps.tabSize });
    if (!focus) return;
    const blocks = blocksBetween(doc, this.deps.tabSize, session.anchorBlock, focus);
    const selection = selectBlocks(blocks);
    session.selection = selection;
    this.runtime().setSelection(selection);
  }
  currentSelection() {
    const state = this.runtime().state;
    if (state.type !== "selecting") return null;
    return state.selection.selection;
  }
  cancelPress(reason, pointerType) {
    const session = this.pressSession;
    if (session == null ? void 0 : session.dragActive) {
      this.emitModule("onCancel", session, session.start, session.pointer);
    }
    this.clearPress();
    this.runtime().cancel(reason, pointerType);
  }
  clearPress() {
    var _a, _b;
    if (!this.pressSession) return;
    this.clearTimers();
    (_b = (_a = this.pressSession).releaseCapture) == null ? void 0 : _b.call(_a);
    this.pressSession = null;
  }
  /** One press-session shape for every branch — only the differing fields
   * are passed; the shared defaults (no range/toggle sweep yet, not an
   * active drag) live here. */
  makeSession(params) {
    return {
      ...params,
      rangeActive: false,
      toggleSweep: false,
      dragActive: false
    };
  }
  clearTimers() {
    const session = this.pressSession;
    if (!session) return;
    if (session.armTimer !== null) {
      this.deps.scheduler.clearTimer(session.armTimer);
      session.armTimer = null;
    }
    if (session.multiSelectTimer !== null) {
      this.deps.scheduler.clearTimer(session.multiSelectTimer);
      session.multiSelectTimer = null;
    }
  }
};
function distanceBetween(a, b) {
  return Math.hypot(b.x - a.x, b.y - a.y);
}
function blocksBetween(doc, tabSize, anchor, focus) {
  const start = Math.min(anchor.lines.startLine, focus.lines.startLine);
  const end = Math.max(anchor.lines.endLine, focus.lines.endLine);
  const blocks = [];
  let cursor = start;
  while (cursor <= end) {
    const block = detectBlock(doc, cursor, { tabSize });
    if (!block) {
      cursor += 1;
      continue;
    }
    blocks.push(block);
    cursor = block.lines.endLine + 1;
  }
  return blocks;
}
function xorSelection(base, range) {
  let next = base;
  for (const block of range) {
    next = hasBlock(next, block) ? removeBlocks(next, [block]) : addBlocks(next, [block]);
  }
  return next;
}
var IDLE_PIPELINE_STATE = { type: "idle" };
var DragPipeline = class {
  constructor(options = {}) {
    this.options = options;
    this.currentState = IDLE_PIPELINE_STATE;
  }
  get state() {
    return this.currentState;
  }
  enter(event) {
    var _a, _b;
    const previous = this.currentState;
    const transition = transitionPipelineState(previous, event);
    this.currentState = transition.state;
    const output = {
      outputs: this.decorateOutputs(previous, this.currentState, event, transition.outputs)
    };
    (_b = (_a = this.options).onChange) == null ? void 0 : _b.call(_a, output);
    return output;
  }
  clear() {
    return this.enter({ type: "destroy" });
  }
  decorateOutputs(previous, current, event, outputs) {
    const decorated = [...outputs];
    if (shouldClearSelectionVisual(previous, current) && !hasSelectionClearOutput(decorated)) {
      decorated.push({ type: "selection_changed", selection: null });
    }
    if (previous.type !== "dragging" && current.type === "dragging") {
      decorated.push({
        type: "drag_source_changed",
        selection: current.drag.selection,
        sourceDoc: current.drag.sourceDoc
      });
    }
    if (previous.type === "dragging" && current.type !== "dragging" && current.type !== "idle") {
      decorated.push({ type: "drag_source_changed", selection: null, sourceDoc: null });
    }
    if (previous.type !== "idle" && current.type === "idle") {
      decorated.push({ type: "drag_source_changed", selection: null, sourceDoc: null });
    }
    const terminalReason = resolveTerminalReason(previous, current, event);
    if (terminalReason) {
      decorated.push({ type: "terminal", reason: terminalReason });
    }
    return decorated;
  }
};
function shouldClearSelectionVisual(previous, current) {
  return previous.type === "selecting" && current.type !== "selecting";
}
function hasSelectionClearOutput(outputs) {
  return outputs.some((output) => output.type === "selection_changed" && output.selection === null);
}
function resolveTerminalReason(previous, current, event) {
  if (previous.type === "idle" || current.type !== "idle") return null;
  switch (event.type) {
    case "drop":
      return "drop";
    case "cancel":
      return "cancel";
    case "destroy":
      return "destroy";
    default:
      return null;
  }
}
function transitionPipelineState(state, event) {
  var _a;
  switch (event.type) {
    case "hold_start":
      return onHoldStart(state, event);
    case "hold_ready":
      return onHoldReady(state, event);
    case "selection_set":
      return onSelectionSet(state, event);
    case "selection_clear":
      return clearSelection(state);
    case "drag_start":
      return onDragStart(state, event);
    case "drag_over":
      return onDragOver(state, event);
    case "drop":
      return onDrop(state, event);
    case "cancel":
      return cancelPipeline(state, event.reason, (_a = event.pointerType) != null ? _a : null);
    case "destroy":
      return destroyPipeline();
  }
}
function onHoldStart(_state, event) {
  const next = {
    type: "holding",
    hold: {
      sessionId: event.sessionId,
      selection: event.selection
    }
  };
  return {
    state: next,
    outputs: [{ type: "state_changed", state: next }]
  };
}
function onHoldReady(state, event) {
  if (state.type !== "holding" || state.hold.sessionId !== event.sessionId) {
    return { state, outputs: [] };
  }
  const next = {
    type: "ready_to_drag",
    hold: state.hold
  };
  return {
    state: next,
    outputs: [{ type: "state_changed", state: next }]
  };
}
function onSelectionSet(_state, event) {
  const next = {
    type: "selecting",
    selection: {
      selection: event.selection
    }
  };
  return {
    state: next,
    outputs: [
      { type: "state_changed", state: next },
      { type: "selection_changed", selection: event.selection }
    ]
  };
}
function dragSourceFrom(state) {
  switch (state.type) {
    case "ready_to_drag":
      return state.hold.selection;
    case "selecting":
      return state.selection.selection;
    default:
      return null;
  }
}
function onDragStart(state, event) {
  var _a;
  if (state.type !== "ready_to_drag" && state.type !== "selecting") {
    return { state, outputs: [] };
  }
  const source = dragSourceFrom(state);
  if (source === null) {
    return { state, outputs: [] };
  }
  const sessionId = state.type === "ready_to_drag" ? state.hold.sessionId : event.sessionId;
  if (sessionId !== event.sessionId) {
    return { state, outputs: [] };
  }
  const next = {
    type: "dragging",
    drag: {
      sessionId: event.sessionId,
      selection: source,
      drop: event.drop,
      sourceDoc: event.sourceDoc
    }
  };
  return {
    state: next,
    outputs: [
      { type: "state_changed", state: next },
      ...dragOver({
        selection: next.drag.selection,
        drop: event.drop,
        sourceDoc: next.drag.sourceDoc,
        pointerType: (_a = event.pointerType) != null ? _a : null
      })
    ]
  };
}
function onDragOver(state, event) {
  var _a;
  if (state.type !== "dragging" || state.drag.sessionId !== event.sessionId) {
    return { state, outputs: [] };
  }
  const next = {
    type: "dragging",
    drag: {
      ...state.drag,
      drop: event.drop
    }
  };
  return {
    state: next,
    outputs: [
      { type: "state_changed", state: next },
      ...dragOver({
        selection: next.drag.selection,
        drop: event.drop,
        sourceDoc: next.drag.sourceDoc,
        pointerType: (_a = event.pointerType) != null ? _a : null
      })
    ]
  };
}
function onDrop(state, event) {
  var _a;
  if (state.type !== "dragging" || state.drag.sessionId !== event.sessionId) {
    return { state, outputs: [] };
  }
  return {
    state: IDLE_PIPELINE_STATE,
    outputs: [
      { type: "state_changed", state: IDLE_PIPELINE_STATE },
      ...drop({
        selection: state.drag.selection,
        resolution: event.resolution,
        pointerType: (_a = event.pointerType) != null ? _a : null
      })
    ]
  };
}
function dragOver(params) {
  return [
    {
      type: "drag_over",
      selection: params.selection,
      drop: params.drop,
      sourceDoc: params.sourceDoc,
      pointerType: params.pointerType
    }
  ];
}
function drop(params) {
  var _a, _b;
  if (params.resolution.type === "cancel") {
    return cancelDrop({
      selection: params.selection,
      reason: (_b = (_a = params.resolution.reason) != null ? _a : params.resolution.drop.rejectReason) != null ? _b : "no_target",
      pointerType: params.pointerType
    });
  }
  return [
    {
      type: "dropped",
      selection: params.selection,
      drop: params.resolution.drop,
      pointerType: params.pointerType
    }
  ];
}
function cancelDrop(params) {
  return [
    {
      type: "cancelled",
      selection: params.selection,
      reason: params.reason,
      pointerType: params.pointerType
    }
  ];
}
function cancelPipeline(state, reason, pointerType) {
  if (state.type === "idle") {
    return { state, outputs: [] };
  }
  const source = state.type === "holding" || state.type === "ready_to_drag" ? state.hold.selection : state.type === "selecting" ? state.selection.selection : state.drag.selection;
  return {
    state: IDLE_PIPELINE_STATE,
    outputs: [
      { type: "state_changed", state: IDLE_PIPELINE_STATE },
      ...cancelDrop({
        selection: source,
        reason,
        pointerType
      })
    ]
  };
}
function clearSelection(state) {
  if (state.type !== "selecting") {
    return { state, outputs: [] };
  }
  return {
    state: IDLE_PIPELINE_STATE,
    outputs: [
      { type: "selection_changed", selection: null },
      { type: "state_changed", state: IDLE_PIPELINE_STATE }
    ]
  };
}
function destroyPipeline() {
  return {
    state: IDLE_PIPELINE_STATE,
    outputs: [{ type: "state_changed", state: IDLE_PIPELINE_STATE }]
  };
}
var DraggerRuntime = class {
  constructor(options) {
    this.options = options;
    this.activeDragSession = null;
    this.mounted = false;
    this.nextSessionNumber = 1;
    this.ux = null;
    this.pipeline = new DragPipeline({
      onChange: (result) => {
        var _a, _b;
        return (_b = (_a = this.options).onChange) == null ? void 0 : _b.call(_a, result);
      }
    });
  }
  get state() {
    return this.pipeline.state;
  }
  mount() {
    if (this.mounted) return;
    this.mounted = true;
    this.ux = this.buildUx();
    this.ux.mount();
  }
  destroy() {
    var _a;
    (_a = this.ux) == null ? void 0 : _a.destroy();
    this.ux = null;
    this.endDragSession();
    this.pipeline.clear();
    this.mounted = false;
  }
  isGestureActive() {
    return this.pipeline.state.type === "dragging";
  }
  createSessionId() {
    const sessionId = `runtime-${this.nextSessionNumber}`;
    this.nextSessionNumber += 1;
    return sessionId;
  }
  beginHold(sessionId, selection, pointerType) {
    this.pipeline.enter({ type: "hold_start", sessionId, selection, pointerType });
  }
  markHoldReady(sessionId, pointerType) {
    if (this.pipeline.state.type !== "holding") return;
    if (this.pipeline.state.hold.sessionId !== sessionId) return;
    this.pipeline.enter({ type: "hold_ready", sessionId, pointerType });
  }
  beginDrag(sessionId, selection, point, pointer, pointerType, releaseCapture) {
    this.endDragSession();
    const position = this.resolvePosition(point, selection);
    this.activeDragSession = { sessionId, pointer, selection, position, releaseCapture };
    this.pipeline.enter({
      type: "drag_start",
      sessionId,
      drop: this.buildDropSnapshot(selection, position),
      sourceDoc: this.options.document.getDoc(),
      pointerType
    });
  }
  moveDrag(sessionId, point, pointer, pointerType) {
    const drag = this.activeDragSession;
    if (!drag || drag.sessionId !== sessionId || !samePointer(drag.pointer, pointer)) return;
    drag.position = this.resolvePosition(point, drag.selection);
    this.pipeline.enter({
      type: "drag_over",
      sessionId: drag.sessionId,
      drop: this.buildDropSnapshot(drag.selection, drag.position),
      pointerType
    });
  }
  commitDrop(sessionId, point, pointer, pointerType) {
    var _a, _b;
    const drag = this.activeDragSession;
    if (!drag || drag.sessionId !== sessionId || !samePointer(drag.pointer, pointer)) return;
    drag.position = this.resolvePosition(point, drag.selection);
    const dropSnapshot = this.buildDropSnapshot(drag.selection, drag.position);
    const planned = this.plan(drag.selection, drag.position);
    if (planned.type !== "ok" || !drag.position) {
      this.pipeline.enter({
        type: "drop",
        sessionId: drag.sessionId,
        resolution: this.cancelDrop(dropSnapshot, planned.type === "ok" ? "no_target" : planned.reason),
        pointerType
      });
      this.endDragSession();
      return { kind: "rejected" };
    }
    const edits = moveTx({ sourceDoc: this.options.document.getDoc(), plan: planned.value });
    if (!Array.isArray(edits)) {
      this.pipeline.enter({
        type: "drop",
        sessionId: drag.sessionId,
        resolution: this.cancelDrop(dropSnapshot, edits.reason),
        pointerType
      });
      this.endDragSession();
      return { kind: "rejected" };
    }
    this.pipeline.enter({
      type: "drop",
      sessionId: drag.sessionId,
      resolution: { type: "platform_commit", drop: dropSnapshot },
      pointerType
    });
    this.endDragSession();
    (_b = (_a = this.options.commit).apply) == null ? void 0 : _b.call(_a, edits);
    return { kind: "applied", edits };
  }
  setSelection(selection) {
    this.pipeline.enter({ type: "selection_set", selection });
  }
  clearSelection() {
    this.pipeline.enter({ type: "selection_clear" });
  }
  cancel(reason = "press_cancelled", pointerType = null) {
    this.endDragSession();
    this.pipeline.enter({ type: "cancel", reason, pointerType });
  }
  clearSelectionOrCancel(reason = "press_cancelled") {
    if (!this.isGestureActive() && this.pipeline.state.type === "selecting") {
      this.clearSelection();
      return true;
    }
    if (this.pipeline.state.type === "idle") return false;
    this.cancel(reason);
    return true;
  }
  buildUx() {
    var _a, _b, _c;
    if (typeof this.options.ux === "function") return this.options.ux(this);
    const uxConfig = (_a = this.options.ux) != null ? _a : {};
    const scheduler = (_b = this.options.scheduler) != null ? _b : {
      setTimer: (cb, ms) => setTimeout(cb, ms),
      clearTimer: (token) => clearTimeout(token)
    };
    return new DefaultUx({
      input: this.options.input,
      runtime: this,
      getDoc: () => this.options.document.getDoc(),
      sourceLineFromInput: (input) => this.options.locate.sourceLineFromInput(input),
      lineFromPoint: (point) => {
        var _a2, _b2, _c2;
        return (_c2 = (_b2 = (_a2 = this.options.locate).lineFromPoint) == null ? void 0 : _b2.call(_a2, point)) != null ? _c2 : null;
      },
      tabSize: this.config().tabSize,
      gestureConfig: () => this.resolveGestureConfig(uxConfig),
      scheduler,
      modules: (_c = uxConfig.modules) != null ? _c : []
    });
  }
  resolveGestureConfig(uxConfig) {
    const raw = typeof uxConfig.gesture === "function" ? uxConfig.gesture() : uxConfig.gesture;
    return { ...DEFAULT_GESTURE_CONFIG, ...raw };
  }
  endDragSession() {
    var _a, _b;
    (_b = (_a = this.activeDragSession) == null ? void 0 : _a.releaseCapture) == null ? void 0 : _b.call(_a);
    this.activeDragSession = null;
  }
  resolvePosition(point, selection) {
    const position = this.options.locate.resolveDropPosition(point, { selection });
    if (!position) return null;
    const doc = position.doc;
    const line = Math.max(1, Math.min(doc.lines + 1, position.line));
    return {
      doc,
      line,
      parent: position.parent
    };
  }
  plan(selection, position) {
    if (position === null) return { type: "reject", reason: "no_target" };
    const { tabSize, listIndentUnit } = this.config();
    return planMove({
      sourceDoc: this.options.document.getDoc(),
      selection,
      position,
      tabSize,
      indentUnit: listIndentUnit
    });
  }
  buildDropSnapshot(selection, position) {
    return {
      position,
      rejectReason: position === null ? "no_target" : this.dropRejectReason(selection, position)
    };
  }
  dropRejectReason(selection, position) {
    const planned = this.plan(selection, position);
    if (planned.type === "ok") return null;
    return isDragCancelReason(planned.reason) ? planned.reason : "selection_invalid";
  }
  cancelDrop(drop2, reason) {
    return {
      type: "cancel",
      drop: drop2,
      reason: isDragCancelReason(reason) ? reason : "selection_invalid"
    };
  }
  config() {
    const raw = typeof this.options.config === "function" ? this.options.config() : this.options.config;
    if (!raw) {
      throw new Error("DraggerRuntime: config is required (tabSize, listIndentUnit)");
    }
    if (!(raw.tabSize > 0)) {
      throw new Error(`DraggerRuntime: config.tabSize must be positive, got ${String(raw.tabSize)}`);
    }
    if (!(raw.listIndentUnit > 0)) {
      throw new Error(
        `DraggerRuntime: config.listIndentUnit must be positive, got ${String(raw.listIndentUnit)}`
      );
    }
    return raw;
  }
};
function isDragCancelReason(reason) {
  return reason !== "empty_selection";
}
function lineBand(view, line, options) {
  var _a;
  const doc = view.state.doc;
  if (line < 1 || line > doc.lines) return null;
  const docLine = doc.line(line);
  const parsed = parseLine(docLine.text, view.state.facet(import_state2.EditorState.tabSize));
  const bandFrom = docLine.from + parsed.quote.prefix.length + parsed.indent.raw.length;
  let left;
  if (((_a = parsed.marker) == null ? void 0 : _a.kind) === "list" && parsed.quote.prefix.length === 0) {
    const level = parsed.indent.width / resolveListIndentUnit(options);
    left = view.contentDOM.getBoundingClientRect().left + level * resolveListIndentWidthPx(options, view);
  } else {
    const content = view.coordsAtPos(bandFrom, 1);
    if (!content) return null;
    left = content.left;
  }
  const block = view.lineBlockAt(docLine.from);
  return {
    left,
    right: Math.max(left, view.contentDOM.getBoundingClientRect().right),
    top: view.documentTop + block.top,
    bottom: view.documentTop + block.bottom
  };
}
function dropSeam(view, position, options) {
  const doc = position.doc;
  const targetLine = position.line;
  const bandLine = targetLine <= 1 ? 1 : Math.min(targetLine - 1, doc.lines);
  if (bandLine < 1 || bandLine > doc.lines) return null;
  let left;
  if (position.parent) {
    const anchor = lineBand(view, position.parent.lines.startLine, options);
    if (!anchor) return null;
    left = anchor.left + resolveListIndentWidthPx(options, view);
  } else {
    left = view.contentDOM.getBoundingClientRect().left;
  }
  const block = view.lineBlockAt(doc.line(bandLine).from);
  return {
    left,
    right: Math.max(left, view.contentDOM.getBoundingClientRect().right),
    y: view.documentTop + (targetLine <= 1 ? block.top : block.bottom)
  };
}
var DROP_SEAM_CLASS = "md-dragger-drop-seam";
var DROP_SEAM_TOP_CLASS = "md-dragger-drop-seam-top";
var DROP_SEAM_BELOW_CLASS = "md-dragger-drop-seam-below";
var INVALID_CLASS = "is-invalid";
var DRAG_SOURCE_LINE_CLASS = "md-dragger-drag-source";
var SOURCE_LEVEL_STYLE_VAR = "--d-source-level";
var listIndentUnitFacet = import_state.Facet.define({
  combine: (values) => values[values.length - 1]
});
function sourceListLevel(lineText, tabSize, indentUnit) {
  var _a;
  const parsed = parseLine(lineText, tabSize);
  if (((_a = parsed.marker) == null ? void 0 : _a.kind) !== "list" || parsed.quote.prefix.length > 0) return 0;
  return Math.round(parsed.indent.width / indentUnit);
}
function sourceHighlightDecoration(outputs, state) {
  const selection = selectionFromOutputs(outputs);
  if (selection === null) return import_view2.Decoration.none;
  const sourceDoc = dragSelectionDoc(outputs);
  if (sourceDoc !== null && sourceDoc !== state.doc) return import_view2.Decoration.none;
  const tabSize = state.facet(import_state.EditorState.tabSize);
  const indentUnit = state.facet(listIndentUnitFacet);
  if (!(indentUnit > 0)) {
    throw new Error(
      "mdDragger: listIndentUnitFacet is not configured \u2014 mdDragger() registers it; when composing manually, add listIndentUnitFacet.of(config.listIndentUnit) to the extension array"
    );
  }
  const decorations = [];
  for (const range of selectionLineRanges(state.doc.lines, selection)) {
    for (let line = range.startLine; line <= range.endLine; line++) {
      if (line < 1 || line > state.doc.lines) continue;
      decorations.push(
        import_view2.Decoration.line({
          class: DRAG_SOURCE_LINE_CLASS,
          attributes: {
            style: `${SOURCE_LEVEL_STYLE_VAR}: ${sourceListLevel(state.doc.line(line).text, tabSize, indentUnit)}`
          }
        }).range(state.doc.line(line).from)
      );
    }
  }
  return import_view2.Decoration.set(decorations);
}
function dropSeamDecoration(outputs, state) {
  const { position, invalid } = dropSeamState(outputs, state.doc);
  if (position === null) return import_view2.Decoration.none;
  const top = position.line <= 1;
  const seamRow = top ? 1 : Math.min(position.line - 1, state.doc.lines);
  return import_view2.Decoration.set([
    import_view2.Decoration.line({
      class: `${DROP_SEAM_CLASS} ${top ? DROP_SEAM_TOP_CLASS : DROP_SEAM_BELOW_CLASS}${invalid ? ` ${INVALID_CLASS}` : ""}`
    }).range(state.doc.line(seamRow).from)
  ]);
}
function seamOffset(view, position, options) {
  const seam = dropSeam(view, position, options);
  if (!seam) return null;
  const contentLeft = view.contentDOM.getBoundingClientRect().left;
  return {
    left: Math.max(0, seam.left - contentLeft),
    width: Math.max(0, seam.right - seam.left)
  };
}
function createDefaultHandle() {
  const handle = document.createElement("button");
  handle.type = "button";
  handle.className = HANDLE_CLASS;
  handle.setAttribute("aria-label", "Drag markdown block");
  handle.textContent = "\u22EE\u22EE";
  return handle;
}
var BlockHandleMarker = class _BlockHandleMarker extends import_view3.GutterMarker {
  constructor(startLine, render) {
    super();
    this.startLine = startLine;
    this.render = render;
  }
  eq(other) {
    return other instanceof _BlockHandleMarker && other.startLine === this.startLine && other.render === this.render;
  }
  toDOM() {
    var _a, _b;
    const handle = (_b = (_a = this.render) == null ? void 0 : _a.call(this)) != null ? _b : createDefaultHandle();
    handle.setAttribute("data-block-start", String(this.startLine));
    return handle;
  }
};
function dragHandleGutter(options) {
  var _a;
  return (0, import_view3.gutter)({
    class: "md-dragger-gutter",
    side: (_a = options.handle) == null ? void 0 : _a.side,
    lineMarker: (view, line) => {
      var _a2;
      if (!isDraggerEnabled(options, view)) return null;
      const startLine = blockStartLine(view, line, options);
      if (startLine === null) return null;
      return new BlockHandleMarker(startLine, (_a2 = options.handle) == null ? void 0 : _a2.render);
    }
  });
}
function blockStartLine(view, line, options) {
  const docLine = view.state.doc.lineAt(line.from);
  if (docLine.from !== line.from) return null;
  const block = detectBlock(view.state.doc, docLine.number, {
    tabSize: resolveTabSize(options)
  });
  if (!block || block.lines.startLine !== docLine.number) return null;
  return block.lines.startLine;
}
var liveViews = /* @__PURE__ */ new Set();
function registerView(view) {
  liveViews.add(view);
  return () => {
    liveViews.delete(view);
  };
}
function broadcastToLiveViews(dispatch) {
  for (const view of liveViews) dispatch(view);
}
function viewForDoc(doc) {
  for (const view of liveViews) {
    if (view.state.doc === doc) return view;
  }
  return null;
}
function viewAtPoint(x, y) {
  if (liveViews.size === 0) return null;
  const hit = typeof document !== "undefined" ? document.elementFromPoint(x, y) : null;
  if (hit) {
    for (const view of liveViews) {
      if (view.dom.contains(hit)) return view;
    }
  }
  for (const view of liveViews) {
    const rect = view.dom.getBoundingClientRect();
    if (x >= rect.left && x <= rect.right && y >= rect.top && y <= rect.bottom) {
      return view;
    }
  }
  return null;
}
function applyCommit(edits) {
  for (const edit of edits) {
    const view = viewForDoc(edit.doc);
    if (!view) continue;
    dispatchChanges(view, edit.changes);
  }
}
function dispatchChanges(view, changes) {
  if (changes.length === 0) return;
  view.dispatch({ changes });
}
function pointerInput(view) {
  return {
    onPress: (handler) => {
      const listener = (event) => {
        handler({
          point: { x: event.clientX, y: event.clientY },
          pointer: { id: event.pointerId, type: event.pointerType },
          button: event.button,
          modifiers: {
            altKey: event.altKey,
            ctrlKey: event.ctrlKey,
            metaKey: event.metaKey,
            shiftKey: event.shiftKey
          },
          native: event,
          claim: () => claimPointerEvent(event),
          capture: () => capturePointer(view.dom, event.pointerId),
          releaseCapture: () => releasePointerCapture(view.dom, event.pointerId)
        });
      };
      view.dom.addEventListener("pointerdown", listener, true);
      return () => view.dom.removeEventListener("pointerdown", listener, true);
    },
    onMove: (handler) => {
      const listener = (event) => {
        handler({
          point: { x: event.clientX, y: event.clientY },
          pointer: { id: event.pointerId, type: event.pointerType },
          native: event,
          claim: () => claimPointerEvent(event)
        });
      };
      window.addEventListener("pointermove", listener, { capture: true, passive: false });
      return () => window.removeEventListener("pointermove", listener, true);
    },
    onRelease: (handler) => {
      const listener = (event) => {
        handler({
          point: { x: event.clientX, y: event.clientY },
          pointer: { id: event.pointerId, type: event.pointerType },
          native: event,
          claim: () => claimPointerEvent(event),
          releaseCapture: () => releasePointerCapture(view.dom, event.pointerId)
        });
      };
      window.addEventListener("pointerup", listener, { capture: true, passive: false });
      return () => window.removeEventListener("pointerup", listener, true);
    },
    onCancel: (handler) => {
      const pointerCancelListener = (event) => {
        handler({
          pointer: { id: event.pointerId, type: event.pointerType },
          reason: "pointer_cancelled",
          native: event,
          releaseCapture: () => releasePointerCapture(view.dom, event.pointerId)
        });
      };
      window.addEventListener("pointercancel", pointerCancelListener, { capture: true, passive: false });
      const cancelFallback = () => handler({
        pointer: { id: -1, type: null },
        reason: "pointer_cancelled"
      });
      const onWindowBlur = () => cancelFallback();
      const onVisibilityChange = () => {
        if (document.visibilityState === "hidden") cancelFallback();
      };
      window.addEventListener("blur", onWindowBlur);
      document.addEventListener("visibilitychange", onVisibilityChange);
      return () => {
        window.removeEventListener("pointercancel", pointerCancelListener, true);
        window.removeEventListener("blur", onWindowBlur);
        document.removeEventListener("visibilitychange", onVisibilityChange);
      };
    },
    onEscape: (handler) => {
      const listener = (event) => {
        if (event.key !== "Escape") return;
        if (handler()) {
          event.preventDefault();
          event.stopPropagation();
        }
      };
      window.addEventListener("keydown", listener, true);
      return () => window.removeEventListener("keydown", listener, true);
    }
  };
}
function nativePointerEvent(value) {
  return value instanceof PointerEvent ? value : null;
}
function claimPointerEvent(event) {
  event.preventDefault();
  event.stopPropagation();
}
function capturePointer(target, pointerId) {
  try {
    target.setPointerCapture(pointerId);
  } catch (e) {
  }
}
function releasePointerCapture(target, pointerId) {
  try {
    target.releasePointerCapture(pointerId);
  } catch (e) {
  }
}
function sourceLineFromInput(view, input) {
  var _a;
  const event = nativePointerEvent(input.native);
  const target = (event == null ? void 0 : event.target) instanceof Element ? event.target : null;
  const handle = (_a = target == null ? void 0 : target.closest(`.${HANDLE_CLASS}`)) != null ? _a : null;
  if (!handle || !view.dom.contains(handle)) return null;
  const fromAttr = Number(handle.getAttribute("data-block-start"));
  if (Number.isInteger(fromAttr) && fromAttr >= 1 && fromAttr <= view.state.doc.lines) {
    return fromAttr;
  }
  return lineAtPoint(view, input.point);
}
function lineAtPoint(view, point) {
  const contentRect = view.contentDOM.getBoundingClientRect();
  if (point.y <= contentRect.top) return 1;
  if (point.y >= contentRect.bottom) return view.state.doc.lines + 1;
  const pos = view.posAtCoords({ x: Math.max(contentRect.left + 1, point.x), y: point.y }, false);
  if (typeof pos !== "number") return null;
  return view.state.doc.lineAt(pos).number;
}
function resolveDropPosition(view, point, selection, sourceIndentWidth, targetIndentWidth, options) {
  const hitLine = lineAtPoint(view, point);
  if (hitLine === null) return null;
  const doc = view.state.doc;
  const tabSize = view.state.facet(import_state4.EditorState.tabSize);
  const indentUnit = resolveListIndentUnit(options);
  const inDoc = hitLine >= 1 && hitLine <= doc.lines;
  return locateDropPosition({
    doc,
    selection,
    hitLine,
    belowMid: inDoc ? belowMid(view, hitLine, point.y) : hitLine > doc.lines,
    sourceIndentWidth,
    targetIndentWidth,
    tabSize,
    indentUnit
  });
}
var NO_SNAP_REASONS = /* @__PURE__ */ new Set(["self_range_blocked", "self_embedding"]);
var SNAP_RADIUS = 4;
function snapDropPosition(input) {
  const { raw, sourceDoc, selection, sourceIndentWidth, targetIndentWidth, tabSize, indentUnit } = input;
  const doc = raw.doc;
  const seam = raw.line;
  const maxLine = doc.lines + 1;
  const plan = (position) => planMove({ sourceDoc, selection, position, tabSize, indentUnit });
  const rawPlan = plan(raw);
  if (rawPlan.type === "ok" || NO_SNAP_REASONS.has(rawPlan.reason)) return raw;
  const candidates = [];
  const push = (line) => {
    if (line < 1 || line > maxLine || candidates.includes(line)) return;
    candidates.push(line);
  };
  for (const probe of [seam, seam - 1]) {
    const block = detectBlock(doc, probe, { tabSize });
    if (!block) continue;
    push(block.lines.startLine);
    push(block.lines.endLine + 1);
  }
  for (let d = 1; d <= SNAP_RADIUS; d++) {
    push(seam - d);
    push(seam + d);
  }
  const byDistance = [...candidates].sort((a, b) => Math.abs(a - seam) - Math.abs(b - seam) || b - a);
  for (const line of byDistance) {
    const position = locateDropPosition({
      doc,
      selection,
      hitLine: line,
      belowMid: false,
      sourceIndentWidth,
      targetIndentWidth,
      tabSize,
      indentUnit
    });
    const planned = plan(position);
    if (planned.type === "ok" || NO_SNAP_REASONS.has(planned.reason)) return position;
  }
  return raw;
}
function resolveDropPositionAtPoint(sourceView, point, selection, options) {
  const source = selection.blocks[0];
  if (!source) return null;
  const sourceDoc = sourceView.state.doc;
  if (source.lines.startLine < 1 || source.lines.startLine > sourceDoc.lines) return null;
  const originBand = lineBand(sourceView, source.lines.startLine, options);
  if (!originBand) return null;
  const indentUnit = resolveListIndentUnit(options);
  const sourceIndentWidth = source.type === "list-item" ? parseLine(sourceDoc.line(source.lines.startLine).text, sourceView.state.facet(import_state4.EditorState.tabSize)).indent.width : 0;
  let targetIndentWidth = sourceIndentWidth;
  if (source.type === "list-item") {
    const horizontalSteps = Math.round((point.x - originBand.left) / resolveListIndentWidthPx(options, sourceView));
    targetIndentWidth += horizontalSteps * indentUnit;
  }
  const target = viewAtPoint(point.x, point.y);
  if (!target) return null;
  const position = resolveDropPosition(target, point, selection, sourceIndentWidth, targetIndentWidth, options);
  if (position === null) return null;
  return snapDropPosition({
    raw: position,
    sourceDoc,
    selection,
    sourceIndentWidth,
    targetIndentWidth,
    tabSize: target.state.facet(import_state4.EditorState.tabSize),
    indentUnit
  });
}
function lineAtScreenPoint(point) {
  const target = viewAtPoint(point.x, point.y);
  if (!target) return null;
  return lineAtPoint(target, point);
}
function belowMid(view, line, y) {
  const from = view.state.doc.line(line).from;
  try {
    const block = view.lineBlockAt(from);
    return y > view.documentTop + (block.top + block.bottom) / 2;
  } catch (e) {
    const coords = view.coordsAtPos(from, 1);
    if (!coords) return false;
    return y > coords.top + view.defaultLineHeight / 2;
  }
}
var dragTransitionEffect = import_state3.StateEffect.define();
function dragRuntime(options) {
  return import_view4.ViewPlugin.fromClass(
    class {
      constructor(view) {
        this.view = view;
        this.runtime = null;
        this.unregisterView = null;
        if (!isDraggerEnabled(options, view)) return;
        this.unregisterView = registerView(view);
        const locateOverride = resolveLocateOptions(options.locate, view);
        const rawInput = pointerInput(view);
        const input = {
          ...rawInput,
          onPress: (handler) => rawInput.onPress((press) => {
            if (isDraggerEnabled(options, view)) handler(press);
          })
        };
        this.runtime = new DraggerRuntime({
          input,
          document: {
            getDoc: () => view.state.doc
          },
          locate: {
            sourceLineFromInput: (input2) => {
              var _a, _b;
              return (_b = (_a = locateOverride == null ? void 0 : locateOverride.sourceLineFromInput) == null ? void 0 : _a.call(locateOverride, input2)) != null ? _b : sourceLineFromInput(view, input2);
            },
            resolveDropPosition: (point, context) => {
              var _a, _b;
              return (_b = (_a = locateOverride == null ? void 0 : locateOverride.resolveDropPosition) == null ? void 0 : _a.call(locateOverride, point, context)) != null ? _b : resolveDropPositionAtPoint(view, point, context.selection, options);
            },
            lineFromPoint: (point) => {
              var _a, _b, _c;
              return (_c = (_b = (_a = locateOverride == null ? void 0 : locateOverride.lineFromPoint) == null ? void 0 : _a.call(locateOverride, point)) != null ? _b : lineAtScreenPoint(point)) != null ? _c : lineAtPoint(view, point);
            }
          },
          commit: {
            apply: (edits) => applyCommit(edits)
          },
          onChange: (output) => {
            var _a;
            const hasDragOutput = output.outputs.some(
              (o) => o.type === "drag_source_changed" || o.type === "drag_over" || o.type === "dropped" || o.type === "cancelled" || o.type === "terminal"
            );
            if (hasDragOutput) {
              broadcastToLiveViews((v) => v.dispatch({ effects: dragTransitionEffect.of(output) }));
            } else {
              view.dispatch({ effects: dragTransitionEffect.of(output) });
            }
            (_a = options.onChange) == null ? void 0 : _a.call(options, output);
          },
          config: () => {
            const raw = typeof options.config === "function" ? options.config() : options.config;
            return resolveConfig({
              tabSize: view.state.facet(import_state3.EditorState.tabSize),
              listIndentUnit: raw.listIndentUnit
            });
          },
          ux: options.ux
        });
        this.runtime.mount();
      }
      destroy() {
        var _a, _b;
        (_a = this.runtime) == null ? void 0 : _a.destroy();
        (_b = this.unregisterView) == null ? void 0 : _b.call(this);
      }
    }
  );
}
function scrollPort(getDoc = () => document) {
  return {
    nudge(point, cfg) {
      var _a;
      const scroller = (_a = getDoc().elementFromPoint(point.x, point.y)) == null ? void 0 : _a.closest(".cm-scroller");
      if (!scroller) return;
      const rect = scroller.getBoundingClientRect();
      let dy = 0;
      const top = point.y - rect.top;
      const bottom = rect.bottom - point.y;
      if (top >= 0 && top < cfg.edgeZonePx) {
        dy = -cfg.maxSpeedPx * (1 - top / cfg.edgeZonePx);
      } else if (bottom >= 0 && bottom < cfg.edgeZonePx) {
        dy = cfg.maxSpeedPx * (1 - bottom / cfg.edgeZonePx);
      }
      if (dy !== 0) scroller.scrollTop += dy;
    }
  };
}
var editorAttributes = import_view.EditorView.editorAttributes.of({ class: EDITOR_CLASS });
function mdDragger(options) {
  return [
    editorAttributes,
    listIndentUnitFacet.of(resolveListIndentUnit(options)),
    dragHandleGutter(options),
    dragRuntime(options)
  ];
}

// node_modules/.pnpm/md-dragger@2.0.1_@codemirror+state@6.7.1_@codemirror+view@6.43.7/node_modules/md-dragger/dist/npm/domain.mjs
function isHorizontalRuleLine2(text) {
  if (!text) return false;
  const trimmed = text.trim();
  if (trimmed.length < 3) return false;
  return /^([-*_])(?:\s*\1){2,}$/.test(trimmed);
}
function isCalloutLine2(text) {
  if (!text) return false;
  return /^(\s*> ?)+\s*\[!/.test(text.trimStart());
}
function isTableLine2(text) {
  if (!text) return false;
  return text.trimStart().startsWith("|");
}
function isMathFenceLine2(text) {
  if (!text) return false;
  return text.trimStart().startsWith("$$");
}
function isCodeFenceLine2(text) {
  if (!text) return false;
  return text.trimStart().startsWith("```");
}
var fenceLazyScanCache2 = /* @__PURE__ */ new WeakMap();
function isSingleLineMathFence2(lineText) {
  const trimmed = lineText.trimStart();
  if (!trimmed.startsWith("$$")) return false;
  return trimmed.slice(2).includes("$$");
}
function assignFenceRangeByLine2(rangeByLine, startLine, endLine) {
  const range = { startLine, endLine };
  for (let i = startLine; i <= endLine; i++) {
    rangeByLine.set(i, range);
  }
}
function createFenceLazyScanState2() {
  return {
    scannedUntilLine: 0,
    openCodeStartLine: 0,
    openMathStartLine: 0,
    fullyScanned: false,
    codeRangeByLine: /* @__PURE__ */ new Map(),
    mathRangeByLine: /* @__PURE__ */ new Map()
  };
}
function getFenceLazyScanState2(doc) {
  const cached = fenceLazyScanCache2.get(doc);
  if (cached) return cached;
  const created = createFenceLazyScanState2();
  fenceLazyScanCache2.set(doc, created);
  return created;
}
function scanFenceLine2(state, lineNumber, text) {
  if (state.openCodeStartLine !== 0) {
    if (isCodeFenceLine2(text)) {
      assignFenceRangeByLine2(state.codeRangeByLine, state.openCodeStartLine, lineNumber);
      state.openCodeStartLine = 0;
    }
    return;
  }
  if (state.openMathStartLine !== 0) {
    if (isMathFenceLine2(text)) {
      assignFenceRangeByLine2(state.mathRangeByLine, state.openMathStartLine, lineNumber);
      state.openMathStartLine = 0;
    }
    return;
  }
  if (isCodeFenceLine2(text)) {
    state.openCodeStartLine = lineNumber;
    return;
  }
  if (isMathFenceLine2(text)) {
    if (isSingleLineMathFence2(text)) {
      assignFenceRangeByLine2(state.mathRangeByLine, lineNumber, lineNumber);
    } else {
      state.openMathStartLine = lineNumber;
    }
  }
}
function finalizeFenceStateAtDocEnd2(state) {
  if (state.openCodeStartLine !== 0) {
    assignFenceRangeByLine2(state.codeRangeByLine, state.openCodeStartLine, state.openCodeStartLine);
    state.openCodeStartLine = 0;
  }
  state.openMathStartLine = 0;
  state.fullyScanned = true;
}
function ensureFenceScanComplete2(doc) {
  const state = getFenceLazyScanState2(doc);
  if (state.fullyScanned) return state;
  let cursor = state.scannedUntilLine + 1;
  while (cursor <= doc.lines) {
    scanFenceLine2(state, cursor, doc.line(cursor).text);
    cursor++;
  }
  state.scannedUntilLine = Math.max(state.scannedUntilLine, cursor - 1);
  finalizeFenceStateAtDocEnd2(state);
  return state;
}
function findMathBlockRange2(doc, lineNumber) {
  var _a;
  if (lineNumber < 1 || lineNumber > doc.lines) return null;
  const state = ensureFenceScanComplete2(doc);
  return (_a = state.mathRangeByLine.get(lineNumber)) != null ? _a : null;
}
function findCodeBlockRange2(doc, lineNumber) {
  var _a;
  if (lineNumber < 1 || lineNumber > doc.lines) return null;
  const state = ensureFenceScanComplete2(doc);
  return (_a = state.codeRangeByLine.get(lineNumber)) != null ? _a : null;
}
function indentWidth2(raw, tabSize) {
  let width = 0;
  for (const ch of raw) {
    width += ch === "	" ? tabSize : 1;
  }
  return width;
}
function splitQuote2(line) {
  const match = line.match(/^(\s*> ?)+/);
  if (!match) return { prefix: "", depth: 0, rest: line };
  const prefix = match[0];
  return {
    prefix,
    depth: (prefix.match(/>/g) || []).length,
    rest: line.slice(prefix.length)
  };
}
function parseMarkerAndBody2(rest) {
  var _a;
  const indentMatch = rest.match(/^(\s*)/);
  const indentRaw = (_a = indentMatch == null ? void 0 : indentMatch[1]) != null ? _a : "";
  const afterIndent = rest.slice(indentRaw.length);
  const headingMatch = afterIndent.match(/^(#{1,6})\s+/);
  if (headingMatch) {
    const text = headingMatch[0];
    const level = headingMatch[1].length;
    return {
      indent: { raw: indentRaw, width: 0 },
      marker: { kind: "heading", text, level },
      body: afterIndent.slice(text.length)
    };
  }
  if (isHorizontalRuleLine2(afterIndent)) {
    return {
      indent: { raw: indentRaw, width: 0 },
      marker: { kind: "hr", text: afterIndent },
      body: ""
    };
  }
  if (isCodeFenceLine2(afterIndent)) {
    const info = afterIndent.replace(/^```\s*/, "").trim() || void 0;
    return {
      indent: { raw: indentRaw, width: 0 },
      marker: { kind: "fence", text: afterIndent, fence: "code", info },
      body: ""
    };
  }
  if (isMathFenceLine2(afterIndent)) {
    return {
      indent: { raw: indentRaw, width: 0 },
      marker: { kind: "fence", text: afterIndent, fence: "math" },
      body: ""
    };
  }
  if (isTableLine2(afterIndent)) {
    return {
      indent: { raw: indentRaw, width: 0 },
      marker: { kind: "table-row", text: afterIndent },
      body: ""
    };
  }
  if (isCalloutLine2(afterIndent) || /^\[![^\]]+\]/.test(afterIndent)) {
    const m = afterIndent.match(/^\[!([^\]]+)\]\s*/);
    if (m) {
      return {
        indent: { raw: indentRaw, width: 0 },
        marker: { kind: "callout", text: m[0], calloutType: m[1] },
        body: afterIndent.slice(m[0].length)
      };
    }
  }
  const taskMatch = afterIndent.match(/^([-*+])\s\[([ xX])\]\s+/);
  if (taskMatch) {
    const text = taskMatch[0];
    const checked = taskMatch[2] !== " ";
    return {
      indent: { raw: indentRaw, width: 0 },
      marker: { kind: "list", text, markerType: "task", checked },
      body: afterIndent.slice(text.length)
    };
  }
  const unorderedMatch = afterIndent.match(/^([-*+])\s+/);
  if (unorderedMatch) {
    const text = unorderedMatch[0];
    return {
      indent: { raw: indentRaw, width: 0 },
      marker: { kind: "list", text, markerType: "unordered" },
      body: afterIndent.slice(text.length)
    };
  }
  const orderedMatch = afterIndent.match(/^(\d+)[.)]\s+/);
  if (orderedMatch) {
    const text = orderedMatch[0];
    return {
      indent: { raw: indentRaw, width: 0 },
      marker: { kind: "list", text, markerType: "ordered" },
      body: afterIndent.slice(text.length)
    };
  }
  return {
    indent: { raw: indentRaw, width: 0 },
    marker: null,
    body: afterIndent
  };
}
function parseLine2(text, tabSize) {
  const { prefix, depth, rest } = splitQuote2(text);
  const { indent, marker, body } = parseMarkerAndBody2(rest);
  return {
    raw: text,
    quote: { depth, prefix },
    indent: {
      raw: indent.raw,
      width: indentWidth2(indent.raw, tabSize)
    },
    marker,
    body
  };
}
function isListLine2(p) {
  var _a;
  return ((_a = p.marker) == null ? void 0 : _a.kind) === "list";
}
var lineMapCache2 = /* @__PURE__ */ new WeakMap();
var EMPTY_LINE_META2 = {
  isEmpty: true,
  isList: false,
  isQuote: false,
  isCallout: false,
  isTable: false,
  isHr: false,
  indentWidth: 0,
  quoteDepth: 0
};
function createLineMetaFromText2(text, tabSize) {
  const parsed = parseLine2(text, tabSize);
  const isEmpty = text.trim().length === 0;
  return {
    isEmpty,
    isList: isListLine2(parsed),
    isQuote: parsed.quote.depth > 0,
    isCallout: isCalloutLine2(text),
    isTable: text.trimStart().startsWith("|"),
    isHr: isHorizontalRuleLine2(text),
    indentWidth: parsed.indent.width,
    quoteDepth: parsed.quote.depth
  };
}
function createLineMetaArray2(doc, tabSize) {
  var _a;
  const lineMeta = Array(doc.lines + 1);
  lineMeta[0] = EMPTY_LINE_META2;
  for (let i = 1; i <= doc.lines; i++) {
    lineMeta[i] = createLineMetaFromText2((_a = doc.line(i).text) != null ? _a : "", tabSize);
  }
  return lineMeta;
}
function buildLineMapIndexes2(lineMeta, totalLines) {
  var _a, _b, _c;
  const prevNonEmpty2 = new Int32Array(totalLines + 2);
  const nextNonEmpty2 = new Int32Array(totalLines + 2);
  const prevListLine = new Int32Array(totalLines + 2);
  const listParentLine = new Int32Array(totalLines + 2);
  const listSubtreeEndLine = new Int32Array(totalLines + 2);
  let previous = 0;
  let previousList = 0;
  const listStack = [];
  for (let i = 1; i <= totalLines; i++) {
    const meta = (_a = lineMeta[i]) != null ? _a : EMPTY_LINE_META2;
    if (!meta.isEmpty) {
      previous = i;
    }
    prevNonEmpty2[i] = previous;
    if (meta.isEmpty) {
      prevListLine[i] = previousList;
      continue;
    }
    while (listStack.length > 0) {
      const topLine = listStack[listStack.length - 1];
      const topMeta = (_b = lineMeta[topLine]) != null ? _b : EMPTY_LINE_META2;
      if (meta.indentWidth > topMeta.indentWidth) {
        break;
      }
      listStack.pop();
    }
    for (const ancestorLine of listStack) {
      listSubtreeEndLine[ancestorLine] = i;
    }
    prevListLine[i] = previousList;
    if (!meta.isList) {
      continue;
    }
    listParentLine[i] = listStack.length > 0 ? listStack[listStack.length - 1] : 0;
    listSubtreeEndLine[i] = i;
    listStack.push(i);
    previousList = i;
  }
  let next = 0;
  for (let i = totalLines; i >= 1; i--) {
    const meta = (_c = lineMeta[i]) != null ? _c : EMPTY_LINE_META2;
    if (!meta.isEmpty) {
      next = i;
    }
    nextNonEmpty2[i] = next;
  }
  return {
    prevNonEmpty: prevNonEmpty2,
    nextNonEmpty: nextNonEmpty2,
    prevListLine,
    listParentLine,
    listSubtreeEndLine
  };
}
function createLineMapFromMeta2(doc, tabSize, lineMeta) {
  const indexes = buildLineMapIndexes2(lineMeta, doc.lines);
  return {
    doc,
    lineMeta,
    prevNonEmpty: indexes.prevNonEmpty,
    nextNonEmpty: indexes.nextNonEmpty,
    prevListLine: indexes.prevListLine,
    listParentLine: indexes.listParentLine,
    listSubtreeEndLine: indexes.listSubtreeEndLine,
    tabSize
  };
}
function buildLineMap2(doc, options) {
  const tabSize = options.tabSize;
  const lineMeta = createLineMetaArray2(doc, tabSize);
  return createLineMapFromMeta2(doc, tabSize, lineMeta);
}
function getCachedLineMapForDoc2(doc, tabSize) {
  var _a, _b;
  if (!doc || typeof doc !== "object") return null;
  return (_b = (_a = lineMapCache2.get(doc)) == null ? void 0 : _a.get(tabSize)) != null ? _b : null;
}
function setCachedLineMapForDoc2(doc, tabSize, lineMap) {
  const byTabSize = lineMapCache2.get(doc);
  if (byTabSize) {
    byTabSize.set(tabSize, lineMap);
    return;
  }
  lineMapCache2.set(doc, /* @__PURE__ */ new Map([[tabSize, lineMap]]));
}
function getLineMap2(doc, options) {
  const tabSize = options.tabSize;
  if (!doc || typeof doc !== "object") {
    return buildLineMap2(doc, { tabSize });
  }
  const cached = getCachedLineMapForDoc2(doc, tabSize);
  if (cached) {
    return cached;
  }
  const built = buildLineMap2(doc, { tabSize });
  setCachedLineMapForDoc2(doc, tabSize, built);
  return built;
}
function peekCachedLineMap2(doc, options) {
  const tabSize = options.tabSize;
  if (!doc || typeof doc !== "object") return null;
  return getCachedLineMapForDoc2(doc, tabSize);
}
function getLineMetaAt2(lineMap, lineNumber) {
  var _a;
  if (lineNumber < 1 || lineNumber >= lineMap.lineMeta.length) return null;
  return (_a = lineMap.lineMeta[lineNumber]) != null ? _a : null;
}
var BlockType2 = /* @__PURE__ */ ((BlockType22) => {
  BlockType22["Paragraph"] = "paragraph";
  BlockType22["Heading"] = "heading";
  BlockType22["ListItem"] = "list-item";
  BlockType22["CodeBlock"] = "code-block";
  BlockType22["Blockquote"] = "blockquote";
  BlockType22["Table"] = "table";
  BlockType22["MathBlock"] = "math-block";
  BlockType22["Callout"] = "callout";
  BlockType22["HorizontalRule"] = "hr";
  BlockType22["Unknown"] = "unknown";
  return BlockType22;
})(BlockType2 || {});
function detectBlockType2(lineText, tabSize) {
  var _a, _b, _c, _d, _e, _f;
  const p = parseLine2(lineText, tabSize);
  if (((_a = p.marker) == null ? void 0 : _a.kind) === "heading") return "heading";
  if (((_b = p.marker) == null ? void 0 : _b.kind) === "hr") return "hr";
  if (((_c = p.marker) == null ? void 0 : _c.kind) === "list") return "list-item";
  if (((_d = p.marker) == null ? void 0 : _d.kind) === "fence") {
    return p.marker.fence === "code" ? "code-block" : "math-block";
  }
  if (((_e = p.marker) == null ? void 0 : _e.kind) === "table-row") return "table";
  if (((_f = p.marker) == null ? void 0 : _f.kind) === "callout") return "callout";
  if (p.quote.depth > 0) return "blockquote";
  if (p.body.trim().length === 0 && !p.marker) return "unknown";
  return "paragraph";
}
function isCalloutHeaderLine2(text, tabSize) {
  var _a;
  return ((_a = parseLine2(text, tabSize).marker) == null ? void 0 : _a.kind) === "callout";
}
function isInsideCalloutContainer2(doc, lineNumber, depth, tabSize) {
  var _a;
  for (let i = lineNumber; i >= 1; i--) {
    const text = doc.line(i).text;
    const p = parseLine2(text, tabSize);
    if (p.quote.depth === 0 || p.quote.depth < depth) break;
    if (((_a = p.marker) == null ? void 0 : _a.kind) === "callout" || isCalloutHeaderLine2(text, tabSize)) return true;
  }
  return false;
}
function getBlockquoteContainerRange2(doc, lineNumber, depth, tabSize) {
  let startLine = lineNumber;
  for (let i = lineNumber - 1; i >= 1; i--) {
    const d = parseLine2(doc.line(i).text, tabSize).quote.depth;
    if (d === 0 || d < depth) break;
    startLine = i;
  }
  let endLine = lineNumber;
  for (let i = lineNumber + 1; i <= doc.lines; i++) {
    const d = parseLine2(doc.line(i).text, tabSize).quote.depth;
    if (d === 0 || d < depth) break;
    endLine = i;
  }
  return { startLine, endLine };
}
function getListItemSubtreeRange2(doc, lineNumber, tabSize) {
  const current = parseLine2(doc.line(lineNumber).text, tabSize);
  const currentIndent = current.indent.width;
  let endLine = lineNumber;
  for (let i = lineNumber + 1; i <= doc.lines; i++) {
    const nextText = doc.line(i).text;
    if (nextText.trim().length === 0) {
      const lookahead = findNextNonEmptyLine2(doc, i + 1, tabSize);
      if (!lookahead || lookahead.isList && lookahead.indentWidth <= currentIndent || lookahead.indentWidth <= currentIndent) {
        break;
      }
      endLine = i;
      continue;
    }
    const next = parseLine2(nextText, tabSize);
    if (isListLine2(next) && next.indent.width <= currentIndent) {
      break;
    }
    if (isListLine2(next) || next.indent.width > currentIndent) {
      endLine = i;
      continue;
    }
    break;
  }
  return { startLine: lineNumber, endLine };
}
function findNextNonEmptyLine2(doc, fromLine, tabSize) {
  for (let i = fromLine; i <= doc.lines; i++) {
    const text = doc.line(i).text;
    if (text.trim().length === 0) continue;
    const p = parseLine2(text, tabSize);
    return { isList: isListLine2(p), indentWidth: p.indent.width };
  }
  return null;
}
var blockDetectionCache2 = /* @__PURE__ */ new WeakMap();
var LINE_MAP_EAGER_MAX2 = 3e4;
var YAML_FENCE_RE2 = /^-{3}\s*$/;
var yamlEndCache2 = /* @__PURE__ */ new WeakMap();
function yamlEndLine2(doc) {
  const cached = yamlEndCache2.get(doc);
  if (cached !== void 0) return cached;
  let endLine = 0;
  if (doc.lines >= 2 && YAML_FENCE_RE2.test(doc.line(1).text)) {
    for (let i = 2; i <= doc.lines; i++) {
      if (YAML_FENCE_RE2.test(doc.line(i).text)) {
        endLine = i;
        break;
      }
    }
  }
  yamlEndCache2.set(doc, endLine);
  return endLine;
}
function inYamlFrontmatter2(doc, lineNumber) {
  const endLine = yamlEndLine2(doc);
  return endLine > 0 && lineNumber >= 1 && lineNumber <= endLine;
}
function detectBlockUncached2(doc, lineNumber, tabSize) {
  if (lineNumber < 1 || lineNumber > doc.lines) {
    return null;
  }
  if (inYamlFrontmatter2(doc, lineNumber)) {
    return null;
  }
  const lineText = doc.line(lineNumber).text;
  let blockType = detectBlockType2(lineText, tabSize);
  const codeRange = findCodeBlockRange2(doc, lineNumber);
  const mathRange = findMathBlockRange2(doc, lineNumber);
  if (codeRange) {
    blockType = "code-block";
  }
  if (mathRange) {
    blockType = "math-block";
  }
  if (blockType === "unknown") {
    return null;
  }
  let startLine = lineNumber;
  let endLine = lineNumber;
  if (blockType === "code-block" && codeRange) {
    startLine = codeRange.startLine;
    endLine = codeRange.endLine;
  }
  if (blockType === "math-block" && mathRange) {
    startLine = mathRange.startLine;
    endLine = mathRange.endLine;
  }
  if (blockType === "list-item") {
    let lineMap = peekCachedLineMap2(doc, { tabSize });
    if (!lineMap && doc.lines <= LINE_MAP_EAGER_MAX2) {
      lineMap = getLineMap2(doc, { tabSize });
    }
    const lineMeta = lineMap ? getLineMetaAt2(lineMap, lineNumber) : null;
    const subtreeEndLine = (lineMeta == null ? void 0 : lineMeta.isList) && lineMap ? lineMap.listSubtreeEndLine[lineNumber] : 0;
    if (subtreeEndLine >= lineNumber) {
      endLine = subtreeEndLine;
    } else {
      endLine = getListItemSubtreeRange2(doc, lineNumber, tabSize).endLine;
    }
  }
  if (blockType === "blockquote" || blockType === "callout") {
    const quoteDepth = parseLine2(lineText, tabSize).quote.depth;
    const inCallout = blockType === "callout" || isInsideCalloutContainer2(doc, lineNumber, quoteDepth, tabSize);
    if (inCallout) {
      const range = getBlockquoteContainerRange2(doc, lineNumber, quoteDepth, tabSize);
      startLine = range.startLine;
      endLine = range.endLine;
      blockType = "callout";
    } else {
      startLine = lineNumber;
      endLine = lineNumber;
      blockType = "blockquote";
    }
  }
  if (blockType === "table") {
    for (let i = lineNumber - 1; i >= 1; i--) {
      if (isTableLine2(doc.line(i).text)) startLine = i;
      else break;
    }
    for (let i = lineNumber + 1; i <= doc.lines; i++) {
      if (isTableLine2(doc.line(i).text)) endLine = i;
      else break;
    }
  }
  return {
    type: blockType,
    lines: { startLine, endLine }
  };
}
function detectBlock2(doc, lineNumber, options) {
  var _a;
  const tabSize = options.tabSize;
  let cacheByTabSize = blockDetectionCache2.get(doc);
  if (!cacheByTabSize) {
    cacheByTabSize = /* @__PURE__ */ new Map();
    blockDetectionCache2.set(doc, cacheByTabSize);
  }
  let perDocCache = cacheByTabSize.get(tabSize);
  if (!perDocCache) {
    perDocCache = /* @__PURE__ */ new Map();
    cacheByTabSize.set(tabSize, perDocCache);
  }
  if (perDocCache.has(lineNumber)) {
    return (_a = perDocCache.get(lineNumber)) != null ? _a : null;
  }
  const block = detectBlockUncached2(doc, lineNumber, tabSize);
  if (block) {
    perDocCache.set(block.lines.startLine, block);
    for (let n = block.lines.startLine + 1; n <= block.lines.endLine; n++) {
      if (isListLine2(parseLine2(doc.line(n).text, tabSize))) {
        continue;
      }
      perDocCache.set(n, block);
    }
  } else {
    perDocCache.set(lineNumber, null);
  }
  return block;
}
function planConvert(params) {
  var _a, _b;
  const span = (_b = (_a = params.block) == null ? void 0 : _a.lines) != null ? _b : params.lines;
  if (!span) return [];
  return planConvertLines(params.doc, span.startLine, span.endLine, params.to);
}
function planConvertLines(doc, startLine, endLine, to) {
  var _a;
  const fenced = readFencedContent(doc, startLine, endLine);
  if (isFenceTarget(to)) {
    if ((fenced == null ? void 0 : fenced.type) === to.type) return [];
    return wrapAsFence(doc, startLine, endLine, to, (_a = fenced == null ? void 0 : fenced.contentLines) != null ? _a : null);
  }
  if (fenced) {
    return unwrapFence(doc, startLine, endLine, fenced.contentLines, to);
  }
  const changes = [];
  for (let n = startLine; n <= endLine; n++) {
    const line = doc.line(n);
    const next = convertLine(line.text, to, n - startLine + 1);
    if (next !== line.text) {
      changes.push({ from: line.from, to: line.to, insert: next });
    }
  }
  return changes;
}
function isFenceTarget(to) {
  return to.type === "code-block" || to.type === "math-block";
}
function readFencedContent(doc, startLine, endLine) {
  const startText = doc.line(startLine).text;
  const endText = doc.line(endLine).text;
  if (isCodeFenceLine2(startText) && startLine < endLine && isCodeFenceLine2(endText)) {
    return { type: "code-block", contentLines: innerLines(doc, startLine, endLine) };
  }
  if (isMathFenceLine2(startText)) {
    if (startLine === endLine) {
      const content = singleLineMathBody(startText);
      if (content !== null) {
        return { type: "math-block", contentLines: [content] };
      }
    }
    if (startLine < endLine && isMathFenceLine2(endText)) {
      return { type: "math-block", contentLines: innerLines(doc, startLine, endLine) };
    }
  }
  return null;
}
function innerLines(doc, startLine, endLine) {
  const out = [];
  for (let n = startLine + 1; n < endLine; n++) out.push(doc.line(n).text);
  return out;
}
function singleLineMathBody(text) {
  const trimmed = text.trim();
  if (!trimmed.startsWith("$$") || !trimmed.endsWith("$$") || trimmed.length < 4) return null;
  return trimmed.slice(2, -2).trim();
}
function unwrapFence(doc, startLine, endLine, contentLines, to) {
  const from = doc.line(startLine).from;
  const toPos = doc.line(endLine).to;
  const insert = contentLines.map((line, i) => {
    const { indentRaw, body } = splitIndent(line);
    return formatBody(indentRaw, body, to, i + 1);
  }).join("\n");
  return [{ from, to: toPos, insert }];
}
function wrapAsFence(doc, startLine, endLine, to, existingContent) {
  const from = doc.line(startLine).from;
  const toPos = doc.line(endLine).to;
  const content = existingContent ? existingContent.join("\n") : Array.from(
    { length: endLine - startLine + 1 },
    (_, i) => stripPrefix(doc.line(startLine + i).text).body
  ).join("\n");
  const fence = to.type === "code-block" ? "```" : "$$";
  return [{ from, to: toPos, insert: `${fence}
${content}
${fence}` }];
}
function convertLine(text, to, ordinal) {
  const { indentRaw, body } = stripPrefix(text);
  return formatBody(indentRaw, body, to, ordinal);
}
function formatBody(indentRaw, body, to, ordinal) {
  switch (to.type) {
    case "paragraph":
      return `${indentRaw}${body}`;
    case "heading":
      return `${indentRaw}${"#".repeat(to.level)} ${body}`;
    case "list-item":
      return `${indentRaw}${listMarker(to.markerType, ordinal)}${body}`;
    case "blockquote":
      return `> ${indentRaw}${body}`;
  }
}
function listMarker(markerType, ordinal) {
  switch (markerType) {
    case "ordered":
      return `${ordinal}. `;
    case "task":
      return "- [ ] ";
    case "unordered":
      return "- ";
  }
}
function stripPrefix(text) {
  var _a;
  const quoteMatch = text.match(/^(\s*>\s?)*/);
  const withoutQuote = text.slice((_a = quoteMatch == null ? void 0 : quoteMatch[0].length) != null ? _a : 0);
  const { indentRaw, body } = splitIndent(withoutQuote);
  let rest = body.replace(/^#{1,6}\s+/, "");
  const listMatch = rest.match(/^((?:[-*+]\s\[[ xX]\]\s+)|(?:[-*+]\s+)|(?:\d+[.)]\s+))/);
  if (listMatch) rest = rest.slice(listMatch[0].length);
  return { indentRaw, body: rest };
}
function splitIndent(text) {
  var _a;
  const m = text.match(/^(\s*)/);
  const indentRaw = (_a = m == null ? void 0 : m[0]) != null ? _a : "";
  return { indentRaw, body: text.slice(indentRaw.length) };
}
function normalizeLineRange2(docLines, startLine, endLine) {
  if (docLines <= 0) {
    return { startLine: 1, endLine: 1 };
  }
  const safeStart = Math.max(1, Math.min(docLines, Math.min(startLine, endLine)));
  const safeEnd = Math.max(1, Math.min(docLines, Math.max(startLine, endLine)));
  return { startLine: safeStart, endLine: safeEnd };
}
function mergeLineRanges2(docLines, ranges) {
  const normalized = ranges.map((range) => normalizeLineRange2(docLines, range.startLine, range.endLine)).sort((a, b) => a.startLine - b.startLine || a.endLine - b.endLine);
  const merged = [];
  for (const range of normalized) {
    const last = merged[merged.length - 1];
    if (!last || range.startLine > last.endLine + 1) {
      merged.push({ ...range });
      continue;
    }
    if (range.endLine > last.endLine) {
      last.endLine = range.endLine;
    }
  }
  return merged;
}
function isLineNumberInRanges2(line, ranges) {
  for (const range of ranges) {
    if (line >= range.startLine && line <= range.endLine) return true;
  }
  return false;
}
function selectOne2(block) {
  return { blocks: [block] };
}
function selectionLineRanges2(docLines, selection) {
  return mergeLineRanges2(
    docLines,
    selection.blocks.map((block) => block.lines)
  );
}
var ALL_TYPES2 = Object.values(BlockType2);
function rejectEntries2(types, slot, reason) {
  return types.map((t2) => [`${t2}|${slot}`, reason]);
}
var REJECT_RULES2 = new Map([
  ...rejectEntries2(ALL_TYPES2, "inside_code_block", "inside_code_block"),
  ...rejectEntries2(ALL_TYPES2, "inside_math_block", "inside_math_block"),
  ...rejectEntries2(
    ALL_TYPES2.filter(
      (t2) => t2 !== "list-item"
      /* ListItem */
    ),
    "inside_list",
    "inside_list"
  ),
  ...rejectEntries2(
    ALL_TYPES2.filter(
      (t2) => t2 !== "blockquote"
      /* Blockquote */
    ),
    "inside_quote_run",
    "inside_quote_run"
  ),
  ...rejectEntries2([
    "callout"
    /* Callout */
  ], "quote_before", "quote_boundary"),
  ...rejectEntries2(
    ALL_TYPES2.filter(
      (t2) => t2 !== "blockquote"
      /* Blockquote */
    ),
    "quote_after",
    "quote_boundary"
  ),
  ...rejectEntries2(ALL_TYPES2, "callout_after", "callout_after"),
  ...rejectEntries2(ALL_TYPES2, "table_before", "table_before"),
  ...rejectEntries2(ALL_TYPES2, "hr_before", "hr_before")
]);
function reject2(reason) {
  return { type: "reject", reason };
}
function isReject(value) {
  return typeof value === "object" && value !== null && value.type === "reject";
}
function planDelete(params) {
  const { doc, selection } = params;
  const ranges = selectionLineRanges2(doc.lines, selection);
  if (ranges.length === 0) return reject2("empty_selection");
  const changes = ranges.map((range) => {
    const startLine = doc.line(range.startLine);
    const endLine = doc.line(range.endLine);
    const deletesOnlyFinalLine = range.startLine === range.endLine && range.endLine === doc.lines && range.startLine > 1;
    return {
      from: deletesOnlyFinalLine ? startLine.from - 1 : startLine.from,
      to: range.endLine === doc.lines ? doc.length : Math.min(doc.length, endLine.to + 1),
      insert: ""
    };
  }).filter((change) => change.to > change.from).sort((a, b) => b.from - a.from);
  if (changes.length === 0) return reject2("empty_selection");
  return { doc, changes };
}

// node_modules/.pnpm/md-dragger@2.0.1_@codemirror+state@6.7.1_@codemirror+view@6.43.7/node_modules/md-dragger/dist/npm/runtime.mjs
function selectionFromOutputs2(outputs) {
  let selection = null;
  for (const output of outputs) {
    if (output.type === "selection_changed" || output.type === "drag_source_changed" || output.type === "drag_over") {
      selection = output.selection;
    } else if (output.type === "cancelled" || output.type === "terminal" || output.type === "dropped") {
      selection = null;
    }
  }
  return selection;
}
function dropSeamState2(outputs, doc) {
  let position = null;
  let invalid = false;
  for (const output of outputs) {
    if (output.type === "drag_over") {
      const onView = output.drop.position && output.drop.position.doc === doc ? output.drop.position : null;
      position = onView;
      invalid = onView !== null && output.drop.rejectReason != null;
    } else if (output.type === "dropped" || output.type === "cancelled" || output.type === "terminal") {
      position = null;
    }
  }
  return { position, invalid };
}
function dragSelectionDoc2(outputs) {
  let doc = null;
  for (const output of outputs) {
    if (output.type === "drag_source_changed") {
      doc = output.sourceDoc;
    } else if (output.type === "drag_over") {
      doc = output.sourceDoc;
    } else if (output.type === "cancelled" || output.type === "terminal" || output.type === "dropped") {
      doc = null;
    }
  }
  return doc;
}
var BlockType3 = /* @__PURE__ */ ((BlockType22) => {
  BlockType22["Paragraph"] = "paragraph";
  BlockType22["Heading"] = "heading";
  BlockType22["ListItem"] = "list-item";
  BlockType22["CodeBlock"] = "code-block";
  BlockType22["Blockquote"] = "blockquote";
  BlockType22["Table"] = "table";
  BlockType22["MathBlock"] = "math-block";
  BlockType22["Callout"] = "callout";
  BlockType22["HorizontalRule"] = "hr";
  BlockType22["Unknown"] = "unknown";
  return BlockType22;
})(BlockType3 || {});
var ALL_TYPES3 = Object.values(BlockType3);
function rejectEntries3(types, slot, reason) {
  return types.map((t2) => [`${t2}|${slot}`, reason]);
}
var REJECT_RULES3 = new Map([
  ...rejectEntries3(ALL_TYPES3, "inside_code_block", "inside_code_block"),
  ...rejectEntries3(ALL_TYPES3, "inside_math_block", "inside_math_block"),
  ...rejectEntries3(
    ALL_TYPES3.filter(
      (t2) => t2 !== "list-item"
      /* ListItem */
    ),
    "inside_list",
    "inside_list"
  ),
  ...rejectEntries3(
    ALL_TYPES3.filter(
      (t2) => t2 !== "blockquote"
      /* Blockquote */
    ),
    "inside_quote_run",
    "inside_quote_run"
  ),
  ...rejectEntries3([
    "callout"
    /* Callout */
  ], "quote_before", "quote_boundary"),
  ...rejectEntries3(
    ALL_TYPES3.filter(
      (t2) => t2 !== "blockquote"
      /* Blockquote */
    ),
    "quote_after",
    "quote_boundary"
  ),
  ...rejectEntries3(ALL_TYPES3, "callout_after", "callout_after"),
  ...rejectEntries3(ALL_TYPES3, "table_before", "table_before"),
  ...rejectEntries3(ALL_TYPES3, "hr_before", "hr_before")
]);

// node_modules/.pnpm/md-dragger@2.0.1_@codemirror+state@6.7.1_@codemirror+view@6.43.7/node_modules/md-dragger/dist/npm/runtime/modules.mjs
var TICK_MS = 16;
function autoScroll(port, config) {
  const cfg = () => typeof config === "function" ? config() : config;
  let tick = null;
  let point = null;
  const nudge = () => {
    if (point === null) return;
    const active = cfg();
    if (active.edgeZonePx <= 0 || active.maxSpeedPx <= 0) return;
    port.nudge(point, active);
  };
  const start = () => {
    stop();
    tick = setInterval(nudge, TICK_MS);
  };
  const stop = () => {
    if (tick !== null) {
      clearInterval(tick);
      tick = null;
    }
  };
  return {
    name: "auto-scroll",
    onDragStart(ctx) {
      point = ctx.point;
      start();
      nudge();
    },
    onDragMove(ctx) {
      point = ctx.point;
      nudge();
    },
    onDragEnd: stop,
    onCancel: stop,
    destroy: stop
  };
}

// src/plugin/block-type-menu.ts
var import_obsidian = require("obsidian");
var import_view6 = require("@codemirror/view");

// src/plugin/block-type-commands.ts
var import_state5 = require("@codemirror/state");
var import_view5 = require("@codemirror/view");
var PARAGRAPH_BLOCK_TYPE_OPTION = {
  target: { type: BlockType2.Paragraph },
  label: "Paragraph",
  icon: "pilcrow"
};
var HEADING_BLOCK_TYPE_OPTIONS = [
  { target: { type: BlockType2.Heading, level: 1 }, label: "Heading 1", icon: "heading-1" },
  { target: { type: BlockType2.Heading, level: 2 }, label: "Heading 2", icon: "heading-2" },
  { target: { type: BlockType2.Heading, level: 3 }, label: "Heading 3", icon: "heading-3" },
  { target: { type: BlockType2.Heading, level: 4 }, label: "Heading 4", icon: "heading-4" },
  { target: { type: BlockType2.Heading, level: 5 }, label: "Heading 5", icon: "heading-5" },
  { target: { type: BlockType2.Heading, level: 6 }, label: "Heading 6", icon: "heading-6" }
];
var LIST_BLOCK_TYPE_OPTIONS = [
  { target: { type: BlockType2.ListItem, markerType: "unordered" }, label: "Bullet list", icon: "list" },
  { target: { type: BlockType2.ListItem, markerType: "ordered" }, label: "Numbered list", icon: "list-ordered" },
  { target: { type: BlockType2.ListItem, markerType: "task" }, label: "Task list", icon: "list-checks" }
];
var SIMPLE_BLOCK_TYPE_OPTIONS = [
  { target: { type: BlockType2.Blockquote }, label: "Quote", icon: "quote" },
  { target: { type: BlockType2.CodeBlock }, label: "Code block", icon: "code" },
  { target: { type: BlockType2.MathBlock }, label: "Math block", icon: "sigma" }
];
function convertCurrentBlockType(view, conversion, lineNumber) {
  const block = getBlockAt(view, lineNumber);
  if (!block) return false;
  const changes = planConvert({
    doc: view.state.doc,
    block,
    to: conversion
  });
  if (changes.length === 0) return false;
  view.dispatch({
    changes,
    scrollIntoView: false
  });
  return true;
}
function deleteCurrentBlock(view, lineNumber) {
  const block = getBlockAt(view, lineNumber);
  if (!block) return false;
  const result = planDelete({
    doc: view.state.doc,
    selection: selectOne2(block)
  });
  if (isReject(result)) return false;
  view.dispatch({
    changes: result.changes,
    scrollIntoView: false
  });
  return true;
}
async function copyCurrentBlock(view, lineNumber) {
  const block = getBlockAt(view, lineNumber);
  if (!block) return false;
  const from = view.state.doc.line(block.lines.startLine).from;
  const to = view.state.doc.line(block.lines.endLine).to;
  const text = view.state.doc.sliceString(from, to);
  if (typeof navigator === "undefined" || !navigator.clipboard) return false;
  try {
    await navigator.clipboard.writeText(text);
    return true;
  } catch (e) {
    return false;
  }
}
async function cutCurrentBlock(view, lineNumber) {
  const copied = await copyCurrentBlock(view, lineNumber);
  if (!copied) return false;
  return deleteCurrentBlock(view, lineNumber);
}
function getBlockAt(view, lineNumber) {
  const resolved = lineNumber != null ? lineNumber : view.state.doc.lineAt(view.state.selection.main.head).number;
  return detectBlock2(view.state.doc, resolved, {
    tabSize: view.state.facet(import_state5.EditorState.tabSize)
  });
}

// src/plugin/block-type-menu.ts
var NESTED_GROUPS = [
  { label: "Heading", icon: "heading", options: HEADING_BLOCK_TYPE_OPTIONS },
  { label: "List", icon: "list", options: LIST_BLOCK_TYPE_OPTIONS }
];
var FLYOUT_CLASS = "d-block-type-flyout";
var FLYOUT_ITEM_CLASS = "d-block-type-flyout-item";
var menuBlockLine = 0;
var flyoutEl = null;
var flyoutTrigger = null;
var flyoutCloseTimer = null;
var rootMenu = null;
function openBlockTypeMenu(view, event, lineNumber) {
  disposeFlyout();
  menuBlockLine = lineNumber != null ? lineNumber : view.state.doc.lineAt(view.state.selection.main.head).number;
  showRootMenu(view, event);
}
function showRootMenu(view, event) {
  const menu = new import_obsidian.Menu();
  menu.setUseNativeMenu(false);
  rootMenu = menu;
  const line = menuBlockLine;
  menu.onHide(() => {
    window.requestAnimationFrame(() => {
      if (rootMenu === menu) rootMenu = null;
      disposeFlyout();
    });
  });
  addConversionItem(menu, view, PARAGRAPH_BLOCK_TYPE_OPTION, line, () => menu.hide());
  for (const group of NESTED_GROUPS) {
    menu.addItem((item) => {
      item.setTitle(createGroupTitle(group.label)).setIcon(group.icon);
      if (import_obsidian.Platform.isMobile) {
        item.onClick(() => {
          showMobileGroupPage(view, group, line);
        });
      }
    });
  }
  for (const option of SIMPLE_BLOCK_TYPE_OPTIONS) {
    addConversionItem(menu, view, option, line, () => menu.hide());
  }
  menu.addSeparator();
  addActionItem(menu, {
    label: "Copy block",
    icon: "copy",
    run: () => copyCurrentBlock(view, line),
    failureNotice: "Unable to copy block."
  });
  addActionItem(menu, {
    label: "Cut block",
    icon: "scissors",
    run: () => cutCurrentBlock(view, line),
    failureNotice: "Unable to cut block."
  });
  addActionItem(menu, {
    label: "Delete block",
    icon: "trash-2",
    warning: true,
    run: () => deleteCurrentBlock(view, line),
    failureNotice: "Unable to delete block."
  });
  showMenuAt(menu, view, event);
  if (import_obsidian.Platform.isDesktop) {
    window.queueMicrotask(() => bindDesktopGroupHover(view, line));
  }
}
function showMobileGroupPage(view, group, line) {
  const menu = new import_obsidian.Menu();
  menu.setUseNativeMenu(false);
  menu.addItem(
    (item) => item.setTitle("Back").setIcon("chevron-left").onClick(() => {
      showRootMenu(view, null);
    })
  );
  for (const option of group.options) {
    addConversionItem(menu, view, option, line, () => menu.hide());
  }
  showMenuAt(menu, view, null);
}
function bindDesktopGroupHover(view, line) {
  var _a, _b;
  const menuEl = latestMenuElement();
  if (!menuEl) return;
  for (const item of Array.from(menuEl.querySelectorAll(".menu-item"))) {
    if (item.dataset.dGroupHoverBound === "true") continue;
    const title = (_b = (_a = item.querySelector(".d-block-type-submenu-title-label")) == null ? void 0 : _a.textContent) == null ? void 0 : _b.trim();
    const group = NESTED_GROUPS.find((candidate) => candidate.label === title);
    if (!group) continue;
    item.dataset.dGroupHoverBound = "true";
    item.addEventListener("pointerenter", () => {
      openFlyout(view, group, item, line);
    });
    item.addEventListener("pointerleave", (event) => {
      const related = event.relatedTarget;
      if (related instanceof Node && (flyoutEl == null ? void 0 : flyoutEl.contains(related))) {
        cancelFlyoutClose();
        return;
      }
      scheduleFlyoutClose();
    });
  }
}
function openFlyout(view, group, trigger, line) {
  cancelFlyoutClose();
  if (flyoutEl && flyoutTrigger === trigger) return;
  disposeFlyout();
  const panel = activeWindow.createDiv();
  panel.className = `menu ${FLYOUT_CLASS}`;
  panel.setAttribute("role", "menu");
  for (const option of group.options) {
    panel.appendChild(createFlyoutItem(view, option, line));
  }
  panel.addEventListener("pointerenter", () => {
    cancelFlyoutClose();
  });
  panel.addEventListener("pointerleave", (event) => {
    const related = event.relatedTarget;
    if (related instanceof Node && (flyoutTrigger == null ? void 0 : flyoutTrigger.contains(related))) {
      cancelFlyoutClose();
      return;
    }
    scheduleFlyoutClose();
  });
  activeDocument.body.appendChild(panel);
  positionFlyout(panel, trigger);
  flyoutEl = panel;
  flyoutTrigger = trigger;
}
function createFlyoutItem(view, option, line) {
  const target = option.target;
  const row = activeWindow.createDiv();
  row.className = `menu-item ${FLYOUT_ITEM_CLASS}`;
  row.setAttribute("role", "menuitem");
  row.tabIndex = 0;
  const icon = activeWindow.createDiv();
  icon.className = "menu-item-icon";
  (0, import_obsidian.setIcon)(icon, option.icon);
  const title = activeWindow.createDiv();
  title.className = "menu-item-title";
  title.textContent = option.label;
  row.append(icon, title);
  const apply = (event) => {
    event.preventDefault();
    event.stopPropagation();
    if (!convertCurrentBlockType(view, target, line)) {
      new import_obsidian.Notice("Unable to change block type.");
      return;
    }
    disposeFlyout();
    rootMenu == null ? void 0 : rootMenu.hide();
  };
  row.addEventListener("pointerdown", apply);
  row.addEventListener("keydown", (event) => {
    if (event.key !== "Enter" && event.key !== " ") return;
    apply(event);
  });
  return row;
}
function positionFlyout(panel, trigger) {
  const rect = trigger.getBoundingClientRect();
  const width = panel.offsetWidth || 160;
  const height = panel.offsetHeight || 0;
  let x = rect.right + 4;
  let y = rect.top;
  if (x + width > activeWindow.innerWidth - 8) {
    x = Math.max(8, rect.left - width - 4);
  }
  if (y + height > activeWindow.innerHeight - 8) {
    y = Math.max(8, activeWindow.innerHeight - height - 8);
  }
  panel.setCssStyles({
    position: "fixed",
    left: `${x}px`,
    top: `${y}px`,
    zIndex: "10000"
  });
}
function scheduleFlyoutClose() {
  cancelFlyoutClose();
  flyoutCloseTimer = window.setTimeout(() => {
    disposeFlyout();
  }, 100);
}
function cancelFlyoutClose() {
  if (flyoutCloseTimer === null) return;
  window.clearTimeout(flyoutCloseTimer);
  flyoutCloseTimer = null;
}
function disposeFlyout() {
  cancelFlyoutClose();
  flyoutEl == null ? void 0 : flyoutEl.remove();
  flyoutEl = null;
  flyoutTrigger = null;
}
function latestMenuElement() {
  var _a;
  const menus = Array.from(activeDocument.querySelectorAll(".menu"));
  return (_a = menus[menus.length - 1]) != null ? _a : null;
}
function addConversionItem(menu, view, option, line, afterApply) {
  const target = option.target;
  menu.addItem(
    (item) => item.setTitle(option.label).setIcon(option.icon).onClick(() => {
      if (!convertCurrentBlockType(view, target, line)) {
        new import_obsidian.Notice("Unable to change block type.");
        return;
      }
      afterApply();
    })
  );
}
function addActionItem(menu, action) {
  menu.addItem((item) => {
    item.setTitle(action.label).setIcon(action.icon).onClick(() => {
      void (async () => {
        const ok = await action.run();
        if (!ok) {
          new import_obsidian.Notice(action.failureNotice);
          return;
        }
        menu.hide();
      })();
    });
    if (action.warning) item.setWarning(true);
  });
}
function createGroupTitle(labelText) {
  const fragment = activeWindow.createFragment();
  const title = activeWindow.createSpan();
  title.className = "d-block-type-submenu-title";
  const label = activeWindow.createSpan();
  label.className = "d-block-type-submenu-title-label";
  label.textContent = labelText;
  const chevron = activeWindow.createSpan();
  chevron.className = "d-block-type-submenu-title-chevron";
  chevron.setAttribute("aria-hidden", "true");
  (0, import_obsidian.setIcon)(chevron, "chevron-right");
  title.append(label, chevron);
  fragment.appendChild(title);
  return fragment;
}
function showMenuAt(menu, view, event) {
  let x = null;
  let y = null;
  if (event && typeof event.clientX === "number" && typeof event.clientY === "number") {
    x = event.clientX;
    y = event.clientY;
  } else {
    const coords = view.coordsAtPos(view.state.selection.main.head);
    if (coords) {
      x = coords.left;
      y = coords.bottom;
    }
  }
  if (x === null || y === null) {
    x = activeWindow.innerWidth / 2;
    y = activeWindow.innerHeight / 2;
  }
  menu.showAtPosition({ x, y });
}

// src/shared/dom-selectors.ts
var ROOT_EDITOR_CLASS = "d-root-editor";
var DRAGGING_BODY_CLASS = "d-dragging";
var MOBILE_GESTURE_LOCK_CLASS = "d-mobile-gesture-lock";
var DRAG_SOURCE_STYLE_ATTR = "data-d-drag-source-style";
var DRAG_SOURCE_HIGHLIGHT_ATTR = "data-d-drag-source-highlight";
var HANDLE_ICON_ATTR = "data-d-handle-icon";

// src/platform/codemirror/obsidian-dragger.ts
var LIST_INDENT_UNIT = 4;
function dragHandleExtension(plugin) {
  const options = {
    // tabSize is always read live from EditorState.tabSize by the adapter.
    config: {
      tabSize: 4,
      listIndentUnit: LIST_INDENT_UNIT
    },
    listIndentWidthPx: (view) => listIndentStepPx(view),
    handle: {
      render: () => createObsidianHandle(),
      side: plugin.settings.handleGutterPosition === "right" ? "after" : "before"
    },
    // Obsidian's Live Preview renders tables as HTML widgets; clicking a
    // cell opens a transient nested CM6 editor for that cell's text. The
    // dragger must stay dormant there (no handles, no drags, no gesture
    // interception): its document is just the cell text. The predicate is
    // evaluated per render/press because the nested editor is mounted
    // detached and only becomes identifiable once attached into the
    // table widget.
    enabled: isDraggerView,
    locate: (view) => ({
      sourceLineFromInput: (input) => {
        if (!plugin.isMobilePlatform() || !plugin.isMobileDragModeEnabled()) {
          return sourceLineFromInput(view, input);
        }
        const fromHandle = sourceLineFromInput(view, input);
        if (fromHandle !== null) return fromHandle;
        const event = input.native instanceof PointerEvent ? input.native : null;
        const target = (event == null ? void 0 : event.target) instanceof Element ? event.target : null;
        if (target && !view.dom.contains(target)) return null;
        return lineAtPoint(view, input.point);
      }
    }),
    ux: {
      gesture: () => gestureConfig(plugin),
      modules: [
        autoScroll(
          // Adapter port scrolls the .cm-scroller under the pointer;
          // activeDocument keeps pop-out windows working.
          scrollPort(() => activeDocument),
          () => ({
            edgeZonePx: plugin.settings.autoScrollEdgeZonePx,
            maxSpeedPx: plugin.settings.autoScrollMaxSpeedPx
          })
        )
      ]
    },
    onChange: (result) => {
      for (const item of result.outputs) {
        if (item.type === "dropped") plugin.notifyDragDrop();
      }
    }
  };
  return [
    import_view7.EditorView.editorAttributes.of({ class: ROOT_EDITOR_CLASS }),
    ...mdDragger(options),
    dropIndicatorPaint(options),
    selectionPaint(),
    handleHover(),
    gestureShell(plugin)
  ];
}
function listIndentStepPx(view) {
  const cs = getComputedStyle(view.contentDOM);
  const em = parseFloat(cs.getPropertyValue("--indent-unit")) * parseFloat(cs.getPropertyValue("--indent-size"));
  return em * parseFloat(cs.fontSize);
}
function createObsidianHandle() {
  const handle = activeWindow.createDiv();
  handle.className = HANDLE_CLASS;
  const core = activeWindow.createSpan();
  core.className = "d-handle-core";
  core.setAttribute("aria-hidden", "true");
  handle.appendChild(core);
  return handle;
}
function isDraggerView(view) {
  return view.dom.closest(".cm-table-widget") === null;
}
function gestureConfig(plugin) {
  const mobile = plugin.isMobilePlatform();
  return {
    dragArmMs: mobile ? plugin.settings.mobileDragLongPressMs : 0,
    multiSelectMs: plugin.settings.mouseRangeSelectLongPressMs,
    dragStartMoveThresholdPx: mobile ? 8 : 4,
    dragCancelMoveThresholdPx: Number.POSITIVE_INFINITY,
    multiSelectEnabled: plugin.settings.enableMultiLineSelection !== false
  };
}
function dropIndicatorPaint(options) {
  const dropIndicatorField = import_state6.StateField.define({
    create: () => import_view7.Decoration.none,
    update(deco, tr) {
      deco = deco.map(tr.changes);
      for (const effect of tr.effects) {
        if (effect.is(dragTransitionEffect)) {
          deco = dropSeamDecoration(effect.value.outputs, tr.state);
        }
      }
      return deco;
    },
    provide: (field) => import_view7.EditorView.decorations.from(field)
  });
  return [
    dropIndicatorField,
    import_view7.ViewPlugin.fromClass(
      class {
        constructor(view) {
          this.view = view;
          this.position = null;
        }
        update(update) {
          let seamMoved = false;
          for (const tr of update.transactions) {
            for (const effect of tr.effects) {
              if (effect.is(dragTransitionEffect)) {
                this.position = dropSeamState2(effect.value.outputs, update.state.doc).position;
                seamMoved = true;
              }
            }
          }
          if (seamMoved || update.geometryChanged) this.sync();
        }
        sync() {
          if (this.position === null) {
            this.removeSeamVars();
            return;
          }
          const offset = seamOffset(this.view, this.position, options);
          if (!offset) {
            this.removeSeamVars();
            return;
          }
          this.view.dom.style.setProperty("--d-seam-left", `${offset.left}px`);
          this.view.dom.style.setProperty("--d-seam-width", `${offset.width}px`);
        }
        removeSeamVars() {
          this.view.dom.style.removeProperty("--d-seam-left");
          this.view.dom.style.removeProperty("--d-seam-width");
        }
      }
    )
  ];
}
var dragSourceLinesField = import_state6.StateField.define({
  create: () => import_view7.Decoration.none,
  update(deco, tr) {
    deco = deco.map(tr.changes);
    for (const effect of tr.effects) {
      if (effect.is(dragTransitionEffect)) {
        deco = sourceHighlightDecoration(effect.value.outputs, tr.state);
      }
    }
    return deco;
  },
  provide: (field) => import_view7.EditorView.decorations.from(field)
});
function selectionPaint() {
  return [
    dragSourceLinesField,
    import_view7.ViewPlugin.fromClass(
      class {
        constructor(view) {
          this.view = view;
          this.selectedRanges = [];
          this.indentStepSet = false;
        }
        update(update) {
          for (const tr of update.transactions) {
            for (const effect of tr.effects) {
              if (effect.is(dragTransitionEffect)) {
                const outputs = effect.value.outputs;
                const sourceDoc = dragSelectionDoc2(outputs);
                if (sourceDoc !== null && sourceDoc !== update.state.doc) continue;
                const selection = selectionFromOutputs2(outputs);
                this.selectedRanges = selectionLineRanges2(
                  update.state.doc.lines,
                  selection != null ? selection : { blocks: [] }
                );
              }
            }
          }
          if (!this.indentStepSet || update.geometryChanged) {
            this.indentStepSet = true;
            this.view.dom.style.setProperty("--d-list-indent-step", `${listIndentStepPx(this.view)}px`);
          }
          this.syncSelectedHandles();
        }
        destroy() {
          this.selectedRanges = [];
          this.syncSelectedHandles();
        }
        syncSelectedHandles() {
          const handles = Array.from(this.view.dom.querySelectorAll(`.${HANDLE_CLASS}[data-block-start]`));
          for (const handle of handles) {
            const line = Number(handle.getAttribute("data-block-start"));
            handle.classList.toggle(
              "is-selected",
              Number.isInteger(line) && isLineNumberInRanges2(line, this.selectedRanges)
            );
          }
        }
      }
    )
  ];
}
function handleHover() {
  return import_view7.ViewPlugin.fromClass(
    class {
      constructor(view) {
        this.view = view;
        this.visible = null;
        this.onMove = (e) => {
          if (!isDraggerView(this.view)) {
            this.setVisible(null);
            return;
          }
          if (activeDocument.body.classList.contains(DRAGGING_BODY_CLASS)) {
            this.setVisible(null);
            return;
          }
          const line = lineAtPoint(this.view, { x: e.clientX, y: e.clientY });
          if (line === null) {
            this.setVisible(null);
            return;
          }
          const block = detectBlock2(this.view.state.doc, line, {
            tabSize: this.view.state.facet(import_state6.EditorState.tabSize)
          });
          if (!block) {
            this.setVisible(null);
            return;
          }
          const handle = this.view.dom.querySelector(
            `.${HANDLE_CLASS}[data-block-start="${block.lines.startLine}"]`
          );
          this.setVisible(handle);
        };
        this.onLeave = () => this.setVisible(null);
        this.view.dom.addEventListener("pointermove", this.onMove);
        this.view.dom.addEventListener("pointerleave", this.onLeave);
      }
      destroy() {
        this.view.dom.removeEventListener("pointermove", this.onMove);
        this.view.dom.removeEventListener("pointerleave", this.onLeave);
        this.setVisible(null);
      }
      setVisible(handle) {
        var _a;
        if (this.visible === handle) return;
        (_a = this.visible) == null ? void 0 : _a.classList.remove("is-visible");
        this.visible = handle;
        handle == null ? void 0 : handle.classList.add("is-visible");
      }
    }
  );
}
function gestureShell(plugin) {
  return import_view7.ViewPlugin.fromClass(
    class {
      constructor(view) {
        this.view = view;
        this.lastPress = null;
        this.locked = false;
        this.onPointerDown = (e) => {
          if (!isDraggerView(this.view)) return;
          this.lastPress = {
            event: e,
            onHandle: e.target instanceof Element && e.target.closest(`.${HANDLE_CLASS}`) !== null
          };
          if (plugin.isMobilePlatform() && plugin.isMobileDragModeEnabled()) {
            e.preventDefault();
          }
        };
        this.onTouchMove = (e) => {
          e.preventDefault();
        };
        this.onContextMenu = (e) => {
          if (!isDraggerView(this.view)) return;
          if (!plugin.isMobilePlatform() || !plugin.isMobileDragModeEnabled()) return;
          e.preventDefault();
        };
        this.view.dom.addEventListener("pointerdown", this.onPointerDown, true);
        this.view.dom.addEventListener("contextmenu", this.onContextMenu, true);
      }
      update(update) {
        for (const tr of update.transactions) {
          for (const effect of tr.effects) {
            if (effect.is(dragTransitionEffect)) this.consume(effect.value.outputs);
          }
        }
      }
      destroy() {
        this.setLock(false);
        activeDocument.body.classList.remove(DRAGGING_BODY_CLASS);
        this.view.dom.removeEventListener("pointerdown", this.onPointerDown, true);
        this.view.dom.removeEventListener("contextmenu", this.onContextMenu, true);
      }
      consume(outputs) {
        var _a, _b;
        for (const output of outputs) {
          if (output.type === "state_changed") {
            const t2 = output.state.type;
            this.setLock(t2 !== "idle");
            activeDocument.body.classList.toggle(DRAGGING_BODY_CLASS, t2 === "dragging");
          }
          if (output.type === "cancelled" && output.reason === "press_cancelled") {
            const press = this.lastPress;
            this.lastPress = null;
            const startLine = (_b = (_a = output.selection) == null ? void 0 : _a.blocks[0]) == null ? void 0 : _b.lines.startLine;
            if (press && press.onHandle && typeof startLine === "number") {
              const { clientX, clientY } = press.event;
              window.requestAnimationFrame(() => {
                openBlockTypeMenu(this.view, { clientX, clientY }, startLine);
              });
            }
          }
          if (output.type === "dropped" || output.type === "terminal") {
            this.lastPress = null;
          }
        }
      }
      setLock(locked) {
        if (this.locked === locked) return;
        this.locked = locked;
        activeDocument.body.classList.toggle(MOBILE_GESTURE_LOCK_CLASS, locked);
        if (locked) {
          activeDocument.addEventListener("touchmove", this.onTouchMove, { capture: true, passive: false });
        } else {
          activeDocument.removeEventListener("touchmove", this.onTouchMove, true);
        }
      }
    }
  );
}

// src/shared/constants.ts
var DEFAULT_HANDLE_SIZE_PX = 20;
var MIN_HANDLE_SIZE_PX = 10;
var MAX_HANDLE_SIZE_PX = 40;
var HANDLE_CORE_SIZE_RATIO = 0.5;
var GRIP_DOTS_CORE_SIZE_RATIO = 0.8;

// src/plugin/settings.ts
var import_obsidian2 = require("obsidian");

// src/plugin/i18n/index.ts
var obsidian = __toESM(require("obsidian"));

// src/plugin/i18n/en.ts
var en = {
  headingAppearance: "Appearance",
  headingBehavior: "Behavior",
  handleColor: "Handle color",
  handleColorDesc: "Follow theme accent or pick a custom color",
  optionTheme: "Theme",
  optionCustom: "Custom",
  handleVisibility: "Handle visibility",
  handleVisibilityDesc: "Control how drag handles are displayed",
  optionHover: "Hover",
  optionAlways: "Always",
  optionHidden: "Hidden",
  selectionVisualStyle: "Block selection visual style",
  selectionVisualStyleDesc: "Shared highlight style",
  optionBlockSelectionVisualOutline: "Outline only",
  optionBlockSelectionVisualSubtle: "Subtle highlight",
  optionBlockSelectionVisualFilled: "Filled highlight",
  enableBlockSelectionHighlight: "Block selection highlight",
  enableBlockSelectionHighlightDesc: "Highlight the block being dragged",
  handleIcon: "Handle icon",
  handleIconDesc: "Choose the icon style for drag handles",
  iconDot: "\u25CF dot",
  iconGripDots: "\u283F grip dots",
  iconGripLines: "\u2630 grip lines",
  iconSquare: "\u25A0 square",
  handleSize: "Handle size",
  handleSizeDesc: "Adjust the size of drag handles (px)",
  handleOffset: "Handle horizontal offset",
  handleOffsetDesc: "Negative = left, positive = right",
  handleGutterPosition: "Handle gutter side",
  handleGutterPositionDesc: "Show the handle gutter on the left or right side of the editor",
  optionLeft: "Left",
  optionRight: "Right",
  indicatorColor: "Indicator color",
  indicatorColorDesc: "Follow theme accent or pick a custom color",
  multiLineSelection: "Multi-line selection",
  multiLineSelectionDesc: "Disable to keep single-block drag only",
  mobileDragLongPressMs: "Mobile drag arm duration",
  mobileDragLongPressMsDesc: "On mobile (drag mode on), hold this long before a press can start a drag (ms)",
  mouseRangeSelectLongPressMs: "Multi-select long-press duration",
  mouseRangeSelectLongPressMsDesc: "Hold a handle (or a row in mobile drag mode) this long to enter multi-block selection (ms)",
  autoScrollEdgeZonePx: "Auto-scroll edge zone",
  autoScrollEdgeZonePxDesc: "Distance from viewport edge to trigger auto-scroll while dragging (px)",
  autoScrollMaxSpeedPx: "Auto-scroll max speed",
  autoScrollMaxSpeedPxDesc: "Maximum pixels scrolled per frame during auto-scroll",
  disableMobileDragModeAfterDrop: "Disable drag mode after move",
  disableMobileDragModeAfterDropDesc: "On mobile, automatically exit drag mode after a block is moved successfully",
  mobileTextLongPressDrag: "Mobile text long-press drag",
  mobileTextLongPressDragDesc: "On mobile, long-press a text line or rendered block content to drag the current block directly without using the left handle",
  optionMobileDragModeToggleViewAction: "View actions"
};

// src/plugin/i18n/zh-cn.ts
var zhCn = {
  // Headings
  headingAppearance: "\u5916\u89C2",
  headingBehavior: "\u884C\u4E3A",
  // Handle color
  handleColor: "\u624B\u67C4\u989C\u8272",
  handleColorDesc: "\u8DDF\u968F\u4E3B\u9898\u5F3A\u8C03\u8272\u6216\u81EA\u5B9A\u4E49\u989C\u8272",
  optionTheme: "\u8DDF\u968F\u4E3B\u9898\u8272",
  optionCustom: "\u81EA\u5B9A\u4E49",
  // Handle visibility
  handleVisibility: "\u624B\u67C4\u663E\u793A\u6A21\u5F0F",
  handleVisibilityDesc: "\u63A7\u5236\u62D6\u62FD\u624B\u67C4\u7684\u663E\u793A\u65B9\u5F0F",
  optionHover: "\u60AC\u505C\u663E\u793A",
  optionAlways: "\u59CB\u7EC8\u663E\u793A",
  optionHidden: "\u9690\u85CF",
  selectionVisualStyle: "\u62D6\u62FD\u6E90\u89C6\u89C9\u6837\u5F0F",
  selectionVisualStyleDesc: "\u7EDF\u4E00\u9AD8\u4EAE\u6837\u5F0F",
  optionBlockSelectionVisualOutline: "\u7EAF\u8FB9\u6846",
  optionBlockSelectionVisualSubtle: "\u7B80\u7EA6\u9AD8\u4EAE",
  optionBlockSelectionVisualFilled: "\u80CC\u666F\u589E\u5F3A",
  enableBlockSelectionHighlight: "\u62D6\u62FD\u6E90\u9AD8\u4EAE",
  enableBlockSelectionHighlightDesc: "\u9AD8\u4EAE\u88AB\u62D6\u52A8\u7684\u6E90\u5757",
  // Handle icon
  handleIcon: "\u624B\u67C4\u56FE\u6807",
  handleIconDesc: "\u9009\u62E9\u62D6\u62FD\u624B\u67C4\u7684\u56FE\u6807\u6837\u5F0F",
  iconDot: "\u25CF \u5706\u70B9",
  iconGripDots: "\u283F \u516D\u70B9\u6293\u624B",
  iconGripLines: "\u2630 \u4E09\u6A2A\u7EBF",
  iconSquare: "\u25A0 \u65B9\u5757",
  // Handle size
  handleSize: "\u624B\u67C4\u5927\u5C0F",
  handleSizeDesc: "\u8C03\u6574\u62D6\u62FD\u624B\u67C4\u7684\u5927\u5C0F\uFF08\u50CF\u7D20\uFF09",
  // Handle offset
  handleOffset: "\u624B\u67C4\u6A2A\u5411\u4F4D\u7F6E",
  handleOffsetDesc: "\u5411\u5DE6\u4E3A\u8D1F\u503C\uFF0C\u5411\u53F3\u4E3A\u6B63\u503C",
  handleGutterPosition: "\u624B\u67C4\u6240\u5728\u4FA7",
  handleGutterPositionDesc: "\u63A7\u5236\u624B\u67C4 gutter \u663E\u793A\u5728\u7F16\u8F91\u5668\u5DE6\u4FA7\u8FD8\u662F\u53F3\u4FA7",
  optionLeft: "\u5DE6\u4FA7",
  optionRight: "\u53F3\u4FA7",
  // Indicator color
  indicatorColor: "\u6307\u793A\u5668\u989C\u8272",
  indicatorColorDesc: "\u8DDF\u968F\u4E3B\u9898\u5F3A\u8C03\u8272\u6216\u81EA\u5B9A\u4E49\u989C\u8272",
  // Multi-line selection
  multiLineSelection: "\u591A\u884C\u9009\u53D6",
  multiLineSelectionDesc: "\u5173\u95ED\u540E\u4EC5\u4FDD\u7559\u5355\u5757\u62D6\u62FD\uFF0C\u4E0D\u8FDB\u5165\u591A\u884C\u9009\u53D6\u6D41\u7A0B",
  mobileDragLongPressMs: "\u79FB\u52A8\u7AEF\u62D6\u62FD\u5C31\u7EEA\u65F6\u957F",
  mobileDragLongPressMsDesc: "\u79FB\u52A8\u7AEF\uFF08\u62D6\u62FD\u6A21\u5F0F\u5F00\u542F\u65F6\uFF09\u6309\u4F4F\u591A\u4E45\u540E\u53EF\u4EE5\u5F00\u59CB\u62D6\u62FD\uFF08\u6BEB\u79D2\uFF09",
  mouseRangeSelectLongPressMs: "\u591A\u9009\u957F\u6309\u65F6\u957F",
  mouseRangeSelectLongPressMsDesc: "\u6309\u4F4F\u624B\u67C4\uFF08\u6216\u79FB\u52A8\u7AEF\u62D6\u62FD\u6A21\u5F0F\u4E0B\u7684\u884C\uFF09\u591A\u4E45\u540E\u8FDB\u5165\u591A\u5757\u9009\u62E9\uFF08\u6BEB\u79D2\uFF09",
  autoScrollEdgeZonePx: "\u81EA\u52A8\u6EDA\u52A8\u89E6\u53D1\u8DDD\u79BB",
  autoScrollEdgeZonePxDesc: "\u62D6\u62FD\u65F6\u6307\u9488\u8DDD\u79BB\u89C6\u53E3\u8FB9\u7F18\u591A\u5C11\u50CF\u7D20\u5F00\u59CB\u81EA\u52A8\u6EDA\u52A8",
  autoScrollMaxSpeedPx: "\u81EA\u52A8\u6EDA\u52A8\u6700\u5927\u901F\u5EA6",
  autoScrollMaxSpeedPxDesc: "\u81EA\u52A8\u6EDA\u52A8\u6BCF\u5E27\u6700\u5927\u6EDA\u52A8\u50CF\u7D20\u6570",
  disableMobileDragModeAfterDrop: "\u79FB\u52A8\u540E\u5173\u95ED\u62D6\u62FD\u6A21\u5F0F",
  disableMobileDragModeAfterDropDesc: "\u5F00\u542F\u540E\uFF0C\u79FB\u52A8\u7AEF\u6BCF\u6B21\u6210\u529F\u79FB\u52A8\u6587\u672C\u5757\u540E\u4F1A\u81EA\u52A8\u9000\u51FA\u62D6\u62FD\u6A21\u5F0F",
  mobileTextLongPressDrag: "\u79FB\u52A8\u7AEF\u6587\u672C\u957F\u6309\u62D6\u62FD",
  mobileTextLongPressDragDesc: "\u79FB\u52A8\u7AEF\u5728\u6587\u672C\u6574\u884C\u6216\u5757\u5185\u5BB9\u533A\u57DF\u957F\u6309\u53EF\u76F4\u63A5\u62D6\u62FD\u5F53\u524D\u5757\uFF0C\u65E0\u9700\u5DE6\u4FA7\u624B\u67C4",
  optionMobileDragModeToggleViewAction: "\u89C6\u56FE\u64CD\u4F5C\u680F"
};

// src/plugin/i18n/ru.ts
var ru = {
  headingAppearance: "\u0412\u043D\u0435\u0448\u043D\u0438\u0439 \u0432\u0438\u0434",
  headingBehavior: "\u041F\u043E\u0432\u0435\u0434\u0435\u043D\u0438\u0435",
  handleColor: "\u0426\u0432\u0435\u0442 \u043C\u0430\u0440\u043A\u0435\u0440\u0430",
  handleColorDesc: "\u0418\u0441\u043F\u043E\u043B\u044C\u0437\u043E\u0432\u0430\u0442\u044C \u0446\u0432\u0435\u0442 \u0430\u043A\u0446\u0435\u043D\u0442\u0430 \u0442\u0435\u043C\u044B \u0438\u043B\u0438 \u0432\u044B\u0431\u0440\u0430\u0442\u044C \u0441\u0432\u043E\u0439",
  optionTheme: "\u0422\u0435\u043C\u0430",
  optionCustom: "\u0421\u0432\u043E\u0439",
  handleVisibility: "\u0412\u0438\u0434\u0438\u043C\u043E\u0441\u0442\u044C \u043C\u0430\u0440\u043A\u0435\u0440\u0430",
  handleVisibilityDesc: "\u0423\u043F\u0440\u0430\u0432\u043B\u0435\u043D\u0438\u0435 \u043E\u0442\u043E\u0431\u0440\u0430\u0436\u0435\u043D\u0438\u0435\u043C \u043C\u0430\u0440\u043A\u0435\u0440\u043E\u0432 \u043F\u0435\u0440\u0435\u0442\u0430\u0441\u043A\u0438\u0432\u0430\u043D\u0438\u044F",
  optionHover: "\u041F\u0440\u0438 \u043D\u0430\u0432\u0435\u0434\u0435\u043D\u0438\u0438",
  optionAlways: "\u0412\u0441\u0435\u0433\u0434\u0430",
  optionHidden: "\u0421\u043A\u0440\u044B\u0442",
  selectionVisualStyle: "\u0421\u0442\u0438\u043B\u044C \u0432\u044B\u0434\u0435\u043B\u0435\u043D\u0438\u044F \u0431\u043B\u043E\u043A\u0430",
  selectionVisualStyleDesc: "\u041E\u0431\u0449\u0438\u0439 \u0441\u0442\u0438\u043B\u044C \u043F\u043E\u0434\u0441\u0432\u0435\u0442\u043A\u0438 \u043F\u0440\u0438 \u043F\u0435\u0440\u0435\u0442\u0430\u0441\u043A\u0438\u0432\u0430\u043D\u0438\u0438",
  optionBlockSelectionVisualOutline: "\u0422\u043E\u043B\u044C\u043A\u043E \u043A\u043E\u043D\u0442\u0443\u0440",
  optionBlockSelectionVisualSubtle: "\u041B\u0451\u0433\u043A\u0430\u044F \u043F\u043E\u0434\u0441\u0432\u0435\u0442\u043A\u0430",
  optionBlockSelectionVisualFilled: "\u0417\u0430\u043B\u0438\u0432\u043A\u0430",
  enableBlockSelectionHighlight: "\u041F\u043E\u0434\u0441\u0432\u0435\u0442\u043A\u0430 \u043F\u0435\u0440\u0435\u0442\u0430\u0441\u043A\u0438\u0432\u0430\u0435\u043C\u043E\u0433\u043E \u0431\u043B\u043E\u043A\u0430",
  enableBlockSelectionHighlightDesc: "\u041F\u043E\u0434\u0441\u0432\u0435\u0447\u0438\u0432\u0430\u0442\u044C \u0431\u043B\u043E\u043A \u0432\u043E \u0432\u0440\u0435\u043C\u044F \u043F\u0435\u0440\u0435\u0442\u0430\u0441\u043A\u0438\u0432\u0430\u043D\u0438\u044F",
  handleIcon: "\u0418\u043A\u043E\u043D\u043A\u0430 \u043C\u0430\u0440\u043A\u0435\u0440\u0430",
  handleIconDesc: "\u0412\u044B\u0431\u0435\u0440\u0438\u0442\u0435 \u0441\u0442\u0438\u043B\u044C \u0438\u043A\u043E\u043D\u043A\u0438 \u0434\u043B\u044F \u043C\u0430\u0440\u043A\u0435\u0440\u043E\u0432 \u043F\u0435\u0440\u0435\u0442\u0430\u0441\u043A\u0438\u0432\u0430\u043D\u0438\u044F",
  iconDot: "\u25CF \u0442\u043E\u0447\u043A\u0430",
  iconGripDots: "\u283F \u0442\u043E\u0447\u043A\u0438 \u0437\u0430\u0445\u0432\u0430\u0442\u0430",
  iconGripLines: "\u2630 \u043B\u0438\u043D\u0438\u0438 \u0437\u0430\u0445\u0432\u0430\u0442\u0430",
  iconSquare: "\u25A0 \u043A\u0432\u0430\u0434\u0440\u0430\u0442",
  handleSize: "\u0420\u0430\u0437\u043C\u0435\u0440 \u043C\u0430\u0440\u043A\u0435\u0440\u0430",
  handleSizeDesc: "\u041D\u0430\u0441\u0442\u0440\u043E\u0439\u043A\u0430 \u0440\u0430\u0437\u043C\u0435\u0440\u0430 \u043C\u0430\u0440\u043A\u0435\u0440\u043E\u0432 \u043F\u0435\u0440\u0435\u0442\u0430\u0441\u043A\u0438\u0432\u0430\u043D\u0438\u044F (px)",
  handleOffset: "\u0413\u043E\u0440\u0438\u0437\u043E\u043D\u0442\u0430\u043B\u044C\u043D\u043E\u0435 \u0441\u043C\u0435\u0449\u0435\u043D\u0438\u0435 \u043C\u0430\u0440\u043A\u0435\u0440\u0430",
  handleOffsetDesc: "\u041E\u0442\u0440\u0438\u0446\u0430\u0442\u0435\u043B\u044C\u043D\u043E\u0435 = \u0432\u043B\u0435\u0432\u043E, \u043F\u043E\u043B\u043E\u0436\u0438\u0442\u0435\u043B\u044C\u043D\u043E\u0435 = \u0432\u043F\u0440\u0430\u0432\u043E",
  handleGutterPosition: "\u0421\u0442\u043E\u0440\u043E\u043D\u0430 \u043C\u0430\u0440\u043A\u0435\u0440\u0430",
  handleGutterPositionDesc: "\u041F\u043E\u043A\u0430\u0437\u044B\u0432\u0430\u0442\u044C \u043C\u0430\u0440\u043A\u0435\u0440 \u0441\u043B\u0435\u0432\u0430 \u0438\u043B\u0438 \u0441\u043F\u0440\u0430\u0432\u0430 \u043E\u0442 \u0440\u0435\u0434\u0430\u043A\u0442\u043E\u0440\u0430",
  optionLeft: "\u0421\u043B\u0435\u0432\u0430",
  optionRight: "\u0421\u043F\u0440\u0430\u0432\u0430",
  indicatorColor: "\u0426\u0432\u0435\u0442 \u0438\u043D\u0434\u0438\u043A\u0430\u0442\u043E\u0440\u0430",
  indicatorColorDesc: "\u0418\u0441\u043F\u043E\u043B\u044C\u0437\u043E\u0432\u0430\u0442\u044C \u0446\u0432\u0435\u0442 \u0430\u043A\u0446\u0435\u043D\u0442\u0430 \u0442\u0435\u043C\u044B \u0438\u043B\u0438 \u0432\u044B\u0431\u0440\u0430\u0442\u044C \u0441\u0432\u043E\u0439",
  multiLineSelection: "\u0412\u044B\u0431\u043E\u0440 \u043D\u0435\u0441\u043A\u043E\u043B\u044C\u043A\u0438\u0445 \u0431\u043B\u043E\u043A\u043E\u0432",
  multiLineSelectionDesc: "\u041E\u0442\u043A\u043B\u044E\u0447\u0438\u0442\u0435, \u0447\u0442\u043E\u0431\u044B \u043F\u0435\u0440\u0435\u0442\u0430\u0441\u043A\u0438\u0432\u0430\u0442\u044C \u0442\u043E\u043B\u044C\u043A\u043E \u043E\u0434\u0438\u043D \u0431\u043B\u043E\u043A \u0437\u0430 \u0440\u0430\u0437",
  mobileDragLongPressMs: "\u0414\u043B\u0438\u0442\u0435\u043B\u044C\u043D\u043E\u0441\u0442\u044C \u0433\u043E\u0442\u043E\u0432\u043D\u043E\u0441\u0442\u0438 \u043A \u043F\u0435\u0440\u0435\u0442\u0430\u0441\u043A\u0438\u0432\u0430\u043D\u0438\u044E (\u043C\u043E\u0431.)",
  mobileDragLongPressMsDesc: "\u041D\u0430 \u043C\u043E\u0431\u0438\u043B\u044C\u043D\u043E\u043C (\u0440\u0435\u0436\u0438\u043C \u043F\u0435\u0440\u0435\u0442\u0430\u0441\u043A\u0438\u0432\u0430\u043D\u0438\u044F \u0432\u043A\u043B\u044E\u0447\u0451\u043D) \u0441\u043A\u043E\u043B\u044C\u043A\u043E \u0443\u0434\u0435\u0440\u0436\u0438\u0432\u0430\u0442\u044C \u043F\u0435\u0440\u0435\u0434 \u043D\u0430\u0447\u0430\u043B\u043E\u043C \u043F\u0435\u0440\u0435\u0442\u0430\u0441\u043A\u0438\u0432\u0430\u043D\u0438\u044F (\u043C\u0441)",
  mouseRangeSelectLongPressMs: "\u0414\u043B\u0438\u0442\u0435\u043B\u044C\u043D\u043E\u0441\u0442\u044C \u0434\u043E\u043B\u0433\u043E\u0433\u043E \u043D\u0430\u0436\u0430\u0442\u0438\u044F \u0434\u043B\u044F \u043C\u0443\u043B\u044C\u0442\u0438\u0432\u044B\u0431\u043E\u0440\u0430",
  mouseRangeSelectLongPressMsDesc: "\u0421\u043A\u043E\u043B\u044C\u043A\u043E \u0443\u0434\u0435\u0440\u0436\u0438\u0432\u0430\u0442\u044C \u043C\u0430\u0440\u043A\u0435\u0440 (\u0438\u043B\u0438 \u0441\u0442\u0440\u043E\u043A\u0443 \u0432 \u043C\u043E\u0431\u0438\u043B\u044C\u043D\u043E\u043C \u0440\u0435\u0436\u0438\u043C\u0435) \u043F\u0435\u0440\u0435\u0434 \u0432\u0445\u043E\u0434\u043E\u043C \u0432 \u0432\u044B\u0431\u043E\u0440 \u043D\u0435\u0441\u043A\u043E\u043B\u044C\u043A\u0438\u0445 \u0431\u043B\u043E\u043A\u043E\u0432 (\u043C\u0441)",
  autoScrollEdgeZonePx: "\u0417\u043E\u043D\u0430 \u0430\u0432\u0442\u043E\u043F\u0440\u043E\u043A\u0440\u0443\u0442\u043A\u0438",
  autoScrollEdgeZonePxDesc: "\u0420\u0430\u0441\u0441\u0442\u043E\u044F\u043D\u0438\u0435 \u043E\u0442 \u043A\u0440\u0430\u044F \u043E\u043A\u043D\u0430 \u0434\u043B\u044F \u0437\u0430\u043F\u0443\u0441\u043A\u0430 \u0430\u0432\u0442\u043E\u043F\u0440\u043E\u043A\u0440\u0443\u0442\u043A\u0438 \u043F\u0440\u0438 \u043F\u0435\u0440\u0435\u0442\u0430\u0441\u043A\u0438\u0432\u0430\u043D\u0438\u0438 (\u043F\u0438\u043A\u0441)",
  autoScrollMaxSpeedPx: "\u041C\u0430\u043A\u0441. \u0441\u043A\u043E\u0440\u043E\u0441\u0442\u044C \u0430\u0432\u0442\u043E\u043F\u0440\u043E\u043A\u0440\u0443\u0442\u043A\u0438",
  autoScrollMaxSpeedPxDesc: "\u041C\u0430\u043A\u0441\u0438\u043C\u0430\u043B\u044C\u043D\u043E\u0435 \u043A\u043E\u043B\u0438\u0447\u0435\u0441\u0442\u0432\u043E \u043F\u0438\u043A\u0441\u0435\u043B\u0435\u0439 \u043F\u0440\u043E\u043A\u0440\u0443\u0442\u043A\u0438 \u0437\u0430 \u043A\u0430\u0434\u0440",
  disableMobileDragModeAfterDrop: "\u041E\u0442\u043A\u043B\u044E\u0447\u0430\u0442\u044C \u0440\u0435\u0436\u0438\u043C \u043F\u0435\u0440\u0435\u0442\u0430\u0441\u043A\u0438\u0432\u0430\u043D\u0438\u044F \u043F\u043E\u0441\u043B\u0435 \u043F\u0435\u0440\u0435\u043C\u0435\u0449\u0435\u043D\u0438\u044F",
  disableMobileDragModeAfterDropDesc: "\u041D\u0430 \u043C\u043E\u0431\u0438\u043B\u044C\u043D\u043E\u043C \u0430\u0432\u0442\u043E\u043C\u0430\u0442\u0438\u0447\u0435\u0441\u043A\u0438 \u0432\u044B\u0445\u043E\u0434\u0438\u0442\u044C \u0438\u0437 \u0440\u0435\u0436\u0438\u043C\u0430 \u043F\u0435\u0440\u0435\u0442\u0430\u0441\u043A\u0438\u0432\u0430\u043D\u0438\u044F \u043F\u043E\u0441\u043B\u0435 \u0443\u0441\u043F\u0435\u0448\u043D\u043E\u0433\u043E \u043F\u0435\u0440\u0435\u043C\u0435\u0449\u0435\u043D\u0438\u044F \u0431\u043B\u043E\u043A\u0430",
  mobileTextLongPressDrag: "\u041F\u0435\u0440\u0435\u0442\u0430\u0441\u043A\u0438\u0432\u0430\u043D\u0438\u0435 \u0434\u043E\u043B\u0433\u0438\u043C \u043D\u0430\u0436\u0430\u0442\u0438\u0435\u043C \u043D\u0430 \u043C\u043E\u0431\u0438\u043B\u044C\u043D\u043E\u043C",
  mobileTextLongPressDragDesc: "\u041D\u0430 \u043C\u043E\u0431\u0438\u043B\u044C\u043D\u043E\u043C \u0443\u0434\u0435\u0440\u0436\u0438\u0432\u0430\u0439\u0442\u0435 \u0441\u0442\u0440\u043E\u043A\u0443 \u0442\u0435\u043A\u0441\u0442\u0430 \u0438\u043B\u0438 \u0431\u043B\u043E\u043A, \u0447\u0442\u043E\u0431\u044B \u043F\u0435\u0440\u0435\u0442\u0430\u0449\u0438\u0442\u044C \u0435\u0433\u043E \u043D\u0430\u043F\u0440\u044F\u043C\u0443\u044E \u0431\u0435\u0437 \u0438\u0441\u043F\u043E\u043B\u044C\u0437\u043E\u0432\u0430\u043D\u0438\u044F \u043C\u0430\u0440\u043A\u0435\u0440\u0430",
  optionMobileDragModeToggleViewAction: "\u041F\u0430\u043D\u0435\u043B\u044C \u0434\u0435\u0439\u0441\u0442\u0432\u0438\u0439"
};

// src/plugin/i18n/index.ts
var translationsByLocale = {
  en,
  ru,
  zh: zhCn,
  "zh-cn": zhCn,
  "zh-hk": zhCn,
  "zh-tw": zhCn
};
function detectLanguage(api) {
  const language = typeof api.getLanguage === "function" ? api.getLanguage() : api.moment.locale();
  return language.trim().toLowerCase();
}
function selectTranslations(language) {
  var _a, _b;
  const normalizedLanguage = language.trim().toLowerCase();
  return (_b = (_a = translationsByLocale[normalizedLanguage]) != null ? _a : translationsByLocale[normalizedLanguage.split("-")[0]]) != null ? _b : en;
}
function t() {
  return selectTranslations(detectLanguage(obsidian));
}

// src/plugin/settings-types.ts
var NUMERIC_SETTING_RANGES = {
  handleSize: { min: MIN_HANDLE_SIZE_PX, max: MAX_HANDLE_SIZE_PX, step: 2 },
  handleHorizontalOffsetPx: { min: -80, max: 80, step: 1 },
  mobileDragLongPressMs: { min: 50, max: 800, step: 10 },
  mouseRangeSelectLongPressMs: { min: 50, max: 2e3, step: 10 },
  autoScrollEdgeZonePx: { min: 20, max: 200, step: 4 },
  autoScrollMaxSpeedPx: { min: 4, max: 60, step: 2 }
};
var DEFAULT_SETTINGS = {
  handleColorMode: "theme",
  handleColor: "#8a8a8a",
  handleVisibility: "hover",
  handleIcon: "grip-dots",
  handleSize: DEFAULT_HANDLE_SIZE_PX,
  indicatorColorMode: "theme",
  indicatorColor: "#7a7a7a",
  enableMultiLineSelection: true,
  mobileDragLongPressMs: 200,
  // Slightly longer than drag-arm so multi-select is intentional, not accidental.
  mouseRangeSelectLongPressMs: 700,
  autoScrollEdgeZonePx: 60,
  autoScrollMaxSpeedPx: 12,
  disableMobileDragModeAfterDrop: true,
  enableMobileTextLongPressDrag: true,
  mobileDragModeToggleEnabled: true,
  enableBlockSelectionHighlight: true,
  selectionVisualStyle: "subtle",
  handleHorizontalOffsetPx: -8,
  handleGutterPosition: "left"
};

// src/plugin/settings.ts
var DragNDropSettingTab = class extends import_obsidian2.PluginSettingTab {
  constructor(app, plugin) {
    super(app, plugin);
    this.plugin = plugin;
  }
  getSettingDefinitions() {
    const i = t();
    const numeric = (key, range, name, desc) => ({
      name,
      desc,
      control: {
        type: "slider",
        key,
        min: range.min,
        max: range.max,
        step: range.step
      }
    });
    const mobileOnly = () => import_obsidian2.Platform.isMobile && this.plugin.settings.enableMobileTextLongPressDrag;
    return [
      {
        type: "page",
        name: i.headingAppearance,
        items: [
          {
            name: i.handleIcon,
            desc: i.handleIconDesc,
            control: {
              type: "dropdown",
              key: "handleIcon",
              options: {
                dot: i.iconDot,
                "grip-dots": i.iconGripDots,
                "grip-lines": i.iconGripLines,
                square: i.iconSquare
              }
            }
          },
          {
            name: i.handleColor,
            desc: i.handleColorDesc,
            control: {
              type: "dropdown",
              key: "handleColorMode",
              options: { theme: i.optionTheme, custom: i.optionCustom }
            }
          },
          {
            name: i.handleColor,
            desc: i.handleColorDesc,
            visible: () => this.plugin.settings.handleColorMode === "custom",
            control: { type: "color", key: "handleColor" }
          },
          numeric("handleSize", NUMERIC_SETTING_RANGES.handleSize, i.handleSize, i.handleSizeDesc),
          {
            name: i.handleVisibility,
            desc: i.handleVisibilityDesc,
            control: {
              type: "dropdown",
              key: "handleVisibility",
              options: { hover: i.optionHover, always: i.optionAlways, hidden: i.optionHidden }
            }
          },
          {
            name: i.handleGutterPosition,
            desc: i.handleGutterPositionDesc,
            control: {
              type: "dropdown",
              key: "handleGutterPosition",
              options: { left: i.optionLeft, right: i.optionRight }
            }
          },
          numeric(
            "handleHorizontalOffsetPx",
            NUMERIC_SETTING_RANGES.handleHorizontalOffsetPx,
            i.handleOffset,
            i.handleOffsetDesc
          ),
          {
            name: i.selectionVisualStyle,
            desc: i.selectionVisualStyleDesc,
            control: {
              type: "dropdown",
              key: "selectionVisualStyle",
              options: {
                outline: i.optionBlockSelectionVisualOutline,
                subtle: i.optionBlockSelectionVisualSubtle,
                filled: i.optionBlockSelectionVisualFilled
              }
            }
          },
          {
            name: i.enableBlockSelectionHighlight,
            desc: i.enableBlockSelectionHighlightDesc,
            control: { type: "toggle", key: "enableBlockSelectionHighlight" }
          },
          {
            name: i.indicatorColor,
            desc: i.indicatorColorDesc,
            control: {
              type: "dropdown",
              key: "indicatorColorMode",
              options: { theme: i.optionTheme, custom: i.optionCustom }
            }
          },
          {
            name: i.indicatorColor,
            desc: i.indicatorColorDesc,
            visible: () => this.plugin.settings.indicatorColorMode === "custom",
            control: { type: "color", key: "indicatorColor" }
          }
        ]
      },
      {
        type: "page",
        name: i.headingBehavior,
        items: [
          {
            name: i.multiLineSelection,
            desc: i.multiLineSelectionDesc,
            control: { type: "toggle", key: "enableMultiLineSelection" }
          },
          numeric(
            "mobileDragLongPressMs",
            NUMERIC_SETTING_RANGES.mobileDragLongPressMs,
            i.mobileDragLongPressMs,
            i.mobileDragLongPressMsDesc
          ),
          numeric(
            "mouseRangeSelectLongPressMs",
            NUMERIC_SETTING_RANGES.mouseRangeSelectLongPressMs,
            i.mouseRangeSelectLongPressMs,
            i.mouseRangeSelectLongPressMsDesc
          ),
          numeric(
            "autoScrollEdgeZonePx",
            NUMERIC_SETTING_RANGES.autoScrollEdgeZonePx,
            i.autoScrollEdgeZonePx,
            i.autoScrollEdgeZonePxDesc
          ),
          numeric(
            "autoScrollMaxSpeedPx",
            NUMERIC_SETTING_RANGES.autoScrollMaxSpeedPx,
            i.autoScrollMaxSpeedPx,
            i.autoScrollMaxSpeedPxDesc
          ),
          {
            name: i.mobileTextLongPressDrag,
            desc: i.mobileTextLongPressDragDesc,
            control: {
              type: "toggle",
              key: "enableMobileTextLongPressDrag",
              disabled: () => !import_obsidian2.Platform.isMobile
            }
          },
          {
            name: i.disableMobileDragModeAfterDrop,
            desc: i.disableMobileDragModeAfterDropDesc,
            visible: mobileOnly,
            control: { type: "toggle", key: "disableMobileDragModeAfterDrop" }
          },
          {
            name: i.optionMobileDragModeToggleViewAction,
            visible: mobileOnly,
            control: { type: "toggle", key: "mobileDragModeToggleEnabled" }
          }
        ]
      }
    ];
  }
  getControlValue(key) {
    return this.plugin.settings[key];
  }
  async setControlValue(key, value) {
    this.plugin.settings[key] = value;
    await this.plugin.saveSettings();
  }
};

// src/plugin/settings-migrations.ts
var SCHEMA_VERSION_KEY = "schemaVersion";
var CURRENT_SCHEMA_VERSION = 7;
var MIGRATIONS = [
  // v0 -> v1: consolidate legacy field shapes from versions before the
  // migration system existed.
  (data) => {
    const next = { ...data };
    if ("alwaysShowHandles" in next && !("handleVisibility" in next)) {
      next.handleVisibility = next.alwaysShowHandles ? "always" : "hover";
    }
    delete next.alwaysShowHandles;
    if (next.selectionVisualStyle === "none") {
      next.selectionVisualStyle = "outline";
      if (!("enableBlockSelectionHighlight" in next)) {
        next.enableBlockSelectionHighlight = false;
      }
    }
    delete next.requireMobileDragMode;
    return next;
  },
  // v1 -> v2: retune auto-scroll defaults. Existing installs persisted a
  // complete data.json on load, so legacy default values need an explicit
  // migration; custom values are left untouched.
  (data) => {
    const next = { ...data };
    if (next.autoScrollEdgeZonePx === 88) {
      next.autoScrollEdgeZonePx = DEFAULT_SETTINGS.autoScrollEdgeZonePx;
    }
    if (next.autoScrollMaxSpeedPx === 22) {
      next.autoScrollMaxSpeedPx = DEFAULT_SETTINGS.autoScrollMaxSpeedPx;
    }
    return next;
  },
  // v2 -> v3: reduce accidental desktop multi-select entry during normal
  // handle drag. Existing installs persisted the old default value, so only
  // migrate that value and preserve custom timings.
  (data) => {
    const next = { ...data };
    if (next.mouseRangeSelectLongPressMs === 260) {
      next.mouseRangeSelectLongPressMs = 500;
    }
    return next;
  },
  // v3 -> v4: remove the old cross-file drag toggle and its implementation.
  (data) => {
    const next = { ...data };
    delete next.enableCrossFileDrag;
    return next;
  },
  // v4 -> v5: drop unused multiLineSelectionLongPressMs. Multi-select timing
  // is mouseRangeSelectLongPressMs; mobile drag arm is mobileDragLongPressMs.
  (data) => {
    const next = { ...data };
    delete next.multiLineSelectionLongPressMs;
    return next;
  },
  // v5 -> v6: lengthen default multi-select hold so it is harder to enter by
  // accident after the mobile drag-arm threshold. Preserve custom values.
  (data) => {
    const next = { ...data };
    if (next.mouseRangeSelectLongPressMs === 500) {
      next.mouseRangeSelectLongPressMs = DEFAULT_SETTINGS.mouseRangeSelectLongPressMs;
    }
    return next;
  },
  // v6 -> v7: the single-value mobile drag mode toggle location collapsed to
  // a boolean. A persisted array is kept (any 'view-action' entry means on);
  // the legacy key is dropped in favour of mobileDragModeToggleEnabled. If
  // the legacy key is absent the default applies.
  (data) => {
    const next = { ...data };
    if ("mobileDragModeToggleLocations" in next) {
      const raw = next.mobileDragModeToggleLocations;
      next.mobileDragModeToggleEnabled = Array.isArray(raw) ? raw.includes("view-action") : Boolean(raw);
      delete next.mobileDragModeToggleLocations;
    }
    return next;
  }
];
function isRecord(value) {
  return typeof value === "object" && value !== null && !Array.isArray(value);
}
function clampNumericSettings(settings) {
  for (const key of Object.keys(NUMERIC_SETTING_RANGES)) {
    const { min, max } = NUMERIC_SETTING_RANGES[key];
    const raw = settings[key];
    settings[key] = Number.isFinite(raw) ? Math.round(Math.min(max, Math.max(min, raw))) : DEFAULT_SETTINGS[key];
  }
}
function migrateSettings(saved) {
  const raw = isRecord(saved) ? { ...saved } : {};
  const storedVersion = raw[SCHEMA_VERSION_KEY];
  const fromVersion = typeof storedVersion === "number" ? storedVersion : 0;
  let data = raw;
  for (let v = fromVersion; v < CURRENT_SCHEMA_VERSION; v++) {
    data = MIGRATIONS[v](data);
  }
  const merged = {
    ...DEFAULT_SETTINGS,
    ...data,
    [SCHEMA_VERSION_KEY]: CURRENT_SCHEMA_VERSION
  };
  clampNumericSettings(merged);
  return merged;
}

// src/plugin/mobile-toolbar-commands.ts
var import_obsidian3 = require("obsidian");

// src/platform/obsidian/views.ts
function getActiveMarkdownView(app) {
  var _a, _b;
  const leaf = (_a = app.workspace.getMostRecentLeaf()) != null ? _a : null;
  if (!leaf) return null;
  const view = leaf.view;
  return ((_b = view.getViewType) == null ? void 0 : _b.call(view)) === "markdown" ? view : null;
}
function getCodeMirrorView(markdownView) {
  var _a;
  const maybeView = (_a = markdownView.editor) == null ? void 0 : _a.cm;
  return maybeView != null ? maybeView : null;
}

// src/plugin/mobile-toolbar-commands.ts
function registerMobileToolbarCommands(plugin) {
  plugin.addCommand({
    id: "open-current-block-type-menu",
    name: "Change current block type",
    icon: "replace",
    mobileOnly: true,
    checkCallback: (checking) => {
      if (!import_obsidian3.Platform.isMobile) return false;
      const markdownView = getActiveMarkdownView(plugin.app);
      if (!markdownView) return false;
      const view = getCodeMirrorView(markdownView);
      if (!view) return false;
      if (!checking) {
        openBlockTypeMenu(view, null);
      }
      return true;
    }
  });
  plugin.addCommand({
    id: "toggle-mobile-drag-mode",
    name: "Toggle mobile drag mode",
    icon: "hand",
    mobileOnly: true,
    checkCallback: (checking) => {
      if (!import_obsidian3.Platform.isMobile) return false;
      if (!checking) {
        plugin.toggleMobileDragMode();
      }
      return true;
    }
  });
}

// src/plugin/main.ts
var DragNDropPlugin2 = class extends import_obsidian4.Plugin {
  constructor() {
    super(...arguments);
    this.mobileDragModeActionByView = /* @__PURE__ */ new WeakMap();
    this.mobileDragModeActionEls = /* @__PURE__ */ new Set();
    this.mobileDragModeEnabled = false;
    // Suppress native caret/text selection while mobile drag mode is on.
    // Scroll/pan is NOT locked for the whole mode — only during active gesture
    // (see mobile gesture lock class driven by state_changed).
    this.onSelectStartWhileDragMode = (event) => {
      if (!this.mobileDragModeEnabled) return;
      event.preventDefault();
    };
    this.onSelectionChangeWhileDragMode = () => {
      if (!this.mobileDragModeEnabled) return;
      this.clearNativeSelection();
    };
  }
  async onload() {
    await this.loadSettings();
    this.registerEditorExtension(dragHandleExtension(this));
    registerMobileToolbarCommands(this);
    this.app.workspace.onLayoutReady(() => this.registerMobileDragModeActions());
    this.registerEvent(this.app.workspace.on("layout-change", () => this.registerMobileDragModeActions()));
    this.registerEvent(this.app.workspace.on("active-leaf-change", () => this.registerMobileDragModeActions()));
    this.registerEvent(this.app.workspace.on("file-open", () => this.registerMobileDragModeActions()));
    this.addSettingTab(new DragNDropSettingTab(this.app, this));
  }
  onunload() {
    this.setMobileDragModeEnabled(false);
    for (const actionEl of this.mobileDragModeActionEls) {
      actionEl.remove();
    }
    this.mobileDragModeActionEls.clear();
  }
  async loadSettings() {
    this.settings = migrateSettings(await this.loadData());
    await this.saveData(this.settings);
    this.applySettings();
  }
  async saveSettings() {
    this.applySettings();
    await this.saveData(this.settings);
  }
  applySettings() {
    const body = activeDocument.body;
    if (!this.settings.enableMobileTextLongPressDrag) {
      this.mobileDragModeEnabled = false;
    }
    const visibility = this.settings.handleVisibility;
    body.classList.toggle("d-handles-always", visibility === "always");
    body.classList.toggle("d-handles-hidden", visibility === "hidden");
    body.classList.toggle(
      "d-mobile-handles-hidden",
      import_obsidian4.Platform.isMobile && !this.settings.enableMobileTextLongPressDrag
    );
    body.classList.toggle("d-mobile-drag-mode-enabled", this.mobileDragModeEnabled);
    const selectionVisualStyle = this.settings.selectionVisualStyle;
    body.setAttribute(DRAG_SOURCE_STYLE_ATTR, selectionVisualStyle);
    body.setAttribute(DRAG_SOURCE_HIGHLIGHT_ATTR, this.settings.enableBlockSelectionHighlight ? "on" : "off");
    const handleOffset = this.settings.handleHorizontalOffsetPx;
    const effectiveOffset = this.settings.handleGutterPosition === "right" ? -handleOffset : handleOffset;
    body.setCssProps({
      "--d-handle-horizontal-offset-px": `${effectiveOffset}px`
    });
    let colorValue = "";
    if (this.settings.handleColorMode === "theme") {
      colorValue = "var(--interactive-accent)";
    } else if (this.settings.handleColor) {
      colorValue = this.settings.handleColor;
    }
    if (colorValue) {
      body.setCssProps({
        "--d-handle-color": colorValue,
        "--d-handle-color-hover": colorValue
      });
    } else {
      body.setCssProps({
        "--d-handle-color": "",
        "--d-handle-color-hover": ""
      });
    }
    let indicatorColorValue = "";
    if (this.settings.indicatorColorMode === "custom" && this.settings.indicatorColor) {
      indicatorColorValue = this.settings.indicatorColor;
    }
    if (indicatorColorValue) {
      body.setCssProps({
        "--d-drop-indicator-color": indicatorColorValue
      });
    } else {
      body.style.removeProperty("--d-drop-indicator-color");
    }
    const handleSize = this.settings.handleSize;
    body.setCssProps({
      "--d-handle-size": `${handleSize}px`,
      "--d-handle-core-size": `${Math.round(handleSize * HANDLE_CORE_SIZE_RATIO)}px`,
      "--d-grip-dots-core-size": `${Math.round(handleSize * GRIP_DOTS_CORE_SIZE_RATIO)}px`
    });
    body.setAttribute(HANDLE_ICON_ATTR, this.settings.handleIcon);
    this.syncMobileDragModeActionVisibility();
  }
  // Called when a drop commits. Drives mobile-mode auto-disable.
  notifyDragDrop() {
    if (!import_obsidian4.Platform.isMobile) return;
    if (this.settings.disableMobileDragModeAfterDrop === false) return;
    this.setMobileDragModeEnabled(false);
  }
  isMobileDragModeEnabled() {
    return this.mobileDragModeEnabled;
  }
  isMobilePlatform() {
    return import_obsidian4.Platform.isMobile;
  }
  toggleMobileDragMode() {
    if (!this.settings.enableMobileTextLongPressDrag) {
      this.setMobileDragModeEnabled(false);
      return false;
    }
    this.setMobileDragModeEnabled(!this.mobileDragModeEnabled);
    return this.mobileDragModeEnabled;
  }
  setMobileDragModeEnabled(enabled) {
    if (this.mobileDragModeEnabled === enabled) return;
    this.mobileDragModeEnabled = enabled;
    if (enabled) {
      this.dismissActiveMobileInput();
      this.installMobileSelectionLock();
    } else {
      this.removeMobileSelectionLock();
    }
    this.applySettings();
    this.syncMobileDragModeActionIcons();
  }
  installMobileSelectionLock() {
    if (!import_obsidian4.Platform.isMobile) return;
    activeDocument.addEventListener("selectstart", this.onSelectStartWhileDragMode, true);
    activeDocument.addEventListener("selectionchange", this.onSelectionChangeWhileDragMode, true);
    this.clearNativeSelection();
  }
  removeMobileSelectionLock() {
    activeDocument.removeEventListener("selectstart", this.onSelectStartWhileDragMode, true);
    activeDocument.removeEventListener("selectionchange", this.onSelectionChangeWhileDragMode, true);
  }
  clearNativeSelection() {
    var _a, _b, _c;
    try {
      const selection = (_c = (_a = activeWindow.getSelection) == null ? void 0 : _a.call(activeWindow)) != null ? _c : (_b = window.getSelection) == null ? void 0 : _b.call(window);
      if (selection && selection.rangeCount > 0) selection.removeAllRanges();
    } catch (e) {
    }
  }
  dismissActiveMobileInput() {
    if (!import_obsidian4.Platform.isMobile) return;
    const win = activeWindow;
    const active = activeDocument.activeElement;
    if (!(active instanceof win.HTMLElement)) return;
    const shouldBlur = active.instanceOf(win.HTMLInputElement) || active.instanceOf(win.HTMLTextAreaElement) || active.isContentEditable || !!active.closest(".cm-content");
    if (!shouldBlur) return;
    active.blur();
    this.clearNativeSelection();
  }
  registerMobileDragModeActions() {
    if (!import_obsidian4.Platform.isMobile) return;
    if (!this.isMobileDragModeToggleEnabled()) {
      this.removeMobileDragModeActions();
      return;
    }
    for (const leaf of this.app.workspace.getLeavesOfType("markdown")) {
      const view = leaf.view;
      if (!(view instanceof import_obsidian4.MarkdownView)) continue;
      const existingActionEl = this.mobileDragModeActionByView.get(view);
      if (existingActionEl == null ? void 0 : existingActionEl.isConnected) continue;
      if (existingActionEl) {
        this.mobileDragModeActionEls.delete(existingActionEl);
      }
      const actionEl = view.addAction(
        this.getMobileDragModeActionIcon(),
        this.getMobileDragModeActionTitle(),
        (event) => {
          event.preventDefault();
          event.stopPropagation();
          this.toggleMobileDragMode();
        }
      );
      this.mobileDragModeActionByView.set(view, actionEl);
      this.mobileDragModeActionEls.add(actionEl);
      this.syncMobileDragModeActionEl(actionEl);
    }
  }
  syncMobileDragModeActionVisibility() {
    if (!import_obsidian4.Platform.isMobile) return;
    if (!this.isMobileDragModeToggleEnabled()) {
      this.removeMobileDragModeActions();
      return;
    }
    this.registerMobileDragModeActions();
  }
  removeMobileDragModeActions() {
    for (const actionEl of Array.from(this.mobileDragModeActionEls)) {
      actionEl.remove();
    }
    this.mobileDragModeActionEls.clear();
    this.mobileDragModeActionByView = /* @__PURE__ */ new WeakMap();
  }
  syncMobileDragModeActionIcons() {
    for (const actionEl of Array.from(this.mobileDragModeActionEls)) {
      if (!actionEl.isConnected) {
        this.mobileDragModeActionEls.delete(actionEl);
        continue;
      }
      this.syncMobileDragModeActionEl(actionEl);
    }
  }
  syncMobileDragModeActionEl(actionEl) {
    const title = this.getMobileDragModeActionTitle();
    (0, import_obsidian4.setIcon)(actionEl, this.getMobileDragModeActionIcon());
    actionEl.setAttribute("aria-label", title);
    actionEl.setAttribute("aria-pressed", String(this.mobileDragModeEnabled));
    actionEl.setAttribute("title", title);
  }
  getMobileDragModeActionIcon() {
    return this.mobileDragModeEnabled ? "check" : "hand";
  }
  getMobileDragModeActionTitle() {
    return this.mobileDragModeEnabled ? "Drag mode enabled" : "Drag mode disabled";
  }
  isMobileDragModeToggleEnabled() {
    return this.settings.enableMobileTextLongPressDrag && this.settings.mobileDragModeToggleEnabled;
  }
};

/* nosourcemap */