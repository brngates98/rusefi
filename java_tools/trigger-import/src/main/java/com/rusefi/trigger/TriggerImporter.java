package com.rusefi.trigger;

import java.io.*;
import java.util.*;

/**
 * Main class for importing trigger patterns from logic analyzer captures.
 * 
 * Supports multiple capture formats:
 * - Sigrok/PulseView CSV exports
 * - Saleae Logic CSV exports
 * - VCD (Value Change Dump) files
 * 
 * Usage:
 *   TriggerImporter --input capture.csv --format sigrok --output pattern.yaml
 */
public class TriggerImporter {
    
    private static final String VERSION = "1.0.0";
    
    public static void main(String[] args) {
        if (args.length == 0 || args[0].equals("--help")) {
            printHelp();
            return;
        }
        
        try {
            ImportConfig config = parseArguments(args);
            runImport(config);
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
            e.printStackTrace();
            System.exit(1);
        }
    }
    
    private static void printHelp() {
        System.out.println("TriggerImporter v" + VERSION);
        System.out.println();
        System.out.println("Usage:");
        System.out.println("  TriggerImporter --input <file> --format <format> --output <file> [options]");
        System.out.println();
        System.out.println("Options:");
        System.out.println("  --input <file>      Input capture file");
        System.out.println("  --format <format>   Input format: sigrok, saleae, or vcd");
        System.out.println("  --output <file>     Output YAML pattern file");
        System.out.println("  --name <name>       Pattern name (default: derived from filename)");
        System.out.println("  --channels <list>   Comma-separated channel names (e.g., primary,secondary)");
        System.out.println("  --help              Show this help message");
        System.out.println();
        System.out.println("Examples:");
        System.out.println("  TriggerImporter --input capture.csv --format sigrok --output mazda.yaml");
        System.out.println("  TriggerImporter --input data.vcd --format vcd --output honda.yaml --name \"Honda K20\"");
    }
    
    private static ImportConfig parseArguments(String[] args) {
        ImportConfig config = new ImportConfig();
        
        for (int i = 0; i < args.length; i++) {
            switch (args[i]) {
                case "--input":
                    config.inputFile = args[++i];
                    break;
                case "--format":
                    config.format = args[++i].toLowerCase();
                    break;
                case "--output":
                    config.outputFile = args[++i];
                    break;
                case "--name":
                    config.patternName = args[++i];
                    break;
                case "--channels":
                    config.channels = Arrays.asList(args[++i].split(","));
                    break;
            }
        }
        
        // Validate required arguments
        if (config.inputFile == null || config.outputFile == null) {
            throw new IllegalArgumentException("Missing required arguments: --input and --output");
        }
        
        if (config.format == null) {
            // Try to detect format from file extension
            if (config.inputFile.endsWith(".vcd")) {
                config.format = "vcd";
            } else if (config.inputFile.endsWith(".csv")) {
                config.format = "sigrok"; // Default CSV format
            } else {
                throw new IllegalArgumentException("Cannot detect format. Please specify --format");
            }
        }
        
        return config;
    }
    
    private static void runImport(ImportConfig config) throws IOException {
        System.out.println("Importing trigger pattern...");
        System.out.println("  Input: " + config.inputFile);
        System.out.println("  Format: " + config.format);
        System.out.println("  Output: " + config.outputFile);
        
        // Parse the input file
        TriggerCapture capture;
        switch (config.format) {
            case "sigrok":
                capture = new SigrokCsvParser().parse(config.inputFile);
                break;
            case "saleae":
                capture = new SaleaeParser().parse(config.inputFile);
                break;
            case "vcd":
                capture = new VcdParser().parse(config.inputFile);
                break;
            default:
                throw new IllegalArgumentException("Unsupported format: " + config.format);
        }
        
        // Analyze the capture to find patterns
        PatternAnalyzer analyzer = new PatternAnalyzer();
        TriggerPattern pattern = analyzer.analyze(capture);
        
        // Apply user-specified configuration
        if (config.patternName != null) {
            pattern.setName(config.patternName);
        }
        if (config.channels != null) {
            pattern.setChannelNames(config.channels);
        }
        
        // Generate YAML output
        YamlGenerator generator = new YamlGenerator();
        generator.generate(pattern, config.outputFile);
        
        System.out.println("Successfully generated: " + config.outputFile);
        System.out.println("  Pattern name: " + pattern.getName());
        System.out.println("  Channels: " + pattern.getChannelCount());
        System.out.println("  Events: " + pattern.getEventCount());
    }
    
    private static class ImportConfig {
        String inputFile;
        String outputFile;
        String format;
        String patternName;
        List<String> channels;
    }
}
