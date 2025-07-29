I have created a Repo from [Template](https://github.com/oleksandrmaslov/zmk-pointing-acceleration).
**Please create a template based on the original source. Do not fork from here.**

**Modifications by NUOVOTAKA** - Enhanced code quality, improved error handling, and better maintainability.

---

# ZMK MULTI-LEVEL POINTING ACCELERATION

This repository contains a **3-level** pointer acceleration implementation for pointing devices in ZMK, designed to accommodate users from beginners to advanced enthusiasts.

The acceleration makes fine cursor control more precise at slow speeds while allowing faster cursor movement when moving quickly. It supports customizable acceleration curves and can be configured for different input devices.

## 🎯 **Choose Your Level**

### **Level 1: Simple** (Recommended for most users)

- **3 basic settings** + **preset configurations**
- Perfect for getting started quickly
- Includes presets: Office, Gaming, High Sensitivity

### **Level 2: Standard** (Balanced features)

- **6 configurable settings**
- Speed-based acceleration with Y-axis boost
- Great for users who want more control

### **Level 3: Advanced** (Full customization)

- **8+ detailed settings**
- Advanced curve types and precision control
- For enthusiasts who want complete control

**Device Compatibility Note:** This module has been tested with Cirque trackpads and should work with other pointing devices (trackballs, trackpoints, other trackpads). Use with non-Cirque devices at your own risk.

**Before you start, you should make sure that you have a working
input device by following this: https://zmk.dev/docs/features/pointing**

## ✨ Features by Level

### **Level 1: Simple Features**

- ✅ **Preset configurations** (Office, Gaming, High Sensitivity)
- ✅ **Basic sensitivity** adjustment
- ✅ **3 curve types** (Linear, Mild, Strong)
- ✅ **Plug-and-play** setup

### **Level 2: Standard Features**

- ✅ All Level 1 features
- ✅ **Speed-based acceleration** with configurable thresholds
- ✅ **Y-axis boost** for widescreen displays
- ✅ **6 configurable parameters**

### **Level 3: Advanced Features**

- ✅ All Level 1 & 2 features
- ✅ **8+ detailed parameters**
- ✅ **Multiple acceleration curves** with mathematical precision:
  - **Linear curve (1)**: `f(t) = t` - Constant acceleration rate
  - **Exponential curves (2-5)**: Natural, smooth acceleration feel
    - Mild exponential (2): `f(t) = e^(2t) - 1` - Balanced for general use
    - Moderate exponential (3): `f(t) = e^(3t) - 1` - More responsive
    - Strong exponential (4): `f(t) = e^(4t) - 1` - Aggressive acceleration
    - Aggressive exponential (5): `f(t) = e^(5t) - 1` - Maximum responsiveness
  - **Polynomial curves (10-13)**: Predictable mathematical progression
    - Quadratic (10): `f(t) = t²` - Classic smooth curve
    - Cubic (11): `f(t) = t³` - Steeper acceleration
    - Quartic (12): `f(t) = t⁴` - Very steep acceleration
    - Quintic (13): `f(t) = t⁵` - Extremely steep acceleration
- ✅ **Fractional movement tracking** for pixel-perfect precision
- ✅ Compatible with any relative input device (mouse, trackball, touchpad)

## Installation & Usage

To use pointer acceleration, there are several steps necessary:

- adjust the `west.yml` to make the acceleration module available
- import the dependencies into your configuration files
- configure the acceleration parameters
- add the acceleration processor to your input chain

We'll go through these steps one by one.

### Adjust west.yml

Add the acceleration module to your `west.yml`:

```yaml
manifest:
  remotes:
    - name: zmkfirmware
      url-base: https://github.com/zmkfirmware
    - name: nuovotaka
      url-base: https://github.com/nuovotaka
  projects:
    - name: zmk-pointing-acceleration-alpha
      remote: nuovotaka
      revision: main
    - name: zmk
      remote: zmkfirmware
      revision: main
      import: app/west.yml
```

> [!WARNING]  
> Run `west update` if you're building on your local machine (not github actions).

### Import the dependencies

Add the necessary includes to your device overlay file (e.g. `yourkeyboard_left.overlay`):

```C
#include <input/processors.dtsi>
#include <behaviors/input_gestures_accel.dtsi>
```

### Configure Your Level

Choose your configuration level in your `prj.conf` file:

```ini
# Choose your level (1, 2, or 3)
CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_SIMPLE=y      # Level 1: Simple
# CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_STANDARD=y  # Level 2: Standard
# CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_ADVANCED=y  # Level 3: Advanced
```

### Configure Acceleration

Add the acceleration configuration to your device overlay. This configuration should go BEFORE your _input-listener_.

#### **Level 1: Simple Configuration**

**Option A: Use a Preset (Recommended)**

```ini
# In your prj.conf, choose a preset:
CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING=y       # Gaming preset
# CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_OFFICE=y     # Office preset
# CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_HIGH_SENS=y  # High sensitivity preset
```

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>;
    // Preset values are automatically applied!
};
```

**Option B: Custom Simple Settings**

```ini
# In your prj.conf:
CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_CUSTOM=y
```

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>;
    sensitivity = <1300>;     // 1.3x base sensitivity
    max-factor = <2800>;      // 2.8x maximum acceleration
    curve-type = <1>;         // 0=Linear, 1=Mild, 2=Strong
    sensor-dpi = <800>;       // 800 DPI sensor (optional, defaults to 800)
};
```

