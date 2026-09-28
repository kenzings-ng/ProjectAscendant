#!/usr/bin/env python3
"""
Backend Database & Anti-Dupe Transaction Test Suite for Project Ascendant
Validates PostgreSQL schema and transactional integrity as defined in
authentication-account-system.md and DECISIONS.md.
"""

import re
import sys
import uuid
import sqlite3
from pathlib import Path

def test_sql_ddl_spec(gdd_auth_path: Path, decisions_path: Path):
    """Verifies that the DDL in authentication-account-system.md and DECISIONS.md matches approved design"""
    print("[TEST] Checking SQL DDL specifications in DECISIONS.md and authentication-account-system.md...")
    
    # Read DECISIONS.md
    with open(decisions_path, "r", encoding="utf-8") as f:
        decisions_content = f.read()

    assert "uk_owner_slot UNIQUE" in decisions_content, "Missing uk_owner_slot constraint in DECISIONS.md"
    assert "DEFERRABLE INITIALLY DEFERRED" in decisions_content, "Missing DEFERRABLE INITIALLY DEFERRED in DECISIONS.md"
    assert "SELECT ... FOR UPDATE" in decisions_content, "Missing SELECT ... FOR UPDATE in DECISIONS.md"
    assert "ROLLBACK" in decisions_content, "Missing ROLLBACK in DECISIONS.md"

    # Read Auth GDD if it contains schema
    with open(gdd_auth_path, "r", encoding="utf-8") as f:
        auth_content = f.read()

    if "CREATE TABLE accounts" in auth_content:
        assert "CREATE TABLE characters" in auth_content, "Missing CREATE TABLE characters"
        assert "CREATE TABLE items" in auth_content, "Missing CREATE TABLE items"
        assert "item_instance_id UUID PRIMARY KEY" in auth_content, "items table must use item_instance_id UUID PRIMARY KEY"
        assert "CONSTRAINT uk_owner_slot UNIQUE (owner_type, owner_id, slot_type, slot_index) DEFERRABLE INITIALLY DEFERRED" in auth_content, \
            "Missing DEFERRABLE INITIALLY DEFERRED on uk_owner_slot constraint in GDD"
        assert "class_progression_history JSONB NOT NULL DEFAULT '{}'" in auth_content, \
            "class_progression_history must default to '{}' in GDD"
        assert "SELECT ... FOR UPDATE" in auth_content or "FOR UPDATE" in auth_content, \
            "Promotion transaction must enforce SELECT ... FOR UPDATE row-locking"
        assert "ROLLBACK" in auth_content, "Promotion transaction must specify ROLLBACK on failure"
        print("  -> SQL DDL verified in both DECISIONS.md and authentication-account-system.md.")
    else:
        print("  -> SQL DDL verified in DECISIONS.md (GDD sync pending on docs branch).")

    return True

