/*
 * @file trigger_audi.cpp
 *
 * @date Nov 2, 2024
 * @author GitHub Copilot
 */

#include "pch.h"
#include "trigger_audi.h"
#include "trigger_universal.h"

/**
 * Audi 5 cylinder trigger (also used in some 4, 6, 8 cylinder engines)
 * 
 * Primary wheel (crank): 135 evenly spaced teeth on the starter gear (flywheel)
 * Secondary wheel (crank): ONE reference pin at 62° BTDC cylinder 1
 *                          (or 134° BTDC cylinder 5 with firing order 12453)
 * Cam HALL sensor: One pulse per cam rotation (720°) that masks the secondary signal
 *                  to provide phase discrimination
 * 
 * The cam-HALL signal masks one of the crankhome pulses, leaving only one pulse
 * per 720° cycle for proper 4-stroke synchronization.
 * 
 * This implementation uses:
 * - T_PRIMARY: 135 tooth wheel (evenly spaced)
 * - T_SECONDARY: crankhome reference pin (1 tooth per crank revolution)
 *                masked by cam-HALL to give 1 pulse per 720°
 */
void initializeAudi5Cyl(TriggerWaveform *s) {
	s->initialize(FOUR_STROKE_CRANK_SENSOR, SyncEdge::RiseOnly);

	// 135 evenly spaced teeth around 720° (4-stroke cycle)
	int totalTeethCount = 135;
	float engineCycle = FOUR_STROKE_ENGINE_CYCLE;
	float toothWidth = 0.5;
	
	// Add 135 evenly spaced teeth on the primary wheel (no missing teeth)
	// These teeth are on the starter gear/flywheel
	float toothAngle = engineCycle / totalTeethCount;
	for (int i = 0; i < totalTeethCount; i++) {
		float angle = i * toothAngle;
		s->addEvent720(angle, TriggerValue::RISE, TriggerWheel::T_PRIMARY);
		s->addEvent720(angle + toothAngle * toothWidth, TriggerValue::FALL, TriggerWheel::T_PRIMARY);
	}

	// Secondary wheel: crankhome reference pin
	// Located at 62° BTDC cylinder 1 (firing order 12453: 1-2-4-5-3)
	// With cam-HALL masking, this appears once per 720°
	// The pin position: 720 - 62 = 658° (before completing the cycle)
	float crankhomeAngle = 658.0f;
	s->addEvent720(crankhomeAngle, TriggerValue::RISE, TriggerWheel::T_SECONDARY);
	s->addEvent720(crankhomeAngle + 5.0f, TriggerValue::FALL, TriggerWheel::T_SECONDARY);

	s->needSecondTriggerInput = true;
	s->isSecondWheelCam = false; // Both signals are from crank, but cam-HALL masks secondary

	// The 135 evenly spaced teeth have no gaps, so we rely on the secondary signal
	// for synchronization
	s->setTriggerSynchronizationGap(1.0); // No gap in primary
	s->setSecondTriggerSynchronizationGap(1.0);

	// TDC position at cylinder 1
	// Reference pin is 62° BTDC, so TDC is 62° after the pin
	s->tdcPosition = crankhomeAngle + 62.0f;
}
