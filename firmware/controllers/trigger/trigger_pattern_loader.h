/**
 * @file	trigger_pattern_loader.h
 * @brief	Universal trigger pattern loader
 *
 * @date Dec 3, 2024
 * @author rusEFI Team
 */

#pragma once

#include "trigger_pattern.h"
#include "trigger_structure.h"

/**
 * @brief Initialize a trigger waveform from pattern data
 * 
 * This is the universal loader that replaces the giant switch statement
 * in trigger_structure.cpp. It can handle all pattern types.
 * 
 * @param waveform The waveform to initialize
 * @param pattern The pattern data to load
 */
void initializeFromPattern(TriggerWaveform* waveform, const TriggerPatternData* pattern);

/**
 * @brief Generate a skipped tooth pattern
 * 
 * @param waveform The waveform to initialize
 * @param totalTeeth Total number of teeth
 * @param missingTeeth Number of missing teeth
 * @param operationMode Operation mode (crank sensor, cam sensor, etc.)
 * @param syncEdge Edge to use for synchronization
 */
void generateSkippedToothPattern(TriggerWaveform* waveform, uint16_t totalTeeth, 
                                  uint16_t missingTeeth, operation_mode_e operationMode,
                                  SyncEdge syncEdge);

/**
 * @brief Generate a multi-tooth pattern
 * 
 * @param waveform The waveform to initialize
 * @param totalTeeth Total number of teeth
 * @param operationMode Operation mode
 * @param syncEdge Edge to use for synchronization
 */
void generateMultiToothPattern(TriggerWaveform* waveform, uint16_t totalTeeth,
                                operation_mode_e operationMode, SyncEdge syncEdge);

/**
 * @brief Generate a multi-tooth pattern with home pulse
 * 
 * @param waveform The waveform to initialize
 * @param totalTeeth Total number of teeth on crank
 * @param homeAngle Angle of home pulse
 * @param homeWidth Width of home pulse in degrees
 * @param operationMode Operation mode
 * @param syncEdge Edge to use for synchronization
 */
void generateMultiToothHomePattern(TriggerWaveform* waveform, uint16_t totalTeeth,
                                    angle_t homeAngle, angle_t homeWidth,
                                    operation_mode_e operationMode, SyncEdge syncEdge);
