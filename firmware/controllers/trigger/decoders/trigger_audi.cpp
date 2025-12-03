/*
 * @file trigger_audi.cpp
 *
 * Audi Trigger DivbyN (coil-type) for Audi 5-cylinder engines
 * 
 * Hardware:
 * - Primary Input: 135-tooth multitooth crank sensor (VR) on flywheel/starter gear
 * - Secondary Input: Crank home sensor - single reference pin on flywheel (VR) at 62° BTDC #1
 * - Cam Input: Cam sensor - single window Hall sensor (cambelt pulley or distributor)
 *
 * The DivbyN Algorithm divides the incoming tooth count to create virtual trigger events:
 * Virtual teeth = (Actual teeth × 2) / Divider
 * Example: (135 × 2) / 3 = 90 virtual teeth per 720° engine cycle
 *
 * The Cam-HALL signal gates the crank home pulse:
 * - Cam-HALL LOW → Masks/blocks crank home pulse (62° BTDC #1)
 * - Cam-HALL HIGH → Allows crank home pulse through (134° BTDC #5)
 * This creates ONE sync pulse per 720° engine cycle with zero slop.
 *
 * Based on VEMS implementation: https://www.vems.hu/wiki/index.php?page=InputTrigger%2FAudiTrigger
 *
 * @date Dec 3, 2024
 * @author Andrey Belomutskiy, (c) 2012-2024
 */

#include "pch.h"
#include "trigger_audi.h"
#include "trigger_universal.h"

void initializeAudiDivbyN(TriggerWaveform *s) {
	// Get configuration parameters
	auto& config = engineConfiguration->trigger;
	
	// Use default values if not set
	uint8_t divider = config.audiTriggerDivider > 0 ? config.audiTriggerDivider : 3;
	uint8_t actualTeeth = config.audiActualTeeth > 0 ? config.audiActualTeeth : 135;
	uint8_t firstTriggerTooth = config.audiFirstTriggerTooth;
	float tdcAfterTrigger = config.audiTdcAfterTrigger > 0 ? config.audiTdcAfterTrigger : 58.0f;
	float toothAngularWidth = config.audiToothAngularWidth > 0 ? config.audiToothAngularWidth : 8.0f;
	float crankPinBTDC = config.audiCrankPinBTDC > 0 ? config.audiCrankPinBTDC : 62.0f;
	
	// Calculate virtual teeth using DivbyN algorithm
	// Virtual teeth = (Actual teeth × 2) / Divider
	int virtualTeeth = (actualTeeth * 2) / divider;
	
	// Initialize as four-stroke crank sensor
	s->initialize(FOUR_STROKE_CRANK_SENSOR, SyncEdge::Rise);
	
	// This trigger requires the secondary input for crank home pulse
	s->needSecondTriggerInput = true;
	
	// The cam sensor gates the secondary input, making it a cam-synced system
	s->isSecondWheelCam = true;
	
	// Calculate tooth angle based on 720° cycle and virtual teeth
	float toothAngle = FOUR_STROKE_ENGINE_CYCLE / virtualTeeth;
	
	// Set up the virtual teeth pattern on the primary wheel
	// Each virtual tooth is created by dividing actual teeth
	float toothWidth = 0.5f; // 50% duty cycle for virtual teeth
	
	// Add all virtual teeth to the primary trigger wheel
	for (int i = 0; i < virtualTeeth; i++) {
		float angleStart = i * toothAngle;
		float angleEnd = angleStart + (toothAngle * toothWidth);
		
		s->addEventClamped(angleStart, TriggerValue::RISE, TriggerWheel::T_PRIMARY, NO_LEFT_FILTER, NO_RIGHT_FILTER);
		s->addEventClamped(angleEnd, TriggerValue::FALL, TriggerWheel::T_PRIMARY, NO_LEFT_FILTER, NO_RIGHT_FILTER);
	}
	
	// Add the crank home reference pulse on secondary channel
	// This occurs at crankPinBTDC before TDC #1 (e.g., 62° BTDC)
	// The cam sensor gates this, so only one pulse per 720° cycle gets through
	float crankHomeAngle = FOUR_STROKE_ENGINE_CYCLE - crankPinBTDC;
	float crankHomePulseWidth = 2.0f; // Small pulse width in degrees
	
	s->addEventClamped(crankHomeAngle, TriggerValue::RISE, TriggerWheel::T_SECONDARY, NO_LEFT_FILTER, NO_RIGHT_FILTER);
	s->addEventClamped(crankHomeAngle + crankHomePulseWidth, TriggerValue::FALL, TriggerWheel::T_SECONDARY, NO_LEFT_FILTER, NO_RIGHT_FILTER);
	
	// Set TDC position relative to the sync point
	// TDC #1 is at tdcAfterTrigger degrees after the crank home pulse
	s->tdcPosition = crankHomeAngle + tdcAfterTrigger;
	
	// Set up gap detection for synchronization
	// The crank home pulse creates a distinctive sync pattern
	// We need to detect the gap caused by the gated pulse
	s->setTriggerSynchronizationGap(2.0f);
	s->setSecondTriggerSynchronizationGap(1.5f);
	
	// Use both primary and secondary for sync
	s->useOnlyPrimaryForSync = false;
}
