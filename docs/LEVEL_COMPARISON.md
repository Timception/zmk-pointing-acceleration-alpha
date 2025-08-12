[ English | [Japanese](LEVEL_COMPARISON-j.md) ]

# Level 1 vs Level 2: Detailed Comparison Guide

This guide explains the fundamental differences between Level 1 (Simple) and Level 2 (Standard) acceleration systems.

## 🚨 **Critical Understanding**

**The same preset name behaves completely differently between Level 1 and Level 2** due to different calculation methods and curve systems. This is not a bug - it's by design to provide two distinct acceleration experiences.

## 📊 **Calculation Methods**

### **Level 1: Input-Based Acceleration**

```
Input Value → Curve Application → Output Value
```

- **Response**: **Immediate** - larger input values get instant acceleration
- **Calculation**: Based directly on the size of each input movement
- **Predictability**: Same input always produces same output
- **Best For**: Gaming, immediate response, predictable behavior

### **Level 2: Speed-Based Acceleration**

```
Input Value → Speed Calculation → Speed-Based Curve → Output Value
```

- **Response**: **Gradual** - acceleration builds up based on movement speed over time
- **Calculation**: Analyzes movement speed history and applies acceleration accordingly
- **Adaptability**: Responds to movement patterns and speed changes
- **Best For**: Office work, natural feel, smooth acceleration

## ⚙️ **Settings Usage by Level**

| Setting                 | Level 1        | Level 2        | Purpose                                    |
| ----------------------- | -------------- | -------------- | ------------------------------------------ |
| `sensitivity`           | ✅ **Used**    | ✅ **Used**    | Base sensitivity multiplier                |
| `max-factor`            | ✅ **Used**    | ✅ **Used**    | Maximum acceleration factor                |
| `curve-type`            | ✅ **Used**    | ❌ **Ignored** | Level 1 curve (0=Linear, 1=Mild, 2=Strong) |
| `y-boost`               | ✅ **Used**    | ✅ **Used**    | Y-axis sensitivity boost                   |
| `sensor-dpi`            | ✅ **Used**    | ✅ **Used**    | DPI compensation                           |
| `speed-threshold`       | ❌ **Not set** | ✅ **Used**    | Speed to start acceleration                |
| `speed-max`             | ❌ **Not set** | ✅ **Used**    | Speed for maximum acceleration             |
| `min-factor`            | ❌ **Not set** | ✅ **Used**    | Minimum acceleration factor                |
| `acceleration-exponent` | ❌ **Not set** | ✅ **Used**    | Level 2 exponential curve (1-5)            |

## 🎯 **Curve Systems**

### **Level 1 Curves (`curve-type`)**

**Type 0: Linear**

```c
acceleration = 1.0x + (input_value × linear_multiplier)
```

- Constant acceleration rate
- Predictable scaling

**Type 1: Mild (Quadratic)**

```c
acceleration = 1.0x + (input_value² × 10 / 100)
```

- Gentle curve for smooth feel
- Moderate acceleration increase

**Type 2: Strong (Quadratic)**

```c
acceleration = 1.0x + (input_value² × 20 / 100)
```

- Aggressive curve for gaming
- Rapid acceleration increase

### **Level 2 Curves (`acceleration-exponent`)**

**Exponent 1: Linear**

```c
f(t) = t
```

- Constant acceleration rate

**Exponent 2: Mild Exponential**

```c
f(t) = e^(2t) - 1
```

- Balanced for general use
- Natural acceleration feel

**Exponent 3: Moderate Exponential**

```c
f(t) = e^(3t) - 1
```

- More responsive than mild
- Good for mixed usage

**Exponent 4: Strong Exponential**

```c
f(t) = e^(4t) - 1
```

- Aggressive acceleration
- Gaming-oriented

**Exponent 5: Aggressive Exponential**

```c
f(t) = e^(5t) - 1
```

- Maximum responsiveness
- Expert-level control

## 🔄 **Same Preset, Different Behavior**

### Example: `gaming_optical` Preset

**Preset Settings:**

