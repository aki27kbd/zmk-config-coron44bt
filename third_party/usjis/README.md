# US symbols on a Windows JIS host

The MIT-licensed resolver and event filter are adapted from
[goyamamoto/zmk-kb1-usjis at 3a0f2b0](https://github.com/goyamamoto/zmk-kb1-usjis/tree/3a0f2b03aa994ddd42a6655fcf776b718dc58d1f).
The complete copyright and permission notice is in [LICENSE](LICENSE).
No Keychron firmware or core source is included.

The original 20-row substitution table and press-record policy are retained.
Changes for this keyboard:

- Gate translation on `zmk_oskey_get_os() == OSKEY_OS_WINDOWS`, on every layer.
- Use current ZMK's `zmk_endpoint_send_report()` and parameterless SYS_INIT.
- Use stack-owned current ZMK events; resume dispatch after hid_listener.
- Enable translation by default; omit the Keychron toggle behavior and board Kconfig.

This assumes Windows is set to Japanese/JIS and macOS/Linux to US. Bluetooth
profile OS overrides select the policy; no Windows settings are changed.
Existing `&kp` symbol bindings and shifted numbers are translated together.
Do not replace bindings with JIS usages: that would translate them twice.

Modifiers are global in HID. If multiple keys that need different Shift states
overlap, the latest held key determines the report's modifiers. Press records
restore the earlier key's Shift when the later key is released. OS changes do
not reinterpret held keys; endpoint HID clears invalidate their held output.

`tests/usjis` tests this filter against the firmware's pinned ZMK HID and event
manager. It records reports at the transport boundary; it does not emulate
Windows, IME conversion, Bluetooth timing, or the physical keyboard.
