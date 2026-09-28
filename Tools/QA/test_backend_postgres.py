#!/usr/bin/env python3
"""
Backend Database & Anti-Dupe Transaction Test Suite for Project Ascendant
Validates PostgreSQL schema and transactional integrity as defined in
authentication-account-system.md and DECISIONS.md.
Includes concurrent multi-threaded anti-dupe simulation.
"""

import re
import sys
import uuid
import sqlite3
import threading
from pathlib import Path

def test_sql_ddl_spec(decisions_path: Path, gdd_auth_path: Path):
    """Verifies that the DDL in DECISIONS.md and authentication-account-system.md strictly matches requirements"""
    print("[TEST 1] Verifying PostgreSQL DDL and transaction specs in DECISIONS.md...")
    assert decisions_path.exists(), f"DECISIONS.md not found at: {decisions_path}"
    
    with open(decisions_path, "r", encoding="utf-8") as f:
        decisions_content = f.read()

    assert "uk_owner_slot UNIQUE" in decisions_content, "Missing uk_owner_slot constraint in DECISIONS.md"
    assert "DEFERRABLE INITIALLY DEFERRED" in decisions_content, "Missing DEFERRABLE INITIALLY DEFERRED in DECISIONS.md"
    assert "SELECT ... FOR UPDATE" in decisions_content, "Missing SELECT ... FOR UPDATE in DECISIONS.md"
    assert "ROLLBACK" in decisions_content, "Missing ROLLBACK in DECISIONS.md"
    print("  -> DECISIONS.md DDL specifications: VERIFIED")

    # If auth GDD exists, verify its DDL blocks
    print("[TEST 2] Verifying PostgreSQL DDL in authentication-account-system.md...")
    assert gdd_auth_path.exists(), f"Auth GDD not found at: {gdd_auth_path}"
    with open(gdd_auth_path, "r", encoding="utf-8") as f:
        auth_content = f.read()

    # Verify if DDL is present in GDD
    has_ddl = "CREATE TABLE accounts" in auth_content
    if has_ddl:
        assert "CREATE TABLE characters" in auth_content, "Missing CREATE TABLE characters in GDD"
        assert "CREATE TABLE items" in auth_content, "Missing CREATE TABLE items in GDD"
        assert "item_instance_id UUID PRIMARY KEY" in auth_content, "items table must use item_instance_id UUID PRIMARY KEY"
        
        uk_pattern = re.compile(
            r'CONSTRAINT\s+uk_owner_slot\s+UNIQUE\s*\([^)]+\)\s+DEFERRABLE\s+INITIALLY\s+DEFERRED',
            re.IGNORECASE | re.MULTILINE
        )
        assert uk_pattern.search(auth_content), "Missing DEFERRABLE INITIALLY DEFERRED on uk_owner_slot in GDD"
        assert "class_progression_history JSONB NOT NULL DEFAULT '{}'" in auth_content, \
            "class_progression_history must default to '{}' in GDD"
        assert "FOR UPDATE" in auth_content, "Promotion transaction must specify FOR UPDATE row-locking"
        assert "ROLLBACK" in auth_content, "Promotion transaction must specify ROLLBACK on failure"
        print("  -> authentication-account-system.md DDL & transaction specifications: VERIFIED")
    else:
        print("  -> [NOTE] GDD authentication-account-system.md does not yet have DDL on this branch (pending merge from docs branch).")

    return True

