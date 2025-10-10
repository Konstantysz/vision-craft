#!/usr/bin/env python3
"""Check that header files have proper include guards."""
import re
import sys
from pathlib import Path


def check_header_guard(filepath):
    """Check if a header file has a proper include guard."""
    path = Path(filepath)

    # Generate expected guard name from file path
    # Example: src/VisionCraftApp/Layers/Layer.h -> VISIONCRAFT_LAYERS_LAYER_H
    relative_path = path.as_posix()
    if 'src/' in relative_path:
        relative_path = relative_path.split('src/', 1)[1]

    expected_guard = relative_path.replace('/', '_').replace('.', '_').upper()

    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
    except Exception as e:
        print(f"Error reading {filepath}: {e}")
        return False

    # Look for #pragma once (preferred) or include guards
    if '#pragma once' in content:
        return True

    # Check for traditional include guards
    ifndef_pattern = rf'#ifndef\s+{re.escape(expected_guard)}'
    define_pattern = rf'#define\s+{re.escape(expected_guard)}'

    has_ifndef = re.search(ifndef_pattern, content)
    has_define = re.search(define_pattern, content)

    if has_ifndef and has_define:
        return True

    print(f"{filepath}: Missing proper header guard (expected: {expected_guard} or #pragma once)")
    return False


def main():
    if len(sys.argv) < 2:
        return 0

    files = sys.argv[1:]
    failed = []

    for filepath in files:
        if not check_header_guard(filepath):
            failed.append(filepath)

    if failed:
        print(f"\n{len(failed)} file(s) with header guard issues")
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())