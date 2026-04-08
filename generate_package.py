#!/usr/bin/env python3
"""
generate_package.py — hexiOS Board Manager package generator

LOCAL (testing only):
    python generate_package.py
    → uses file:/// URL (only works on your machine)

GITHUB (for sharing with others):
    python generate_package.py --github yourusername/hexy-board
    → uses https://github.com/.../releases/download/... URLs
    → package_hexy_index.json should be hosted at:
      https://raw.githubusercontent.com/yourusername/hexy-board/main/package_hexy_index.json

Workflow for publishing:
    1. python generate_package.py --github yourusername/hexy-board
    2. git add package_hexy_index.json && git commit -m "release v1.0.0" && git push
    3. Create a GitHub Release tagged v1.0.0, attach hexy-platform-1.0.0.zip as an asset
    4. Users add this URL to Arduino IDE > File > Preferences > Additional boards manager URLs:
          https://raw.githubusercontent.com/yourusername/hexy-board/main/package_hexy_index.json
"""

import argparse
import hashlib
import json
import sys
import zipfile
from pathlib import Path

# ── Config ────────────────────────────────────────────────────────────────────
BOARD_NAME    = "hexiOS (HEXY App Platform)"
MAINTAINER    = "hexiOS"
BOARD_ID      = "hexy"
VERSION       = "1.0.3"
ARCHITECTURE  = "esp32"
PACKAGE_DIR   = Path(__file__).parent / "package"
OUTPUT_DIR    = Path(__file__).parent
ZIP_NAME      = f"hexy-platform-{VERSION}.zip"
INDEX_NAME    = "package_hexy_index.json"

# ── Helpers ───────────────────────────────────────────────────────────────────

def zip_directory(src: Path, dst: Path) -> None:
    with zipfile.ZipFile(dst, "w", zipfile.ZIP_DEFLATED) as zf:
        for file in sorted(src.rglob("*")):
            if file.is_file():
                arcname = Path(BOARD_ID) / file.relative_to(src)
                zf.write(file, arcname)
    print(f"  Created : {dst.name}  ({dst.stat().st_size:,} bytes)")


def sha256_of(path: Path) -> str:
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def github_zip_url(repo: str, version: str, zip_name: str) -> str:
    """https://github.com/<repo>/releases/download/v<version>/<zip_name>"""
    return f"https://github.com/{repo}/releases/download/v{version}/{zip_name}"


def github_index_url(repo: str) -> str:
    """https://raw.githubusercontent.com/<repo>/main/<INDEX_NAME>"""
    return f"https://raw.githubusercontent.com/{repo}/main/{INDEX_NAME}"


# ── Main ──────────────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(description="hexiOS Board Manager package generator")
    parser.add_argument(
        "--github", metavar="USER/REPO",
        help="GitHub repo (e.g. yourusername/hexy-board). "
             "Generates public https:// URLs for distribution. "
             "Without this flag, generates local file:/// URLs for testing only."
    )
    args = parser.parse_args()

    if not PACKAGE_DIR.is_dir():
        print(f"ERROR: package/ directory not found at {PACKAGE_DIR}", file=sys.stderr)
        sys.exit(1)

    zip_path   = OUTPUT_DIR / ZIP_NAME
    index_path = OUTPUT_DIR / INDEX_NAME

    print("hexiOS Board Manager package generator")
    print("=" * 45)
    if args.github:
        print(f"  Mode    : GitHub release  ({args.github})")
    else:
        print("  Mode    : Local file:/// (testing only)")
    print()

    # 1. Build ZIP
    print(f"[1/3] Zipping {PACKAGE_DIR.name}/ ...")
    zip_directory(PACKAGE_DIR, zip_path)

    # 2. Compute checksum / size
    print("[2/3] Computing SHA-256 ...")
    checksum = sha256_of(zip_path)
    size     = zip_path.stat().st_size
    print(f"  SHA-256 : {checksum}")
    print(f"  Size    : {size:,} bytes")

    # 3. Determine URLs
    if args.github:
        zip_url   = github_zip_url(args.github, VERSION, ZIP_NAME)
        index_url = github_index_url(args.github)
    else:
        # Local — Path.as_uri() handles Windows drive letters, spaces, parens
        zip_url   = zip_path.resolve().as_uri()
        index_url = index_path.resolve().as_uri()

    print(f"  ZIP URL : {zip_url}")

    # 4. Write package index
    print("[3/3] Writing package_hexy_index.json ...")
    index = {
        "packages": [
            {
                "name": MAINTAINER,
                "maintainer": MAINTAINER,
                "websiteURL": "",
                "email": "",
                "help": {"online": ""},
                "platforms": [
                    {
                        "name": BOARD_NAME,
                        "architecture": ARCHITECTURE,
                        "version": VERSION,
                        "category": "Contributed",
                        "help": {"online": ""},
                        "url": zip_url,
                        "archiveFileName": ZIP_NAME,
                        "checksum": f"SHA-256:{checksum}",
                        "size": str(size),
                        "boards": [{"name": "HEXY (hexiOS App)"}],
                        "toolsDependencies": [
                            {"packager": "esp32", "name": "esp-x32",              "version": "2511"},
                            {"packager": "esp32", "name": "esptool_py",           "version": "5.1.0"},
                            {"packager": "esp32", "name": "esp32s3-libs",         "version": "3.3.7"},
                            {"packager": "esp32", "name": "mkspiffs",             "version": "0.2.3"},
                            {"packager": "esp32", "name": "mklittlefs",           "version": "4.0.2-db0513a"},
                        ]
                    }
                ],
                "tools": []
            }
        ]
    }

    with open(index_path, "w", encoding="utf-8") as f:
        json.dump(index, f, indent=2)
    print(f"  Created : {index_path.name}")

    # ── Done — print next steps ───────────────────────────────────────────────
    print()
    print("=" * 45)

    if args.github:
        print("GITHUB RELEASE STEPS:")
        print()
        print("  1. Commit and push package_hexy_index.json:")
        print(f"       git add {INDEX_NAME}")
        print(f"       git commit -m \"release v{VERSION}\"")
        print("       git push")
        print()
        print(f"  2. Create a GitHub Release tagged  v{VERSION}")
        print(f"     Attach this file as a release asset:")
        print(f"       {zip_path.resolve()}")
        print()
        print("  3. Users add this URL to Arduino IDE:")
        print("       File > Preferences > Additional boards manager URLs:")
        print()
        print(f"       {index_url}")
        print()
        print("  4. Users: Tools > Board > Boards Manager > search HEXY > Install")
    else:
        print("LOCAL TEST — add this URL to Arduino IDE (your machine only):")
        print()
        print(f"  {index_url}")
        print()
        print("  File > Preferences > Additional boards manager URLs")
        print("  Then: Tools > Board > Boards Manager > search HEXY > Install")
        print()
        print("  To share with others, run:")
        print(f"    python generate_package.py --github yourusername/hexy-board")

    print()
    print("Also install the hexiOS library:")
    print("  Sketch > Include Library > Add .ZIP Library")
    print(f"  Select the hexiOS/ folder: {(OUTPUT_DIR / 'hexiOS').resolve()}")


if __name__ == "__main__":
    main()
