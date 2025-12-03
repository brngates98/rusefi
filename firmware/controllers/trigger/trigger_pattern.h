/**
 * @file	trigger_pattern.h
 * @brief	Data structures for trigger pattern definitions
 *
 * @date Dec 3, 2024
 * @author rusEFI Team
 */

#pragma once

#include "trigger_channel.h"
#include "rusefi_enums.h"

/**
 * @brief Type of trigger pattern
 */
enum class PatternType : uint8_t {
	SKIPPED_TOOTH,           // N-M pattern (e.g., 60-2, 36-1)
	MULTITOOTH,              // Multiple teeth without home pulse
	MULTITOOTH_HOME,         // Multiple teeth with home/index pulse
	MULTITOOTH_HOME_CAM,     // Multiple teeth with home and cam signals
	CUSTOM,                  // Fully custom pattern defined by events
};

/**
 * @brief Edge type for trigger events
 */
enum class EdgeType : uint8_t {
	RISE = 0,
	FALL = 1,
};

/**
 * @brief Individual tooth/event specification
 */
struct ToothSpec {
	angle_t angle;           // Angle position in degrees
	EdgeType edge;           // Rising or falling edge
	TriggerChannel channel;  // Which input channel
};

/**
 * @brief Gap specification for synchronization
 */
struct GapSpec {
	uint16_t index;          // Gap index (0-based)
	float ratioFrom;         // Minimum gap ratio
	float ratioTo;           // Maximum gap ratio
};

/**
 * @brief Complete trigger pattern data structure
 * 
 * This structure defines a complete trigger pattern that can be
 * loaded at runtime or compiled from YAML at build time.
 */
struct TriggerPatternData {
	// Basic identification
	const char* name;
	trigger_type_e id;
	
	// Operation characteristics
	operation_mode_e operationMode;
	SyncEdge syncEdge;
	
	// Pattern type and configuration
	PatternType patternType;
	
	// For skipped tooth patterns
	uint16_t totalTeeth;
	uint16_t missingTeeth;
	
	// For custom patterns
	const ToothSpec* events;
	uint16_t eventCount;
	
	// Synchronization gaps
	const GapSpec* gaps;
	uint8_t gapCount;
	bool useOnlyPrimaryForSync;
	
	// Metadata
	angle_t tdcPosition;
	bool isCrankBased;
	bool hasSecondChannel;
	bool hasThirdChannel;
	bool knownOperationMode;
};
