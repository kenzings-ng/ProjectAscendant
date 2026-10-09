#!/usr/bin/env python3
"""
Project Ascendant - Real PostgreSQL Backend Database & Anti-Dupe Test Suite
Validates schema, constraints, deferred slot swapping, and concurrent row-locking
against a real PostgreSQL database (via CI service container or local docker compose).
"""

import os
import socket
import sys
import uuid
import time
import threading

# Import PostgreSQL driver (psycopg2 or psycopg3)
try:
    import psycopg2
    from psycopg2 import errors as pg_errors
    PG_DRIVER = "psycopg2"
except ImportError:
    try:
        import psycopg
        from psycopg import errors as pg_errors
        PG_DRIVER = "psycopg"
    except ImportError:
        PG_DRIVER = None

PG_HOST = os.environ.get("PGHOST", os.environ.get("DB_HOST", "localhost"))
PG_PORT = int(os.environ.get("PGPORT", os.environ.get("DB_PORT", "5432")))
PG_USER = os.environ.get("PGUSER", os.environ.get("DB_USER", "postgres"))
PG_PASSWORD = os.environ.get("PGPASSWORD", os.environ.get("DB_PASSWORD", "password"))
PG_DATABASE = os.environ.get("PGDATABASE", os.environ.get("DB_NAME", "project_ascendant"))

# Dedicated exit code: PostgreSQL server is genuinely unreachable (nothing listening /
# host not resolvable). Callers (run_headless_tests.sh) may skip the gate locally ONLY
# on this code. Any other non-zero code means tests ran (or could have run) and failed.
EXIT_SERVER_UNREACHABLE = 3

def is_server_reachable(timeout: float = 3.0) -> bool:
    """Returns True if something accepts connections at PG_HOST:PG_PORT (TCP or unix socket)."""
    if PG_HOST.startswith("/"):
        sock_path = os.path.join(PG_HOST, f".s.PGSQL.{PG_PORT}")
        if not os.path.exists(sock_path):
            return False
        try:
            with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as s:
                s.settimeout(timeout)
                s.connect(sock_path)
            return True
        except OSError:
            return False
    try:
        with socket.create_connection((PG_HOST, PG_PORT), timeout=timeout):
            return True
    except OSError:
        return False

def get_db_connection():
    """Establishes a new dedicated connection to the PostgreSQL database."""
    if PG_DRIVER == "psycopg2":
        return psycopg2.connect(
            host=PG_HOST,
            port=PG_PORT,
            user=PG_USER,
            password=PG_PASSWORD,
            dbname=PG_DATABASE
        )
    elif PG_DRIVER == "psycopg":
        return psycopg.connect(
            host=PG_HOST,
            port=PG_PORT,
            user=PG_USER,
            password=PG_PASSWORD,
            dbname=PG_DATABASE
        )
    else:
        raise RuntimeError("No PostgreSQL driver found. Please install psycopg2-binary: pip install psycopg2-binary")

def setup_schema(conn):
    """Applies canonical PostgreSQL DDL schema with uk_owner_slot DEFERRABLE INITIALLY DEFERRED."""
    with conn.cursor() as cur:
        cur.execute("""
        CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

        DROP TABLE IF EXISTS items CASCADE;
        DROP TABLE IF EXISTS characters CASCADE;
        DROP TABLE IF EXISTS accounts CASCADE;

        -- 1. ACCOUNTS TABLE
        CREATE TABLE accounts (
            account_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
            email VARCHAR(255) UNIQUE NOT NULL,
            password_hash VARCHAR(255) NOT NULL,
            gold_balance BIGINT NOT NULL DEFAULT 0 CHECK (gold_balance >= 0),
            created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW()
        );

        -- 2. CHARACTERS TABLE
        CREATE TABLE characters (
            character_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
            account_id UUID NOT NULL REFERENCES accounts(account_id) ON DELETE CASCADE,
            character_name VARCHAR(64) UNIQUE NOT NULL,
            character_level INT NOT NULL DEFAULT 1 CHECK (character_level BETWEEN 1 AND 50),
            character_exp BIGINT NOT NULL DEFAULT 0 CHECK (character_exp >= 0),
            primary_class_tag VARCHAR(128) NOT NULL,
            secondary_class_tag VARCHAR(128),
            class_progression_history JSONB NOT NULL DEFAULT '{}',
            completed_quests TEXT[] NOT NULL DEFAULT '{}',
            created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW()
        );

        -- 3. ITEMS TABLE (With DEFERRABLE INITIALLY DEFERRED unique constraint)
        CREATE TABLE items (
            item_instance_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
            item_def_id VARCHAR(64) NOT NULL,
            owner_type VARCHAR(16) NOT NULL,
            owner_id UUID NOT NULL,
            slot_type VARCHAR(32) NOT NULL,
            slot_index INT NOT NULL,
            quantity INT NOT NULL DEFAULT 1 CHECK (quantity > 0),
            durability FLOAT NOT NULL DEFAULT 100.0,
            enhancement_level INT NOT NULL DEFAULT 0,
            item_data JSONB NOT NULL DEFAULT '{}',
            b_is_locked BOOLEAN NOT NULL DEFAULT FALSE,
            created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
            CONSTRAINT uk_owner_slot UNIQUE (owner_type, owner_id, slot_type, slot_index) DEFERRABLE INITIALLY DEFERRED
        );
        """)
        conn.commit()
    print("  -> PostgreSQL schema migrated and verified cleanly.")

