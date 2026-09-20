#!/usr/bin/env python3
"""
Anaconda XB 360 - Catalog Watcher
==================================

Watches Catalog/Content/ for changes and runs generate.py whenever
a .7z file is added, modified, or deleted.

Usage:
    python Tools/generate-catalog/watch.py

Press Ctrl+C to stop.

Requires:
    pip install watchdog
"""

import subprocess
import sys
import time
from pathlib import Path

try:
    from watchdog.observers import Observer
    from watchdog.events import FileSystemEventHandler
except ImportError:
    print("Error: watchdog is not installed.")
    print("Install it with:  pip install watchdog")
    sys.exit(1)

HERE = Path(__file__).resolve().parent
REPO_ROOT = HERE.parents[1]

WATCH_DIR = REPO_ROOT / "Catalog" / "Content"
GENERATOR = HERE / "generate.py"

# Debounce: how long to wait after the last file event before running
DEBOUNCE_SECONDS = 2.0


class Handler(FileSystemEventHandler):
    def __init__(self):
        self.last_event = 0.0
        self.pending = False

    def on_any_event(self, event):
        if event.is_directory:
            return
        if not event.src_path.lower().endswith(".7z"):
            return
        self.pending = True
        self.last_event = time.time()


def run_generator():
    print("\nChange detected — regenerating catalog ...")
    result = subprocess.run(
        [sys.executable, str(GENERATOR)],
        cwd=str(REPO_ROOT),
    )
    if result.returncode != 0:
        print("Generator exited with an error.")
    else:
        print("Ready. Watching for changes ...")


def main():
    if not WATCH_DIR.exists():
        print(f"Error: {WATCH_DIR} does not exist")
        return 1

    handler = Handler()
    observer = Observer()
    observer.schedule(handler, str(WATCH_DIR), recursive=True)
    observer.start()

    print(f"Watching {WATCH_DIR}")
    print("Drop a .7z file in any subfolder to regenerate the catalog.")
    print("Press Ctrl+C to stop.\n")

    try:
        while True:
            time.sleep(0.5)
            if handler.pending:
                if time.time() - handler.last_event >= DEBOUNCE_SECONDS:
                    handler.pending = False
                    run_generator()
    except KeyboardInterrupt:
        print("\nStopping watcher ...")
    finally:
        observer.stop()
        observer.join()

    return 0


if __name__ == "__main__":
    sys.exit(main())
