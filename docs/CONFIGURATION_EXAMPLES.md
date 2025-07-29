# Configuration Examples

## Level 1: Simple Configuration

### Using Presets (Recommended)

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

```ini
# In prj.conf
CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_STANDARD=y
```

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>;
    sensitivity = <1200>;         // 1.2x base sensitivity
    max-factor = <3000>;          // 3.0x maximum acceleration
    curve-type = <1>;             // Mild curve
    y-boost = <1300>;             // 1.3x Y-axis boost
    speed-threshold = <600>;      // Start acceleration at 600 counts/sec
    speed-max = <3500>;           // Max acceleration at 3500 counts/sec
    sensor-dpi = <800>;           // 800 DPI sensor (optional)
};
```

## Level 3: Advanced Configuration

```ini
# In prj.conf
CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_ADVANCED=y
```

```devicetree
&pointer_accel {
    input-type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>;
    track-remainders;             // Enable precision tracking

    min-factor = <1000>;          // 1.0x minimum
    max-factor = <4000>;          // 4.0x maximum acceleration
    speed-threshold = <500>;      // Start at 500 counts/sec
    speed-max = <4000>;           // Max at 4000 counts/sec
    acceleration-exponent = <2>;  // Mild exponential curve
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

- Enable `track-remainders` (Level 3)
- Reduce `max-factor`
- Use mild curve (`curve-type = <1>`)
