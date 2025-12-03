package com.rusefi.trigger;

import java.util.*;

/**
 * Represents an analyzed trigger pattern ready for YAML export.
 */
public class TriggerPattern {
    private String name;
    private String patternType;
    private List<ChannelPattern> channels;
    
    public TriggerPattern() {
        this.channels = new ArrayList<>();
        this.name = "Unknown Pattern";
        this.patternType = "custom";
    }
    
    public void setName(String name) {
        this.name = name;
    }
    
    public String getName() {
        return name;
    }
    
    public void setPatternType(String patternType) {
        this.patternType = patternType;
    }
    
    public String getPatternType() {
        return patternType;
    }
    
    public void addChannel(ChannelPattern channel) {
        this.channels.add(channel);
    }
    
    public List<ChannelPattern> getChannels() {
        return channels;
    }
    
    public int getChannelCount() {
        return channels.size();
    }
    
    public int getEventCount() {
        return channels.stream()
            .mapToInt(ChannelPattern::getEventCount)
            .sum();
    }
    
    public void setChannelNames(List<String> names) {
        for (int i = 0; i < Math.min(names.size(), channels.size()); i++) {
            channels.get(i).setName(names.get(i));
        }
    }
    
    /**
     * Represents a single channel's pattern.
     */
    public static class ChannelPattern {
        private String name;
        private List<Event> events;
        
        public ChannelPattern(String name) {
            this.name = name;
            this.events = new ArrayList<>();
        }
        
        public void setName(String name) {
            this.name = name;
        }
        
        public String getName() {
            return name;
        }
        
        public void addEvent(double angle, boolean rising) {
            events.add(new Event(angle, rising));
        }
        
        public List<Event> getEvents() {
            return events;
        }
        
        public int getEventCount() {
            return events.size();
        }
    }
    
    /**
     * Represents a single edge event.
     */
    public static class Event {
        private double angle;
        private boolean rising;
        
        public Event(double angle, boolean rising) {
            this.angle = angle;
            this.rising = rising;
        }
        
        public double getAngle() {
            return angle;
        }
        
        public boolean isRising() {
            return rising;
        }
        
        public boolean isFalling() {
            return !rising;
        }
    }
}
