I have created a Repo from [Template](https://github.com/oleksandrmaslov/zmk-pointing-acceleration).
**Please create a template based on the original source.**
**If necessary, please feel free to create a repository from `Use this Template`.**

**Modifications by NUOVOTAKA** - Enhanced code quality, improved error handling, and better maintainability.

---

# ZMK MULTI-LEVEL POINTING ACCELERATION

This repository features a **two-level** pointer acceleration system for ZMK, suitable for both beginners and advanced users.

The acceleration enhances precise cursor movement at slow speeds and enables rapid movement at higher speeds. Acceleration curves are customizable, and settings can be tailored for various input devices.

## 🎯 **Choose Your Level**

### **Level 1: Simple** (Recommended for most users)

- **5 configurable settings for 3 pointing-device types** and **4 sensitivity levels**
- Quick and easy setup
- Automatic DPI adjustment for different sensors
- Presets for Office, Gaming, High Sensitivity, with options for Optical, Laser, Trackball, and Trackpad

### **Level 2: Standard** (Advanced features)

- **8 configurable settings for 3 pointing-device types** and **4 sensitivity levels**
- Speed-dependent acceleration and Y-axis boost
- Advanced exponential curve options (1–5)
- Ideal for users seeking full customization and device-specific tuning

**Device Compatibility:** Tested with PAW3222 trackball and Azoteq IQS7211E trackpad sensors. Should work with other devices, but sensor differences may affect performance.

**Before starting, confirm your input device works using: https://zmk.dev/docs/features/pointing**

## ✨ Features by Level

### **Level 1: Simple Features**

- ✅ **Preset configurations** (Office, Gaming, High Sensitivity)
- ✅ **Basic sensitivity** adjustment
- ✅ **3 curve types** (Linear, Mild, Strong)
- ✅ **Y-axis boost** for display optimization
- ✅ **Plug-and-play** setup

### **Level 2: Standard Features**

- ✅ All Level 1 features **including presets**
- ✅ **Speed-based acceleration** with configurable thresholds
- ✅ **Y-axis boost** for widescreen displays
- ✅ **Advanced exponential curves** with mathematical precision:
  - **Linear curve (1)**: `f(t) = t` - Constant acceleration rate
  - **Exponential curves (2-5)**: Natural, smooth acceleration feel
    - Mild exponential (2): `f(t) = e^(2t) - 1` - Balanced for general use
    - Moderate exponential (3): `f(t) = e^(3t) - 1` - More responsive
    - Strong exponential (4): `f(t) = e^(4t) - 1` - Aggressive acceleration
    - Aggressive exponential (5): `f(t) = e^(5t) - 1` - Maximum responsiveness
- ✅ **8 configurable parameters** for complete customization
- ✅ Compatible with any relative input device (pointing devices, trackball, touchpad)

## 🔍 **Understanding Level Differences**

**IMPORTANT:** Level 1 and Level 2 use completely different calculation methods. The same preset name will behave differently between levels.

📖 **[Read the detailed Level Comparison Guide →](docs/LEVEL_COMPARISON.md)**  
📋 **[View all Preset Configurations →](docs/PRESETS.md)**

### **Quick Summary**

- **Level 1**: Input-based, immediate response, gaming-style acceleration
- **Level 2**: Speed-based, gradual response, natural smooth acceleration

Choose Level 1 for predictable instant acceleration, or Level 2 for adaptive smooth acceleration.

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

### Enable the acceleration module

**IMPORTANT:** Add this required setting to your `prj.conf` file:

```ini
# REQUIRED: Enable the acceleration module
CONFIG_ZMK_INPUT_PROCESSOR_ACCELERATION=y
```

### Configure Your Level

Choose your configuration level in your `prj.conf` file:

```ini
# Choose your level (1 or 2)
CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_SIMPLE=y      # Level 1: Simple
# CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_STANDARD=y  # Level 2: Standard
```

### Configure Acceleration

Add the acceleration configuration to your device overlay. This configuration should go BEFORE your _input-listener_.

#### **Level 1: Simple Configuration**

**Option A: Use a Preset (Recommended)**

Available presets work for both Level 1 and Level 2 (12 device-specific presets):

**Office Presets:**

- `CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_OFFICE_OPTICAL=y` - Optical mouse (800 DPI)
- `CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_OFFICE_LASER=y` - Laser mouse (1600 DPI)
- `CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_OFFICE_TRACKBALL=y` - Trackball (400 DPI)
- `CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_OFFICE_TRACKPAD=y` - Trackpad (1000 DPI)

**Gaming Presets:**

- `CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING_OPTICAL=y` - Optical mouse (1200 DPI)
- `CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING_LASER=y` - Laser mouse (3200 DPI)
- `CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING_TRACKBALL=y` - Trackball (800 DPI)
- `CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING_TRACKPAD=y` - Trackpad (1200 DPI)

**High Sensitivity Presets:**

- `CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_HIGH_SENS_OPTICAL=y` - Optical mouse (1600 DPI)
- `CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_HIGH_SENS_LASER=y` - Laser mouse (6400 DPI)
- `CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_HIGH_SENS_TRACKBALL=y` - Trackball (800 DPI)
- `CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_HIGH_SENS_TRACKPAD=y` - Trackpad (1200 DPI)

```ini
# In your prj.conf, choose a device-specific preset:
CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING_OPTICAL=y     # Gaming optical mouse
# CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_OFFICE_LASER=y     # Office laser mouse
# CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_HIGH_SENS_TRACKPAD=y # High sensitivity trackpad
```

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>;
    // Preset values are automatically applied!
};
```

**Option B: Custom Simple Settings**

> **Note**: Level 1 custom settings are designed for beginners and use conservative acceleration limits to prevent excessive cursor movement. The system intentionally limits maximum acceleration to 3.0x and uses moderate speed thresholds for predictable behavior. If you need more aggressive acceleration, please try Level 2 presets first, then consider Level 2 custom settings.

```ini
# In your prj.conf:
CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_CUSTOM=y
```

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>;
    sensitivity = <1300>;     // 1.3x base sensitivity
    max-factor = <2800>;      // 2.8x maximum acceleration (capped at 3.0x for safety)
    curve-type = <1>;         // 0=Linear, 1=Mild, 2=Strong
    y-boost = <1200>;         // 1.2x Y-axis boost (optional, defaults to 1000)
    sensor-dpi = <800>;       // 800 DPI sensor (optional, defaults to 800)
};
```

