# Mouse pointer and shake-binding roadmap

Status: partial input plumbing exists, including tested client-to-rendered-viewport coordinates; user-facing binding settings and in-game verification are not complete. The project is not playable yet.

## Goal

Make the Wii Remote's on-screen pointer and shake actions usable with a PC mouse, keyboard, and game controller while preserving the input data and edge-trigger behavior expected by Petari's original game code. This is a host-input translation layer, not Wii Remote emulation.

## User-confirmed requirements (2026-10-11)

- The game’s Wii cursor/pointing functions should follow the PC mouse. This includes menu cursor movement and gameplay pointing, using the rendered game viewport rather than desktop coordinates.
- Wii Remote shake should be bindable to a keyboard key and to a supported game-controller button. Both bindings should be configurable from the eventual input menu and feed the same logical shake action.
- These are roadmap requirements only; neither setting is complete until a normal game boot and in-game validation are possible.

## Planned default controls

| Wii Remote action | PC input | Planned default / behavior |
| --- | --- | --- |
| Point at screen (Wii IR cursor) | Mouse movement | Map the mouse to the game viewport in client coordinates; letterbox bars are outside the valid pointer area. |
| Pointer trigger (Wii B) | Left mouse button | Press/release edges map to the original trigger state. |
| Shake | Keyboard or controller button | Keep Q and XInput X as initial defaults; allow the player to rebind to a keyboard key or supported controller button. |

These are adapter defaults, not a claim that a settings menu or a playable game is already available. The mapping must be checked against the actual game's pointer consumers before it is called complete.

## Explicit requested controls

- Bind the game's Wii Remote cursor/pointing functions to mouse movement. Convert the mouse position into the active game viewport's coordinate system so the pointer follows the cursor over the rendered game, including after resize and with letterboxing. Preserve the original Wii pointer-validity and button semantics consumed by the game.
- Make Wii Remote shake a logical action that can be bound to either a keyboard key or a supported game-controller button. Keyboard and controller bindings must share the same press-edge/release behavior; simultaneous holds must not cause duplicate shakes. The eventual menu should let the player choose the keyboard binding and controller binding independently.
- Include both controls in the input/settings menu plan and acceptance checks. Verify actual pointer-driven game actions and shake-driven actions in a booted game before marking either end-to-end feature complete.

## Existing foundation

- The host keyboard/mouse sampler runs from `GameSystem::frameLoop()` and can be associated with a game window.
- Mouse motion is translated to Wii pointer coordinates; `nativeSetGamePointerViewport` can map the rendered viewport in client coordinates, and letterbox bars are invalid pointer positions. If no viewport is supplied, the whole client area is used. Left-click currently supplies the pointer trigger/Wii B path.
- Q and XInput X currently provide the shake action and create a debounced acceleration impulse. The virtual keyboard key and XInput mask have native setter functions.
- Original `WPadAcceleration` and `WPadHVSwing` tests verify that one press produces one swing trigger, held input does not repeat, and release re-arms it.
- A merged keyboard/controller source test now runs the source union through those same original classes: overlapping holds and handoff do not retrigger, and releasing all sources re-arms the next press.
- The game window, normal game boot, configurable settings UI, and physical-controller validation are not yet available.

## Source audit snapshot (2026-10-11)

- The host adapter maps a valid client/viewport point to normalized `KPADStatus::pos` coordinates in `[-1, 1]` on each axis, with positive Y downward; it marks the pointer valid with `dpd_valid_fg >= 2`. A point in a letterbox bar is invalid.
- `NativePointerMotion` derives `vec` from the change between accepted normalized positions and `speed` from that displacement. This matters for swipe/velocity interactions such as Star Pointer rubbing and aiming; validate thresholds at actual frame rates before calling the mouse equivalent complete.
- Left mouse currently contributes `WPAD_BUTTON_B` through the host state mapper, which derives the trigger/release edges in `nativeSubmitKpadState`. The original game uses B-trigger checks alongside Star Pointer validity/pointing, including star-piece shooting; retain the distinction between pointer validity, position, and button press.
- The original `StarPointerUtil` and `StarPointerDirector` are shared by menu/layout cursor presentation, target pointing, world-ray/plane calculations, and pointer velocity users. A successful coordinate unit test therefore does not prove all cursor consumers behave correctly.
- The host input sampler still has no normal game `HWND` creation/binding path, and the viewport setter is not called by a game renderer. The mouse adapter is not yet connected end-to-end to a running port.

