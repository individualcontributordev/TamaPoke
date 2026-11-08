#!/bin/bash
# Builds the public installer site into _site/ (used by .github/workflows/pages.yml).
# Compiles the Gen-3 release firmware, validates it and copies only public files.
set -euo pipefail

cd "$(dirname "$0")/.."
FQBN="esp32:esp32:esp32s3:CDCOnBoot=cdc,FlashSize=16M,PSRAM=opi,PartitionScheme=app3M_fat9M_16MB"
VERSION="$(sed -n 's/^#define FW_VERSION "\(.*-gentle\)"$/\1/p' TamaPoke.ino)"
[[ -n $VERSION ]] || { echo "No -gentle FW_VERSION in TamaPoke.ino" >&2; exit 1; }

BUILD="$(mktemp -d)"
arduino-cli compile --fqbn "$FQBN" \
  --build-property compiler.cpp.extra_flags=-DTAMAPOKE_GEN3_RELEASE \
  --build-path "$BUILD" .

rm -rf _site && mkdir -p _site/firmware
cp "$BUILD/TamaPoke.ino.bootloader.bin" "_site/firmware/tamapoke-$VERSION-bootloader.bin"
cp "$BUILD/TamaPoke.ino.partitions.bin"  "_site/firmware/tamapoke-$VERSION-partitions.bin"
cp "$BUILD/boot_app0.bin"                "_site/firmware/tamapoke-$VERSION-boot_app0.bin"
cp "$BUILD/TamaPoke.ino.bin"             "_site/firmware/tamapoke-$VERSION-app.bin"
# shortcut: explicit allow-list; dev.html, manifest-local.json and old firmware never ship.
cp web/index.html web/installer-gen3.js web/manifest.json web/sprites.pak web/sprites-gen3-update.pak _site/

python3 tools/validate_web_build.py --dex-header dex.h \
  --app "_site/firmware/tamapoke-$VERSION-app.bin" --manifest _site/manifest.json \
  --pak _site/sprites.pak --expected-version "$VERSION" \
  --expected-dex 386 --expected-files 503 --expected-thumbs 251
python3 tools/validate_web_build.py \
  --pak _site/sprites-gen3-update.pak --expected-files 271 --expected-thumbs 386
if strings "_site/firmware/tamapoke-$VERSION-app.bin" | grep -Eq 'TESTMON|TESTEVO'; then
  echo "Debug command strings found in public firmware." >&2
  exit 1
fi
echo "OK: _site ready ($VERSION)"
