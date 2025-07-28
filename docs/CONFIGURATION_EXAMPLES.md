# ZMK Input Processor Acceleration - Configuration Examples

This document provides practical configuration examples for all three levels of the ZMK Input Processor Acceleration system.

## Prerequisites

Before using these examples, ensure you have:

1. **Added the module to your `west.yml`**
2. **Included the necessary headers** in your overlay file:
   ```c
   #include <input/processors.dtsi>
   #include <behaviors/input_gestures_accel.dtsi>
   ```
3. **Chosen your configuration level** in `prj.conf`

## Configuration Level Selection

Add one of these to your `prj.conf`:

```ini
# Level 1: Simple (3 settings + presets)
CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_SIMPLE=y

# Level 2: Standard (6 settings)
# CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_STANDARD=y

# Level 3: Advanced (12+ settings)
# CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_ADVANCED=y
```

---

## Level 1: Simple Examples

### Using Presets (Recommended)

#### Gaming Preset

```ini
# In prj.conf:
CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_SIMPLE=y
CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING=y
```

```devicetree
// In your overlay file (e.g., your_keyboard.overlay):
/ {
    trackpad_input_processor {
        acceleration {
            compatible = "zmk,input-processor-acceleration";
            input-type = <INPUT_EV_REL>;
            codes = <INPUT_REL_X>, <INPUT_REL_Y>;
            // Gaming preset values are automatically applied:
            // sensitivity = 1500 (1.5x)
            // max-factor = 3500 (3.5x)
            // curve-type = 2 (Strong)
        };
    };
};

// Add to your input device:
&your_trackpad {
    input-processors = <&trackpad_input_processor>;
};
```

#### Office Preset

```ini
# In prj.conf:
CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_SIMPLE=y
CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_OFFICE=y
```

```devicetree
/ {
    trackpad_input_processor {
        acceleration {
            compatible = "zmk,input-processor-acceleration";
            input-type = <INPUT_EV_REL>;
            codes = <INPUT_REL_X>, <INPUT_REL_Y>;
            // Office preset values are automatically applied:
            // sensitivity = 1000 (1.0x)
            // max-factor = 2000 (2.0x)
            // curve-type = 0 (Linear)
        };
    };
};
```

#### 4K Display Preset

```ini
# In prj.conf:
CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_SIMPLE=y
CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_4K=y
```

```devicetree
/ {
    trackpad_input_processor {
        acceleration {
            compatible = "zmk,input-processor-acceleration";
            input-type = <INPUT_EV_REL>;
            codes = <INPUT_REL_X>, <INPUT_REL_Y>;
            // 4K preset values are automatically applied:
            // sensitivity = 1800 (1.8x)
            // max-factor = 4000 (4.0x)
            // curve-type = 1 (Mild)
        };
    };
};
```

### Custom Simple Configuration

```ini
# In prj.conf:
CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_SIMPLE=y
CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_CUSTOM=y
```

```devicetree
/ {
    trackpad_input_processor {
        acceleration {
            compatible = "zmk,input-processor-acceleration";
            input-type = <INPUT_EV_REL>;
            codes = <INPUT_REL_X>, <INPUT_REL_Y>;
            sensitivity = <1300>;     // 1.3x base sensitivity
            max-factor = <2800>;      // 2.8x maximum acceleration
            curve-type = <1>;         // 0=Linear, 1=Mild, 2=Strong
        };
    };
};
```

---

## Level 2: Standard Examples

### Balanced Configuration

```ini
# In prj.conf:
CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_STANDARD=y
```

```devicetree
/ {
    trackpad_input_processor {
        acceleration {
            compatible = "zmk,input-processor-acceleration";
            input-type = <INPUT_EV_REL>;
            codes = <INPUT_REL_X>, <INPUT_REL_Y>;
            sensitivity = <1200>;         // 1.2x base sensitivity
            max-factor = <3000>;          // 3.0x maximum acceleration
            curve-type = <1>;             // Mild curve
            y-boost = <1300>;             // 1.3x Y-axis boost for widescreen
            speed-threshold = <600>;      // Start acceleration at 600 counts/sec
            speed-max = <3500>;           // Max acceleration at 3500 counts/sec
        };
    };
};
```

### 4K Display Configuration

```devicetree
/ {
    trackpad_input_processor {
        acceleration {
            compatible = "zmk,input-processor-acceleration";
            input-type = <INPUT_EV_REL>;
            codes = <INPUT_REL_X>, <INPUT_REL_Y>;
            sensitivity = <1800>;         // Higher sensitivity for 4K
            max-factor = <4000>;          // Higher max acceleration
            curve-type = <2>;             // Strong curve for quick traversal
            y-boost = <1400>;             // Extra Y-axis boost
            speed-threshold = <400>;      // Lower threshold for easier acceleration
            speed-max = <4000>;           // Higher max speed
        };
    };
};
```

