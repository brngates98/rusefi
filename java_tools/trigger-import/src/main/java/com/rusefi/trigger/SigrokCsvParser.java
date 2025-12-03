package com.rusefi.trigger;

import java.io.*;
import java.util.*;

/**
 * Parser for Sigrok/PulseView CSV exports.
 * 
 * Expected format:
 * Time [s], Channel1, Channel2, ...
 * 0.000000, 0, 1, ...
 * 0.000100, 1, 1, ...
 */
public class SigrokCsvParser {
    
    public TriggerCapture parse(String filename) throws IOException {
        TriggerCapture capture = new TriggerCapture();
        
        try (BufferedReader reader = new BufferedReader(new FileReader(filename))) {
            // Read header line
            String headerLine = reader.readLine();
            if (headerLine == null) {
                throw new IOException("Empty file");
            }
            
            String[] headers = headerLine.split(",");
            List<String> channelNames = new ArrayList<>();
            
            // First column is time, rest are channels
            for (int i = 1; i < headers.length; i++) {
                channelNames.add(headers[i].trim());
                capture.addChannel(new TriggerCapture.Channel(headers[i].trim()));
            }
            
            // Track previous state to detect edges
            boolean[] prevState = new boolean[channelNames.size()];
            Arrays.fill(prevState, false);
            boolean firstLine = true;
            
            // Read data lines
            String line;
            while ((line = reader.readLine()) != null) {
                String[] parts = line.split(",");
                if (parts.length < 2) continue;
                
                double timestamp = Double.parseDouble(parts[0].trim());
                
                for (int i = 0; i < channelNames.size() && (i + 1) < parts.length; i++) {
                    boolean state = parts[i + 1].trim().equals("1");
                    
                    // Detect edge
                    if (!firstLine && state != prevState[i]) {
                        TriggerCapture.Edge edge = new TriggerCapture.Edge(timestamp, state);
                        capture.getChannels().get(i).addEdge(edge);
                    }
                    
                    prevState[i] = state;
                }
                
                firstLine = false;
                capture.setDuration(timestamp);
            }
        }
        
        return capture;
    }
}