#### **Level 2: Standard Configuration**

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>;
    sensitivity = <1200>;         // 1.2x base sensitivity
    max-factor = <3000>;          // 3.0x maximum acceleration
    curve-type = <1>;             // Mild curve
    y-boost = <1300>;             // 1.3x Y-axis boost for widescreen
    speed-threshold = <600>;      // Start acceleration at 600 counts/sec
    speed-max = <3500>;           // Max acceleration at 3500 counts/sec
    sensor-dpi = <800>;           // 800 DPI sensor (optional, defaults to 800)
};
```

#### **Level 3: Advanced Configuration**

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>;
    track-remainders;             // Enable precision tracking (boolean property)

    // Advanced acceleration settings
    min-factor = <1000>;          // 1.0x minimum (no deceleration)
    max-factor = <4000>;          // 4.0x maximum acceleration
    speed-threshold = <500>;      // Acceleration starts at 500 counts/sec
    speed-max = <4000>;           // Max acceleration at 4000 counts/sec
    acceleration-exponent = <2>;  // Mild exponential curve
    sensor-dpi = <800>;           // 800 DPI sensor (optional, defaults to 800)
};
```

### Add to Input Chain

Add the acceleration processor to your input device's processor chain:

```devicetree
/ {
    tpad0: tpad0 {
        compatible = "zmk,input-listener";
        status = "okay";
        device = <&glidepoint>;
        input-processors = <
            &pointer_accel      // Acceleration processor
            &zip_xy_transform
        >;
    };
};
```

## Configuration Options

**Visualisation of these settings here: https://pointing.streamlit.app/**

The acceleration processor provides several settings to customize how your pointing device behaves. Here's a detailed explanation of each option:

### Basic Settings

- `min-factor`: (Default: 1000)

  - Controls how slow movements are handled
  - Values below 1000 will make slow movements even slower for precision
  - Values are in thousandths (e.g., 800 = 0.8x speed)
  - Example: `min-factor = <800>` makes slow movements 20% slower

- `max-factor`: (Default: 3500)
  - Controls maximum acceleration at high speeds
  - Values are in thousandths (e.g., 3500 = 3.5x speed)
  - Example: `max-factor = <3000>` means fast movements are up to 3x faster

### Speed Settings

- `speed-threshold`: (Default: 1000)

  - Speed at which acceleration starts
  - Measured in counts per second
  - Below this speed, min-factor is applied
  - Above this speed, acceleration begins
  - Example: `speed-threshold = <1200>` means acceleration starts at moderate speeds

- `speed-max`: (Default: 6000)
  - Speed at which maximum acceleration is reached
  - Measured in counts per second
  - At this speed and above, max-factor is applied
  - Example: `speed-max = <6000>` means you reach max acceleration at high speeds

### Acceleration Behavior

