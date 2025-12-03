package com.rusefi.trigger;

import java.util.*;

/**
 * Represents a captured trigger waveform from a logic analyzer.
 */
public class TriggerCapture {
    private double sampleRate;
    private List<Channel> channels;
    private double duration;
    
    public TriggerCapture() {
        this.channels = new ArrayList<>();
    }
    
    public void setSampleRate(double sampleRate) {
        this.sampleRate = sampleRate;
    }
    
    public double getSampleRate() {
        return sampleRate;
    }
    
    public void addChannel(Channel channel) {
        this.channels.add(channel);
    }
    
    public List<Channel> getChannels() {
        return channels;
    }
    
    public void setDuration(double duration) {
        this.duration = duration;
    }
    
    public double getDuration() {
        return duration;
    }
    
    /**
     * Represents a single channel in the capture.
     */
    public static class Channel {
        private String name;
        private List<Edge> edges;
        
        public Channel(String name) {
            this.name = name;
            this.edges = new ArrayList<>();
        }
        
        public void addEdge(Edge edge) {
            this.edges.add(edge);
        }
        
        public String getName() {
            return name;
        }
        
        public List<Edge> getEdges() {
            return edges;
        }
    }
    
    /**
     * Represents an edge (transition) in the waveform.
     */
    public static class Edge {
        private double timestamp;
        private boolean rising;
        
        public Edge(double timestamp, boolean rising) {
            this.timestamp = timestamp;
            this.rising = rising;
        }
        
        public double getTimestamp() {
            return timestamp;
        }
        
        public boolean isRising() {
            return rising;
        }
        
        public boolean isFalling() {
            return !rising;
        }
    }
}
