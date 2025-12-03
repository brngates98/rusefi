package com.rusefi.trigger;

import java.io.*;

/**
 * Parser for Saleae Logic CSV exports.
 */
public class SaleaeParser {
    
    public TriggerCapture parse(String filename) throws IOException {
        // Similar to Sigrok parser but handles Saleae-specific format
        // For now, delegate to Sigrok parser as formats are similar
        return new SigrokCsvParser().parse(filename);
    }
}
