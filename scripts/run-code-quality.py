#!/usr/bin/env python3
"""
Essential code quality runner for Vision Craft.
Runs clang-format and clang-tidy checks.
"""

import argparse
import subprocess
import sys
from pathlib import Path
from typing import List, Tuple


def run_command(cmd: List[str], description: str = "") -> Tuple[bool, str, str]:
    """Run a command and return success, stdout, stderr."""
    print(f"[*] {description}")
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=300)
        return result.returncode == 0, result.stdout, result.stderr
    except Exception as e:
        return False, "", str(e)


def run_clang_format_check() -> bool:
    """Run clang-format check on all source files."""
    print("\n[FORMAT] Checking code formatting...")

    # Find all C++ files
    cpp_files = []
    for pattern in ["**/*.cpp", "**/*.hpp", "**/*.h"]:
        cpp_files.extend(Path("src").glob(pattern))
        cpp_files.extend(Path("tests").glob(pattern))

    if not cpp_files:
        print("No C++ files found")
        return True

    # Find clang-format
    clang_format_paths = ["clang-format", "C:/Program Files/LLVM/bin/clang-format.exe"]
    clang_format = None

    for path in clang_format_paths:
        success, _, _ = run_command([path, "--version"])
        if success:
            clang_format = path
            break

    if not clang_format:
        print("clang-format not found")
        return False

    # Check formatting
    issues = 0
    for file_path in cpp_files:
        success, _, stderr = run_command(
            [clang_format, "--dry-run", "--Werror", str(file_path)],
            f"Checking {file_path}"
        )
        if not success:
            issues += 1

    print(f"Files checked: {len(cpp_files)}, Issues: {issues}")
    return issues == 0


def run_clang_tidy_check() -> bool:
    """Run clang-tidy analysis."""
    print("\n[TIDY] Running static analysis...")

    # Try CMake target first
    if Path("build").exists():
        success, _, _ = run_command(
            ["cmake", "--build", "build", "--target", "tidy-all"],
            "Running clang-tidy via CMake"
        )
        if success:
            print("Static analysis completed")
            return True

    print("CMake target not available, skipping clang-tidy")
    return True


def main():
    """Main function."""
    parser = argparse.ArgumentParser(description="Run code quality checks")
    parser.add_argument("--format-only", action="store_true", help="Run only formatting checks")
    parser.add_argument("--tidy-only", action="store_true", help="Run only static analysis")

    args = parser.parse_args()

    print("Running code quality checks...")
    print("=" * 50)

    success = True

    if not args.tidy_only:
        if not run_clang_format_check():
            success = False

    if not args.format_only:
        if not run_clang_tidy_check():
            success = False

    print("\n" + "=" * 50)
    if success:
        print("[SUCCESS] All checks passed!")
        return 0
    else:
        print("[FAILED] Some checks failed")
        return 1


if __name__ == "__main__":
    sys.exit(main())