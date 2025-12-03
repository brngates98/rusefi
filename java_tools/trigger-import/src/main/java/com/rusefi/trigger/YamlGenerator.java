package com.rusefi.trigger;

import java.io.*;

/**
 * Generates YAML output from analyzed trigger patterns.
 */
public class YamlGenerator {
    
    public void generate(TriggerPattern pattern, String outputFile) throws IOException {
        try (PrintWriter writer = new PrintWriter(new FileWriter(outputFile))) {
            writer.println("name: \"" + pattern.getName() + "\"");
            writer.println("id: TT_CUSTOM_IMPORTED");
            writer.println("operation_mode: FOUR_STROKE_CRANK_SENSOR");
            writer.println("sync_edge: RiseOnly");
            writer.println();
            
            writer.println("channels:");
            
            // Write primary channel
            if (pattern.getChannelCount() > 0) {
                writeChannel(writer, pattern.getChannels().get(0), "primary");
            }
            
            // Write secondary channel if present
            if (pattern.getChannelCount() > 1) {
                writeChannel(writer, pattern.getChannels().get(1), "secondary");
            }
            
            // Write tertiary channel if present
            if (pattern.getChannelCount() > 2) {
                writeChannel(writer, pattern.getChannels().get(2), "tertiary");
            }
            
            writer.println();
            writer.println("sync:");
            writer.println("  use_only_primary: true");
            writer.println();
            
            writer.println("metadata:");
            writer.println("  tdc_position: 0.0");
            writer.println("  is_crank_based: true");
            writer.println("  has_second_channel: " + (pattern.getChannelCount() > 1));
            writer.println("  has_third_channel: " + (pattern.getChannelCount() > 2));
            writer.println("  known_operation_mode: true");
            writer.println();
            
            writer.println("notes: |");
            writer.println("  Auto-imported pattern from logic analyzer capture");
            writer.println("  Pattern type: " + pattern.getPatternType());
            writer.println("  Total events: " + pattern.getEventCount());
        }
    }
    
    private void writeChannel(PrintWriter writer, TriggerPattern.ChannelPattern channel, 
                              String channelName) {
        writer.println("  " + channelName + ":");
        writer.println("    type: custom");
        writer.println("    events:");
        
        for (TriggerPattern.Event event : channel.getEvents()) {
            String edge = event.isRising() ? "rise" : "fall";
            writer.println(String.format("      - { angle: %.2f, edge: %s }", 
                event.getAngle(), edge));
        }
        writer.println();
    }
}
