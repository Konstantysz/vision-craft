#!/usr/bin/env python3
"""
Vision-Craft Test Coverage Script

This script runs tests with code coverage analysis using LLVM tools.
Generates both HTML and text coverage reports.

Requirements:
    - LLVM tools (llvm-cov, llvm-profdata) in PATH
    - CMake 3.26+
    - vcpkg installed
    - Google Test (installed via vcpkg)

Usage:
    python scripts/run-coverage.py [--clean] [--open] [--no-build]

Options:
    --clean      Clean build directory before running
    --open       Open HTML report in browser after generation
    --no-build   Skip build step (only regenerate coverage from existing data)
    --help       Show this help message
"""

import argparse
import os
import platform
import shutil
import subprocess
import sys
from pathlib import Path
from typing import Optional


class Colors:
    """ANSI color codes for terminal output."""

    RED = "\033[0;31m"
    GREEN = "\033[0;32m"
    YELLOW = "\033[1;33m"
    BLUE = "\033[0;34m"
    CYAN = "\033[0;36m"
    BOLD = "\033[1m"
    NC = "\033[0m"  # No Color

    @staticmethod
    def disable():
        """Disable colors for Windows or pipes."""
        Colors.RED = ""
        Colors.GREEN = ""
        Colors.YELLOW = ""
        Colors.BLUE = ""
        Colors.CYAN = ""
        Colors.BOLD = ""
        Colors.NC = ""


# Disable colors on Windows by default (unless ANSI is enabled)
if platform.system() == "Windows" and not os.environ.get("ANSICON"):
    Colors.disable()


def print_header(text: str):
    """Print a section header."""
    print()
    print(f"{Colors.BLUE}{'=' * 70}{Colors.NC}")
    print(f"{Colors.BLUE}{Colors.BOLD}  {text}{Colors.NC}")
    print(f"{Colors.BLUE}{'=' * 70}{Colors.NC}")
    print()


def print_step(text: str):
    """Print a step message."""
    print(f"{Colors.GREEN}→{Colors.NC} {text}")


def print_warning(text: str):
    """Print a warning message."""
    print(f"{Colors.YELLOW}⚠{Colors.NC} {text}")


def print_error(text: str):
    """Print an error message."""
    print(f"{Colors.RED}✗{Colors.NC} {text}")


def print_success(text: str):
    """Print a success message."""
    print(f"{Colors.GREEN}✓{Colors.NC} {text}")


def check_command(command: str) -> bool:
    """Check if a command exists in PATH."""
    result = shutil.which(command)
    if result:
        print_success(f"{command} found: {result}")
        return True
    else:
        print_error(f"{command} not found in PATH")
        return False


def run_command(
    cmd: list[str], cwd: Optional[Path] = None, check: bool = True
) -> subprocess.CompletedProcess:
    """Run a command and return the result."""
    try:
        result = subprocess.run(
            cmd,
            cwd=cwd,
            check=check,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )
        return result
    except subprocess.CalledProcessError as e:
        print_error(f"Command failed: {' '.join(cmd)}")
        print(e.stdout)
        raise


def get_vcpkg_toolchain() -> Path:
    """Get the vcpkg toolchain file path."""
    # Check environment variable first
    vcpkg_root = os.environ.get("VCPKG_ROOT")
    if vcpkg_root:
        toolchain = Path(vcpkg_root) / "scripts" / "buildsystems" / "vcpkg.cmake"
        if toolchain.exists():
            return toolchain

    # Default locations by platform
    if platform.system() == "Windows":
        default_path = Path("C:/ProgramData/vcpkg/scripts/buildsystems/vcpkg.cmake")
    else:
        default_path = Path.home() / "vcpkg" / "scripts" / "buildsystems" / "vcpkg.cmake"

    if default_path.exists():
        return default_path

    print_error(f"vcpkg toolchain not found at: {default_path}")
    print_warning("Set VCPKG_ROOT environment variable or install vcpkg")
    sys.exit(1)


