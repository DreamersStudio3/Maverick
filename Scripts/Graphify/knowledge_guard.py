#!/usr/bin/env python3
"""Create and verify Maverick's pre-push Graphify wrap-up attestation."""

from __future__ import annotations

import argparse
import hashlib
import importlib.metadata
import json
import re
import subprocess
import sys
from functools import lru_cache
from datetime import datetime, timezone
from pathlib import Path
from typing import Iterable, Sequence
from urllib.parse import unquote


SCHEMA = "maverick.graphify-wrap-up/v1"
EXPORT_SCHEMA = "maverick.graphify-export/v1"
STAMP_PATH = "graphify-out/wrap-up.json"
EXPORT_PROVENANCE_PATH = "graphify-out/export-provenance.json"
ZERO_OID = re.compile(r"^0+$")

SOURCE_EXCLUDED_PREFIXES = (
    "TODO/",
    "graphify-out/",
)

SPECIAL_SOURCE_PATHS = {
    ".gitattributes",
    ".gitignore",
    ".graphifyignore",
}

PROJECT_SOURCE_SUFFIXES = {
    ".csv",
    ".ini",
    ".toml",
    ".tsv",
    ".uplugin",
    ".uproject",
}

REQUIRED_ARTIFACTS = {
    "graphify-out/.graphify_labels.json",
    "graphify-out/.graphify_labels.json.sig",
    "graphify-out/GRAPH_REPORT.md",
    EXPORT_PROVENANCE_PATH,
    "graphify-out/graph.html",
    "graphify-out/graph.json",
    "graphify-out/manifest.json",
    "graphify-out/obsidian/.graphify_obsidian_manifest.json",
    "graphify-out/obsidian/graph.canvas",
    "graphify-out/wiki/index.md",
}


class GuardError(RuntimeError):
    """A user-actionable wrap-up validation failure."""


