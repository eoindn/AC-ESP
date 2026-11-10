# ExternalHack: AssaultCube external ESP (dissertation test subject)

## What this is
An external ESP for **AssaultCube 1.2.0.2** (NOT 1.3: Don has a working CE table for 1.2,
so local player = `ac_client.exe+0x10F4F4`, health = `+0xF8`; the 1.3 values `0x18AC00`/`0xEC` are wrong here.
The reference repo `../ac-multihack-external` targets 1.3, so use it for structure only, not offsets), written in C++ by Don as part of a final-year
CS dissertation (St Mary's University, module CPS6001). It is a **test subject**: once it's
finished it gets tagged and frozen, and only then does work start on the anti-cheat detector,
which lives in a separate project. Freezing it first is deliberate, so the cheat can't be
tuned to make the detector look good.

- Runs **offline / single-player against bots only**. Never use it on public servers or against real players.
- External only: `OpenProcess` + `ReadProcessMemory`. No DLL injection, no `CreateRemoteThread`.
- Read-only is the target. The ESP shouldn't need `PROCESS_VM_WRITE`.

## Dissertation context (why the details matter)
Research question: for external user-mode cheats, how much of the protection kernel-level
anti-cheat gives can a user-mode detector achieve, and at what cost (detection rate, false
positives, performance overhead, bypassability)? This ESP, small probe programs and 2–3
public open-source cheats will be run against a user-mode detector and a kernel driver
(ObRegisterCallbacks). So:
- **The access mask this ESP requests is an experimental variable.** Don't change it casually,
  and note it in the README.
- The frozen tag's commit hash gets cited in the report. **Don't modify code after the
  `pre-detector` tag**. Make a new tag/branch instead and record why.

Key dates: mid-project presentation 11 Dec 2026; report + artefact + viva 7 May 2027.
Time limits: text ESP by ~10 Nov 2026, overlay boxes + tag by ~20 Nov 2026. If the overlay
runs late, tag the text version and move on.

## Build
- Visual Studio solution `ExternalHack.slnx`, C++ console project in `ExternalHack/`.
- **Build as x86.** AssaultCube's `ac_client.exe` is 32-bit, so a 32-bit tool keeps pointer
  sizes matched (4 bytes). Use explicit `uint32_t` for game pointers anyway, so the intent is clear.
- Use the explicit wide (`W`) Win32 APIs and `L"..."` strings. Don't rely on the project's
  character-set setting.

## Roadmap (build in this order)
1. Find PID by exe name (`CreateToolhelp32Snapshot` / `Process32FirstW`/`NextW`)
2. `OpenProcess` with the minimum access mask
3. Get the module base of `ac_client.exe` (`Module32FirstW`/`NextW`)
4. Read the local player via a static pointer chain (health, position)
5. Read the entity list: player count, each enemy's position/health/team/name → print as a console table (text ESP)
6. Read the view matrix (4x4 floats, static address)
7. World-to-screen: matrix × vec4 → divide by w → NDC → pixels
8. Transparent topmost click-through overlay (`WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST`), double-buffered drawing
9. Boxes from head/feet screen points (height → width ≈ height/2)

Offsets: find/verify them yourself in Cheat Engine for the exact game version. Keep them in one
header (`offsets.h`) with a comment saying which version they're for. Reference-repo offsets
are a sanity check only.

## Code conventions
- **Every handle opened is closed** (snapshots and process handles alike). Prefer a small RAII wrapper once the basics are understood.
- Check every Win32 return value. On failure, print `GetLastError()`.
- Plain, readable, C-style C++. No clever templates. Clarity beats brevity; this code also has to be explained in a viva.
- Keep the memory-reading layer separate from rendering, so the reading code can be reused as a detector probe.

## How to help Don (important)
- Don isn't very confident at programming yet (more Python/JS background) and wants to learn C
  and reverse engineering, which is the career goal. **Teaching is the point, not just working code.**
- **Explain the concept before writing code**, then work in **small chunks: one function or step at a time.**
  Don't dump whole files or jump ahead in the roadmap.
- Explain new code line by line, and keep reinforcing the core principles: handles, access masks,
  virtual memory, pointers/addresses, struct layout, pointer chains, and the maths for world-to-screen.
- Point out how each step looks **from the detector's side** (e.g. listing processes needs no
  handle and can't be seen that way; `OpenProcess` is where a cheat becomes visible).
- Use analogies. Give direct, honest feedback, including about scope and deadlines.
- Casual tone. Dark humour and brainrot humour are welcome.
- End each chunk with a small task or question so Don checks their own understanding.

### Who writes what
- **New concepts** (handles, RPM, pointer chains, world-to-screen maths, overlay): **Don types it.**
  Claude explains the concept, gives a skeleton with `TODO`s + hints (not the full answer), then reviews
  what Don wrote. Only hand over the full solution if Don asks for it.
- **Plumbing already understood** (vcxproj/filters, .gitignore, repeated boilerplate): Claude may write it;
  Don reads it and should be able to explain it back.
- **Bugs:** Don gets ~10 min with the compiler error first. Then Claude explains the *cause*, not just the fix.
- Golden rule: nothing gets committed that Don can't explain line by line (viva prep).
