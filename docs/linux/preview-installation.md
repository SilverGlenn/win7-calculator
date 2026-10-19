# User-local preview installation

The operator requested installation of the current build before the remaining legacy UI work on 2026-10-19. This is an explicitly requested **preview**, not completion of the full design or the final packaging/review task.

## Installed payload

- Source commit: `76f8c92f6213d2edb05ab548c59d8c000b82ffa1`.
- Build: Release; all nine current CTest processes passed immediately before deployment.
- Executable: `~/.local/lib/win7-calculator/win7-calculator` (a real copied binary, not a build-tree link).
- Command: `~/.local/bin/win7-calculator`, symlinked to that installed executable.
- Start-menu entry: `~/.local/share/applications/org.win7calculator.Calculator.desktop`, named **Calculator**.
- Licenses, installation record and uninstall instructions: `~/.local/lib/win7-calculator/`.
- Diagnostic log: `~/.local/state/win7-calculator/preview.log`.

The desktop entry uses an absolute user-local Exec path, the semantic `accessories-calculator` icon and the application's `org.win7calculator.Calculator` identity. The desktop file passed `desktop-file-validate`; user-local desktop/KDE application caches were refreshed. No system calculator/default application, global theme, font configuration or system package was changed.

The installed binary's SHA-256 is `3bed1740635e96cb1022214e3de0e3142cb48bafdbd9f5410ecc10d3b8569b3b`. `INSTALL.json` records the payload checksums and source revision. All runtime resources are embedded; launching through the desktop entry from `/tmp` verified independence from the source/build working directory.

## Live verification

A detached desktop-entry launch was verified in a separate tool invocation: the process executable resolved to the installed copy, and KWin reported caption `Calculator`, desktop identity `org.win7calculator.Calculator`, a real native frame (`noBorder: false`), taskbar visibility, and outer size 228×322. An initial non-detached launcher process did not remain alive after its command session ended; the verified launch uses an independent session rather than relying on a build/tool process remaining open.

Standard, Scientific, Programmer and Statistics are available. History, unit conversion, date calculation and worksheets remain disabled. Final visual calibration and the matching numeric font are pending. This installation is not a pixel-perfect acceptance claim.

## Future updates

The remaining Task 12 work still owns portable installation/update/uninstall scripts, clean-environment artifact tests, CI and the final whole-branch review. Preserve this preview's paths/identity when adding those tools. Stage replacement binaries and rename atomically: do not truncate a running executable or forcibly restart a calculator the operator may be using. An already running process may need a normal close/reopen to pick up a later update.
