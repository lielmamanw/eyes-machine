# eyes-machine

An animatronic pair of eyes, built on an ESP32-S3 (N16R8) and six servo motors, that idles with lifelike autonomous movement, can be puppeted by an external controller, and can be switched off with its own little "going to sleep" animation. This document walks through the hardware, the firmware architecture, and the math behind the behavior - top to bottom, the same order you'd read the code in if you were seeing it for the first time.

## Hardware

The machine has six servos, all driven through a single **PCA9685** 16-channel PWM driver board over I2C, rather than directly from the ESP32's own pins:

| Motor | PCA9685 channel | Role |
|---|---|---|
| Right lower lid | 0 | Right eye eyelid (paired with upper) |
| Right upper lid | 1 | Right eye eyelid (paired with lower) |
| X axis | 2 | Horizontal gaze, shared by both eyes |
| Y axis | 3 | Vertical gaze, shared by both eyes |
| Left lower lid | 4 | Left eye eyelid (paired with upper) |
| Left upper lid | 5 | Left eye eyelid (paired with lower) |

(See `MotorChannel` in [constants.hpp](constants.hpp).)

The PCA9685 is wired to the ESP32-S3's I2C bus on `SDA = GPIO21`, `SCL = GPIO20`. A single onboard RGB LED (`GPIO48`) reflects the machine's current mode as a color, and a push button on `GPIO0` (the board's BOOT button) cycles through modes.

Every servo is cheap and imprecise, so none of them can be trusted to hit "true" 0°/180° reliably - each one gets its own measured, calibrated pulse-width range instead of a shared default. That calibration data lives in `constants.hpp` and is where the hardware's physical quirks are meant to be absorbed, so nothing above it ever has to think in pulses or degrees again.

## Architecture: three layers

The firmware is organized bottom-up, and each layer only knows about the one directly beneath it:

```
.ino (setup/loop, button handling)
   -> EyesMachine (mode switching, phase-driven state machine)
        -> MachineState implementations (OffState, AutoState, ControllerState)
             -> EyesActionsController (eyes-level intent: gaze, eyelids)
                  -> Motor (one PCA9685 channel, calibrated)
```

Nothing above `EyesActionsController` ever touches a PWM channel or a pulse width directly, and nothing below it knows what a "blink" or a "mode" is. That boundary is what lets the exact same hardware layer be driven interchangeably by autonomous randomness, a future external controller, or - eventually - a phone app, without any of those callers needing to know or care about the others.

## The hardware layer: `Motor` and `EyesActionsController`

At the bottom, [motor.hpp](motor.hpp)/[motor.cpp](motor.cpp) defines `Motor` - a thin wrapper around one PCA9685 channel:

```cpp
struct MotorLimits { uint16_t minPulse; uint16_t maxPulse; bool inverted = false; };

class Motor {
  public:
    Motor(Adafruit_PWMServoDriver& driver, uint8_t channel, MotorLimits limits);
    void setValue(float value); // normalized 0.0 - 1.0
};
```

`Motor::setValue()` never accepts a raw pulse width - only a normalized `0.0`-`1.0`. Internally it linearly interpolates that into the motor's own calibrated pulse range:

```
pulse = minPulse + ratio * (maxPulse - minPulse)
```

where `ratio` is the input value (flipped to `1 - value` first if the motor is mounted `inverted`, since two lid motors on the same eye are often mirror-mounted and need opposite pulse directions to converge on "open" together). This is the one place in the whole codebase where "0.0-1.0" gets translated into a specific number of PCA9685 ticks, and it's why every motor can be recalibrated independently just by editing its `MotorLimits` in `constants.hpp` - nothing else has to change.

One level up, [eyes-actions.controller.hpp](eyes-actions.controller.hpp) owns the single shared `Adafruit_PWMServoDriver` instance and all six `Motor`s, and exposes the actual programmer-facing interface for "what should the eyes do":

```cpp
class EyesActionsController {
  public:
    void begin();

    void setHorizontal(float value);       // -1.0 (left)  .. 1.0 (right)
    void setVertical(float value);         // -1.0 (down)  .. 1.0 (up)
    void setOpenness(float value);         // both eyes,  0.0 (closed) .. 1.0 (open)
    void setRightEyeOpenness(float value); // 0.0 (closed) .. 1.0 (open)
    void setLeftEyeOpenness(float value);  // 0.0 (closed) .. 1.0 (open)
};
```

`setHorizontal`/`setVertical` take a *signed* -1..1 value (more natural for "look left/right" than an unsigned range), and internally remap it into `Motor`'s 0..1 space with `(value + 1) / 2` before forwarding it. `setRightEyeOpenness`/`setLeftEyeOpenness` each drive their eye's upper and lower lid motor together from one value, so every caller above this layer thinks in terms of "how open is this eye," never in terms of two separate lid motors. `setOpenness` is just both eyes at once.

Every one of these setters routes through a private `validateVal()` that **clamps** out-of-range input rather than rejecting or asserting on it - on a physical device, "move as close as safely possible" is a better failure mode than silently doing nothing, especially once input starts arriving from an external controller where bad values are inevitable.

**A hardware-init gotcha worth knowing before touching this layer**: `EyesActionsController`'s constructor (and `Motor`'s) only ever stores configuration - channel numbers, calibration limits, references. Neither ever touches I2C or the PCA9685. All real hardware initialization happens in `begin()`. This matters because `EyesActionsController` lives inside the globally-constructed `EyesMachine machine;` in the `.ino` file, and global objects are constructed during C++ static initialization - *before* the ESP32 Arduino core's own hardware init has run. Anything that touches a peripheral from inside a constructor at that point either silently fails or leaves the peripheral's driver permanently broken for every later call. `begin()` exists specifically so the real I2C/PWM setup happens later, once `setup()` calls it explicitly - after hardware init is actually done.