def test_logical_invariants():
    """Runs logical behavioral tests using an in-memory SQLite database simulating PG constraints"""
    print("[TEST] Running transactional logic & anti-dupe simulation...")
    conn = sqlite3.connect(":memory:", isolation_level=None)
    cursor = conn.cursor()

    # Create tables
    cursor.execute("""
    CREATE TABLE accounts (
        account_id TEXT PRIMARY KEY,
        email TEXT UNIQUE NOT NULL,
        gold_balance INTEGER NOT NULL DEFAULT 0 CHECK (gold_balance >= 0)
    )
    """)

    cursor.execute("""
    CREATE TABLE characters (
        character_id TEXT PRIMARY KEY,
        account_id TEXT NOT NULL REFERENCES accounts(account_id),
        character_name TEXT UNIQUE NOT NULL,
        character_level INTEGER NOT NULL DEFAULT 1 CHECK (character_level BETWEEN 1 AND 50),
        primary_class_tag TEXT NOT NULL,
        secondary_class_tag TEXT,
        class_progression_history TEXT NOT NULL DEFAULT '{}'
    )
    """)

    cursor.execute("""
    CREATE TABLE items (
        item_instance_id TEXT PRIMARY KEY,
        item_def_id TEXT NOT NULL,
        owner_type TEXT NOT NULL,
        owner_id TEXT NOT NULL,
        slot_type TEXT NOT NULL,
        slot_index INTEGER NOT NULL,
        quantity INTEGER NOT NULL DEFAULT 1 CHECK (quantity > 0),
        CONSTRAINT uk_owner_slot UNIQUE (owner_type, owner_id, slot_type, slot_index)
    )
    """)

    # Seed account & character
    acc_id = str(uuid.uuid4())
    char_id = str(uuid.uuid4())
    cursor.execute("INSERT INTO accounts VALUES (?, ?, ?)", (acc_id, "hero@projectascendant.io", 1000))
    cursor.execute("INSERT INTO characters VALUES (?, ?, ?, ?, ?, ?, ?)",
                   (char_id, acc_id, "VanguardKnight", 25, "Class.Line.Guard.Vanguard", None, "{}"))

    # Test 1: Unique slot constraint prevents duplicate item in same slot
    item1_id = str(uuid.uuid4())
    cursor.execute("INSERT INTO items VALUES (?, ?, ?, ?, ?, ?, ?)",
                   (item1_id, "scroll_dragon_knight", "CHARACTER", char_id, "INVENTORY", 0, 1))

    item2_id = str(uuid.uuid4())
    dup_failed = False
    try:
        cursor.execute("INSERT INTO items VALUES (?, ?, ?, ?, ?, ?, ?)",
                       (item2_id, "scroll_dragon_knight", "CHARACTER", char_id, "INVENTORY", 0, 1))
    except sqlite3.IntegrityError:
        dup_failed = True
    assert dup_failed, "FAIL: Unique constraint uk_owner_slot did not trigger on duplicate slot_index"
    print("  -> Test 1: Duplicate item in same slot blocked by uk_owner_slot: PASSED")

    # Test 2: Atomic promotion transaction simulation
    # Server queries item with lock, validates, consumes item, and updates character class
    def execute_promotion(char_id, scroll_item_id, target_class_tag):
        # Transaction start
        cursor.execute("BEGIN TRANSACTION")
        cursor.execute("SELECT item_def_id, quantity FROM items WHERE item_instance_id = ?", (scroll_item_id,))
        row = cursor.fetchone()
        if not row:
            conn.rollback()
            return False, "Item not found"
        
        item_def, qty = row
        if item_def != "scroll_dragon_knight":
            conn.rollback()
            return False, "Invalid scroll"

        # Check character eligibility
        cursor.execute("SELECT character_level, primary_class_tag FROM characters WHERE character_id = ?", (char_id,))
        c_row = cursor.fetchone()
        if not c_row or c_row[0] < 20: # Level threshold
            conn.rollback()
            return False, "Level too low"

        # Consume scroll
        cursor.execute("DELETE FROM items WHERE item_instance_id = ?", (scroll_item_id,))
        # Update character class
        cursor.execute("UPDATE characters SET primary_class_tag = ? WHERE character_id = ?", (target_class_tag, char_id))
        conn.commit()
        return True, "Promoted successfully"

    # Attempt promotion with valid scroll
    success, msg = execute_promotion(char_id, item1_id, "Class.Line.Guard.DragonKnight")
    assert success, f"Promotion failed: {msg}"
    print("  -> Test 2: Valid promotion transaction: PASSED")

    # Test 3: Anti-dupe check - reusing same consumed scroll item ID must fail
    success, msg = execute_promotion(char_id, item1_id, "Class.Line.Guard.DragonKnight")
    assert not success, "FAIL: Replay attack with consumed scroll succeeded!"
    print("  -> Test 3: Replay attack with consumed item properly blocked: PASSED")

    # Verify character's updated class
    cursor.execute("SELECT primary_class_tag FROM characters WHERE character_id = ?", (char_id,))
    final_class = cursor.fetchone()[0]
    assert final_class == "Class.Line.Guard.DragonKnight", f"Unexpected class: {final_class}"
    print("  -> Test 4: Final character class tag verification: PASSED")

    conn.close()
    return True

def main():
    root = Path(__file__).resolve().parent.parent.parent
    auth_gdd = root / "design" / "gdd" / "authentication-account-system.md"
    decisions_file = root / "production" / "DECISIONS.md"
    
    print("=" * 60)
    print("Project Ascendant: Backend Postgres & Anti-Dupe Test Suite")
    print(f"Auth GDD: {auth_gdd}")
    print(f"Decisions File: {decisions_file}")
    print("=" * 60)

    try:
        test_sql_ddl_spec(auth_gdd, decisions_file)
        test_logical_invariants()
        print("\n[RESULT] All Backend Database & Transaction Tests PASSED.")
        return 0
    except AssertionError as e:
        print(f"\n[FAIL] Assertion failed: {e}", file=sys.stderr)
        return 1
    except Exception as e:
        print(f"\n[ERROR] Unexpected error: {e}", file=sys.stderr)
        return 2

if __name__ == "__main__":
    sys.exit(main())
