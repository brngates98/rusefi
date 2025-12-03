# Trigger Pattern Migration Guide

This guide explains how to migrate the remaining ~75 trigger patterns from C++ to YAML format.

## Overview

The trigger system refactor is partially complete:
- ✅ Infrastructure is in place
- ✅ Build system working
- ✅ 21 patterns migrated as examples
- ⏳ ~75 patterns remaining to migrate

## Migration Process

### Step 1: Choose a Pattern to Migrate

Pick a trigger from one of these C++ files:
- `firmware/controllers/trigger/decoders/trigger_chrysler.cpp`
- `firmware/controllers/trigger/decoders/trigger_ford.cpp`
- `firmware/controllers/trigger/decoders/trigger_gm.cpp`
- `firmware/controllers/trigger/decoders/trigger_honda.cpp`
- `firmware/controllers/trigger/decoders/trigger_mazda.cpp`
- `firmware/controllers/trigger/decoders/trigger_mitsubishi.cpp`
- `firmware/controllers/trigger/decoders/trigger_nissan.cpp`
- `firmware/controllers/trigger/decoders/trigger_subaru.cpp`
- `firmware/controllers/trigger/decoders/trigger_suzuki.cpp`
- `firmware/controllers/trigger/decoders/trigger_toyota.cpp`
- `firmware/controllers/trigger/decoders/trigger_universal.cpp`
- `firmware/controllers/trigger/decoders/trigger_misc.cpp`
- `firmware/controllers/trigger/decoders/trigger_rover.cpp`
- `firmware/controllers/trigger/decoders/trigger_renault.cpp`
- `firmware/controllers/trigger/decoders/trigger_renix.cpp`
- `firmware/controllers/trigger/decoders/trigger_vw.cpp`

### Step 2: Analyze the C++ Implementation

Look at the initialization function. Example from `trigger_mazda.cpp`:

```cpp
void initializeMazdaMiataNaShape(TriggerWaveform *s) {
    s->initialize(FOUR_STROKE_CAM_SENSOR, SyncEdge::RiseOnly);
    
    s->addEvent720(4.0f, TriggerValue::RISE);
    s->addEvent720(8.0f, TriggerValue::FALL);
    s->addEvent720(66.0f, TriggerValue::RISE);
    s->addEvent720(70.0f, TriggerValue::FALL);
    // ... more events
    
    s->setTriggerSynchronizationGap(3.0);
    s->tdcPosition = 0.0;
}
```

### Step 3: Create YAML File

Create a new YAML file in the appropriate directory:

```yaml
name: "Mazda Miata NA"
id: TT_MAZDA_MIATA_NA
operation_mode: FOUR_STROKE_CAM_SENSOR
sync_edge: RiseOnly

channels:
  primary:
    type: custom
    events:
      - { angle: 4.0, edge: rise }
      - { angle: 8.0, edge: fall }
      - { angle: 66.0, edge: rise }
      - { angle: 70.0, edge: fall }
      # ... continue for all events

sync:
  gaps:
    - { index: 0, from: 3.0, to: 20.0 }
  use_only_primary: true

metadata:
  tdc_position: 0.0
  is_crank_based: false
  has_second_channel: false
  known_operation_mode: true
```

### Step 4: Map C++ to YAML

#### Operation Mode Mapping

| C++ | YAML |
|-----|------|
| `FOUR_STROKE_CAM_SENSOR` | `operation_mode: FOUR_STROKE_CAM_SENSOR` |
| `FOUR_STROKE_CRANK_SENSOR` | `operation_mode: FOUR_STROKE_CRANK_SENSOR` |
| `TWO_STROKE` | `operation_mode: TWO_STROKE` |
| `FOUR_STROKE_SYMMETRICAL_CRANK_SENSOR` | `operation_mode: FOUR_STROKE_SYMMETRICAL_CRANK_SENSOR` |

#### Sync Edge Mapping

