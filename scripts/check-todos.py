#!/usr/bin/env python3
"""Check that TODO/FIXME comments follow proper format."""
import re
import sys


def check_todos(filepath):
    """Check if TODO/FIXME comments have proper format."""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            lines = f.readlines()
    except Exception as e:
        print(f"Error reading {filepath}: {e}")
        return False

    issues = []
    # Pattern: TODO: description or TODO(username): description or FIXME: description
    valid_pattern = re.compile(r'//(TODO|FIXME)(\([^)]+\))?:\s+.+')
    todo_pattern = re.compile(r'//(TODO|FIXME)')

    for i, line in enumerate(lines, 1):
        if todo_pattern.search(line):
            if not valid_pattern.search(line):
                issues.append(f"  Line {i}: {line.strip()}")

    if issues:
        print(f"{filepath}:")
        print("  TODO/FIXME must be formatted as: // TODO: description")
        for issue in issues:
            print(issue)
        return False

    return True


def main():
    if len(sys.argv) < 2:
        return 0

    files = sys.argv[1:]
    failed = []

    for filepath in files:
        if not check_todos(filepath):
            failed.append(filepath)

    if failed:
        print(f"\n{len(failed)} file(s) with TODO/FIXME format issues")
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())