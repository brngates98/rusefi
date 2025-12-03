/**
 * @file trigger_audi.cpp
 *
 * @date Dec 3, 2024
 * @author rusEFI Team
 *
 * Audi 5-cylinder DivbyN trigger decoder
 * Hardware: 135 teeth on flywheel with DivbyN algorithm
 * Creates 90 virtual teeth per 720° cycle (135 * 2 / 3)
 * Uses cam-gated crank home pulse for synchronization
 */

#include "pch.h"

#include "trigger_audi.h"

/**
 * Audi 5-cylinder engines (AAN, ABY, ADU, 3B) trigger pattern
 * 
 * Hardware configuration:
 * - Primary (Crank Teeth): VR sensor - 135 teeth on flywheel
 * - Secondary (Crank Home): VR sensor - 1 pin at 62° BTDC Cylinder #1
 * - Cam Reference: Hall sensor - 1 slot per cam rotation (720°)
 * 
 * DivbyN Algorithm:
 * Virtual teeth = (135 actual teeth × 2) / 3 = 90 virtual teeth per 720°
 * Tooth spacing = 720° / 90 = 8° per virtual tooth
 * 
 * Cam-gating sync:
 * - Cam LOW → Masks crank home pulse at 62° BTDC #1 (ignore)
 * - Cam HIGH → Allows crank home pulse at 134° BTDC #5 (valid sync point)
 * 
 * This creates 1 unambiguous sync pulse per 720° with zero cam slop.
 */
void initializeAudiDivbyN(TriggerWaveform *s) {
	// Initialize as a four-stroke crank sensor with cam synchronization
	s->initialize(FOUR_STROKE_CRANK_SENSOR, SyncEdge::RiseOnly);
	
	// Enable synchronization requirement
	s->isSynchronizationNeeded = true;
	
	// Use cam sensor for second wheel (cam-gated sync)
	s->isSecondWheelCam = true;
	s->needSecondTriggerInput = true;
	
	// Virtual teeth configuration after DivbyN
	// 90 virtual teeth over 720° = 8° spacing per tooth
	int virtualTeethCount = 90;
	float toothWidth = 0.5; // 50% duty cycle
	float engineCycle = FOUR_STROKE_ENGINE_CYCLE; // 720°
	float toothSpacing = engineCycle / virtualTeethCount;
	
	// The sync point is at 134° BTDC #5 when cam is HIGH
	// This corresponds to 360° + (180° - 134°) = 406° after TDC #1
	// We'll set TDC position relative to first tooth
	s->tdcPosition = 406;
	
	// Generate 90 virtual teeth
	for (int i = 0; i < virtualTeethCount; i++) {
		float angle = i * toothSpacing;
		
		// Add rising edge (tooth start)
		s->addEvent720(angle + toothSpacing * (1 - toothWidth), TriggerValue::RISE);
		
		// Add falling edge (tooth end)
		s->addEvent720(angle + toothSpacing, TriggerValue::FALL);
	}
	
	// Add cam sync pulse on secondary channel
	// Cam is HIGH from approximately 270° to 630° (when sync is valid)
	// The crank home pulse at 406° will be gated by this HIGH cam signal
	s->addEvent720(270, TriggerValue::RISE, TriggerWheel::T_SECONDARY);
	s->addEvent720(630, TriggerValue::FALL, TriggerWheel::T_SECONDARY);
	
	// Configure synchronization gap detection
	// With 90 evenly spaced teeth, we rely on the cam pulse for sync
	// Set a small gap tolerance to detect regular tooth spacing
	s->setTriggerSynchronizationGap(1.5);
}