def test_atomic_slot_swapping(conn):
    """
    Verifies that uk_owner_slot DEFERRABLE INITIALLY DEFERRED permits
    swapping two items within a single transaction without triggering a unique violation.
    """
    print("\n[DB TEST 1] Testing Atomic Inventory Slot Swapping with DEFERRABLE INITIALLY DEFERRED...")
    char_id = str(uuid.uuid4())
    acc_id = str(uuid.uuid4())
    item_a = str(uuid.uuid4())
    item_b = str(uuid.uuid4())

    with conn.cursor() as cur:
        # Seed account & character
        cur.execute("INSERT INTO accounts (account_id, email, password_hash) VALUES (%s, %s, %s);",
                    (acc_id, f"swap_{uuid.uuid4()}@ascendant.io", "hash"))
        cur.execute("INSERT INTO characters (character_id, account_id, character_name, primary_class_tag) VALUES (%s, %s, %s, %s);",
                    (char_id, acc_id, f"Swapper_{uuid.uuid4().hex[:6]}", "Class.Line.Guard.Vanguard"))

        # Seed Item A in slot 0, Item B in slot 1
        cur.execute("""
            INSERT INTO items (item_instance_id, item_def_id, owner_type, owner_id, slot_type, slot_index)
            VALUES (%s, 'sword_iron', 'CHARACTER', %s, 'INVENTORY', 0);
        """, (item_a, char_id))
        cur.execute("""
            INSERT INTO items (item_instance_id, item_def_id, owner_type, owner_id, slot_type, slot_index)
            VALUES (%s, 'shield_square', 'CHARACTER', %s, 'INVENTORY', 1);
        """, (item_b, char_id))
        conn.commit()

        # Step 1: Prove that commiting a real duplicate slot causes UniqueViolation
        collision_detected = False
        try:
            with conn.cursor() as c_col:
                c_col.execute("""
                    INSERT INTO items (item_instance_id, item_def_id, owner_type, owner_id, slot_type, slot_index)
                    VALUES (%s, 'potion_heal', 'CHARACTER', %s, 'INVENTORY', 0);
                """, (str(uuid.uuid4()), char_id))
                conn.commit()
        except Exception:
            conn.rollback()
            collision_detected = True
        assert collision_detected, "FAIL: Real collision did not trigger uk_owner_slot UniqueViolation!"
        print("  -> Collision test: Confirmed uk_owner_slot blocks duplicate slots at COMMIT.")

        # Step 2: Swap Item A and Item B inside a single transaction
        # During the intermediate state, both items occupy slot 1 momentarily!
        # Because constraint is DEFERRABLE INITIALLY DEFERRED, PostgreSQL allows this!
        with conn.cursor() as cur_swap:
            cur_swap.execute("UPDATE items SET slot_index = 1 WHERE item_instance_id = %s;", (item_a,))
            cur_swap.execute("UPDATE items SET slot_index = 0 WHERE item_instance_id = %s;", (item_b,))
            conn.commit()

        # Verify final slots
        cur.execute("SELECT slot_index FROM items WHERE item_instance_id = %s;", (item_a,))
        final_a = cur.fetchone()[0]
        cur.execute("SELECT slot_index FROM items WHERE item_instance_id = %s;", (item_b,))
        final_b = cur.fetchone()[0]

        assert final_a == 1 and final_b == 0, f"Swap failed: final_a={final_a}, final_b={final_b}"
        print("  -> [PASS] Atomic slot swapping successfully deferred until commit.")

