#!/usr/bin/env bash
# Project Ascendant - Developer Environment Setup Script
set -euo pipefail

echo "============================================================"
echo "Project Ascendant: Developer Environment Setup"
echo "============================================================"

# 1. Initialize Git LFS
if command -v git-lfs >/dev/null 2>&1; then
    git lfs install
    echo "[OK] Git LFS initialized successfully."
else
    echo "[WARNING] 'git-lfs' command not found. Please install Git LFS (e.g., sudo apt install git-lfs) to track binary game assets." >&2
fi

# 2. Configure Git hooks path
git config core.hooksPath Tools/git-hooks
echo "[OK] Configured git core.hooksPath = Tools/git-hooks"

# 3. Ensure hooks are executable
if [ -d Tools/git-hooks ]; then
    chmod +x Tools/git-hooks/*
    echo "[OK] Ensured Tools/git-hooks/* are executable"
fi

echo "============================================================"
echo "Setup completed successfully."
echo "============================================================"