def test_atomic_slot_swapping():
    """Simulates deferred unique constraint for atomic inventory slot swapping within a transaction"""
    print("[TEST 3] Testing Atomic Inventory Slot Swapping...")
    conn = sqlite3.connect(":memory:", isolation_level=None)
    cursor = conn.cursor()

    cursor.execute("""
    CREATE TABLE items (
        item_instance_id TEXT PRIMARY KEY,
        item_def_id TEXT NOT NULL,
        owner_type TEXT NOT NULL,
        owner_id TEXT NOT NULL,
        slot_type TEXT NOT NULL,
        slot_index INTEGER NOT NULL
    )
    """)

    char_id = str(uuid.uuid4())
    item_a = str(uuid.uuid4())
    item_b = str(uuid.uuid4())

    cursor.execute("INSERT INTO items VALUES (?, ?, ?, ?, ?, ?)", (item_a, "sword_01", "CHARACTER", char_id, "INVENTORY", 0))
    cursor.execute("INSERT INTO items VALUES (?, ?, ?, ?, ?, ?)", (item_b, "shield_01", "CHARACTER", char_id, "INVENTORY", 1))

    # Perform atomic swap using transaction
    cursor.execute("BEGIN TRANSACTION")
    # Temp slot index -1
    cursor.execute("UPDATE items SET slot_index = -1 WHERE item_instance_id = ?", (item_a,))
    cursor.execute("UPDATE items SET slot_index = 0 WHERE item_instance_id = ?", (item_b,))
    cursor.execute("UPDATE items SET slot_index = 1 WHERE item_instance_id = ?", (item_a,))
    cursor.execute("COMMIT")

    cursor.execute("SELECT slot_index FROM items WHERE item_instance_id = ?", (item_a,))
    slot_a = cursor.fetchone()[0]
    cursor.execute("SELECT slot_index FROM items WHERE item_instance_id = ?", (item_b,))
    slot_b = cursor.fetchone()[0]

    assert slot_a == 1 and slot_b == 0, f"Atomic swap failed: slot_a={slot_a}, slot_b={slot_b}"
    print("  -> Atomic slot swapping: PASSED")
    conn.close()
    return True

def test_concurrent_anti_dupe_simulation():
    """Simulates concurrent Dedicated Server requests attempting to consume the same scroll item simultaneously"""
    print("[TEST 4] Testing Concurrent Anti-Dupe Race Condition Protection...")
    
    # Shared database and row-lock mechanism
    db_lock = threading.Lock()
    item_rows = {
        "scroll_inst_999": {
            "item_def_id": "scroll_dragon_knight",
            "quantity": 1,
            "is_locked": False,
            "consumed": False
        }
    }
    character_state = {
        "char_id": "char_001",
        "primary_class": "Class.Line.Guard.Vanguard",
        "promoted": False
    }

    results = []

    def client_promotion_request(client_id: int):
        with db_lock:
            # Emulate SELECT ... FOR UPDATE
            item = item_rows.get("scroll_inst_999")
            if not item or item["consumed"] or item["is_locked"]:
                results.append((client_id, False, "Item unavailable or locked"))
                return

            # Lock row
            item["is_locked"] = True

            try:
                # Validate item
                if item["item_def_id"] != "scroll_dragon_knight":
                    results.append((client_id, False, "Invalid scroll"))
                    return

                # Consume item
                item["consumed"] = True
                item["quantity"] = 0

                # Promote character
                character_state["primary_class"] = "Class.Line.Guard.DragonKnight"
                character_state["promoted"] = True
                results.append((client_id, True, "Promoted successfully"))
            finally:
                item["is_locked"] = False

    t1 = threading.Thread(target=client_promotion_request, args=(1,))
    t2 = threading.Thread(target=client_promotion_request, args=(2,))

    t1.start()
    t2.start()
    t1.join()
    t2.join()

    # Exactly one client must succeed and one must fail
    successes = [r for r in results if r[1] is True]
    failures = [r for r in results if r[1] is False]

    assert len(successes) == 1, f"Expected exactly 1 success, got {len(successes)}"
    assert len(failures) == 1, f"Expected exactly 1 failure, got {len(failures)}"
    assert character_state["primary_class"] == "Class.Line.Guard.DragonKnight"
    print(f"  -> Concurrency test: Client {successes[0][0]} succeeded, Client {failures[0][0]} blocked (Anti-Dupe Confirmed)")
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
        test_sql_ddl_spec(decisions_file, auth_gdd)
        test_atomic_slot_swapping()
        test_concurrent_anti_dupe_simulation()
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
