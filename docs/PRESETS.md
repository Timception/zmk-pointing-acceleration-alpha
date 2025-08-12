[ English | [Japanese](PRESETS-j.md) ]

# Preset Configuration Reference

This document provides detailed information about all available presets and their settings for both Level 1 and Level 2.

## 📋 **Available Presets**

### **Office Presets** (Conservative acceleration for productivity)

- `office_optical` - Optical mouse (800 DPI)
- `office_laser` - Laser mouse (1600 DPI)
- `office_trackball` - Trackball (400 DPI)
- `office_trackpad` - Trackpad (1000 DPI)

### **Gaming Presets** (Balanced acceleration for gaming)

- `gaming_optical` - Gaming optical mouse (1200 DPI)
- `gaming_laser` - Gaming laser mouse (3200 DPI)
- `gaming_trackball` - Gaming trackball (800 DPI)
- `gaming_trackpad` - Gaming trackpad (1200 DPI)

### **High Sensitivity Presets** (Aggressive acceleration for fast movement)

- `high_sens_optical` - High sensitivity optical (1600 DPI)
- `high_sens_laser` - High sensitivity laser (6400 DPI)
- `high_sens_trackball` - High sensitivity trackball (800 DPI)
- `high_sens_trackpad` - High sensitivity trackpad (1200 DPI)

## ⚙️ **Detailed Preset Settings**

### **Office Presets**

#### `office_optical` (800 DPI Optical Mouse Pointing Device)

```c
// Level 1 & 2 Common Settings
sensitivity = 1000;      // 1.0x base sensitivity
max_factor = 2200;       // 2.2x maximum acceleration
curve_type = 1;          // Mild curve
y_boost = 1080;          // 1.08x Y-axis boost
sensor_dpi = 800;        // 800 DPI sensor

// Level 2 Additional Settings
speed_threshold = 700;   // Start acceleration at 700 counts/sec
speed_max = 2600;        // Max acceleration at 2600 counts/sec
min_factor = 980;        // 0.98x minimum for precision
acceleration_exponent = 1; // Linear acceleration
```

**Characteristics:**

- Conservative acceleration for office work
- Slight Y-axis boost for productivity
- Linear acceleration curve for predictability

#### `office_laser` (1600 DPI Laser Mouse Pointing Device)

```c
// Level 1 & 2 Common Settings
sensitivity = 1000;      // 1.0x base sensitivity (DPI compensated)
max_factor = 1500;       // 1.5x conservative acceleration
curve_type = 0;          // Linear curve
y_boost = 1000;          // 1.0x Y-axis (standard)
sensor_dpi = 1600;       // 1600 DPI laser sensor

// Level 2 Additional Settings
speed_threshold = 1000;  // High threshold for stability
speed_max = 2000;        // Low max speed for stability
min_factor = 1000;       // 1.0x minimum (no deceleration)
acceleration_exponent = 1; // Linear acceleration
```

**Characteristics:**

- Very conservative for high-DPI laser sensors
- Stability-focused settings
- Minimal acceleration to prevent cursor jumping

#### `office_trackball` (400 DPI Trackball)

```c
// Level 1 & 2 Common Settings
sensitivity = 1500;      // 1.5x base sensitivity (low DPI compensation)
max_factor = 2000;       // 2.0x moderate acceleration
curve_type = 1;          // Mild curve
y_boost = 1100;          // 1.1x Y-axis boost
sensor_dpi = 400;        // 400 DPI trackball

// Level 2 Additional Settings
speed_threshold = 800;   // Stable response
speed_max = 2400;        // Moderate max speed
min_factor = 950;        // 0.95x for precision
acceleration_exponent = 1; // Linear acceleration
```

**Characteristics:**

- Higher base sensitivity for low-DPI trackball
- Moderate acceleration suitable for trackball use
- Precision-focused minimum factor

### **Gaming Presets**

#### `gaming_optical` (1200 DPI Gaming Mouse Pointing Device)

```c
// Level 1 & 2 Common Settings
sensitivity = 1000;      // 1.0x base sensitivity
max_factor = 2500;       // 2.5x aggressive acceleration
curve_type = 2;          // Strong curve
y_boost = 1120;          // 1.12x Y-axis balance
sensor_dpi = 1200;       // 1200 DPI gaming sensor

// Level 2 Additional Settings
speed_threshold = 550;   // Quick response
speed_max = 2800;        // High-speed movement support
min_factor = 950;        // 0.95x for precise aiming
acceleration_exponent = 2; // Mild exponential
```

**Characteristics:**

- Balanced for gaming with quick response
- Strong acceleration curve for fast movements
- Precision support for aiming

#### `gaming_laser` (3200 DPI Gaming Laser)

```c
// Level 1 & 2 Common Settings
sensitivity = 600;       // 0.6x base sensitivity (high DPI compensation)
max_factor = 2500;       // 2.5x aggressive acceleration
curve_type = 2;          // Strong curve
y_boost = 1120;          // 1.12x Y-axis balance
sensor_dpi = 3200;       // 3200 DPI laser sensor

// Level 2 Additional Settings
speed_threshold = 550;   // Quick response
speed_max = 2800;        // High-speed movement support
min_factor = 950;        // 0.95x for precise aiming
acceleration_exponent = 2; // Mild exponential
```

