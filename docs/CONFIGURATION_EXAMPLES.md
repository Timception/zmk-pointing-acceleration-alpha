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
};
```

### Custom Standard Settings

```ini
# In prj.conf
CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_STANDARD=y
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
