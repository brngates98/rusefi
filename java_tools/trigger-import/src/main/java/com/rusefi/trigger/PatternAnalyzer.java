package com.rusefi.trigger;

import java.util.*;

/**
 * Analyzes captured trigger waveforms to identify patterns.
 */
public class PatternAnalyzer {
    
    private static final double ANGLE_TOLERANCE = 0.5; // degrees
    
    public TriggerPattern analyze(TriggerCapture capture) {
        TriggerPattern pattern = new TriggerPattern();
        
        if (capture.getChannels().isEmpty()) {
            throw new IllegalArgumentException("No channels in capture");
        }
        
        // Analyze first channel (usually primary/crank)
        TriggerCapture.Channel primaryChannel = capture.getChannels().get(0);
        pattern.addChannel(analyzeChannel(primaryChannel, capture));
        
        // Analyze additional channels if present
        for (int i = 1; i < capture.getChannels().size(); i++) {
            TriggerCapture.Channel channel = capture.getChannels().get(i);
            pattern.addChannel(analyzeChannel(channel, capture));
        }
        
        // Try to detect pattern type
        detectPatternType(pattern);
        
        return pattern;
    }
    
    private TriggerPattern.ChannelPattern analyzeChannel(
            TriggerCapture.Channel channel, TriggerCapture capture) {
        
        TriggerPattern.ChannelPattern channelPattern = 
            new TriggerPattern.ChannelPattern(channel.getName());
        
        // Convert timestamps to angles
        // Assume one complete cycle in the capture
        double duration = capture.getDuration();
        if (duration == 0) {
            duration = 1.0; // Avoid division by zero
        }
        
        double cycleDuration = 720.0; // 720 degrees for 4-stroke
        
        for (TriggerCapture.Edge edge : channel.getEdges()) {
            double angle = (edge.getTimestamp() / duration) * cycleDuration;
            channelPattern.addEvent(angle, edge.isRising());
        }
        
        return channelPattern;
    }
    
    private void detectPatternType(TriggerPattern pattern) {
        if (pattern.getChannelCount() == 0) {
            return;
        }
        
        TriggerPattern.ChannelPattern primary = pattern.getChannels().get(0);
        int eventCount = primary.getEventCount();
        
        // Try to detect if it's a skipped tooth pattern
        if (isSkippedToothPattern(primary)) {
            pattern.setPatternType("skipped_tooth");
        } else if (isEvenlySpaced(primary)) {
            pattern.setPatternType("multitooth");
        } else {
            pattern.setPatternType("custom");
        }
    }
    
    private boolean isSkippedToothPattern(TriggerPattern.ChannelPattern channel) {
        // Analyze gaps between rising edges
        List<Double> gaps = new ArrayList<>();
        List<TriggerPattern.Event> events = channel.getEvents();
        
        double prevRisingAngle = -1;
        for (TriggerPattern.Event event : events) {
            if (event.isRising() && prevRisingAngle >= 0) {
                gaps.add(event.getAngle() - prevRisingAngle);
            }
            if (event.isRising()) {
                prevRisingAngle = event.getAngle();
            }
        }
        
        if (gaps.isEmpty()) {
            return false;
        }
        
        // Look for a significantly larger gap (missing teeth)
        double avgGap = gaps.stream().mapToDouble(Double::doubleValue).average().orElse(0);
        long largeGapCount = gaps.stream()
            .filter(gap -> gap > avgGap * 1.5)
            .count();
        
        return largeGapCount > 0;
    }
    
    private boolean isEvenlySpaced(TriggerPattern.ChannelPattern channel) {
        List<Double> gaps = new ArrayList<>();
        List<TriggerPattern.Event> events = channel.getEvents();
        
        double prevRisingAngle = -1;
        for (TriggerPattern.Event event : events) {
            if (event.isRising() && prevRisingAngle >= 0) {
                gaps.add(event.getAngle() - prevRisingAngle);
            }
            if (event.isRising()) {
                prevRisingAngle = event.getAngle();
            }
        }
        
        if (gaps.size() < 2) {
            return false;
        }
        
        // Check if all gaps are similar
        double avgGap = gaps.stream().mapToDouble(Double::doubleValue).average().orElse(0);
        return gaps.stream().allMatch(gap -> 
            Math.abs(gap - avgGap) < ANGLE_TOLERANCE);
    }
}