def validate_environment() -> dict[str, Path]:
    """Validate that all required tools are available."""
    print_header("Validating Environment")
    print_step("Checking for required tools...")

    missing_tools = False

    # Check required commands
    if not check_command("cmake"):
        missing_tools = True
    if not check_command("llvm-cov"):
        missing_tools = True
    if not check_command("llvm-profdata"):
        missing_tools = True

    # Check vcpkg toolchain
    try:
        vcpkg_toolchain = get_vcpkg_toolchain()
        print_success(f"vcpkg toolchain found: {vcpkg_toolchain}")
    except SystemExit:
        missing_tools = True

    if missing_tools:
        print_error("Missing required tools. Please install them and try again.")
        sys.exit(1)

    return {"vcpkg_toolchain": vcpkg_toolchain}


def clean_build_directory(build_dir: Path):
    """Clean the build directory."""
    print_header("Cleaning Build Directory")

    if build_dir.exists():
        print_step(f"Removing {build_dir}...")
        shutil.rmtree(build_dir)
        print_success("Build directory cleaned")
    else:
        print_warning("Build directory doesn't exist, skipping clean")


def configure_cmake(project_root: Path, build_dir: Path, vcpkg_toolchain: Path):
    """Configure CMake with coverage enabled."""
    print_header("Configuring CMake with Coverage")

    print_step("Running CMake configuration...")

    cmd = [
        "cmake",
        "-S",
        str(project_root),
        "-B",
        str(build_dir),
        f"-DCMAKE_TOOLCHAIN_FILE={vcpkg_toolchain}",
        "-DENABLE_COVERAGE=ON",
        "-DBUILD_TESTS=ON",
        "-DCMAKE_BUILD_TYPE=Debug",
    ]

    result = run_command(cmd)
    print(result.stdout)

    print_success("CMake configuration complete")


def build_project(build_dir: Path):
    """Build the project with coverage instrumentation."""
    print_header("Building Project")

    print_step("Building project with coverage instrumentation...")

    cmd = ["cmake", "--build", str(build_dir), "--config", "Debug", "-j"]

    result = run_command(cmd)
    # Only print if there were errors
    if result.returncode != 0:
        print(result.stdout)

    print_success("Build complete")


def clean_coverage_data(build_dir: Path):
    """Clean previous coverage data."""
    print_header("Cleaning Previous Coverage Data")

    print_step("Running coverage-clean target...")

    cmd = ["cmake", "--build", str(build_dir), "--target", "coverage-clean"]

    run_command(cmd, check=False)  # May fail if target doesn't exist

    print_success("Previous coverage data cleaned")


def run_coverage(build_dir: Path):
    """Run tests and generate coverage reports."""
    print_header("Running Tests with Coverage")

    print_step("Running coverage target (this will run tests and generate reports)...")

    cmd = ["cmake", "--build", str(build_dir), "--target", "coverage-html"]

    # This may have warnings, so don't fail on non-zero exit
    result = run_command(cmd, check=False)

    # Check if HTML report was actually generated
    html_report = build_dir / "coverage" / "html" / "index.html"
    if not html_report.exists():
        print_error("HTML report was not generated")
        print(result.stdout)
        sys.exit(1)

    print_success("Coverage reports generated")


def display_summary(build_dir: Path):
    """Display coverage summary."""
    print_header("Coverage Summary")

    print_step("Generating coverage summary...")

    test_exe = build_dir / "tests" / "TestVisionCraftNodes"
    if platform.system() == "Windows":
        test_exe = test_exe.with_suffix(".exe")

    coverage_data = build_dir / "coverage" / "coverage.profdata"

    cmd = [
        "llvm-cov",
        "report",
        str(test_exe),
        f"-instr-profile={coverage_data}",
    ]

    result = run_command(cmd, check=False)
    if result.returncode == 0:
        print(result.stdout)
    else:
        print_warning("Could not generate detailed summary (profdata may have format issues)")
        print_warning("HTML report should still be available")