```
sensitivity = 1000
max-factor = 2500
curve-type = 2 (Strong)
acceleration-exponent = 2 (Mild)
speed-threshold = 550
```

### **Level 1 Behavior:**

```
Input 5  → Strong quadratic curve → Immediate ~2x acceleration
Input 10 → Strong quadratic curve → Immediate ~4x acceleration
Input 15 → Strong quadratic curve → Immediate ~6x acceleration
```

**Characteristics:**

- Instant response to input size
- Uses `curve-type = 2` (Strong)
- Ignores `speed-threshold` and `acceleration-exponent`

### **Level 2 Behavior:**

```
Input 5  → Speed: 200 → Below threshold (550) → Minimal acceleration
Input 10 → Speed: 400 → Below threshold (550) → Minimal acceleration
Input 15 → Speed: 600 → Above threshold (550) → Mild exponential acceleration
```

**Characteristics:**

- Gradual response based on speed history
- Uses `acceleration-exponent = 2` (Mild)
- Ignores `curve-type`
- Considers `speed-threshold = 550`

## 🎮 **Practical Examples**

### **Gaming Scenario**

**Level 1:**

- Flick pointing device quickly → Cursor gets instant high acceleration
- Small adjustments → Predictable small cursor movements
- **Feel**: Responsive, immediate, gaming-like

**Level 2:**

- Flick pointing device quickly → Cursor acceleration builds up gradually
- Small adjustments → Smooth, natural cursor response
- **Feel**: Smooth, adaptive, natural

### **Office Work Scenario**

**Level 1:**

- Large pointing device movements → Immediate cursor acceleration
- Precise selections → Predictable cursor control
- **Feel**: Direct, immediate feedback

**Level 2:**

- Large pointing device movements → Speed-adaptive cursor acceleration
- Precise selections → Natural cursor deceleration for precision
- **Feel**: Natural, adaptive to work patterns

## 🤔 **Which Level Should You Choose?**

### **Choose Level 1 if you:**

- Want immediate, predictable response
- Prefer gaming-style acceleration
- Like direct input-to-output mapping
- Want simple configuration
- Need consistent behavior regardless of movement patterns

### **Choose Level 2 if you:**

- Want natural, smooth acceleration
- Prefer adaptive behavior
- Like acceleration that responds to movement speed
- Want fine control over acceleration curves
- Need professional/office-style smooth operation

## 🔧 **Configuration Tips**

### **Level 1 Configuration Focus:**

- Adjust `sensitivity` for base speed
- Use `max-factor` to control peak acceleration
- Choose `curve-type` for acceleration feel (0=gentle, 2=aggressive)
- Set `y-boost` for display optimization

### **Level 2 Configuration Focus:**

- Start with `speed-threshold` to control when acceleration begins
- Use `speed-max` to define when maximum acceleration is reached
- Adjust `min-factor` for slow movement behavior
- Choose `acceleration-exponent` for curve feel (1=linear, 5=aggressive)

## 📈 **Performance Characteristics**

| Aspect                       | Level 1   | Level 2  |
| ---------------------------- | --------- | -------- |
| **CPU Usage**                | Lower     | Higher   |
| **Memory Usage**             | Minimal   | Moderate |
| **Response Time**            | Immediate | Gradual  |
| **Predictability**           | High      | Adaptive |
| **Configuration Complexity** | Simple    | Advanced |

## 🚀 **Migration Between Levels**

If you're switching between levels with the same preset:

1. **Level 1 → Level 2**: Expect smoother, more gradual acceleration
2. **Level 2 → Level 1**: Expect more immediate, responsive acceleration
3. **Settings**: Some settings will be ignored, others will behave differently
4. **Testing**: Always test your specific use case after switching

## 💡 **Pro Tips**

1. **Start with presets** - they're optimized for each level
2. **Test both levels** with the same preset to feel the difference
3. **Level 1 for gaming**, Level 2 for office work (generally)
4. **Custom settings** require understanding of the calculation differences
5. **Document your settings** - behavior changes between levels

---

**Remember**: The same preset name provides different experiences by design. Choose the level that matches your preferred acceleration style, not just the preset name.
