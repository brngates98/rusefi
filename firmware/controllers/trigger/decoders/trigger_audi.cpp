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
 * Audi 5 cylinder trigger
 * 
 * Primary wheel (crank): 135 teeth with one reference pin (missing tooth)
 * Secondary wheel (cam): Hall effect sensor providing phase information
 * 
 * This is a VR sensor setup with:
 * - Crank sensor reading 135 teeth
 * - Reference pin (represented as a gap/missing tooth)
 * - Hall effect cam sensor for phase discrimination
 */
void initializeAudi5Cyl(TriggerWaveform *s) {
	s->initialize(FOUR_STROKE_CRANK_SENSOR, SyncEdge::RiseOnly);

	// 135 teeth on the crank wheel with one missing tooth (reference)
	// This gives us 136 positions, with position 135 being the reference gap
	int totalTeethCount = 135;
	int skippedCount = 1;

	// Use standard 720 degree engine cycle for 4-stroke
	float engineCycle = FOUR_STROKE_ENGINE_CYCLE;
	
	// Tooth width ratio (50% duty cycle typical for VR sensors)
	float toothWidth = 0.5;

	// Add the toothed pattern on the primary (crank) wheel
	// The 135-1 pattern starts at 0 degrees
	addSkippedToothTriggerEvents(TriggerWheel::T_PRIMARY, s, totalTeethCount, skippedCount, 
		toothWidth, 0, engineCycle, NO_LEFT_FILTER, 690);

	// Add a simple cam sensor pattern on the secondary wheel (Hall effect)
	// This provides phase information for 4-stroke operation
	// One pulse per 720 degrees at a specific position
	s->addEvent720(60, TriggerValue::RISE, TriggerWheel::T_SECONDARY);
	s->addEvent720(120, TriggerValue::FALL, TriggerWheel::T_SECONDARY);

	s->needSecondTriggerInput = true;
	s->isSecondWheelCam = true;

	// Set synchronization gaps for 135-1 pattern
	// Gap ratio is approximately (135-1)/1 = 134, but we use a range for tolerance
	s->setTriggerSynchronizationGap2(2.5, 5.0);
	s->setSecondTriggerSynchronizationGap(0.5);

	// TDC position - this would need to be calibrated for the actual engine
	// For now, setting it after the reference gap
	s->tdcPosition = 60;
}
