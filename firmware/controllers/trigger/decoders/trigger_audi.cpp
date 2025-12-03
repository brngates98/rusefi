/**
 * @file	trigger_audi.cpp
 *
 * Audi 5-cylinder DivbyN trigger implementation
 * Primary: 135-tooth multitooth crank sensor (VR)
 * Secondary 1: Crank home sensor - single reference pin on flywheel (VR)
 * Secondary 2: Cam sensor - single window in distributor (Hall)
 *
 * The DivbyN algorithm takes the number of actual teeth on the trigger wheel,
 * multiplies by 2, then divides by a configurable divider to create virtual trigger teeth:
 * Number of teeth on wheel = (Actual teeth × 2) / Divider
 * Example: (135 × 2) / 3 = 90 virtual teeth per engine cycle
 *
 * @date Dec 3, 2024
 * @author Andrey Belomutskiy, (c) 2012-2024
 */

#include "pch.h"

#include "trigger_audi.h"

void initializeAudiDivbyN(TriggerWaveform *s) {
	// Use FOUR_STROKE_CRANK_SENSOR mode with cam phase detection
	s->initialize(FOUR_STROKE_CRANK_SENSOR, SyncEdge::RiseOnly);

	// Get configuration parameters
	uint8_t actualTeeth = engineConfiguration->trigger.audiActualTeeth;
	uint8_t divider = engineConfiguration->trigger.audiTriggerDivider;
	float tdcAfterTrigger = engineConfiguration->trigger.audiTdcAfterTrigger;
	float toothAngularWidth = engineConfiguration->trigger.audiToothAngularWidth;
	
	// Validate parameters
	if (actualTeeth == 0 || divider == 0) {
		firmwareError(ObdCode::CUSTOM_OBD_TRIGGER_WAVEFORM, "Invalid Audi trigger parameters: teeth=%d, divider=%d", actualTeeth, divider);
		s->setShapeDefinitionError(true);
		return;
	}

	// Calculate virtual tooth count using DivbyN formula
	// virtualTeeth = (actualTeeth * 2) / divider
	int virtualTeeth = (actualTeeth * 2) / divider;
	
	if (virtualTeeth <= 0) {
		firmwareError(ObdCode::CUSTOM_OBD_TRIGGER_WAVEFORM, "Invalid virtual tooth count: %d", virtualTeeth);
		s->setShapeDefinitionError(true);
		return;
	}

	// Calculate angular spacing between teeth
	// For a 4-stroke engine cycle (720 degrees)
	float engineCycle = FOUR_STROKE_ENGINE_CYCLE;
	float toothSpacing = engineCycle / virtualTeeth;

	// Set TDC position - the angle from trigger event #0 to actual engine TDC
	s->tdcPosition = tdcAfterTrigger;

	// With uniform teeth (no missing teeth), the trigger doesn't provide exact TDC location
	// The TDC position is calculated from the trigger events
	s->shapeWithoutTdc = true;
	s->isSynchronizationNeeded = false;
	s->needSecondTriggerInput = false;
	s->isSecondWheelCam = false;

	// Add virtual teeth around the full engine cycle
	// Each tooth is a rise/fall pair
	// The tooth width should be less than tooth spacing to avoid overlap
	float effectiveToothWidth = toothAngularWidth;
	if (effectiveToothWidth >= toothSpacing) {
		// If tooth width would cause overlap, reduce it
		effectiveToothWidth = toothSpacing * 0.5f;  // 50% duty cycle
	}
	
	// Note: angles must be > 0 and <= engineCycle
	for (int i = 0; i < virtualTeeth; i++) {
		float toothStart = i * toothSpacing + 0.01f;  // Small offset to avoid zero
		float toothEnd = toothStart + effectiveToothWidth;
		
		// Special handling for the last tooth - must end exactly at engineCycle
		if (i == virtualTeeth - 1) {
			toothEnd = engineCycle;
		}
		
		// Ensure angles are within valid range (0, 720]
		if (toothEnd > engineCycle) {
			toothEnd = engineCycle;
		}
		
		// Add rise and fall events for this tooth
		s->addEvent720(toothStart, TriggerValue::RISE, TriggerWheel::T_PRIMARY);
		s->addEvent720(toothEnd, TriggerValue::FALL, TriggerWheel::T_PRIMARY);
	}
}
