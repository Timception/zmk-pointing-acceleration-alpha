# Configuration Examples

## Level 1: Simple Configuration

### Using Presets (Recommended)

Available presets:

- `CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_OFFICE=y` - Conservative settings for office work
- `CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING=y` - Balanced settings for gaming
- `CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_HIGH_SENS=y` - High sensitivity for fast movements

```ini
# In prj.conf
CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_SIMPLE=y
CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING=y
```

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>;
    // Preset values are automatically applied
};
```

### Custom Simple Settings

```ini
# In prj.conf
CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_SIMPLE=y
CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_CUSTOM=y
```

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>;
    sensitivity = <1300>;     // 1.3x base sensitivity
    max-factor = <2800>;      // 2.8x maximum acceleration
    curve-type = <1>;         // Mild curve
    sensor-dpi = <800>;       // 800 DPI sensor (optional)
};
```

## Level 2: Standard Configuration

### Using Presets (Recommended)

Available presets for Level 2:

- `CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_OFFICE=y` - Conservative settings for office work
- `CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING=y` - Balanced settings for gaming
- `CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_HIGH_SENS=y` - High sensitivity for fast movements

```ini
# In prj.conf
CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_STANDARD=y
CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING=y
```

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>;
    track-remainders;             // Enable precision tracking
    // Preset values are automatically applied for all Level 2 settings
    // Including: sensitivity, max-factor, curve-type, y-boost,
    // speed-threshold, speed-max, min-factor
};
```

### Custom Standard Settings

```ini
# In prj.conf
CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_STANDARD=y
CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_CUSTOM=y
```

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>;
    track-remainders;             // Enable precision tracking
    sensitivity = <1200>;         // 1.2x base sensitivity
    max-factor = <3000>;          // 3.0x maximum acceleration
    curve-type = <1>;             // Basic mild curve
    y-boost = <1300>;             // 1.3x Y-axis boost
    speed-threshold = <600>;      // Start acceleration at 600 counts/sec
    speed-max = <3500>;           // Max acceleration at 3500 counts/sec
    min-factor = <1000>;          // 1.0x minimum (no deceleration)
    acceleration-exponent = <2>;  // Advanced exponential curve (1-5)
    sensor-dpi = <800>;           // 800 DPI sensor (optional)
};
```

## Parameter Explanations

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

- `curve-type`: (Default: 1)
  - Controls the acceleration curve shape
  - **0 = Linear**: Constant acceleration rate - predictable but less natural
  - **1 = Mild**: Balanced curve - good for most users
  - **2 = Strong**: Aggressive curve - more responsive feel

## Troubleshooting

### Too Sensitive

- Reduce `sensitivity` or `max-factor`
- Increase `speed-threshold`
- Use linear curve (`curve-type = <0>`)

### Not Sensitive Enough

- Increase `sensitivity` or `max-factor`
- Decrease `speed-threshold`
- Use strong curve (`curve-type = <2>`)

### Jerky Movement

- Reduce `max-factor`
- Use mild curve (`curve-type = <1>`)
- Use linear exponential curve (`acceleration-exponent = <1>`) in Level 2
- Enable `track-remainders` for smoother movement

### Small Movements Lost

- Enable `track-remainders` to accumulate fractional movements
- Reduce `min-factor` to preserve small movements
- Lower `speed-threshold` to start acceleration earlier

## Tested Configurations

### Optimized Widescreen Setup (Level 2)

This configuration has been tested and optimized for widescreen displays:

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>;
    track-remainders;             // Enable precision tracking
    sensitivity = <1300>;         // 1.3x base sensitivity
    max-factor = <5000>;          // 5.0x maximum acceleration
    curve-type = <2>;             // Strong curve for responsive feel
    y-boost = <2500>;             // 2.5x Y-axis boost for widescreen
    speed-threshold = <200>;      // Early acceleration start
    speed-max = <4000>;           // Lower speed for max acceleration
    min-factor = <800>;           // 0.8x minimum for precision
    acceleration-exponent = <4>;  // Strong exponential curve
    sensor-dpi = <800>;           // 800 DPI sensor
};
```

**Features:**

- **Responsive acceleration** - Starts accelerating at low speeds (200 counts/sec)
- **Strong Y-axis boost** - 2.5x multiplier for comfortable vertical movement
- **Smooth curves** - Strong exponential acceleration for natural feel
- **High precision** - Remainder tracking enabled for smooth small movements
