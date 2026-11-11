#!/usr/bin/env python3
"""Render the project's small, deliberately limited legal Markdown documents."""
from html import escape
from pathlib import Path
import hashlib
import re

ROOT = Path(__file__).resolve().parents[1]
PAGES = {"terms": "terms", "liability": "liability", "privacy": "privacy", "license-guide": "license"}
# Exact GNU response retrieved from https://www.gnu.org/licenses/agpl-3.0.txt.
AGPL_SHA256 = "0d96a4ff68ad6d4b6f1f30f713b18d5184912ba8dd389f86aa7710db079abcb0"
LOCAL_DOCUMENTS = {
    "../../LICENSE": "AGPL-3.0.txt",
    "../../THIRD_PARTY_NOTICES.md": "THIRD_PARTY_NOTICES.txt",
    "../../CONTRIBUTING.md": "CONTRIBUTING.txt",
}


def inline(text):
    text = escape(text)
    def link(match):
        label, href = match.groups()
        if href in LOCAL_DOCUMENTS:
            href = LOCAL_DOCUMENTS[href]
        elif href.endswith(".md") and href[:-3] in PAGES:
            href = PAGES[href[:-3]] + ".html"
        if not (href.startswith("https://") or href in set(LOCAL_DOCUMENTS.values()) | {v + ".html" for v in PAGES.values()}):
            raise ValueError("Unexpected legal link: " + href)
        return f'<a href="{href}">{label}</a>'
    text = re.sub(r"\[([^\]]+)\]\(([^)]+)\)", link, text)
    return re.sub(r"\*\*(.+?)\*\*", r"<strong>\1</strong>", text)


def main():
    license_bytes = (ROOT / "LICENSE").read_bytes()
    if hashlib.sha256(license_bytes).hexdigest() != AGPL_SHA256:
        raise ValueError("LICENSE differs from the verified official GNU AGPLv3 text; review before rendering.")
    target = ROOT / "site/legal"
    target.mkdir(parents=True, exist_ok=True)
    for source, name in LOCAL_DOCUMENTS.items():
        # Preserve the exact license and keep reference documents available offline.
        (target / name).write_bytes((ROOT / "docs/legal" / source).read_bytes())
    for source, name in PAGES.items():
        markdown = (ROOT / f"docs/legal/{source}.md").read_text(encoding="utf-8")
        title = markdown.splitlines()[0].removeprefix("# ")
        blocks = []
        for block in markdown.strip().split("\n\n"):
            if block.startswith("# "):
                blocks.append("<h1>" + inline(block[2:]) + "</h1>")
            elif block.startswith("## "):
                blocks.append("<h2>" + inline(block[3:]) + "</h2>")
            else:
                blocks.append("<p>" + inline(block.replace("\n", " ")) + "</p>")
        document = f'''<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>{escape(title)} — SUBMOTIF by Habeeb</title><meta name="robots" content="noindex">
<meta name="description" content="{escape(title)} for SUBMOTIF by Habeeb.">
<link rel="icon" href="../favicon.svg"><link rel="stylesheet" href="legal.css"></head>
<body><a class="skip" href="#content">Skip to content</a><header><a class="brand" href="../index.html">SUBMOTIF <span>by Habeeb</span></a><a href="../index.html">Back to the generator</a></header>
<main id="content">{''.join(blocks)}</main><footer><nav aria-label="Legal"><a href="terms.html">Terms of use</a><a href="liability.html">Liability release</a><a href="privacy.html">Privacy</a><a href="license.html">License</a><a href="../credits.html">Credits</a></nav><p>© 2026 Habeeb</p></footer></body></html>
'''
        (target / (name + ".html")).write_text(document, encoding="utf-8")
    print("Rendered four legal pages and local license/reference documents; exact GNU license verified.")


if __name__ == "__main__":
    main()
