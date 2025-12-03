# Trigger Pattern System

This directory contains the new data-driven trigger pattern system for rusEFI.

## Overview

Instead of defining trigger patterns in C++ code with a giant switch statement, patterns are now defined in YAML files and compiled to C++ at build time. This makes the system:

- **More maintainable**: Easy to add new patterns without touching C++ code
- **More memory efficient**: Patterns are stored as compact data structures
- **More extensible**: Supports third trigger input (T_TERTIARY/HOME) for multi-tooth + home patterns
- **Better documented**: YAML format is self-documenting with metadata and notes

## Directory Structure

```
patterns/
├── index.yaml              # Master index of all patterns
├── skipped_tooth/          # Standard N-M patterns (60-2, 36-1, etc.)
├── multitooth_home/        # Multi-tooth with home pulse patterns
├── oem/                    # OEM-specific patterns (Mazda, Honda, etc.)
└── custom/                 # User custom patterns
```

## YAML Pattern Format

Example pattern definition:

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

### Pattern Types

- **skipped_tooth**: Standard N-M patterns (e.g., 60-2, 36-1)
- **multitooth**: Even teeth without gaps
- **multitooth_home**: Multi-tooth with home/index pulse
- **custom**: Fully custom pattern with explicit event list

### Channel Types

- **primary**: Crankshaft position (T_PRIMARY)
- **secondary**: Camshaft sync (T_SECONDARY)
- **tertiary**: Home/index pulse (T_TERTIARY) - new!

### Custom Events

For custom patterns, define explicit events:

```yaml
channels:
  primary:
    type: custom
    events:
      - { angle: 0.0, edge: rise }
      - { angle: 5.0, edge: fall }
      - { angle: 66.0, edge: rise }
      - { angle: 70.0, edge: fall }
```

## Build System

Patterns are compiled to C++ at build time:

```bash
cd firmware/controllers/trigger
python3 build_triggers.py
```

This generates:
- `trigger_patterns_generated.h` - Pattern declarations
- `trigger_patterns_generated.cpp` - Pattern data structures

## Adding a New Pattern

1. Create a YAML file in the appropriate subdirectory:
   - `skipped_tooth/` for N-M patterns
   - `multitooth_home/` for patterns with home pulse
   - `oem/` for manufacturer-specific patterns
   
2. Add the filename to `index.yaml` under the appropriate category

3. Run `build_triggers.py` to regenerate C++ files

4. Pattern is now available in the firmware

## Logic Analyzer Import

Import patterns from logic analyzer captures:

```bash
cd java_tools/trigger-import
java -cp target/trigger-import.jar com.rusefi.trigger.TriggerImporter \
  --input capture.csv \
  --format sigrok \
  --output patterns/oem/my_pattern.yaml \
  --name "My Custom Pattern"
```

Supported formats:
- Sigrok/PulseView CSV
- Saleae Logic CSV
- VCD (Value Change Dump)

The tool will:
1. Parse the capture file
2. Analyze the waveform to detect pattern type
3. Convert timestamps to angles
4. Generate a YAML pattern file

## Migration Status

- ✅ Core infrastructure complete
- ✅ Build system working
- ✅ Logic analyzer import tools created
- ✅ T_TERTIARY/HOME support added
- ✅ Example patterns created (20+ patterns)
- 🔄 Full migration in progress (target: 95+ patterns)

### Completed Patterns

See `index.yaml` for the complete list of migrated patterns.

## Backwards Compatibility

The existing trigger system continues to work alongside the new system. Legacy trigger files remain in `firmware/controllers/trigger/decoders/` and will be gradually phased out as patterns are migrated to YAML.

## Technical Details

### Pattern Data Structure

```cpp
struct TriggerPatternData {
    const char* name;
    trigger_type_e id;
    operation_mode_e operationMode;
    SyncEdge syncEdge;
    PatternType patternType;
    // ... pattern-specific data
};
```

### Pattern Loader

The universal pattern loader (`trigger_pattern_loader.cpp`) replaces the giant switch statement:

```cpp
void initializeFromPattern(TriggerWaveform* waveform, 
                          const TriggerPatternData* pattern);
```

### Trigger Channels

New enum supporting three inputs:

```cpp
enum class TriggerChannel : uint8_t {
    CRANK = 0,        // T_PRIMARY
    CAM_SYNC = 1,     // T_SECONDARY  
    HOME = 2,         // T_TERTIARY - new!
    CAM_EXHAUST = 3,  // Future expansion
};
```

## Future Enhancements

- GUI pattern editor
- Real-time pattern validation
- Pattern similarity detection
- Automatic pattern optimization
- Export patterns to various formats

## References

- [Forum Discussion](https://rusefi.com/forum/)
- [Trigger Decoder Documentation](https://github.com/rusefi/rusefi/wiki/Trigger-Decoder)
- [Supported Trigger Patterns](https://github.com/rusefi/rusefi/wiki/Supported-Triggers)