def test_concurrent_anti_dupe():
    """
    Opens 2 independent PostgreSQL connections running concurrent transactions with
    SELECT ... FOR UPDATE on the same promotion scroll item.
    Asserts that exactly 1 transaction succeeds and 1 transaction aborts/fails.
    """
    print("\n[DB TEST 2] Testing Concurrent SELECT ... FOR UPDATE Anti-Dupe with 2 Separate PostgreSQL Connections...")
    conn_setup = get_db_connection()
    acc_id = str(uuid.uuid4())
    char_id = str(uuid.uuid4())
    scroll_id = str(uuid.uuid4())

    with conn_setup.cursor() as cur:
        cur.execute("INSERT INTO accounts (account_id, email, password_hash) VALUES (%s, %s, %s);",
                    (acc_id, f"dupe_{uuid.uuid4()}@ascendant.io", "hash"))
        cur.execute("""
            INSERT INTO characters (character_id, account_id, character_name, character_level, primary_class_tag)
            VALUES (%s, %s, %s, 25, 'Class.Line.Guard.Vanguard');
        """, (char_id, acc_id, f"Candidate_{uuid.uuid4().hex[:6]}"))
        cur.execute("""
            INSERT INTO items (item_instance_id, item_def_id, owner_type, owner_id, slot_type, slot_index, quantity)
            VALUES (%s, 'scroll_dragon_knight', 'CHARACTER', %s, 'INVENTORY', 5, 1);
        """, (scroll_id, char_id))
        conn_setup.commit()
    conn_setup.close()

    results = []

    def client_worker(worker_id: int):
        conn = get_db_connection()
        try:
            with conn.cursor() as cur:
                # Start transaction
                cur.execute("BEGIN;")

                # Acquire exclusive row lock
                cur.execute("""
                    SELECT item_instance_id, item_def_id, quantity
                    FROM items
                    WHERE item_instance_id = %s
                    FOR UPDATE;
                """, (scroll_id,))
                row = cur.fetchone()

                # If item already consumed or deleted by concurrent transaction
                if not row or row[2] <= 0:
                    conn.rollback()
                    results.append((worker_id, False, "Item already consumed"))
                    return

                # Simulate work and consume item
                time.sleep(0.05)
                cur.execute("DELETE FROM items WHERE item_instance_id = %s;", (scroll_id,))
                cur.execute("""
                    UPDATE characters
                    SET primary_class_tag = 'Class.Line.Guard.DragonKnight'
                    WHERE character_id = %s;
                """, (char_id,))
                conn.commit()
                results.append((worker_id, True, "Promotion committed"))
        except Exception as e:
            conn.rollback()
            results.append((worker_id, False, str(e)))
        finally:
            conn.close()

    t1 = threading.Thread(target=client_worker, args=(1,))
    t2 = threading.Thread(target=client_worker, args=(2,))

    t1.start()
    t2.start()
    t1.join()
    t2.join()

    successes = [r for r in results if r[1] is True]
    failures = [r for r in results if r[1] is False]

    print(f"  Worker 1 Result: {results[0]}")
    print(f"  Worker 2 Result: {results[1]}")

    assert len(successes) == 1, f"Anti-Dupe Failed: Expected exactly 1 success, got {len(successes)}"
    assert len(failures) == 1, f"Anti-Dupe Failed: Expected exactly 1 failure, got {len(failures)}"

    # Verify character updated in database
    conn_verify = get_db_connection()
    with conn_verify.cursor() as cur:
        cur.execute("SELECT primary_class_tag FROM characters WHERE character_id = %s;", (char_id,))
        final_tag = cur.fetchone()[0]
        cur.execute("SELECT COUNT(*) FROM items WHERE item_instance_id = %s;", (scroll_id,))
        item_count = cur.fetchone()[0]
    conn_verify.close()

    assert final_tag == "Class.Line.Guard.DragonKnight", f"Unexpected tag: {final_tag}"
    assert item_count == 0, f"Scroll item was not consumed: count={item_count}"
    print("  -> [PASS] True PostgreSQL row-locking completely prevented duplicate consumption.")

def main():
    print("=" * 60)
    print("Project Ascendant: Real PostgreSQL Backend Test Suite")
    print(f"Target DB: {PG_USER}@{PG_HOST}:{PG_PORT}/{PG_DATABASE}")
    print("=" * 60)

    if not is_server_reachable():
        print(f"[UNREACHABLE] No PostgreSQL server accepting connections at {PG_HOST}:{PG_PORT}.")
        print("\nTo run locally with Docker:")
        print("  docker compose up -d postgres")
        print("\nIn CI (GitHub Actions):")
        print("  This runs automatically in the 'gates' job via the postgres service container.")
        return EXIT_SERVER_UNREACHABLE

    if PG_DRIVER is None:
        print("[ERROR] Neither psycopg2 nor psycopg is installed.")
        print("Install via: pip install psycopg2-binary")
        return 1

    try:
        conn = get_db_connection()
    except Exception as e:
        # Server is listening but the connection failed (auth, missing database, ...):
        # this is a real failure, not a skip.
        print(f"[FAIL] PostgreSQL is reachable at {PG_HOST}:{PG_PORT} but connecting failed: {e}")
        return 1

    try:
        setup_schema(conn)
        test_atomic_slot_swapping(conn)
        conn.close()
        test_concurrent_anti_dupe()
        print("\n" + "=" * 60)
        print(">> ALL REAL POSTGRESQL DATABASE TESTS PASSED SUCCESSFULLY (Exit 0) <<")
        print("=" * 60)
        return 0
    except AssertionError as e:
        print(f"\n[FAIL] Assertion failed: {e}", file=sys.stderr)
        return 1
    except Exception as e:
        print(f"\n[ERROR] Database test encountered exception: {e}", file=sys.stderr)
        return 2

if __name__ == "__main__":
    sys.exit(main())