#### **Level 2: Standard Configuration**

**Option A: Use a Preset (Recommended)**

```ini
# In your prj.conf:
CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_STANDARD=y
CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING_LASER=y       # Gaming laser mouse
# CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_OFFICE_OPTICAL=y   # Office optical mouse
# CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_HIGH_SENS_TRACKBALL=y # High sensitivity trackball
```

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>;
    // Preset values are automatically applied for all Level 2 settings!
    // Including: sensitivity, max-factor, acceleration-exponent, y-boost,
    // speed-threshold, speed-max, min-factor
};
```

**Option B: Custom Standard Settings**

```ini
# In your prj.conf:
CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_STANDARD=y
CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_CUSTOM=y
```

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>;
    sensitivity = <1200>;         // 1.2x base sensitivity
    max-factor = <3000>;          // 3.0x maximum acceleration
    acceleration-exponent = <2>;  // Advanced exponential curve (1-5)
    y-boost = <1300>;             // 1.3x Y-axis boost for widescreen
    speed-threshold = <600>;      // Start acceleration at 600 counts/sec
    speed-max = <3500>;           // Max acceleration at 3500 counts/sec
    min-factor = <1000>;          // 1.0x minimum (no deceleration)
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

The acceleration processor provides several settings to customize how your pointing device behaves. Here's a detailed explanation of each option:

### Basic Settings

- `sensitivity`: (Default: 1000)

  - Controls the base sensitivity multiplier for all pointer movements
  - Values are in thousandths (e.g., 1200 = 1.2x speed, 800 = 0.8x speed)
  - This affects the baseline speed before any acceleration is applied
  - **Lower values (e.g., 800)**: Slower, more precise movements - good for detailed work
  - **Higher values (e.g., 1500)**: Faster movements - good for large displays or quick navigation
  - Example: `sensitivity = <1200>` makes all movements 20% faster than default

- `max-factor`: (Default: 3500)
  - Controls maximum acceleration at high speeds
  - Values are in thousandths (e.g., 3500 = 3.5x speed)
  - **Lower values (e.g., 2000)**: Less aggressive acceleration - more predictable movement
  - **Higher values (e.g., 5000)**: More aggressive acceleration - faster for long distances
  - Example: `max-factor = <3000>` means fast movements are up to 3x faster

### Speed Settings

- `speed-threshold`: (Default: 1000)

  - Speed at which acceleration starts
  - Measured in counts per second
  - Below this speed, base sensitivity is applied
  - Above this speed, acceleration begins
  - Example: `speed-threshold = <1200>` means acceleration starts at moderate speeds

- `speed-max`: (Default: 6000)
  - Speed at which maximum acceleration is reached
  - Measured in counts per second
  - At this speed and above, max-factor is applied
  - Example: `speed-max = <6000>` means you reach max acceleration at high speeds

### Acceleration Behavior

- `min-factor`: (Default: 1000) **[Level 2 Standard only]**

  - Controls how slow movements are handled
  - Values below 1000 will make slow movements even slower for precision
  - Values are in thousandths (e.g., 800 = 0.8x speed)
  - Example: `min-factor = <800>` makes slow movements 20% slower

- `acceleration-exponent`: (Default: 2) **[Level 2 Standard only]**
  - Controls the advanced mathematical curve type for acceleration
  - **Available curves:**
    - 1 = Linear `t` (constant acceleration rate)
    - 2 = Mild exponential `e^(2t) - 1` (default, balanced)
    - 3 = Moderate exponential `e^(3t) - 1` (more responsive)
    - 4 = Strong exponential `e^(4t) - 1` (aggressive)
    - 5 = Aggressive exponential `e^(5t) - 1` (maximum responsiveness)
  - Example: `acceleration-exponent = <3>` for moderate exponential acceleration

### Hardware Settings

- `sensor-dpi`: (Default: 800)
  - DPI/CPI setting of your pointing device sensor
  - Used for automatic sensitivity scaling
  - Common values: 400, 800, 1200, 1600, 3200 DPI
  - Higher DPI sensors will have reduced sensitivity to maintain consistent feel
  - Example: `sensor-dpi = <1600>` for a 1600 DPI sensor

### Advanced Options

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

> **Remember**: These examples were primarily tested with trackballs using the PAW3222 pointing device sensor. If you're using other pointing devices (like trackpads or trackpoints), your mileage may vary - and that's why sharing your experience is so valuable

### General Use (Level 2 Standard):

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>; // X and Y axis events
    sensitivity = <1200>;      // 1.2x base sensitivity
    max-factor = <3000>;       // Good acceleration for large movements
    y-boost = <1200>;          // 1.2x Y-axis boost
    speed-threshold = <1200>;  // Balanced acceleration point
    speed-max = <6000>;        // Max acceleration at 6000 counts/sec
    min-factor = <800>;        // 0.8x minimum for precision
    acceleration-exponent = <2>; // Mild exponential curve
    sensor-dpi = <800>;        // 800 DPI sensor
};
```

