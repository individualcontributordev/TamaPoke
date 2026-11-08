# TamaPoke web installers

## Current public installer

**TamaPoke Gentle** (this fork: the Pokémon never leaves, needs decay slowly):

<https://individualcontributor.dev/TamaPoke/>

`index.html` and `manifest.json` serve firmware `1.36.3-gentle` with Pokémon
#1–386. They expose no debug controls. Step 2 (sprites) is needed only for a new
or empty microSD; a firmware update keeps the save with **Erase device** off.

GitHub Actions builds and publishes the site on every push to `main`
(`.github/workflows/pages.yml` → `tools/build_pages.sh`). The firmware parts are
compiled in CI and are not committed; the page, manifest and sprite packs come
from this folder. Pages source must be set to **GitHub Actions** in the repo
settings.

## Gen‑3 pages

- `dev.html` + `manifest-local.json`: `1.36.2-hoenn-walk-local`, compiled with
  `TAMAPOKE_LOCAL_TEST`. It includes click-driven Pokémon, evolution, battle,
  IMU and step diagnostics.
- `release-gen3.html` + `manifest-gen3.json`: local reference copy of the same
  public-style `1.36.2` page, compiled with `TAMAPOKE_GEN3_RELEASE`.

The release preview deliberately exposes only one sprite action. It transfers
`sprites.pak` and then `sprites-gen3-update.pak`, giving every user the complete
#1–386 set. Firmware flashing without this second step is an incomplete Gen‑3
installation.

There is no `sprites-gen3-full.pak`. The former combined file exceeded 100 MB;
the two-package sequence keeps every GitHub-hosted file below the limit.

## Files

- `sprites.pak`: 502 normal/Shiny files for #1–251 plus 251-entry `thumbs.bin`
  (503 bundle entries).
- `sprites-gen3-update.pak`: 270 normal/Shiny files for #252–386 plus the full
  386-entry `thumbs.bin` (271 bundle entries).
- `firmware/tamapoke-<version>-*.bin`: bootloader, partitions, boot_app0 and app
  parts used by ESP Web Tools without overwriting the NVS save partition.

## Build and validate

```bash
bash tools/build_pages.sh               # CI build -> _site/
bash tools/build_web.sh --check         # local Gen 3 build check
bash tools/build_web_local.sh --check   # Gen 3 debug
bash tools/build_web_gen2_legacy.sh --check # pinned historical Gen 2
```

Omit `--check` to refresh the current public Gen‑3 artifacts. The separate
legacy Gen‑2 command is read-only and never modifies installer files.

Each build validates its firmware version, Dex count, package entry counts and
thumbnail structure/count. The public Gen‑3 build also rejects binaries that
contain local `TESTMON` or `TESTEVO` command strings.

## Test locally

Web Serial and ESP Web Tools require HTTPS or localhost:

```bash
python3 -m http.server 8000 --directory web
```

- <http://127.0.0.1:8000/> — public Gen‑3 page
- <http://127.0.0.1:8000/dev.html> — Gen‑3 debug page
- <http://127.0.0.1:8000/release-gen3.html> — Gen‑3 public preview

Use desktop Chrome or Edge. Close the ESP Web Tools flash dialog before
reconnecting for sprite transfer because only one program can own the serial
port. Leave **Erase device** unchecked when preserving an existing save.

Both Gen‑3 pages use bounded serial reads/writes. A timeout or USB disconnect
cancels the transfer, closes the port and returns the interface to the connect
state so the user can reconnect.

## Release policy

The public flow keeps the single, complete sprite button; step 2 is labelled
"new or empty microSD only". `dev.html` and `manifest-local.json` must never replace
the public page or manifest.

Sprites are from
[PMD SpriteCollab](https://github.com/PMDCollab/SpriteCollab) under CC BY-NC;
see [`../CREDITS.md`](../CREDITS.md).