def open_report(html_report: Path):
    """Open the HTML report in the default browser."""
    print_step("Opening report in browser...")

    try:
        if platform.system() == "Windows":
            os.startfile(str(html_report))
        elif platform.system() == "Darwin":  # macOS
            subprocess.run(["open", str(html_report)], check=True)
        else:  # Linux
            subprocess.run(["xdg-open", str(html_report)], check=True)
        print_success("Opened report in browser")
    except Exception as e:
        print_warning(f"Could not open browser automatically: {e}")


def print_results(build_dir: Path, open_browser: bool):
    """Print results and instructions."""
    print_header("Results")

    html_report = build_dir / "coverage" / "html" / "index.html"
    coverage_data = build_dir / "coverage" / "coverage.profdata"

    if html_report.exists():
        print_success(f"HTML report generated: {html_report}")
        print_success(f"Coverage data: {coverage_data}")

        # File size info
        report_size = html_report.stat().st_size / 1024  # KB
        print_step(f"Report size: {report_size:.1f} KB")

        print()
        print(f"{Colors.BLUE}To view the report:{Colors.NC}")
        print(f"  {Colors.GREEN}1.{Colors.NC} Open in browser: file:///{html_report}")
        print(f"  {Colors.GREEN}2.{Colors.NC} Or double-click: {html_report}")

        if open_browser:
            print()
            open_report(html_report)
    else:
        print_error("HTML report not found. Check build output for errors.")
        sys.exit(1)

    print()
    print(f"{Colors.BLUE}Additional coverage targets:{Colors.NC}")
    print(f"  {Colors.GREEN}•{Colors.NC} cmake --build build --target coverage-report    (text report)")
    print(
        f"  {Colors.GREEN}•{Colors.NC} cmake --build build --target coverage-summary   (brief summary)"
    )
    print(
        f"  {Colors.GREEN}•{Colors.NC} cmake --build build --target coverage-clean     (clean coverage data)"
    )
    print()


def main():
    """Main entry point."""
    parser = argparse.ArgumentParser(
        description="Run test coverage analysis for Vision-Craft",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python scripts/run-coverage.py                    # Run full coverage
  python scripts/run-coverage.py --clean            # Clean build first
  python scripts/run-coverage.py --open             # Open report in browser
  python scripts/run-coverage.py --no-build --open  # Regenerate report only
        """,
    )

    parser.add_argument(
        "--clean", action="store_true", help="Clean build directory before running"
    )
    parser.add_argument(
        "--open", action="store_true", help="Open HTML report in browser after generation"
    )
    parser.add_argument(
        "--no-build",
        action="store_true",
        help="Skip build step (only regenerate coverage from existing data)",
    )

    args = parser.parse_args()

    # Get project directories
    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    build_dir = project_root / "build"

    print()
    print(f"{Colors.CYAN}{Colors.BOLD}Vision-Craft Coverage Analysis{Colors.NC}")
    print(f"{Colors.CYAN}Project: {project_root}{Colors.NC}")

    # Validate environment
    env = validate_environment()

    # Clean if requested
    if args.clean:
        clean_build_directory(build_dir)

    # Configure, build, and run coverage
    if not args.no_build:
        configure_cmake(project_root, build_dir, env["vcpkg_toolchain"])
        build_project(build_dir)
        clean_coverage_data(build_dir)
        run_coverage(build_dir)
    else:
        print_header("Skipping Build (--no-build)")
        print_step("Regenerating coverage report from existing data...")
        run_coverage(build_dir)

    # Display summary
    display_summary(build_dir)

    # Print results
    print_results(build_dir, args.open)

    print_header("Coverage Analysis Complete")


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print()
        print_warning("Interrupted by user")
        sys.exit(130)
    except Exception as e:
        print()
        print_error(f"Unexpected error: {e}")
        import traceback

        traceback.print_exc()
        sys.exit(1)
