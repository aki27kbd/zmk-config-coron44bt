# coron44bt host-aware trackball firmware

The `codex/host-aware-trackball` branch adds automatic host-specific shortcuts
and runtime trackball configuration. The previous firmware is retained on
`codex/studio-usb`.

## Host-specific keys

- USB and BLE OS detection selects the active oskey profile.
- The Command-position key sends Command on macOS and Ctrl on Windows/Linux.
- Layer 1 screenshot sends Cmd+Shift+4 on macOS and Win+Shift+S on Windows.
- Layer 1 search sends Cmd+Space on macOS and Win+S on Windows.
- Layer 1, second row, far left remains `LANGUAGE_2`.
- Layer 2, second row, far left remains `LANGUAGE_1`.

Layers keep their original numbering. This build changes individual shortcuts
per OS instead of duplicating every layer. Additional per-OS behaviors can be
defined in the same way using oskey.

On the Bluetooth/settings layer (hold both Space/layer keys), Q is Studio
Unlock, W selects Windows manually, E selects macOS, and R selects Linux. A
manual selection lasts until a later OS-detection event. If BLE detection is
incorrect, set a persistent per-profile override at
https://cormoran.github.io/zmk-feature-os-detection/ .

USB detection is heuristic and can miss changes through a KVM that does not
re-enumerate. BLE classification is also heuristic; a saved per-profile
override provides a stable choice for a known host. Unknown OS defaults to Mac.
oskey captures the chosen binding on key-down, so switching OS while holding
a modifier still releases the same modifier.

## Trackball configuration

The cormoran PMW3610 driver reports raw movement; the runtime input processors
perform rotation, speed scaling, automouse and scroll conversion. The legacy
driver's scroll conversion and fixed smoothing are removed from this build.

- Sensor CPI: 1000.
- Pointer: 330-degree rotation, X inverted, speed multiplier 1/2.
- Automouse: layer 4, 50 ms activation, 700 ms deactivation.
- Scroll: layer 5, 330-degree rotation, multiplier 1/32; fractional movement is
  retained instead of discarded after each scroll step.
- Minimum sensor report interval: 12 ms, with movement accumulated between
  reports. The sensor uses the driver's power-saving defaults when inactive.
- Inertia is available but initially disabled; enable it on the `scroll`
  processor after checking the basic direction and speed.

Use https://cormoran.github.io/zmk-module-runtime-input-processor/ or
https://studio.dya.cormoran.works/ to change pointer and scroll settings. These
are unofficial custom Studio RPC extensions; the official ZMK Studio remains
the keymap editor. Use the custom-settings editor at
https://cormoran.github.io/zmk-feature-custom-settings/ for export/import of
the extended settings.

## Versions and installation

Every west dependency is pinned in `config/west.yml`. The ZMK base is
`cormoran/zmk` revision `35bb5dafecd5931c432c74e1b630cf12097671e4`
(`v0.3+custom-studio-protocol`), matching the oskey v0.3.0 branch. The
runtime input processor includes the September 2026 inertia update.

Build this branch using GitHub Actions and install
`coron44bt-host-aware-trackball.uf2`. USB Studio and Studio locking disabled
are explicitly selected in the build configuration.

Saved Studio keymaps override compiled bindings. Preserve your current layout
before choosing `Restore Stock Settings` to load these new shortcut bindings.
No settings reset or firmware installation is performed by the build itself.

Both Windows and Mac hardware validation are required after installation; a
successful build alone does not prove OS classification or trackball direction
on the actual hosts. Keep the previous UF2 available for rollback.

## Previous firmware

Source: https://github.com/aki27kbd/zmk-config-coron44bt

The `codex/studio-usb` branch builds the manufacturer's main keymap with USB
support for ZMK Studio and Studio locking disabled. The trackball configuration
and key assignments are retained from upstream `main`.

## Pinned dependencies

- ZMK v0.3.0: `edf5c0814fd3ea202e43aad2d68fd32e882a518c`
- PMW3610 driver: `7b43dfb9a4c9b37a5351a2111c3d0274da436957`

The driver revision removes a duplicate automouse check that read the movement
values before initialization. The existing check after motion processing is
retained. Driver branch: `codex/fix-automouse-initialization`.

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