**Characteristics:**

- Reduced base sensitivity for high-DPI sensor
- Same acceleration characteristics as optical gaming
- Optimized for high-DPI gaming mice

### **High Sensitivity Presets**

#### `high_sens_optical` (1600 DPI High-End Optical)

```c
// Level 1 & 2 Common Settings
sensitivity = 1100;      // 1.1x base sensitivity
max_factor = 2800;       // 2.8x strong acceleration
curve_type = 1;          // Mild curve (controllable)
y_boost = 1150;          // 1.15x Y-axis boost
sensor_dpi = 1600;       // 1600 DPI sensor

// Level 2 Additional Settings
speed_threshold = 450;   // Immediate response
speed_max = 2400;        // Moderate adjustment
min_factor = 900;        // 0.9x for precision
acceleration_exponent = 2; // Mild exponential
```

**Characteristics:**

- High acceleration for fast movement
- Controlled curve to maintain precision
- Quick response threshold

#### `high_sens_laser` (6400 DPI Ultra-High DPI)

```c
// Level 1 & 2 Common Settings
sensitivity = 500;       // 0.5x base sensitivity (ultra-high DPI compensation)
max_factor = 2800;       // 2.8x strong acceleration
curve_type = 1;          // Mild curve (controllable)
y_boost = 1150;          // 1.15x Y-axis boost
sensor_dpi = 6400;       // 6400 DPI ultra-high sensor

// Level 2 Additional Settings
speed_threshold = 450;   // Immediate response
speed_max = 2400;        // Moderate adjustment
min_factor = 900;        // 0.9x for precision
acceleration_exponent = 2; // Mild exponential
```

**Characteristics:**

- Very low base sensitivity for ultra-high DPI
- Strong acceleration to compensate
- Precision-focused for high-DPI sensors

## 🎯 **Choosing the Right Preset**

### **By Pointing Device Type**

**Optical Mouse:**

- Office work: `office_optical`
- Gaming: `gaming_optical`
- Fast movement: `high_sens_optical`

**Laser Mouse:**

- Office work: `office_laser`
- Gaming: `gaming_laser`
- Fast movement: `high_sens_laser`

**Trackball:**

- Office work: `office_trackball`
- Gaming: `gaming_trackball`
- Fast movement: `high_sens_trackball`

**Trackpad:**

- Office work: `office_trackpad`
- Gaming: `gaming_trackpad`
- Fast movement: `high_sens_trackpad`

### **By Usage Pattern**

**Conservative (Office):**

- Lower acceleration factors
- Linear or mild curves
- Stability-focused thresholds

**Balanced (Gaming):**

- Moderate to high acceleration
- Strong curves for responsiveness
- Quick response thresholds

**Aggressive (High Sensitivity):**

- High acceleration factors
- Controlled curves for precision
- Immediate response thresholds

## 🔧 **Customization Guidelines**

### **Starting from a Preset**

1. **Choose the closest preset** to your device and usage
2. **Test the preset** as-is first
3. **Make small adjustments** to individual settings
4. **Document your changes** for future reference

### **Common Adjustments**

**Too Slow Overall:**

- Increase `sensitivity`
- Increase `max_factor`

**Too Fast Overall:**

- Decrease `sensitivity`
- Decrease `max_factor`

**Acceleration Too Aggressive:**

- Lower `curve_type` (Level 1)
- Lower `acceleration_exponent` (Level 2)
- Increase `speed_threshold` (Level 2)

**Not Enough Acceleration:**

- Higher `curve_type` (Level 1)
- Higher `acceleration_exponent` (Level 2)
- Decrease `speed_threshold` (Level 2)

## 📊 **Preset Comparison Table**

| Preset              | Base Sens | Max Accel | Curve Style | Response  | Best For                      |
| ------------------- | --------- | --------- | ----------- | --------- | ----------------------------- |
| `office_optical`    | 1.0x      | 2.2x      | Mild        | Moderate  | Office work, optical mouse    |
| `office_laser`      | 1.0x      | 1.5x      | Linear      | Slow      | Office work, laser mouse      |
| `gaming_optical`    | 1.0x      | 2.5x      | Strong      | Fast      | Gaming, optical mouse         |
| `gaming_laser`      | 0.6x      | 2.5x      | Strong      | Fast      | Gaming, high-DPI laser        |
| `high_sens_optical` | 1.1x      | 2.8x      | Mild        | Immediate | Fast movement, optical        |
| `high_sens_laser`   | 0.5x      | 2.8x      | Mild        | Immediate | Fast movement, ultra-high DPI |

## 💡 **Pro Tips**

1. **DPI Matters**: Higher DPI sensors have lower base sensitivity in presets
2. **Test Both Levels**: Same preset behaves differently in Level 1 vs Level 2
3. **Start Conservative**: Begin with office presets and increase if needed
4. **Document Settings**: Keep track of what works for your setup
5. **Consider Usage**: Gaming presets for games, office presets for work

---

**Remember**: Presets are starting points. Feel free to customize them for your specific needs and preferences.
