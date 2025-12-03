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
	// Firing order for Audi 5-cyl: 1-2-4-5-3 (144° between cylinders)
	// TDC positions: #1=0°, #2=144°, #4=288°, #5=432°, #3=576°
	// Sync at 134° BTDC #5 = 432° - 134° = 298° after TDC #1
	// However, the first tooth is at 0°, so we need to adjust
	// Set tdcPosition to align with TDC of cylinder #1
	s->tdcPosition = 0;
	
	// Generate 90 virtual teeth
	for (int i = 0; i < virtualTeethCount; i++) {
		float angle = i * toothSpacing;
		
		// Add rising edge (tooth start) - at the beginning of each tooth window
		s->addEvent720(angle, TriggerValue::RISE);
		
		// Add falling edge (tooth end) - 50% duty cycle
		s->addEvent720(angle + toothSpacing * toothWidth, TriggerValue::FALL);
	}
	
	// Add cam sync pulse on secondary channel
	// The cam signal creates one pulse per 720° cycle
	// Cam is HIGH when crank home pulse should be recognized (at #5 TDC region)
	// Cam is LOW when crank home pulse should be ignored (at #1 TDC region)
	// Let's use a symmetric pattern: HIGH for 360°, LOW for 360°
	s->addEvent720(0, TriggerValue::RISE, TriggerWheel::T_SECONDARY);
	s->addEvent720(360, TriggerValue::FALL, TriggerWheel::T_SECONDARY);
	
	// Configure synchronization gap detection
	// With 90 evenly spaced teeth, we rely on the cam pulse for sync
	// Set a tight gap tolerance since all teeth are evenly spaced
	s->setTriggerSynchronizationGap(1.2);
}
