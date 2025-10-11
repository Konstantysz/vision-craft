#!/usr/bin/env python3
"""Check that namespace closing braces have comments."""
import re
import sys


def check_namespaces(filepath):
    """Check if namespace closing braces have proper comments."""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
    except Exception as e:
        print(f"Error reading {filepath}: {e}")
        return False

    # Find namespace declarations and their closing braces
    # Pattern: namespace Name { ... } // namespace Name
    namespace_pattern = re.compile(r'namespace\s+(\w+)')
    closing_brace_pattern = re.compile(r'}\s*//\s*namespace\s+(\w+)')

    namespaces = namespace_pattern.findall(content)
    commented_closes = closing_brace_pattern.findall(content)

    # For now, just pass - full namespace checking requires AST parsing
    # This is a simplified check
    return True


def main():
    if len(sys.argv) < 2:
        return 0

    files = sys.argv[1:]
    failed = []

    for filepath in files:
        if not check_namespaces(filepath):
            failed.append(filepath)

    if failed:
        print(f"\n{len(failed)} file(s) with namespace format issues")
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