- `acceleration-exponent`: (Default: 2)
  - Controls the mathematical curve type for acceleration
  - **Exponential curves (recommended for natural feel):**
    - 2 = Mild exponential `e^(2t) - 1` (default, balanced)
    - 3 = Moderate exponential `e^(3t) - 1` (more responsive)
    - 4 = Strong exponential `e^(4t) - 1` (aggressive)
    - 5 = Aggressive exponential `e^(5t) - 1` (maximum)
  - **Polynomial curves (predictable progression):**
    - 10 = Quadratic `t²` (classic smooth curve)
    - 11 = Cubic `t³` (steeper acceleration)
    - 12 = Quartic `t⁴` (very steep)
    - 13 = Quintic `t⁵` (extremely steep)
  - **Linear curve:**
    - 1 = Linear `t` (constant acceleration rate)
  - Example: `acceleration-exponent = <3>` for moderate exponential acceleration

### Hardware Settings

- `sensor-dpi`: (Default: 800)
  - DPI/CPI setting of your mouse sensor
  - Used for automatic sensitivity scaling
  - Common values: 400, 800, 1200, 1600, 3200 DPI
  - Higher DPI sensors will have reduced sensitivity to maintain consistent feel
  - Example: `sensor-dpi = <1600>` for a 1600 DPI sensor

### Advanced Options

- `track-remainders`: (Default: disabled)
  - Enables tracking of fractional movements
  - Improves precision by accumulating small movements
  - Enable with `track-remainders;` in your config

### Visual Examples

Here's how different configurations affect pointer movement:

```
Slow Speed │  Medium Speed  │  High Speed
───────────┼────────────────┼────────────
0.8x      →│      1.0x     →│     3.0x     (Balanced)
0.9x      →│      1.0x     →│     2.0x     (Light)
0.7x      →│      1.0x     →│     4.0x     (Heavy)
0.5x      →│      1.0x     →│     1.5x     (Precision)
```

## Share Your Settings

### App for easy configuration visualisation: https://pointing.streamlit.app/

The configurations under are just starting points - every person's perfect pointer settings are as unique as they are) I'd love to see what works best for you.

### Why Share?

- Help others find their ideal setup
- Contribute to the community knowledge
- Get feedback and suggestions
- Inspire new configuration ideas

### How to Share

- Drop your config in a GitHub issue
- Share on Discord ZMK or my DM (with a quick note about your use case)
- Comment on what worked/didn't work for you

> **Remember**: These examples were primarily tested with Cirque trackpads. If you're using other pointing devices (like trackballs or trackpoints), your mileage may vary - and that's why sharing your experience is so valuable

### General Use:

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>; // X and Y axis events
    min-factor = <800>;        // Slight slowdown for precision
    max-factor = <3000>;       // Good acceleration for large movements
    speed-threshold = <1200>;  // Balanced acceleration point
    speed-max = <6000>;
    acceleration-exponent = <2>; // Mild exponential curve
    track-remainders;         // Track fractional movements
    sensor-dpi = <800>;       // 800 DPI sensor
};
```

### Light Acceleration

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>; // X and Y axis events
    min-factor = <900>;        // 0.9x minimum
    max-factor = <2000>;       // 2.0x maximum
    speed-threshold = <1500>;  // Start accelerating later
    speed-max = <5000>;         // 5000 counts/sec for max accel
    acceleration-exponent = <1>; // Linear acceleration
    track-remainders;          // Track fractional movements
    sensor-dpi = <800>;       // 800 DPI sensor
};
```

### Heavy Acceleration

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>; // X and Y axis events
    min-factor = <700>;        // 0.7x minimum
    max-factor = <4000>;       // 4.0x maximum
    speed-threshold = <1000>;  // Start accelerating earlier
    speed-max = <6000>;          // 6000 counts/sec for max accel
    acceleration-exponent = <3>; // Cubic acceleration curve
    track-remainders;          // Track fractional movements
    sensor-dpi = <800>;       // 800 DPI sensor
};
```

### Precision Mode

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>; // X and Y axis events
    min-factor = <500>;        // 0.5x for fine control
    max-factor = <1500>;       // 1.5x maximum
    speed-threshold = <2000>;  // High threshold for stability
    speed-max = <7000>;          // 6000 counts/sec for max accel
    acceleration-exponent = <1>; // Linear response
    track-remainders;          // Track fractional movements
};
```