def _git(
    root: Path,
    *args: str,
    input_bytes: bytes | None = None,
    check: bool = True,
) -> bytes:
    result = subprocess.run(
        ["git", *args],
        cwd=root,
        input=input_bytes,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if check and result.returncode != 0:
        detail = result.stderr.decode("utf-8", errors="replace").strip()
        raise GuardError(f"git {' '.join(args)} 실패: {detail}")
    return result.stdout


def find_repo_root() -> Path:
    result = subprocess.run(
        ["git", "rev-parse", "--show-toplevel"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if result.returncode != 0:
        raise GuardError("Git 저장소 안에서 실행해야 합니다.")
    return Path(result.stdout.decode("utf-8", errors="replace").strip()).resolve()


def normalize_path(path: str) -> str:
    return path.replace("\\", "/").removeprefix("./")


@lru_cache(maxsize=1)
def graphify_supported_suffixes() -> frozenset[str]:
    try:
        from graphify.detect import (
            CODE_EXTENSIONS,
            DOC_EXTENSIONS,
            GOOGLE_WORKSPACE_EXTENSIONS,
            IMAGE_EXTENSIONS,
            OFFICE_EXTENSIONS,
            PAPER_EXTENSIONS,
            VIDEO_EXTENSIONS,
        )
    except ImportError as exc:
        raise GuardError("Graphify Python 패키지를 찾을 수 없습니다.") from exc
    return frozenset(
        ext.lower()
        for ext in (
            CODE_EXTENSIONS
            | DOC_EXTENSIONS
            | GOOGLE_WORKSPACE_EXTENSIONS
            | IMAGE_EXTENSIONS
            | OFFICE_EXTENSIONS
            | PAPER_EXTENSIONS
            | VIDEO_EXTENSIONS
        )
    )


def is_source_path(path: str) -> bool:
    normalized = normalize_path(path)
    if normalized.startswith(SOURCE_EXCLUDED_PREFIXES):
        return False
    if normalized in SPECIAL_SOURCE_PATHS:
        return True
    if normalized.startswith("Scripts/Graphify/"):
        return "__pycache__" not in normalized
    suffix = Path(normalized).suffix.lower()
    return suffix in graphify_supported_suffixes() or suffix in PROJECT_SOURCE_SUFFIXES


def is_artifact_path(path: str) -> bool:
    normalized = normalize_path(path)
    if normalized in REQUIRED_ARTIFACTS:
        return True
    if normalized.startswith("graphify-out/wiki/"):
        return True
    if not normalized.startswith("graphify-out/obsidian/"):
        return False

    # Obsidian writes per-user UI state into the vault when it opens. Only the
    # files Graphify owns are wrap-up artifacts; workspace/app state must remain
    # local and must not make a valid push fail.
    return normalized.endswith(".md") or normalized == (
        "graphify-out/obsidian/.obsidian/graph.json"
    )


def _parse_index_entries(raw: bytes) -> list[tuple[str, str, str]]:
    entries: list[tuple[str, str, str]] = []
    for record in raw.split(b"\0"):
        if not record:
            continue
        metadata, path_bytes = record.split(b"\t", 1)
        mode, oid, stage = metadata.decode("ascii").split()
        if stage != "0":
            raise GuardError("병합 충돌이 남아 있어 wrap-up stamp를 만들 수 없습니다.")
        path = path_bytes.decode("utf-8", errors="surrogateescape")
        entries.append((normalize_path(path), mode, oid))
    return entries


def index_entries(root: Path) -> list[tuple[str, str, str]]:
    return _parse_index_entries(_git(root, "ls-files", "-s", "-z"))


def commit_entries(root: Path, commit: str) -> list[tuple[str, str, str]]:
    raw = _git(root, "ls-tree", "-r", "-z", "--full-tree", commit)
    entries: list[tuple[str, str, str]] = []
    for record in raw.split(b"\0"):
        if not record:
            continue
        metadata, path_bytes = record.split(b"\t", 1)
        mode, object_type, oid = metadata.decode("ascii").split()
        if object_type != "blob":
            continue
        path = path_bytes.decode("utf-8", errors="surrogateescape")
        entries.append((normalize_path(path), mode, oid))
    return entries


def fingerprint(
    entries: Iterable[tuple[str, str, str]], predicate
) -> tuple[str, int]:
    selected = sorted(entry for entry in entries if predicate(entry[0]))
    digest = hashlib.sha256()
    for path, mode, oid in selected:
        digest.update(path.encode("utf-8", errors="surrogateescape"))
        digest.update(b"\0")
        digest.update(mode.encode("ascii"))
        digest.update(b"\0")
        digest.update(oid.encode("ascii"))
        digest.update(b"\n")
    return digest.hexdigest(), len(selected)


def _nul_paths(raw: bytes) -> list[str]:
    return [
        normalize_path(part.decode("utf-8", errors="surrogateescape"))
        for part in raw.split(b"\0")
        if part
    ]


def scan_graphify_corpus(root: Path) -> dict[str, str]:
    try:
        from graphify.detect import detect
    except ImportError as exc:
        raise GuardError("Graphify Python 패키지를 찾을 수 없습니다.") from exc

    result = detect(root, cache_root=root / "graphify-out/cache")
    if result.get("walk_errors"):
        raise GuardError(
            "Graphify corpus scan 중 읽지 못한 경로가 있습니다: "
            + ", ".join(result["walk_errors"][:5])
        )

    corpus: dict[str, str] = {}
    for file_type, paths in result.get("files", {}).items():
        for raw_path in paths:
            path = Path(raw_path)
            try:
                relative = path.resolve().relative_to(root.resolve()).as_posix()
            except (OSError, ValueError) as exc:
                raise GuardError(f"corpus 경로가 저장소 밖을 가리킵니다: {path}") from exc
            corpus[normalize_path(relative)] = file_type
    return corpus


def ensure_relevant_files_are_staged(root: Path, corpus: dict[str, str]) -> None:
    unstaged = _nul_paths(_git(root, "diff", "--name-only", "-z"))
    untracked = _nul_paths(
        _git(root, "ls-files", "--others", "--exclude-standard", "-z")
    )
    blocking = {
        path for path in unstaged if is_source_path(path) or is_artifact_path(path)
    }
    blocking.update(
        path
        for path in untracked
        if path in corpus
        or path in SPECIAL_SOURCE_PATHS
        or path.startswith("Scripts/Graphify/")
        or is_artifact_path(path)
    )
    blocking = sorted(blocking)
    if blocking:
        preview = "\n  - ".join(blocking[:20])
        extra = "" if len(blocking) <= 20 else f"\n  ... 외 {len(blocking) - 20}개"
        raise GuardError(
            "wrap-up 대상 파일을 먼저 stage해야 합니다.\n"
            f"  - {preview}{extra}\n"
            "위키·코드와 생성 산출물을 stage한 뒤 stamp를 다시 실행하세요."
        )


def _md5(path: Path) -> str:
    digest = hashlib.md5(usedforsecurity=False)
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def content_tree_fingerprint(root: Path) -> tuple[str, int]:
    files = sorted(path for path in root.rglob("*") if path.is_file())
    digest = hashlib.sha256()
    for path in files:
        relative = path.relative_to(root).as_posix()
        digest.update(relative.encode("utf-8", errors="surrogateescape"))
        digest.update(b"\0")
        digest.update(_sha256(path).encode("ascii"))
        digest.update(b"\n")
    return digest.hexdigest(), len(files)


def selected_tree_fingerprint(root: Path, relative_paths: Iterable[str]) -> tuple[str, int]:
    normalized_paths = sorted({normalize_path(path) for path in relative_paths})
    digest = hashlib.sha256()
    for relative in normalized_paths:
        path = root / Path(relative)
        if not path.is_file():
            raise GuardError(f"Graphify export 파일이 없습니다: {path}")
        digest.update(relative.encode("utf-8", errors="surrogateescape"))
        digest.update(b"\0")
        digest.update(_sha256(path).encode("ascii"))
        digest.update(b"\n")
    return digest.hexdigest(), len(normalized_paths)


def obsidian_owned_paths(root: Path) -> set[str]:
    manifest_path = root / "graphify-out/obsidian/.graphify_obsidian_manifest.json"
    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise GuardError(f"Obsidian manifest를 읽을 수 없습니다: {exc}") from exc

    raw_files = manifest.get("files")
    if not isinstance(raw_files, list):
        raise GuardError("Obsidian manifest의 files가 배열이 아닙니다.")

    owned: set[str] = set()
    for raw_path in raw_files:
        if not isinstance(raw_path, str):
            raise GuardError("Obsidian manifest에 문자열이 아닌 파일 경로가 있습니다.")
        normalized = normalize_path(raw_path)
        parts = Path(normalized).parts
        if (
            not normalized
            or normalized.startswith("/")
            or re.match(r"^[A-Za-z]:/", normalized)
            or ".." in parts
        ):
            raise GuardError(f"Obsidian manifest 경로가 vault 밖을 가리킵니다: {raw_path}")
        owned.add(normalized)
    return owned


def validate_manifest(root: Path, corpus: dict[str, str]) -> int:
    manifest_path = root / "graphify-out/manifest.json"
    if not manifest_path.is_file():
        raise GuardError("graphify-out/manifest.json이 없습니다.")
    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise GuardError(f"Graphify manifest를 읽을 수 없습니다: {exc}") from exc

    stale: list[str] = []
    semantic_stale: list[str] = []
    for raw_path, metadata in manifest.items():
        rel_path = normalize_path(raw_path)
        path = root / Path(rel_path)
        if not path.is_file():
            stale.append(rel_path)
            continue
        content_hash = _md5(path)
        if metadata.get("ast_hash") != content_hash:
            stale.append(rel_path)
        if corpus.get(rel_path) not in (None, "code"):
            if metadata.get("semantic_hash") != content_hash:
                semantic_stale.append(rel_path)

    manifest_paths = {normalize_path(path) for path in manifest}
    missing_corpus = sorted(set(corpus).difference(manifest_paths))
    excluded_but_indexed = sorted(manifest_paths.difference(corpus))

    if stale:
        preview = ", ".join(sorted(stale)[:10])
        raise GuardError(
            f"Graphify manifest와 현재 파일이 다릅니다: {preview}. "
            "$graphify . --update를 다시 실행하세요."
        )
    if missing_corpus:
        preview = ", ".join(missing_corpus[:10])
        raise GuardError(
            f"Graphify manifest에 corpus 파일이 없습니다: {preview}. "
            "$graphify . --update를 다시 실행하세요."
        )
    if excluded_but_indexed:
        preview = ", ".join(excluded_but_indexed[:10])
        raise GuardError(
            f"Graphify manifest에 현재 corpus에서 제외된 파일이 남았습니다: {preview}. "
            "제외 범위를 반영한 full/force update를 실행하세요."
        )
    if semantic_stale:
        preview = ", ".join(sorted(semantic_stale)[:10])
        raise GuardError(
            f"의미 추출이 최신이 아닌 문서·미디어가 있습니다: {preview}. "
            "Codex의 Graphify 스킬로 semantic update를 완료하세요."
        )
    return sum(file_type != "code" for file_type in corpus.values())


def validate_artifacts(root: Path, entries: Sequence[tuple[str, str, str]]) -> dict:
    indexed_paths = {path for path, _, _ in entries}
    missing = sorted(REQUIRED_ARTIFACTS.difference(indexed_paths))
    if missing:
        raise GuardError(f"필수 Graphify 산출물이 Git index에 없습니다: {', '.join(missing)}")

    graph_path = root / "graphify-out/graph.json"
    try:
        graph = json.loads(graph_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise GuardError(f"graph.json을 읽을 수 없습니다: {exc}") from exc

    nodes = graph.get("nodes", [])
    links = graph.get("links", graph.get("edges", []))
    communities = {
        node.get("community")
        for node in nodes
        if isinstance(node, dict) and node.get("community") is not None
    }

    obsidian_root = root / "graphify-out/obsidian"
    try:
        canvas = json.loads((obsidian_root / "graph.canvas").read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise GuardError(f"Obsidian export를 읽을 수 없습니다: {exc}") from exc

    declared_obsidian = obsidian_owned_paths(root)
    missing_obsidian = sorted(
        path for path in declared_obsidian if not (obsidian_root / Path(path)).is_file()
    )
    if missing_obsidian:
        raise GuardError(
            "Obsidian manifest가 존재하지 않는 Graphify 파일을 선언합니다: "
            + ", ".join(missing_obsidian[:10])
        )
    declared_index_paths = {
        f"graphify-out/obsidian/{path}" for path in declared_obsidian
    }
    missing_indexed_obsidian = sorted(declared_index_paths.difference(indexed_paths))
    if missing_indexed_obsidian:
        raise GuardError(
            "Graphify가 생성한 Obsidian 파일이 Git index에 없습니다: "
            + ", ".join(missing_indexed_obsidian[:10])
        )

    canvas_nodes = canvas.get("nodes", [])
    canvas_groups = sum(node.get("type") == "group" for node in canvas_nodes)
    canvas_files = [node for node in canvas_nodes if node.get("type") == "file"]
    if canvas_groups != len(communities) or len(canvas_files) != len(nodes):
        raise GuardError("Obsidian canvas가 현재 graph의 node/community 수와 다릅니다.")
    missing_canvas_notes = [
        node.get("file")
        for node in canvas_files
        if not isinstance(node.get("file"), str)
        or normalize_path(node["file"]) not in declared_obsidian
    ]
    if missing_canvas_notes:
        raise GuardError("Obsidian canvas가 존재하지 않는 note를 참조합니다.")

    wiki_root = root / "graphify-out/wiki"
    try:
        wiki_index = (wiki_root / "index.md").read_text(encoding="utf-8")
    except OSError as exc:
        raise GuardError(f"wiki index를 읽을 수 없습니다: {exc}") from exc
    stats_match = re.search(
        r"\*\*(\d+) nodes · (\d+) edges · (\d+) communities\*\*", wiki_index
    )
    expected_stats = (len(nodes), len(links), len(communities))
    if not stats_match or tuple(map(int, stats_match.groups())) != expected_stats:
        raise GuardError("wiki index 통계가 현재 graph와 다릅니다.")
    try:
        community_section = wiki_index.split("## Communities", 1)[1].split("\n## ", 1)[0]
    except IndexError as exc:
        raise GuardError("wiki index에 Communities section이 없습니다.") from exc
    community_links = re.findall(r"^- \[.*?\]\((.*?)\)", community_section, re.MULTILINE)
    missing_wiki = [
        target
        for target in community_links
        if not (wiki_root / unquote(target)).is_file()
    ]
    if len(community_links) != len(communities) or missing_wiki:
        raise GuardError("wiki community export가 현재 graph와 다릅니다.")

    validate_export_provenance(root)

    return {
        "nodes": len(nodes),
        "edges": len(links),
        "communities": len(communities),
        "wiki_community_articles": len(community_links),
        "obsidian_notes": len(canvas_files),
    }


def export_provenance_payload(root: Path) -> dict:
    wiki_hash, wiki_files = content_tree_fingerprint(root / "graphify-out/wiki")
    obsidian_owned = obsidian_owned_paths(root)
    obsidian_hash, obsidian_files = selected_tree_fingerprint(
        root / "graphify-out/obsidian",
        obsidian_owned
        | {
            ".graphify_obsidian_manifest.json",
            "graph.canvas",
        },
    )
    return {
        "$schema": EXPORT_SCHEMA,
        "generated_at_utc": datetime.now(timezone.utc).isoformat(),
        "graph_sha256": _sha256(root / "graphify-out/graph.json"),
        "labels_sha256": _sha256(root / "graphify-out/.graphify_labels.json"),
        "label_signature_sha256": _sha256(
            root / "graphify-out/.graphify_labels.json.sig"
        ),
        "manifest_sha256": _sha256(root / "graphify-out/manifest.json"),
        "graph_html_sha256": _sha256(root / "graphify-out/graph.html"),
        "wiki_tree_sha256": wiki_hash,
        "wiki_files": wiki_files,
        "obsidian_tree_sha256": obsidian_hash,
        "obsidian_files": obsidian_files,
    }


def validate_export_provenance(root: Path) -> None:
    provenance_path = root / EXPORT_PROVENANCE_PATH
    try:
        recorded = json.loads(provenance_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise GuardError(f"export provenance를 읽을 수 없습니다: {exc}") from exc
    if recorded.get("$schema") != EXPORT_SCHEMA:
        raise GuardError("export provenance schema가 현재 버전과 다릅니다.")

    current = export_provenance_payload(root)
    compared_keys = (
        "graph_sha256",
        "labels_sha256",
        "label_signature_sha256",
        "manifest_sha256",
        "graph_html_sha256",
        "wiki_tree_sha256",
        "wiki_files",
        "obsidian_tree_sha256",
        "obsidian_files",
    )
    mismatched = [key for key in compared_keys if recorded.get(key) != current.get(key)]
    if mismatched:
        raise GuardError(
            "graph 이후 생성 뷰가 stale입니다: "
            + ", ".join(mismatched)
            + ". knowledge_guard.py export를 다시 실행하세요."
        )


def export_views(root: Path) -> None:
    commands = (
        ("wiki",),
        ("obsidian", "--dir", "graphify-out/obsidian"),
        ("html",),
    )
    for arguments in commands:
        result = subprocess.run(
            [sys.executable, "-m", "graphify", "export", *arguments],
            cwd=root,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            check=False,
        )
        output = result.stdout.decode("utf-8", errors="replace")
        if output:
            print(output, end="" if output.endswith("\n") else "\n")
        if result.returncode != 0:
            raise GuardError(f"Graphify {' '.join(arguments)} export에 실패했습니다.")

    payload = export_provenance_payload(root)
    provenance_path = root / EXPORT_PROVENANCE_PATH
    provenance_path.write_text(
        json.dumps(payload, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    print(f"Graphify export provenance 생성 완료: {EXPORT_PROVENANCE_PATH}")


def graphify_version() -> str:
    for distribution in ("graphifyy", "graphify"):
        try:
            return importlib.metadata.version(distribution)
        except importlib.metadata.PackageNotFoundError:
            continue
    return "unknown"


def write_stamp(root: Path) -> None:
    corpus = scan_graphify_corpus(root)
    ensure_relevant_files_are_staged(root, corpus)
    entries = index_entries(root)
    semantic_documents = validate_manifest(root, corpus)
    graph_counts = validate_artifacts(root, entries)
    source_fingerprint, source_files = fingerprint(entries, is_source_path)
    artifact_fingerprint, artifact_files = fingerprint(entries, is_artifact_path)
    head = _git(root, "rev-parse", "HEAD").decode("ascii").strip()

    payload = {
        "$schema": SCHEMA,
        "generated_at_utc": datetime.now(timezone.utc).isoformat(),
        "head_at_stamp": head,
        "graphify_version": graphify_version(),
        "source_fingerprint": source_fingerprint,
        "source_files": source_files,
        "artifact_fingerprint": artifact_fingerprint,
        "artifact_files": artifact_files,
        "semantic_documents": semantic_documents,
        "graph": graph_counts,
    }
    stamp_path = root / STAMP_PATH
    stamp_path.parent.mkdir(parents=True, exist_ok=True)
    stamp_path.write_text(
        json.dumps(payload, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    print(
        "Graphify wrap-up stamp 생성 완료: "
        f"{STAMP_PATH} ({source_files} sources, {artifact_files} artifacts)"
    )
    print("stamp 파일을 stage하고 커밋한 뒤 push를 다시 실행하세요.")


def resolve_commit(root: Path, revision: str) -> str:
    return (
        _git(root, "rev-parse", "--verify", f"{revision}^{{commit}}")
        .decode("ascii")
        .strip()
    )


def load_stamp_from_commit(root: Path, commit: str) -> dict:
    raw = _git(root, "show", f"{commit}:{STAMP_PATH}", check=False)
    if not raw:
        raise GuardError(f"{commit[:12]}에 {STAMP_PATH}이 없습니다.")
    try:
        return json.loads(raw.decode("utf-8"))
    except (UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise GuardError(f"{commit[:12]}의 wrap-up stamp가 손상됐습니다: {exc}") from exc


def verify_commit(root: Path, revision: str) -> str:
    commit = resolve_commit(root, revision)
    stamp = load_stamp_from_commit(root, commit)
    if stamp.get("$schema") != SCHEMA:
        raise GuardError(f"{commit[:12]}의 wrap-up schema가 현재 버전과 다릅니다.")

    entries = commit_entries(root, commit)
    indexed_paths = {path for path, _, _ in entries}
    required = REQUIRED_ARTIFACTS | {STAMP_PATH}
    missing = sorted(required.difference(indexed_paths))
    if missing:
        raise GuardError(f"{commit[:12]}에 필수 산출물이 없습니다: {', '.join(missing)}")

    source_fingerprint, source_files = fingerprint(entries, is_source_path)
    artifact_fingerprint, artifact_files = fingerprint(entries, is_artifact_path)
    failures: list[str] = []
    if stamp.get("source_fingerprint") != source_fingerprint:
        failures.append("위키·코드 source fingerprint")
    if stamp.get("artifact_fingerprint") != artifact_fingerprint:
        failures.append("Graphify artifact fingerprint")
    if stamp.get("source_files") != source_files:
        failures.append("위키·코드 파일 수")
    if stamp.get("artifact_files") != artifact_files:
        failures.append("산출물 파일 수")
    if failures:
        raise GuardError(
            f"{commit[:12]}의 wrap-up 이후 변경됨: {', '.join(failures)}. "
            "Graphify wrap-up을 다시 수행하고 stamp를 갱신하세요."
        )
    return commit


def revisions_from_stdin() -> list[str]:
    revisions: list[str] = []
    for line in sys.stdin:
        fields = line.strip().split()
        if len(fields) != 4:
            continue
        local_sha = fields[1]
        if not ZERO_OID.fullmatch(local_sha):
            revisions.append(local_sha)
    return revisions


def check_commits(root: Path, revisions: Sequence[str], *, default_head: bool) -> None:
    targets = list(dict.fromkeys(revisions))
    if not targets and default_head:
        targets = ["HEAD"]
    if not targets:
        print("Graphify pre-push: 삭제 ref만 있어 검사할 outgoing commit이 없습니다.")
        return
    verified = [verify_commit(root, revision) for revision in targets]
    for commit in verified:
        print(f"Graphify pre-push 최신성 확인 완료: {commit[:12]}")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)
    subparsers.add_parser("stamp", help="staged 위키·코드와 산출물의 wrap-up stamp 생성")
    subparsers.add_parser("export", help="wiki·Obsidian·HTML export와 provenance 생성")
    check_parser = subparsers.add_parser("check", help="커밋의 wrap-up stamp 검증")
    check_parser.add_argument("--stdin", action="store_true", help="pre-push stdin 읽기")
    check_parser.add_argument(
        "--commit", action="append", default=[], help="직접 검증할 commit/ref"
    )
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    try:
        root = find_repo_root()
        if args.command == "stamp":
            write_stamp(root)
        elif args.command == "export":
            export_views(root)
        else:
            revisions = list(args.commit)
            if args.stdin:
                revisions.extend(revisions_from_stdin())
            check_commits(root, revisions, default_head=not args.stdin)
    except GuardError as exc:
        print(f"[Graphify guard] {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
