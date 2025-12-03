# Trigger Pattern Import Tool

Java-based tool for importing trigger patterns from logic analyzer captures.

## Overview

This tool allows you to capture real trigger waveforms with a logic analyzer and automatically convert them to rusEFI YAML pattern files. This is especially useful for:

- Reverse-engineering unknown trigger patterns
- Validating pattern definitions against real-world data
- Creating patterns for custom or rare engines
- Educational purposes

## Supported Formats

- **Sigrok/PulseView CSV** - Open-source logic analyzer software
- **Saleae Logic CSV** - Popular commercial logic analyzer
- **VCD (Value Change Dump)** - Standard simulation format

## Requirements

- Java 8 or later
- Maven (for building)

## Building

```bash
cd java_tools/trigger-import
mvn clean package
```

## Usage

Basic usage:

```bash
java -jar target/trigger-import.jar \
  --input capture.csv \
  --format sigrok \
  --output pattern.yaml
```

With options:

```bash
java -jar target/trigger-import.jar \
  --input my_capture.csv \
  --format sigrok \
  --output patterns/oem/my_engine.yaml \
  --name "My Engine Pattern" \
  --channels primary,secondary
```

### Command-Line Options

- `--input <file>` - Input capture file (required)
- `--format <format>` - Format: sigrok, saleae, or vcd (auto-detected if not specified)
- `--output <file>` - Output YAML file (required)
- `--name <name>` - Pattern name (default: derived from filename)
- `--channels <list>` - Comma-separated channel names (e.g., primary,secondary,tertiary)
- `--help` - Show help message

## Capturing Trigger Signals

### Hardware Setup

1. Connect logic analyzer probes to trigger signals:
   - Channel 0: Crankshaft position sensor (primary)
   - Channel 1: Camshaft position sensor (secondary, if present)
   - Channel 2: Home/index pulse (tertiary, if present)

2. Use appropriate voltage levels (usually 5V or 12V tolerant)

3. Ensure good ground connection

### Software Setup

#### Using Sigrok/PulseView

1. Open PulseView
2. Configure sample rate (1MHz+ recommended for most patterns)
3. Capture at least 2-3 complete engine cycles
4. Export as CSV: File → Export → CSV
5. Use the exported CSV file as input

#### Using Saleae Logic

1. Open Saleae Logic software
2. Set sample rate appropriately
3. Capture trigger signals
4. Export: Options → Export Data → CSV
5. Use the exported CSV file as input

### Capture Tips

- **Sample rate**: Use at least 1 MHz for most patterns, higher for very fast teeth
- **Duration**: Capture 2-3 complete engine cycles (1440-2160 degrees)
- **Signal quality**: Ensure clean signals without noise or bounce
- **Engine speed**: Capture at steady, low RPM (500-1000 RPM ideal)
- **Labeling**: Label channels correctly in your analyzer software

## Pattern Analysis

The tool automatically:

1. **Parses** the capture file
2. **Detects edges** (rising/falling transitions)
3. **Converts** timestamps to angles (assumes one cycle in capture)
4. **Analyzes** pattern type:
   - Skipped tooth (N-M patterns)
   - Evenly spaced (multitooth)
   - Custom (irregular patterns)
5. **Generates** YAML with metadata

## Output Format

The generated YAML includes:

```yaml
name: "Auto-imported Pattern"
id: TT_CUSTOM_IMPORTED
operation_mode: FOUR_STROKE_CRANK_SENSOR
sync_edge: RiseOnly

channels:
  primary:
    type: custom
    events:
      - { angle: 0.0, edge: rise }
      - { angle: 5.0, edge: fall }
      # ... more events

metadata:
  tdc_position: 0.0
  is_crank_based: true
  has_second_channel: false
  known_operation_mode: true

notes: |
  Auto-imported pattern from logic analyzer capture
```

## Post-Import Steps

After generating the YAML:

1. **Review** the pattern - verify tooth count and timing
2. **Adjust** metadata - set correct TDC position, operation mode
3. **Set ID** - change from TT_CUSTOM_IMPORTED to proper enum value
4. **Add sync gaps** - define gap ratios for synchronization
5. **Test** in simulator before deploying to hardware

## Examples

### Example 1: Import Mazda pattern

```bash
java -jar trigger-import.jar \
  --input mazda_capture.csv \
  --format sigrok \
  --output ../firmware/controllers/trigger/patterns/oem/mazda_custom.yaml \
  --name "Mazda Custom Pattern"
```

### Example 2: Multi-channel import

```bash
java -jar trigger-import.jar \
  --input nissan_vq.csv \
  --format saleae \
  --output patterns/oem/nissan_vq_custom.yaml \
  --name "Nissan VQ Custom" \
  --channels crank,cam
```

## Troubleshooting

### "No edges detected"
- Check that your capture file has actual signal transitions
- Verify the file format is correct
- Ensure channels have binary (0/1) values

### "Pattern too complex"
- Capture may include noise or bouncing
- Try filtering or cleaning up the signal in your analyzer
- Reduce sample rate if too many samples

### "Incorrect angles"
- Tool assumes one complete cycle in the capture
- Verify you captured a full engine cycle
- Manually adjust angles in the YAML if needed

### "VCD parsing not working"
- VCD parser is not yet fully implemented
- Use CSV export instead
- Contribute VCD parser implementation if you need it!

## Architecture

### Main Classes

- `TriggerImporter.java` - Main entry point and CLI
- `TriggerCapture.java` - Data model for captured waveforms
- `SigrokCsvParser.java` - Parses Sigrok CSV format
- `SaleaeParser.java` - Parses Saleae CSV format
- `VcdParser.java` - Parses VCD format (TODO)
- `PatternAnalyzer.java` - Analyzes patterns and detects type
- `TriggerPattern.java` - Data model for analyzed pattern
- `YamlGenerator.java` - Generates YAML output

### Extending

To add support for a new capture format:

1. Create a new parser class implementing the parse() method
2. Add format detection in TriggerImporter.main()
3. Update documentation

## Contributing

Contributions welcome! Areas for improvement:

- Full VCD parser implementation
- Better pattern type detection algorithms
- Automatic sync gap calculation
- GUI for pattern editing
- Support for more analyzer formats

## License

Same as rusEFI project - GPL v3

## See Also

- [Pattern System README](../../../firmware/controllers/trigger/patterns/README.md)
- [rusEFI Trigger Documentation](https://github.com/rusefi/rusefi/wiki/Trigger-Decoder)
- [Supported Patterns](https://github.com/rusefi/rusefi/wiki/Supported-Triggers)
