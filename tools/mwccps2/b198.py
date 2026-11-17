"""PS2 3.0.1 build 198: where its configuration, build and original are, and which sources and headers are its own.

The version is matched by its own pipeline (tools/mwccps2/matching.py): its units are listed in
config/PS2_3_0_1b198/sources.json and compiled with CodeWarrior 9.2 for Windows' x86 compiler, and the original is
split along config/PS2_3_0_1b198/splits.txt. Its sources and headers share this tree with the other versions
(tools/mwccps2/merge.py): a file of its own, new or under src/ps2 and include/ps2, or a merged file whose
`#if MWCCPS2_B198` blocks are its text.
"""

from __future__ import annotations

import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from mw import config, project  # noqa: E402

VERSION = project.version("PS2_3_0_1b198")
CONFIG = VERSION.config
BUILD = VERSION.build
LINK = BUILD / "link"
ORIGINAL = project.ROOT / config.version_config(VERSION).raw["original"]
SHA1 = config.version_config(VERSION).sha1
SOURCES = CONFIG / "sources.json"
LEDGER = CONFIG / "merge.json"
INCLUDE_DIRS = ("include/ps2", "include", "include/libc")


def sources() -> dict:
    """config/PS2_3_0_1b198/sources.json: each unit's source, and its flags and status."""
    return json.loads(SOURCES.read_text())


def unit_name(source: str) -> str:
    """A unit's name as objdiff reports it: its source path without src/ and the extension."""
    return source.removeprefix("src/").rsplit(".", 1)[0]


def headers() -> dict[Path, str]:
    """The PS2 build's headers, each as this version reads it: the merged ones resolved for PS2 b198."""
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    import merge

    found = {}
    for row in json.loads(LEDGER.read_text()):
        path = project.ROOT / row["file"]
        if path.suffix == ".h":
            text = path.read_text(errors="replace")
            found[path] = merge.resolve(text, True) if row["how"] == "merged" else text
    return found
