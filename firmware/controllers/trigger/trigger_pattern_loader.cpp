/**
 * @file	trigger_pattern_loader.cpp
 * @brief	Universal trigger pattern loader implementation
 *
 * @date Dec 3, 2024
 * @author rusEFI Team
 */

#include "pch.h"
#include "trigger_pattern_loader.h"
#include "trigger_universal.h"
#include "state_sequence.h"

void initializeFromPattern(TriggerWaveform* waveform, const TriggerPatternData* pattern) {
	if (!waveform || !pattern) {
		return;
	}

	// Initialize the waveform with basic parameters
	waveform->initialize(pattern->operationMode, pattern->syncEdge);
	
	switch (pattern->patternType) {
		case PatternType::SKIPPED_TOOTH:
			generateSkippedToothPattern(waveform, pattern->totalTeeth, 
			                             pattern->missingTeeth, 
			                             pattern->operationMode, 
			                             pattern->syncEdge);
			break;
			
		case PatternType::MULTITOOTH:
			generateMultiToothPattern(waveform, pattern->totalTeeth,
			                          pattern->operationMode,
			                          pattern->syncEdge);
			break;
			
		case PatternType::MULTITOOTH_HOME:
			// For now, use custom event-based loading
			// Will be implemented when we have home pulse patterns
			break;
			
		case PatternType::MULTITOOTH_HOME_CAM:
		case PatternType::CUSTOM:
			// Load custom events
			if (pattern->events && pattern->eventCount > 0) {
				for (uint16_t i = 0; i < pattern->eventCount; i++) {
					const ToothSpec& event = pattern->events[i];
					TriggerWheel wheel;
					
					// Map channel to wheel correctly
					switch (event.channel) {
						case TriggerChannel::CRANK:
							wheel = TriggerWheel::T_PRIMARY;
							break;
						case TriggerChannel::CAM_SYNC:
							wheel = TriggerWheel::T_SECONDARY;
							break;
						case TriggerChannel::HOME:
							wheel = TriggerWheel::T_TERTIARY;
							break;
						default:
							wheel = TriggerWheel::T_PRIMARY;
							break;
					}
					
					if (event.edge == EdgeType::RISE) {
						waveform->addEvent(event.angle, TriggerValue::RISE, wheel);
					} else {
						waveform->addEvent(event.angle, TriggerValue::FALL, wheel);
					}
				}
			}
			break;
	}
	
	// Set synchronization gaps
	if (pattern->gaps && pattern->gapCount > 0) {
		for (uint8_t i = 0; i < pattern->gapCount; i++) {
			const GapSpec& gap = pattern->gaps[i];
			waveform->setTriggerSynchronizationGap3(gap.index, gap.ratioFrom, gap.ratioTo);
		}
		waveform->gapTrackingLength = pattern->gapCount;
	}
	
	// Set metadata
	waveform->tdcPosition = pattern->tdcPosition;
	waveform->needSecondTriggerInput = pattern->hasSecondChannel;
	waveform->needsThirdTriggerInput = pattern->hasThirdChannel;
	waveform->useOnlyPrimaryForSync = pattern->useOnlyPrimaryForSync;
	
#if EFI_UNIT_TEST
	waveform->knownOperationMode = pattern->knownOperationMode;
#endif
}

void generateSkippedToothPattern(TriggerWaveform* waveform, uint16_t totalTeeth, 
                                  uint16_t missingTeeth, operation_mode_e operationMode,
                                  SyncEdge syncEdge) {
	// Delegate to existing implementation
	initializeSkippedToothTrigger(waveform, totalTeeth, missingTeeth, 
	                               operationMode, syncEdge);
}

void generateMultiToothPattern(TriggerWaveform* waveform, uint16_t totalTeeth,
                                operation_mode_e operationMode, SyncEdge syncEdge) {
	waveform->initialize(operationMode, syncEdge);
	
	angle_t toothAngle = waveform->getCycleDuration() / totalTeeth;
	
	for (uint16_t i = 0; i < totalTeeth; i++) {
		angle_t angle = i * toothAngle;
		waveform->addEvent(angle, TriggerWheel::T_PRIMARY, TriggerValue::RISE);
		waveform->addEvent(angle + toothAngle / 2, TriggerWheel::T_PRIMARY, TriggerValue::FALL);
	}
	
	waveform->isSynchronizationNeeded = false;
}

void generateMultiToothHomePattern(TriggerWaveform* waveform, uint16_t totalTeeth,
                                    angle_t homeAngle, angle_t homeWidth,
                                    operation_mode_e operationMode, SyncEdge syncEdge) {
	// First generate the regular tooth pattern
	generateMultiToothPattern(waveform, totalTeeth, operationMode, syncEdge);
	
	// Add home pulse on tertiary channel (will need T_TERTIARY support)
	// For now, use T_SECONDARY as placeholder
	waveform->addEvent(homeAngle, TriggerWheel::T_SECONDARY, TriggerValue::RISE);
	waveform->addEvent(homeAngle + homeWidth, TriggerWheel::T_SECONDARY, TriggerValue::FALL);
	
	waveform->needSecondTriggerInput = true;
	waveform->isSynchronizationNeeded = true;
}
