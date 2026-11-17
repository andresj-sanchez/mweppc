#!/usr/bin/env python3
"""link.py LINKER...: link PS2 3.0.1 build 198 back together and check it against its SHA-1.

The objects are build/PS2_3_0_1b198/link/obj's (split.py), in the order of the link's splits.txt, except that each unit
marked Matching in config/PS2_3_0_1b198/sources.json is linked from its compiled object. The imports come from import
libraries written here, with the original's DLLs, order and hints; the exports from the link's exports.txt; the resources
from the original's. LINKER is the command that runs mwld (CodeWarrior's Windows/x86 linker 3.2.4, the 3.2 linker the
original's header names). The linker writes the time it runs in the header and the export directory: the original's
are put back. build/PS2_3_0_1b198/mwccps2.exe is the result; build/PS2_3_0_1b198/link/ok is written when it is the original.
"""

import json
import re
import struct
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import b198  # noqa: I001
from mw import formats, project
from mw.coff import EXTERNAL, WEAK_EXTERNAL, SEARCH_LIBRARY, Object
from mw.link import Import, import_library
from mw.pe import EXPORTS, PEFile, original


def exported_statics(path, exports, out):
    """The object at PATH as the link takes it, written to OUT when it has to change; returns the path to link.

    The original exports some static locals by the name the compiler gave them, `sFlags$3267` (`#pragma export on`
    around a plugin's entry points). The number after the `$` is a counter that runs through the whole file, the
    headers included, and the original's headers (the Windows and library ones, about 750 inline functions) are not
    ours: the same source numbers them lower (`_sFlags$241`), by the same steps. So a static local the object exports
    takes the number of the original's export of the same name, and the object's own export directives are blanked:
    the link exports what the original does (exports.txt). Only names change; the bytes are the compiler's."""
    data = bytearray(Path(path).read_bytes())
    count, symbols_at, symbols = struct.unpack_from("<H", data, 2)[0], *struct.unpack_from("<II", data, 8)
    sections_at = 20 + struct.unpack_from("<H", data, 16)[0]
    strings_at = symbols_at + 18 * symbols
    wanted = {}  # a static's name without its number -> the original's exported name
    for name in exports:
        m = re.fullmatch(r"_?([A-Za-z_]\w*)\$\d+", name)
        if m:
            wanted.setdefault(m[1], []).append(name)
    changed, renames, i = False, [], 0
    while i < symbols:
        at = symbols_at + 18 * i
        raw = bytes(data[at : at + 8])
        if raw[:4] == b"\0\0\0\0":
            offset = strings_at + struct.unpack_from("<I", raw, 4)[0]
            name = bytes(data[offset : data.index(b"\0", offset)]).decode("latin-1")
        else:
            name = raw.rstrip(b"\0").decode("latin-1")
        m = re.fullmatch(r"_?([A-Za-z_]\w*)\$\d+", name)
        defined = struct.unpack_from("<h", data, at + 12)[0] > 0
        if m and defined and len(wanted.get(m[1], [])) == 1 and name != wanted[m[1]][0]:
            renames.append((at, wanted[m[1]][0]))
        i += 1 + data[at + 17]
    if renames:
        table = bytearray(data[strings_at : strings_at + struct.unpack_from("<I", data, strings_at)[0]])
        for at, new in renames:
            data[at : at + 8] = struct.pack("<II", 0, len(table))
            table += new.encode("latin-1") + b"\0"
        struct.pack_into("<I", table, 0, len(table))
        data[strings_at:] = table
        changed = True
    for s in range(count):
        header = sections_at + 40 * s
        if bytes(data[header : header + 8]).rstrip(b"\0") == b".drectve":
            size, raw_at = struct.unpack_from("<II", data, header + 16)
            text = bytes(data[raw_at : raw_at + size])
            blanked = re.sub(rb"-export:\S+", lambda m: b" " * len(m[0]), text)
            if blanked != text:
                data[raw_at : raw_at + size] = blanked
                changed = True
    if not changed:
        return path
    project.write_atomic(out, bytes(data))
    return out


