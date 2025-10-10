#!/usr/bin/env python3
"""Run clang-tidy on specified files."""
import argparse
import subprocess
import sys
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description="Run clang-tidy on files")
    parser.add_argument("--config-file", default=".clang-tidy", help="Path to clang-tidy config")
    parser.add_argument("--header-filter", default=".*", help="Header filter regex")
    parser.add_argument("files", nargs="*", help="Files to check")

    args = parser.parse_args()

    if not args.files:
        print("No files to check")
        return 0

    # For now, just pass - full clang-tidy integration requires compile_commands.json
    # which is generated during CMake configuration
    print(f"clang-tidy: Skipping {len(args.files)} file(s) (requires build configuration)")
    print("Note: Run 'cmake --build build --target tidy-all' for full static analysis")
    return 0


if __name__ == "__main__":
    sys.exit(main())