### Light Acceleration (Level 2 Standard):

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>; // X and Y axis events
    sensitivity = <1100>;      // 1.1x base sensitivity
    max-factor = <2000>;       // 2.0x maximum
    y-boost = <1100>;          // 1.1x Y-axis boost
    speed-threshold = <1500>;  // Start accelerating later
    speed-max = <5000>;        // 5000 counts/sec for max accel
    acceleration-exponent = <1>; // Linear acceleration
    sensor-dpi = <800>;        // 800 DPI sensor
};
```

### Heavy Acceleration (Level 2 Standard):

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>; // X and Y axis events
    sensitivity = <1000>;      // 1.0x base sensitivity
    max-factor = <4000>;       // 4.0x maximum
    y-boost = <1000>;          // 1.0x Y-axis boost
    speed-threshold = <1000>;  // Start accelerating earlier
    speed-max = <6000>;        // 6000 counts/sec for max accel
    min-factor = <700>;        // 0.7x minimum for precision
    acceleration-exponent = <4>; // Strong exponential curve
    sensor-dpi = <800>;        // 800 DPI sensor
};
```

### Optimized Widescreen (Level 2 Standard) - Tested Configuration:

#### Gaming

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>; // X and Y axis events
    sensitivity = <800>;       // 0.8x base sensitivity
    max-factor = <2500>;       // 2.5x maximum acceleration
    y-boost = <1000>;          // 2.5x Y-axis boost for widescreen
    speed-threshold = <300>;   // Early acceleration start
    speed-max = <3000>;        // Lower speed for max acceleration
    min-factor = <800>;        // 0.8x minimum for precision
    acceleration-exponent = <3>; // Moderate exponential curve
    sensor-dpi = <800>;        // 800 DPI sensor
};
```

#### Office

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>; // X and Y axis events

    /* ------------- Office Preset ------------- */
    /* Lower base sensitivity, very gentle acceleration */
    /* Prioritizes slow and precise pointer movement   */

    sensitivity           = <600>;   /* 0.6 × base sensitivity */
    min-factor            = <700>;   /* Starts at 0.7× */
    max-factor            = <1500>;  /* Maximum 1.5× acceleration */
    speed-threshold       = <400>;   /* Acceleration starts at 400 cnt/s */
    speed-max             = <2500>;  /* Reaches max-factor at 2500 cnt/s */
    y-boost               = <1000>;  /* Same multiplier for X and Y axes */
    acceleration-exponent = <1>;     /* Linear (no exponential curve) */
    sensor-dpi            = <800>;   /* Assumes 800 DPI sensor */
};
```

#### High-sens

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>; // X and Y axis events

    /* --- High-sens: No boost at low speed, strong boost at mid/high speed --- */
    /* Initial movement is precise (no acceleration), acceleration increases in mid/high speed */

    sensitivity           = <1000>;   /* 1.0 × base sensitivity (no gain at low speed) */
    min-factor            = <1000>;   /* 1.0× at low speed (no acceleration) */
    max-factor            = <4000>;   /* Up to 4.0× at high speed */
    speed-threshold       = <300>;    /* Acceleration starts above 300 cnt/s */
    speed-max             = <1200>;   /* Reaches max-factor at 1200 cnt/s */
    y-boost               = <1300>;   /* 1.3× for Y axis (matches X) */
    acceleration-exponent = <4>;      /* Strong exponential for mid/high speed */
    sensor-dpi            = <800>;
};
```

### Precision Mode (Level 1 Simple):

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>; // X and Y axis events
    sensitivity = <800>;       // 0.8x for fine control
    max-factor = <1500>;       // 1.5x maximum
    curve-type = <0>;          // Linear curve
    y-boost = <1000>;          // 1.0x Y-axis (no boost for precision)
    sensor-dpi = <800>;        // 800 DPI sensor
};
```
