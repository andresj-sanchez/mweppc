#!/usr/bin/env python3
"""Print each unit's result, and fail when a unit marked Matching is not exact.

    report.py REPORT

REPORT is objdiff's report.json for PS2 3.0.1 build 198. objdiff does not check where a relocation points, so each
unit also goes through check_relocs.py (a build step of its own, build/PS2_3_0_1b198/relocs/UNIT.txt: its output,
then "exit N"); a Matching unit fails when any of its relocations or case tables differ.
"""

import json
import subprocess
import sys
from pathlib import Path

import b198


def main():
    report = json.loads(Path(sys.argv[1]).read_text())
    status = {b198.unit_name(source): settings.get("status") for source, settings in b198.sources().items()}
    failed = False
    for unit in report.get("units", []):
        measures = unit.get("measures", {})
        functions = unit.get("functions", [])
        exact = sum(1 for f in functions if f.get("fuzzy_match_percent") == 100.0)
        code = measures.get("matched_code_percent", 0)
        print(f"{unit['name']}: {exact} of {len(functions)} functions exact, code {code:.2f}%")
        for function in functions:
            if function.get("fuzzy_match_percent") != 100.0:
                print(f"    {function['name']}: {function.get('fuzzy_match_percent', 0):.2f}%")
        matching = status.get(unit["name"]) == "Matching"
        if matching and exact != len(functions):
            failed = True
        checked = b198.BUILD / "relocs" / (unit["name"] + ".txt")
        if checked.exists():
            lines = checked.read_text().strip().splitlines()
            returncode = int(lines.pop()[5:]) if lines and lines[-1].startswith("exit ") else 1
        else:
            script = Path(__file__).parent / "check_relocs.py"
            result = subprocess.run([sys.executable, script, unit["name"]], capture_output=True, text=True)
            lines, returncode = result.stdout.strip().splitlines(), result.returncode
        for line in lines[:-1]:
            print(f"    {line}")
        if lines:
            print(f"    relocations: {lines[-1]}")
        if matching and returncode:
            failed = True
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