---

## Level 3: Advanced Examples

### Gaming Mouse Configuration

```ini
# In prj.conf:
CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_ADVANCED=y
```

```devicetree
/ {
    mouse_input_processor {
        acceleration {
            compatible = "zmk,input-processor-acceleration";
            input-type = <INPUT_EV_REL>;
            codes = <INPUT_REL_X>, <INPUT_REL_Y>;
            track-remainders;             // Enable precision tracking (boolean property)

            // Gaming-optimized settings
            min-factor = <800>;           // Slight deceleration for precision
            max-factor = <5000>;          // High acceleration for quick turns
            speed-threshold = <300>;      // Low threshold for responsive acceleration
            speed-max = <6000>;           // High max speed for fast movements
            acceleration-exponent = <3>;  // Moderate exponential for gaming feel

            // High-DPI gaming mouse settings
            x-aspect-scale = <1000>;      // 1.0x X-axis
            y-aspect-scale = <1200>;      // 1.2x Y-axis for vertical movement
            sensor-dpi = <3200>;          // High-DPI gaming sensor
            target-dpi = <1600>;          // Target effective DPI
            dpi-multiplier = <1200>;      // Moderate multiplier
        };
    };
};
```

### Precision Work Configuration

```devicetree
/ {
    trackpad_input_processor {
        acceleration {
            compatible = "zmk,input-processor-acceleration";
            input-type = <INPUT_EV_REL>;
            codes = <INPUT_REL_X>, <INPUT_REL_Y>;
            track-remainders;             // Enable precision tracking (boolean property)

            // Precision-optimized settings
            min-factor = <600>;           // Deceleration for fine control
            max-factor = <2000>;          // Moderate max acceleration
            speed-threshold = <800>;      // Higher threshold for stable slow movement
            speed-max = <3000>;           // Moderate max speed
            acceleration-exponent = <1>;  // Linear curve for predictability

            // Precision settings
            x-aspect-scale = <1000>;      // 1.0x X-axis
            y-aspect-scale = <1000>;      // 1.0x Y-axis (no bias)
            sensor-dpi = <1600>;          // Standard DPI
            target-dpi = <800>;           // Lower effective DPI for precision
            dpi-multiplier = <800>;       // Reduced multiplier for fine control
        };
    };
};
```

### 4K/5K Display Configuration

```devicetree
/ {
    trackpad_input_processor {
        acceleration {
            compatible = "zmk,input-processor-acceleration";
            input-type = <INPUT_EV_REL>;
            codes = <INPUT_REL_X>, <INPUT_REL_Y>;
            track-remainders;             // Enable precision tracking (boolean property)

            // High-resolution display settings
            min-factor = <1000>;          // 1.0x minimum (no deceleration)
            max-factor = <4000>;          // 4.0x maximum acceleration
            speed-threshold = <500>;      // Acceleration starts at 500 counts/sec
            speed-max = <4000>;           // Max acceleration at 4000 counts/sec
            acceleration-exponent = <2>;  // Mild exponential curve

            // 4K/5K optimized settings
            x-aspect-scale = <1000>;      // 1.0x X-axis scaling
            y-aspect-scale = <1500>;      // 1.5x Y-axis scaling
            sensor-dpi = <1600>;          // 1600 DPI sensor
            target-dpi = <1600>;          // Target 1600 DPI
            dpi-multiplier = <1500>;      // 1.5x DPI multiplier for high-res
        };
    };
};
```

---

## Integration with Input Devices

### Cirque Trackpad Example

```devicetree
&glidepoint {
    status = "okay";
    // ... your trackpad configuration
};

/ {
    trackpad_listener {
        compatible = "zmk,input-listener";
        device = <&glidepoint>;
        input-processors = <&trackpad_input_processor>;
    };
};
```

### Generic Mouse Example

```devicetree
&mouse_device {
    status = "okay";
    // ... your mouse configuration
};

/ {
    mouse_listener {
        compatible = "zmk,input-listener";
        device = <&mouse_device>;
        input-processors = <&mouse_input_processor>;
    };
};
```

---

## Troubleshooting

### Common Issues

1. **Compilation Error**: Make sure you've selected exactly one configuration level in `prj.conf`
2. **No Acceleration**: Verify that your input device is properly configured and the processor is in the input chain
3. **Too Sensitive/Not Sensitive Enough**: Start with a preset (Level 1) and adjust from there

### Testing Your Configuration

1. Start with Level 1 and a preset
2. Test the feel and responsiveness
3. If you need more control, move to Level 2
4. Only use Level 3 if you need specific DPI or aspect ratio control

### Performance Considerations

- Level 1: Minimal CPU overhead
- Level 2: Low CPU overhead
- Level 3: Moderate CPU overhead (due to advanced calculations)

Choose the lowest level that meets your needs for optimal performance.