| C++ | YAML |
|-----|------|
| `SyncEdge::RiseOnly` | `sync_edge: RiseOnly` |
| `SyncEdge::Rise` | `sync_edge: Rise` |
| `SyncEdge::Fall` | `sync_edge: Fall` |
| `SyncEdge::Both` | `sync_edge: Both` |

#### Event Mapping

```cpp
s->addEvent720(angle, TriggerValue::RISE)
```
becomes:
```yaml
- { angle: <angle>, edge: rise }
```

```cpp
s->addEvent720(angle, TriggerValue::FALL)
```
becomes:
```yaml
- { angle: <angle>, edge: fall }
```

#### Gap Synchronization

```cpp
s->setTriggerSynchronizationGap(2.0);
```
becomes:
```yaml
sync:
  gaps:
    - { index: 0, from: 1.5, to: 3.0 }  # Adjust from/to based on gap value
```

```cpp
s->setTriggerSynchronizationGap3(0, 1.6, 3.5);
s->setTriggerSynchronizationGap3(1, 0.7, 1.3);
```
becomes:
```yaml
sync:
  gaps:
    - { index: 0, from: 1.6, to: 3.5 }
    - { index: 1, from: 0.7, to: 1.3 }
```

#### Skipped Tooth Patterns

If the C++ uses `initializeSkippedToothTrigger()`:

```cpp
initializeSkippedToothTrigger(s, 60, 2, FOUR_STROKE_CRANK_SENSOR, SyncEdge::RiseOnly);
```

Use the simplified YAML format:

```yaml
channels:
  primary:
    type: skipped_tooth
    total_teeth: 60
    missing_teeth: 2
```

### Step 5: Choose Output Directory

- **Skipped tooth patterns** → `patterns/skipped_tooth/`
- **Multi-tooth with home** → `patterns/multitooth_home/`
- **OEM-specific** → `patterns/oem/`
- **Custom/experimental** → `patterns/custom/`

### Step 6: Add to Index

Edit `patterns/index.yaml` and add your new pattern file to the appropriate category:

```yaml
oem:
  description: "OEM-specific trigger patterns"
  patterns:
    # ... existing patterns
    - my_new_pattern.yaml  # Add here
```

### Step 7: Regenerate C++

Run the build script:

```bash
cd firmware/controllers/trigger
python3 build_triggers.py
```

Verify the generated files compile correctly.

### Step 8: Test (Optional but Recommended)

If possible, test the pattern in the simulator or with unit tests to ensure it matches the original C++ behavior.

## Common Patterns

### Pattern 1: Simple Skipped Tooth

**C++ Example:**
```cpp
case trigger_type_e::TT_TOOTHED_WHEEL_60_2:
    initializeSkippedToothTrigger(this, 60, 2, triggerOperationMode, SyncEdge::RiseOnly);
    break;
```

**YAML:**
```yaml
name: "60-2 Toothed Wheel"
id: TT_TOOTHED_WHEEL_60_2
operation_mode: FOUR_STROKE_CRANK_SENSOR
sync_edge: RiseOnly

channels:
  primary:
    type: skipped_tooth
    total_teeth: 60
    missing_teeth: 2

sync:
  gaps:
    - { index: 0, from: 1.5, to: 3.0 }
  use_only_primary: true

metadata:
  tdc_position: 0.0
  is_crank_based: true
  has_second_channel: false
  known_operation_mode: true
```

### Pattern 2: Custom Event List

**C++ Example:**
```cpp
void configureFordAspireTriggerWaveform(TriggerWaveform *s) {
    s->initialize(FOUR_STROKE_CRANK_SENSOR, SyncEdge::RiseOnly);
    
    s->addEvent720(0.0f, TriggerValue::RISE);
    s->addEvent720(180.0f, TriggerValue::FALL);
    s->addEvent720(180.0f, TriggerValue::RISE);
    s->addEvent720(360.0f, TriggerValue::FALL);
    
    s->isSynchronizationNeeded = false;
}
```