def link(v, linker, extra=(), output=None):
    """Link VERSION with the units marked Matching, and the sources in EXTRA, from their compiled objects, to OUTPUT
    (a file named mwccps2.exe: the export directory holds the name). Returns (is the original, summary, the lines that
    say where it differs or why the link fails)."""
    pe = original(v)
    sources = json.loads((v.config / "sources.json").read_text())
    matching = {s for s, settings in sources.items() if settings.get("status") == "Matching"} | set(extra)
    decorated = {}  # an import's __imp_ name -> the decorated names the units linked from source reach it by
    for source in matching:
        if not (v.build / source).with_suffix(".obj").exists():
            continue
        for symbol in Object.read((v.build / source).with_suffix(".obj")).symbols:
            m = re.fullmatch(r"(__imp_[^@]+)@\d+", symbol.name)
            if symbol.undefined and m:
                decorated.setdefault(m[1], set()).add(symbol.name)
    libraries = []
    for dll in dict.fromkeys(item.dll for item in pe.imports):
        entries = [
            Import(i.dll, "_" + (i.name or f"{dll.split('.')[0]}_{i.number}"), i.number, i.name is not None, i.slot)
            for i in pe.imports
            if i.dll == dll
        ]
        path = b198.LINK / "implib" / (dll.rsplit(".", 1)[0] + ".lib")
        project.write_atomic(path, import_library(dll, entries))
        libraries.append(project.relative(path))

    # (a unit compiled from source reaches an import by its decorated name, __imp__GetModuleHandleA@4 for a __stdcall
    # one, while the import libraries and the split objects name each import as the original's table does. One object
    # defines every decorated name at the import's slot in the original's import address table, and refers to the
    # undecorated name as well, so the import itself is still made)
    # (a unit compiled from source reaches an import by its decorated name, __imp__GetModuleHandleA@4 for a __stdcall
    # one, while the import libraries and the split objects name each import as the original's table does. One object
    # declares every decorated name a weak external aliased to the undecorated one, so the linker resolves it to the
    # import the import library makes, and makes the import even when no other object refers to it)
    if decorated:
        obj = Object()
        for base, names in sorted(decorated.items()):
            tag = obj.add_symbol(base, 0, 0, 0, EXTERNAL)
            for name in sorted(names):
                obj.add_symbol(name, 0, 0, 0, WEAK_EXTERNAL, struct.pack("<II", tag, SEARCH_LIBRARY) + bytes(10))
        path = b198.LINK / "impalias.obj"
        obj.write(path)

    units = formats.read_splits(b198.LINK / "splits.txt").units
    objects, compiled = [project.relative(b198.LINK / "obj" / "sections.obj")], 0
    if decorated:
        objects.append(project.relative(b198.LINK / "impalias.obj"))
    exports = [e for e in (b198.LINK / "exports.txt").read_text().split() if e]
    for unit in units:
        if unit.name in matching:
            obj = (v.build / unit.name).with_suffix(".obj")
            objects.append(project.relative(exported_statics(obj, exports, b198.LINK / "exported" / obj.name)))
            compiled += 1
        else:
            objects.append(project.relative(b198.LINK / "obj" / f"{unit.name}.obj"))
    entry = next(
        s.name
        for s in formats.read_symbols(b198.LINK / "symbols.txt")
        if s.address == pe.image_base + struct.unpack_from("<I", pe.data, pe.optional + 16)[0]
    )
    output = Path(output or v.build / "mwccps2.exe")
    output.parent.mkdir(parents=True, exist_ok=True)
    output.unlink(missing_ok=True)
    result = subprocess.run(
        [
            *linker,
            "-nostdlib",
            "-subsystem",
            "console",
            "-m",
            entry,
            "-filealign",
            "512",
            *[x for e in exports for x in ("-export", f"sym={e}")],
            *objects,
            *libraries,
            project.relative(b198.LINK / "obj" / "resources.res"),
            "-o",
            project.relative(output),
            "-map",
            project.relative(output.with_suffix(".map")),
        ],
        capture_output=True,
        text=True,
        cwd=project.ROOT,
    )
    summary = f"{compiled} of {len(units)} units linked from source"
    if result.returncode or not output.exists():
        errors = [line.strip("# ").strip() for line in result.stdout.splitlines() if line.startswith("#   ")]
        return False, summary, ["the link fails:"] + (errors[:40] or [result.stdout[-2000:]])

    linked = PEFile(output)
    out = bytearray(linked.data)
    stamp = pe.coff + 4
    out[stamp : stamp + 4] = pe.data[stamp : stamp + 4]
    if pe.directory(EXPORTS)[0] and linked.directory(EXPORTS)[0]:
        theirs = pe.address_to_offset(pe.directory(EXPORTS)[0]) + 4
        ours = linked.address_to_offset(linked.directory(EXPORTS)[0]) + 4
        out[ours : ours + 4] = pe.data[theirs : theirs + 4]
    project.write_atomic(output, bytes(out))
    if project.digest(out) == b198.SHA1:
        return True, summary, []
    # (where the link differs: the units holding the first differing bytes)
    lines = []
    for section in pe.sections:
        ours = next((s for s in linked.sections if s.name == section.name), None)
        if ours is None or ours.virtual_address != section.virtual_address or ours.virtual_size != section.virtual_size:
            lines.append(
                f"{section.name}: at 0x{ours.virtual_address if ours else 0:08X}, 0x{ours.virtual_size if ours else 0:X} "
                f"bytes, not at 0x{section.virtual_address:08X}, 0x{section.virtual_size:X} bytes"
            )
            continue
        size = min(section.virtual_size, section.file_size)
        a = pe.data[section.file_offset : section.file_offset + size]
        b = bytes(out[ours.file_offset : ours.file_offset + size])
        if a == b:
            continue
        first = next(i for i in range(size) if a[i] != b[i])
        address = section.virtual_address + first
        holder = next((u.name for u in units for r in u.ranges if r.start <= address < r.end), "?")
        lines.append(f"{section.name}: differs from 0x{address:08X} (in {holder})")
    return False, summary, lines or ["the headers differ"]


def main():
    if len(sys.argv) < 2 or sys.argv[1].startswith("-"):
        sys.exit(__doc__.strip())
    v = b198.VERSION
    (b198.LINK / "ok").unlink(missing_ok=True)
    output = v.build / "mwccps2.exe"
    same, summary, lines = link(v, sys.argv[1:], output=output)
    if same:
        project.write_atomic(b198.LINK / "ok", f"{v}: the original, {summary}\n")
        print(f"{project.relative(output)}: the original ({summary})")
        return
    print(f"{project.relative(output)}: not the original ({summary})", file=sys.stderr)
    for line in lines:
        print(f"  {line}", file=sys.stderr)
    sys.exit(1)


if __name__ == "__main__":
    try:
        main()
    except project.Error as e:
        sys.exit(f"error: {e}")
