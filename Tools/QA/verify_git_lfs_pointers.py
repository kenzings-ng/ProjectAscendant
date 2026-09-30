#!/usr/bin/env python3
"""
Tools/QA/verify_git_lfs_pointers.py

Verifies that any binary file matching .gitattributes (filter=lfs) committed or staged
in the target range is a genuine Git LFS pointer, and NOT a raw binary file.
"""

import sys
import subprocess
import argparse
from pathlib import Path

def run_cmd(args):
    res = subprocess.run(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    return res.returncode, res.stdout.strip(), res.stderr.strip()

def get_filter_attr(filepath: str) -> str:
    code, stdout, _ = run_cmd(["git", "check-attr", "filter", "--", filepath])
    if code == 0 and stdout:
        parts = stdout.split()
        if len(parts) >= 3:
            return parts[2]
    return ""

def check_lfs_pointer(filepath: str, ref: str = "") -> tuple[bool, str]:
    target = f"{ref}:{filepath}" if ref else f":{filepath}"
    
    # 1. Check blob size
    code, stdout, stderr = run_cmd(["git", "cat-file", "-s", target])
    if code != 0:
        return False, f"Could not find git object for {target}: {stderr}"
    
    try:
        blob_size = int(stdout)
    except ValueError:
        return False, f"Invalid blob size: {stdout}"
    
    if blob_size > 1024:
        return False, f"Blob size is {blob_size} bytes (expected LFS pointer <= 1024 bytes, usually ~130 bytes)"
    
    # 2. Check pointer header
    code, stdout, stderr = run_cmd(["git", "cat-file", "-p", target])
    if code != 0:
        return False, f"Could not read blob content for {target}: {stderr}"
    
    lines = stdout.splitlines()
    if not lines or not lines[0].startswith("version https://git-lfs.github.com/spec/v1"):
        first_line = lines[0] if lines else "<empty>"
        return False, f"Invalid LFS pointer header: '{first_line}'"
    
    return True, f"Valid LFS pointer ({blob_size} bytes)"

def main():
    parser = argparse.ArgumentParser(description="Verify Git LFS pointers for binary assets.")
    parser.add_argument("--base", default="", help="Base ref or branch to diff against (e.g. origin/main)")
    parser.add_argument("--head", default="HEAD", help="Head ref to diff (default: HEAD)")
    parser.add_argument("files", nargs="*", help="Specific files to check. If omitted, diff is checked.")
    args = parser.parse_args()

    files_to_check = []
    
    if args.files:
        files_to_check = args.files
    else:
        # Determine changed files
        if args.base:
            diff_range = f"{args.base}...{args.head}"
            code, stdout, _ = run_cmd(["git", "diff", "--name-only", "--diff-filter=ACMR", diff_range])
            if code == 0 and stdout:
                files_to_check = [f.strip() for f in stdout.splitlines() if f.strip()]
        else:
            # Check staged + uncommitted or HEAD diff against origin/main if available
            code, stdout, _ = run_cmd(["git", "diff", "--cached", "--name-only", "--diff-filter=ACMR"])
            if code == 0 and stdout:
                files_to_check.extend([f.strip() for f in stdout.splitlines() if f.strip()])
            
            # If no staged files, fall back to origin/main diff if remote exists
            if not files_to_check:
                code, stdout, _ = run_cmd(["git", "diff", "--name-only", "--diff-filter=ACMR", "origin/main...HEAD"])
                if code == 0 and stdout:
                    files_to_check.extend([f.strip() for f in stdout.splitlines() if f.strip()])

    if not files_to_check:
        print("[Git LFS Gate] No files to check.")
        sys.exit(0)

    lfs_files = []
    for f in set(files_to_check):
        if not f:
            continue
        attr = get_filter_attr(f)
        if attr == "lfs":
            lfs_files.append(f)

    if not lfs_files:
        print(f"[Git LFS Gate] None of the {len(files_to_check)} checked files require Git LFS.")
        sys.exit(0)

    print(f"[Git LFS Gate] Checking {len(lfs_files)} files tracked by Git LFS:")
    failed = False
    for f in sorted(lfs_files):
        ok, msg = check_lfs_pointer(f)
        if ok:
            print(f"  [PASS] {f} -> {msg}")
        else:
            print(f"  [FAIL] {f} -> {msg}", file=sys.stderr)
            failed = True

    if failed:
        print("\n[ERROR] Found binary files committed without Git LFS pointers!", file=sys.stderr)
        print("Run 'git lfs install' and re-add binary files ('git rm --cached <files> && git add <files>').", file=sys.stderr)
        sys.exit(1)

    print("\n[Git LFS Gate] All Git LFS assets verified successfully.")
    sys.exit(0)

if __name__ == "__main__":
    main()