## The state layer: `MachineState`

The machine's behavior - what the eyes are actually doing right now - lives entirely in classes implementing [machine-state.hpp](machine-state.hpp)'s interface:

```cpp
class MachineState {
  public:
    virtual void reset() = 0;             // called once, right before entering

    virtual void enter() = 0;             // called every tick until isEnterComplete()
    virtual bool isEnterComplete() const = 0;

    virtual void execute() = 0;           // called every tick once entered

    virtual void exit() = 0;              // called every tick until isExitComplete()
    virtual bool isExitComplete() const = 0;
};
```

The three phases exist because a state's entry or exit animation can take real time - OffState's enter phase runs a multi-second blink sequence, for example - and the firmware can never block the main loop while that plays out (no `delay()` anywhere in this codebase, aside from the one deliberately tiny debounce in the button handler). So instead of a state doing its animation in one call, `enter()`/`exit()` get called once per `loop()` tick, do a small amount of work each time, check elapsed time via `millis()`, and report their own completion. `EyesMachine` polls that completion flag and only advances once it's true - the same cooperative, non-blocking scheduling technique used throughout this project.

`EyesMachine` ([eyes-machine.hpp](eyes-machine.hpp)/[eyes-machine.cpp](eyes-machine.cpp)) is what drives this. It holds one instance each of `OffState`, `AutoState`, and `ControllerState`, a pointer to whichever is currently active, and a phase (`ENTER` / `EXECUTE` / `EXIT`):

```cpp
switch (_phase) {
  case ENTER:   activeState->enter();   if (isEnterComplete()) phase = EXECUTE; break;
  case EXECUTE: activeState->execute(); break;
  case EXIT:    activeState->exit();    if (isExitComplete())  { swap to pending state; phase = ENTER; } break;
}
```

Requesting a mode change (`setMode()`, called from the button handler or, later, an external controller) doesn't switch states immediately - it sets a `_pendingState` and drops the current state into its `EXIT` phase. The actual swap only happens once that state's `exit()` reports itself done, which is what makes `OffState`'s "peek before switching away" behavior (below) actually work as a real transition, not just a visual afterthought. Re-requesting the mode a state is already active in, or already transitioning toward, is a no-op - `requestState()` checks both `_activeState` and `_pendingState` before queuing anything.

## `OffState`: a scripted animation

[off-state.cpp](off-state.cpp) is the simplest state and the clearest illustration of the enter/execute/exit pattern:

- **Enter**: centers the gaze, then runs a "tired blink" sequence - the eyes open and close a fixed number of times (`OFF_STATE_TIRED_BLINK_COUNT`), with each cycle's hold duration growing geometrically:

  ```
  holdDuration(blinkIndex) = OFF_STATE_BLINK_BASE_HOLD_MS * OFF_STATE_BLINK_SLOWDOWN_FACTOR ^ blinkIndex
  ```

  so each blink lingers noticeably longer than the last, reading as the eyes getting heavier before finally staying shut. This is driven by a tiny internal `BlinkPhase` (`EYES_OPEN`/`EYES_CLOSED`) checked against `millis()` on every `enter()` call - not a single scripted sequence of `delay()`s.
- **Execute**: does nothing dynamic - it just continuously re-asserts closed eyelids and a centered gaze every tick, as a self-healing steady state.
- **Exit**: opens the right eye alone for `OFF_STATE_PEEK_DURATION_MS` (a "peek," as if checking whether it's safe to fully wake) before reporting complete and letting `EyesMachine` swap to whatever state was requested.

## `AutoState`: autonomous, statistically-driven behavior

This is where the machine is meant to look alive rather than scripted. [auto-state.cpp](auto-state.cpp) runs several independent behaviors every tick, each fired by its own randomly-timed schedule rather than a fixed sequence - so it never repeats the same rhythm twice.

### The randomness underneath: Box-Muller

Arduino's `random()` only produces a **uniform** distribution - every value in a range is equally likely. Realistic timing (blink roughly every 4 seconds, sometimes a bit sooner, sometimes later, rarely far off) needs a **normal (Gaussian)** distribution instead: values clustering around a mean, tapering off symmetrically. [random-utils.cpp](random-utils.cpp) builds that on top of `random()` using the **Box-Muller transform**: given two independent uniform samples `u1, u2` in `(0, 1]`,

```
z0 = sqrt(-2 * ln(u1)) * cos(2π * u2)
```

is an exact sample from a *standard* normal distribution (mean 0, stddev 1). Scaling and shifting it (`mean + z0 * stddev`) gives a sample from any normal distribution you want - that's `sampleGaussian()`. `sampleClampedGaussian()` is the same thing with a min/max clamp on top, since an unbounded Gaussian could occasionally hand back a value like `-3.7`, meaningless outside a servo's `-1..1` range. `sampleUniform01()` is exposed on its own too, for plain probability rolls ("12% chance of X").

### Scheduling: `RandomTimer`

[random-timer.hpp](random-timer.hpp)/[random-timer.cpp](random-timer.cpp) wraps a Gaussian sample into "fire after a randomly-sampled interval, without ever blocking":

```cpp
struct RandomTimerConfig { float meanIntervalMs; float stddevIntervalMs; unsigned long minIntervalMs; };

class RandomTimer {
  public:
    void reset(unsigned long now);
    bool isDue(unsigned long now) const;
    void reschedule(unsigned long now, float speedScale = 1.0f);
    bool tryFire(unsigned long now, float speedScale = 1.0f); // isDue() + reschedule() combined
};
```

`reschedule()` samples a fresh interval from the timer's configured mean/stddev, clamps it to `minIntervalMs` so it can never fire instantly (or on a negative interval, which a Gaussian's tail could otherwise produce), and stores `now + interval` as the next due time. `isDue()` is a cheap comparison safe to call every tick. `tryFire()` is the convenience combination of both, for behaviors that fire-and-reschedule in one step.

`speedScale` is what lets a timer's pace be dialed up or down at runtime: `reschedule()` divides *both* the mean and stddev by `speedScale` before sampling:

```
sampledMs = sampleGaussian(meanIntervalMs / speedScale, stddevIntervalMs / speedScale)
```

Dividing both by the same factor keeps their ratio - the *relative* raggedness of the timing - constant regardless of speed; only the absolute pace changes. `speedScale > 1.0` shortens the average wait (and tightens it proportionally); `< 1.0` stretches it out. `minIntervalMs` itself is **not** scaled - it's a fixed floor regardless of speed, which is a deliberate safety net, though it does mean the scaling stops being linear once the scaled mean gets close to that floor.

### The behaviors

`AutoState` owns five `RandomTimer`s, each configured independently in `constants.hpp` so blinking, looking around, and everything else can each have their own rhythm:

- **Blink**: fires via `_blinkTimer`, closes the eyes, holds for `BLINK_CLOSED_HOLD_MS`, reopens, and only *then* reschedules the next blink - so the configured interval is measured between blinks finishing, not between them starting.
- **Horizontal / vertical saccades**: fire via their own timers, sample a target position from a Gaussian centered at 0 (`HORIZONTAL_SACCADE_TARGET_MEAN/STDDEV`, `VERTICAL_SACCADE_TARGET_MEAN/STDDEV`), and **snap** straight to it rather than easing there - real eyes jump and hold, they don't glide continuously.
- **Idle jitter**: a fast timer (`JITTER_TIMER_CONFIG`, ~200ms) that adds a tiny Gaussian offset (`JITTER_STDDEV`) around the *current saccade target*, so the eyes visibly tremble slightly rather than sitting dead-still between saccades. This is why `_horizontalTarget`/`_verticalTarget` are tracked separately from whatever's actually being written to the motors at any given instant.
- **Energy**: a slow random *walk* - `_energy` gets nudged by a small Gaussian step (`ENERGY_STEP_STDDEV`) roughly every 8 seconds and is clamped to `[ENERGY_MIN, ENERGY_MAX]` (0.5-1.5). It's passed as `speedScale` into the blink and saccade timers, so the machine drifts between sluggish and alert stretches over time instead of maintaining one constant statistical rhythm forever.

Two gimmicks tie these together:

- **Wink**: whenever a blink is about to start, there's a `WINK_PROBABILITY` (12%) chance it closes only one randomly-chosen eye instead of both - a rare, deliberate-feeling quirk rather than a reflex.
- **Blink synced with a saccade**: right after a saccade fires, there's a separate `SACCADE_SYNCED_BLINK_PROBABILITY` (25%) chance of also triggering a blink at that same instant - mimicking how real eyes often blink while changing gaze direction. Both paths funnel through the same `triggerBlink()`, which guards against starting a new blink while one's already mid-close, so this can never corrupt the blink state or double up.

## `ControllerState`: reserved for external control

[controller-state.hpp](controller-state.hpp) is currently a deliberate no-op placeholder - `enter`/`execute`/`exit` do nothing, and both completion checks return `true` immediately, just enough for mode-cycling through the button to not crash on a null state. The intended design, noted in its own TODO comment, is for an external source (originally planned as an ESP-NOW physical remote; a phone-app WebSocket connection is also planned, likely as its own sibling mode/state built the same way) to feed commands into a small buffer via a receive callback, which `execute()` then drains non-blockingly - the same pattern already proven out by `EyesMachine`'s own tick-driven design.

## `EyesMachine` and `.ino`: tying it together

[eyes-machine.ino](eyes-machine.ino) is intentionally thin. `setup()` calls `machine.begin()` (seeds the RNG, initializes the PCA9685) and `machine.setMode(OFF)`; `loop()` calls a debounced button handler and `machine.runInLoop()` every tick, which is the phase-switch shown above. `setMode()` also drives the status LED - it looks up the requested mode's color from `EyesMachineModeColors` in `constants.hpp` and writes it immediately, independent of however long the underlying state transition takes to actually complete.

## Tuning

Every number that shapes behavior - motor calibration, blink/saccade timing and magnitude, wink/sync probabilities, energy bounds, the off-state blink slowdown curve - lives in [constants.hpp](constants.hpp), not scattered through the logic that uses it. That's a deliberate project convention: retuning how the machine feels should never require hunting through `.cpp` files, only editing named constants.

## Adding a new state

Implement `MachineState`, take an `EyesActionsController&` in the constructor (never construct your own `Adafruit_PWMServoDriver` - there's exactly one, shared), keep constructors free of any hardware I/O, add a `MachineMode` enum entry, an instance member on `EyesMachine`, and a case in `stateForMode()`. Everything else - the phase machine, the deferred exit-then-swap transition, the LED color update - is already handled for you.

## Project status

Built and working: the full hardware layer, the `MachineState` framework, `OffState`, and `AutoState` with its gimmicks, all driven from a physical button. `ControllerState` is a stub awaiting real input.

Planned next: a web app acting as a remote "smart controller" - switching modes, manual gaze/eyelid/blink/wink control, and eventually driving the eyes from a video of a human face (via backend face-tracking reduced to the same snap-and-hold event timeline `AutoState` already uses, not raw per-frame streaming). Longer-term ideas on the table: camera-based face following, a microphone/speaker for voice interaction, and in-app remote calibration of motor limits and `AutoState`'s personality constants without needing to reflash.