## Implementation plan

1. **Preserve Wii pointer semantics.** Trace the original `KPADStatus` fields and every StarPointer consumer. Feed the original coordinate, angle/distance, validity, and trigger expectations from the host adapter; never substitute display pixels for game-viewport coordinates without conversion.
2. **Finish mouse-to-pointer mapping.** Use coordinates relative to the bound game client area and the actual rendered game viewport. Account for aspect-ratio letterboxing, DPI scaling, resize, focus loss/reacquisition, and pointer-outside-viewport behavior. Add configurable sensitivity and recentering, while keeping cursor motion stable and avoiding dependence on desktop screen coordinates.
3. **Map pointer buttons.** Keep left-click as the default pointer trigger and expose the Wii actions that use pointer buttons through host mouse buttons. Define behavior for right/middle buttons and clicks outside the viewport from the original game's call sites before binding them.
4. **Make shake a real bindable action.** Keep the existing Q/XInput defaults, but route keyboard and supported controller buttons through one logical shake action. Let the player choose a keyboard key or a button on the active controller; keep bindings device-aware so disconnect/reconnect cannot leave shake stuck. A press must generate the expected short acceleration impulse once; holding must not repeat it, release must re-arm it, and simultaneous sources must not double-fire.
5. **Add an input settings screen after boot/menu UI exists.** Provide mouse pointer sensitivity/recenter options and a “Press a key or button” rebinding flow for shake. Show the current device/bind, allow cancel and reset-to-default, reject or explain conflicts, persist settings per user, and restore safe defaults for invalid settings. Do not present bindings as configurable before this UI is wired to the adapter.
6. **Validate end-to-end in the game.** With a real native game window, check title/menu pointer selection, Star Pointer target acquisition, pointer clicks, shake-only actions, simultaneous inputs, focus changes, pause/resume, and controller hot-plug. Compare observed `KPADStatus` and WPad transitions with the original Wii action call sites; confirm no unintended repeated action while a key is held.

## Requested feature sequence

The user's requested control behavior is represented by the following implementation sequence; the adapter groundwork below is not a substitute for completing these game-facing steps:

1. Trace the original Wii cursor / Star Pointer consumers and record which `KPADStatus` fields, validity checks, and pointer-button transitions each uses.
2. Bind mouse position to those cursor inputs using the active render viewport (including aspect-ratio bars and resize/DPI changes); bind mouse click to the corresponding pointer trigger only where the original action expects it.
3. Route Wii Remote shake to one logical action with independently configurable keyboard and controller binds. Preserve press-edge semantics when either source is held, both overlap, a controller disconnects, or bindings are changed.
4. Add both controls to the eventual settings menu: mouse pointer sensitivity/recenter options and separate “press a key” / “press a controller button” shake binding flows, with reset, cancel, conflict handling, and persistence.
5. Verify both controls in a normally booted game, then mark the mouse cursor, mouse trigger, keyboard shake, and controller shake separately as complete only after their corresponding gameplay actions work.

## Acceptance checks

- Unit tests cover full-client and explicit viewport mapping, including letterbox-bar rejection and viewport-edge/center coordinates, plus trigger edges and pointer validity. Resize integration, DPI-awareness behavior, focus/reacquisition, and out-of-window behavior in a live game window remain unverified.
- Shake tests cover the combined keyboard/controller logical action through Petari's original acceleration and swing classes, including overlap, handoff, release and rearm. Direct physical-controller hot-plug, configurable-menu bindings, rapid-tap behavior and invalid-binding recovery still need validation.
- Petari integration tests continue to pass through `WPadAcceleration` and `WPadHVSwing`.
- A menu-level test confirms settings save and reload. A manual first-level smoke test confirms mouse pointing and each configured shake bind perform the intended in-game action.
- Report each layer separately: adapter tests passing does not mean the settings menu, game boot, pointer gameplay, or full project is verified.

## Dependencies and scope

Finish the native host window and a normal boot/menu path before claiming an in-game settings screen or gameplay acceptance. Keep single-player boot and first-level behavior ahead of co-op. Later co-op work must define whether bindings are per-player and support distinct local devices; network clients must transmit logical actions/state, never raw host key codes.