**YAML:**
```yaml
name: "Ford Aspire"
id: TT_FORD_ASPIRE
operation_mode: FOUR_STROKE_CRANK_SENSOR
sync_edge: RiseOnly

channels:
  primary:
    type: custom
    events:
      - { angle: 0.0, edge: rise }
      - { angle: 180.0, edge: fall }
      - { angle: 180.0, edge: rise }
      - { angle: 360.0, edge: fall }

sync:
  use_only_primary: true

metadata:
  tdc_position: 0.0
  is_crank_based: true
  has_second_channel: false
  known_operation_mode: true
```

### Pattern 3: Dual Channel (Crank + Cam)

**C++ Example:**
```cpp
void initializeSubaru7_6(TriggerWaveform *s) {
    s->initialize(FOUR_STROKE_CAM_SENSOR, SyncEdge::Rise);
    
    // Primary channel (cam)
    s->addEvent720(0.0f, TriggerValue::RISE, TriggerWheel::T_PRIMARY);
    s->addEvent720(5.0f, TriggerValue::FALL, TriggerWheel::T_PRIMARY);
    // ... more events
    
    s->needSecondTriggerInput = false;
    s->setTriggerSynchronizationGap(3.0);
}
```

**YAML:**
```yaml
name: "Subaru 7-6"
id: TT_SUBARU_7_6
operation_mode: FOUR_STROKE_CAM_SENSOR
sync_edge: Rise

channels:
  primary:
    type: custom
    events:
      - { angle: 0.0, edge: rise }
      - { angle: 5.0, edge: fall }
      # ... continue for all events

sync:
  gaps:
    - { index: 0, from: 3.0, to: 20.0 }
  use_only_primary: true

metadata:
  tdc_position: 0.0
  is_crank_based: false
  has_second_channel: false  # Even though it's a cam sensor
  known_operation_mode: true
```

## Remaining Patterns to Migrate

### Priority 1 - Common Patterns
- [ ] TT_TOOTHED_WHEEL (custom N-M)
- [ ] TT_MAZDA_MIATA_VVT_TEST
- [ ] TT_VVT_MIATA_NB
- [ ] TT_SUBARU_EZ30
- [ ] TT_NISSAN_VQ30
- [ ] TT_NISSAN_VQ35

### Priority 2 - OEM Patterns
- [ ] TT_DODGE_RAM
- [ ] TT_JEEP_4_CYL
- [ ] TT_JEEP_18_2_2_2
- [ ] TT_JEEP_EVD_36_2_2
- [ ] TT_CHRYSLER_PHASER
- [ ] TT_FORD_ST170
- [ ] TT_VVT_FORD_COYOTE
- [ ] TT_GM_60_2_2_2
- [ ] TT_GM_24x_5
- [ ] TT_GM_24x_3

### Priority 3 - Remaining Patterns
See the complete list in the problem statement. Approximately 60 more patterns.

## Tips and Tricks

1. **Start simple**: Begin with skipped tooth patterns, they're easiest
2. **Copy similar patterns**: Use existing YAMLs as templates
3. **Check unit tests**: Look at test files to understand expected behavior
4. **Verify angles**: Make sure angles are in correct range (0-720 for 4-stroke)
5. **Document**: Add notes explaining the pattern, especially for unusual ones
6. **Test incrementally**: Regenerate and verify after each pattern

## Troubleshooting

### Build script fails
- Check YAML syntax with a validator
- Ensure all required fields are present
- Check that angles are valid numbers

### Pattern doesn't sync
- Verify gap ratios match the C++ implementation
- Check that sync edge is correct
- Ensure events are in chronological order

### Missing events
- Count total events in C++ vs YAML
- Check for events on secondary channel
- Look for conditional event addition in C++

## Automation Ideas

For bulk migration, consider:
1. Write a C++ parser to extract pattern data
2. Create automated conversion script
3. Use regex to extract event lists
4. Generate YAML templates automatically

## Questions?

Refer to:
- `patterns/README.md` - Pattern system documentation
- `build_triggers.py` - Build script with examples
- Existing YAML files - Working examples
- C++ files - Original implementations

## Progress Tracking

Update the progress checklist as patterns are migrated. Current status: 21/95 complete (22%).
