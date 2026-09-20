#!/usr/bin/env python3
"""
Anaconda XB 360 - Deploy to Console
====================================

Uploads the built .xex to a modded Xbox 360 over FTP.

Usage:
    python Tools/deploy/deploy.py <console-ip>
    python Tools/deploy/deploy.py 192.168.1.50 --user xbox --pass xbox
    python Tools/deploy/deploy.py 192.168.1.50 --xex Build/AnacondaXB360.xex

Defaults:
    user      = xbox
    password  = xbox
    xex       = Build/AnacondaXB360.xex
    remote    = /Hdd1/Apps/AnacondaXB360/AnacondaXB360.xex

Requires:
    pip install ftplib2       (only for FTPS; plain FTP uses stdlib)
"""

import argparse
import ftplib
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]

DEFAULT_USER   = "xbox"
DEFAULT_PASS   = "xbox"
DEFAULT_XEX    = REPO_ROOT / "Build" / "AnacondaXB360.xex"
DEFAULT_REMOTE = "/Hdd1/Apps/AnacondaXB360/AnacondaXB360.xex"

# ------------------------------------------------------------
# Helpers
# ------------------------------------------------------------

def ensure_remote_dir(ftp: ftplib.FTP, path: str):
    """Recursively create the remote directory for a file path."""
    parts = [p for p in path.split("/") if p]
    # Drop the filename
    parts = parts[:-1]

    ftp.cwd("/")
    for part in parts:
        try:
            ftp.cwd(part)
        except ftplib.error_perm:
            ftp.mkd(part)
            ftp.cwd(part)


def upload(ip: str, user: str, password: str,
           local_xex: Path, remote_path: str):
    if not local_xex.exists():
        print(f"Error: {local_xex} not found.")
        print("Build the project first, or pass --xex <path>.")
        return 1

    size_mb = local_xex.stat().st_size / (1024 * 1024)
    print(f"Connecting to {ip} ...")

    try:
        ftp = ftplib.FTP(ip, timeout=15)
        ftp.login(user, password)
    except Exception as e:
        print(f"Could not connect: {e}")
        return 1

    print(f"Connected. Preparing {remote_path} ...")

    try:
        ensure_remote_dir(ftp, remote_path)
        ftp.cwd("/")

        print(f"Uploading {local_xex.name} ({size_mb:.2f} MB) ...")

        with open(local_xex, "rb") as f:
            ftp.storbinary(f"STOR {remote_path}", f)

        print("Upload complete.")
        print(f"Installed to: {remote_path}")
        print()
        print("On the console: launch Aurora and open Anaconda XB 360.")

    except Exception as e:
        print(f"Upload failed: {e}")
        ftp.close()
        return 1

    ftp.close()
    return 0

# ------------------------------------------------------------
# Main
# ------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Deploy Anaconda XB 360 to a modded Xbox 360"
    )
    parser.add_argument("ip", help="Console IP address")
    parser.add_argument("--user", default=DEFAULT_USER,
                        help=f"FTP username (default: {DEFAULT_USER})")
    parser.add_argument("--pass", dest="password", default=DEFAULT_PASS,
                        help=f"FTP password (default: {DEFAULT_PASS})")
    parser.add_argument("--xex", default=str(DEFAULT_XEX),
                        help=f"Local .xex file (default: {DEFAULT_XEX})")
    parser.add_argument("--remote", default=DEFAULT_REMOTE,
                        help=f"Remote path (default: {DEFAULT_REMOTE})")

    args = parser.parse_args()

    return upload(
        ip=args.ip,
        user=args.user,
        password=args.password,
        local_xex=Path(args.xex),
        remote_path=args.remote,
    )


if __name__ == "__main__":
    sys.exit(main())
