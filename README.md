# coron44bt firmware

Source: https://github.com/aki27kbd/zmk-config-coron44bt

The `codex/studio-usb` branch builds the manufacturer's main keymap with USB
support for ZMK Studio and Studio locking disabled. The trackball configuration
and key assignments are retained from upstream `main`.

## Pinned dependencies

- ZMK v0.3.0: `edf5c0814fd3ea202e43aad2d68fd32e882a518c`
- PMW3610 driver: `941441d9faf199cd97c96f7ef163bc2d746e052d`

The workflow uses the same ZMK revision as the firmware source. The coron44bt
build also explicitly sets `CONFIG_ZMK_STUDIO=y` and
`CONFIG_ZMK_STUDIO_LOCKING=n`.

## Build on GitHub Actions

From PowerShell, after signing into GitHub CLI with the repository owner's
account:

```powershell
Set-Location 'C:\Users\arata\kbd\zmk-config-coron44bt'
$gh = 'C:\Users\arata\kbd\tools\github-cli\bin\gh.exe'
& $gh workflow run build.yml --repo aki27kbd/zmk-config-coron44bt --ref codex/studio-usb
& $gh run list --repo aki27kbd/zmk-config-coron44bt --branch codex/studio-usb --limit 5
```

Download the `firmware` artifact from the successful run. The keyboard firmware
is `coron44bt-seeeduino_xiao_ble-zmk.uf2`. The separate `settings_reset` image
erases stored settings and is not the keyboard firmware.

## Install and edit

Double-press the keyboard's reset button to enter the UF2 bootloader. Copy the
coron44bt UF2 file to the bootloader's USB drive. The keyboard restarts when the
copy finishes.

Open https://zmk.studio/ in Chrome or Edge, choose USB, and connect to coron44bt.
This build does not require a Studio unlock key. Save changes in Studio to keep
them across restarts.

Previously saved Studio keymaps can override the compiled keymap. Use
`Restore Stock Settings` only when you intentionally want to replace your saved
Studio keymap with the keymap compiled into this firmware.
