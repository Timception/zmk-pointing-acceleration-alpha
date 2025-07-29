# Quick Start Guide

## 1. Choose Your Level

- **Level 1 (Simple)**: 3 basic settings + presets - recommended for most users
- **Level 2 (Standard)**: 6 settings with speed-based acceleration
- **Level 3 (Advanced)**: 12+ settings for complete customization

## 2. Basic Setup

### Add to west.yml

```yaml
manifest:
  remotes:
    - name: nuovotaka
      url-base: https://github.com/nuovotaka
  projects:
    - name: zmk-pointing-acceleration-alpha
      remote: nuovotaka
      revision: main
```

### Add to your overlay file

```c
#include <input/processors.dtsi>
#include <behaviors/input_gestures_accel.dtsi>
```

### Configure in prj.conf

```ini
# Choose ONE level
CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_SIMPLE=y      # Level 1
# CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_STANDARD=y  # Level 2
# CONFIG_INPUT_PROCESSOR_ACCEL_LEVEL_ADVANCED=y  # Level 3

# For Level 1, choose a preset (optional)
CONFIG_INPUT_PROCESSOR_ACCEL_PRESET_GAMING=y
```

## 3. Add to Input Chain

```devicetree
/ {
    tpad0: tpad0 {
        compatible = "zmk,input-listener";
        device = <&glidepoint>;
        input-processors = <&pointer_accel>;
    };
};
```

## 4. Test and Adjust

Start with defaults, then fine-tune based on your preference.

For detailed configuration options, see [CONFIGURATION_EXAMPLES.md](CONFIGURATION_EXAMPLES.md).
