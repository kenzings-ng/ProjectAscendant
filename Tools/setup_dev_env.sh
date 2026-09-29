#!/usr/bin/env bash
# Project Ascendant - Developer Environment Setup Script
set -euo pipefail

echo "============================================================"
echo "Project Ascendant: Developer Environment Setup"
echo "============================================================"

# 1. Configure Git hooks path
git config core.hooksPath Tools/git-hooks
echo "[OK] Configured git core.hooksPath = Tools/git-hooks"

# 2. Ensure hooks are executable
if [ -f Tools/git-hooks/pre-push ]; then
    chmod +x Tools/git-hooks/pre-push
    echo "[OK] Ensured Tools/git-hooks/pre-push is executable"
fi

echo "============================================================"
echo "Setup completed successfully."
echo "============================================================"
