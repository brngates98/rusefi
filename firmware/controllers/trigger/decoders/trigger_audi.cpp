/*
 * @file trigger_audi.cpp
 *
 * @date Dec 2, 2024
 * @author rusEFI Team
 */

#include "pch.h"

#include "trigger_audi.h"
#include "trigger_universal.h"

// TT_AUDI_5_CYL - Audi 5-cylinder trigger
// Previously known as TT_TRI_TACH
// Primary: 135-tooth wheel on starter ring (provides continuous crank position)
// Secondary: Home pin on flywheel (fires at 62° BTDC cyl1 and 134° BTDC cyl5)
// Cam: Hall sensor used for 720° sync gating (Cam HIGH allows home pulse, Cam LOW masks it)
// Reference: https://www.vems.hu/wiki/index.php?page=InputTrigger%2FAudiTrigger
// Firing order: 1-2-4-5-3

void configureAudi5Cyl(TriggerWaveform *s) {
	s->initialize(FOUR_STROKE_CRANK_SENSOR, SyncEdge::RiseOnly);
	
	// Synchronization is needed - we need the home pulse + cam gating for 720° sync
	s->isSynchronizationNeeded = true;
	
	// We need the secondary trigger input for the home pin
	s->needSecondTriggerInput = true;
	
	float toothWidth = 0.5;
	float engineCycle = FOUR_STROKE_ENGINE_CYCLE;  // 720°
	int totalTeethCount = 135;
	
	// Add 135 teeth on primary channel
	// The 135-tooth wheel provides continuous position information
	// Each tooth represents 360° / 135 = 2.67° of crank rotation
	// Since we're working with a 720° cycle, the pattern appears twice
	addSkippedToothTriggerEvents(TriggerWheel::T_PRIMARY, s, totalTeethCount, 0, 
	                              toothWidth, 0, engineCycle, NO_LEFT_FILTER, NO_RIGHT_FILTER);
	
	// Add home pulses on secondary channel
	// The home pin on the flywheel triggers twice per 720° cycle:
	// 1. At 62° BTDC cylinder 1
	//    TDC cyl 1 is at 0° in the cycle, so 62° Before TDC = 360° - 62° = 298°
	//    (using positive angle notation where cycle runs from 0° to 720°)
	// 2. At 134° BTDC cylinder 5 (occurs in second crank revolution)
	//
	// Cylinder 5 in firing order 1-2-4-5-3:
	// 5 cylinders means 720° / 5 = 144° between firing events
	// Cyl 1: 0°, Cyl 2: 144°, Cyl 4: 288°, Cyl 5: 432°, Cyl 3: 576°
	// 134° BTDC cyl 5 = 432° - 134° = 298°
	// 
	// However, this would place both pulses at the same angle in different revolutions
	// The home pin physically fires twice per crank revolution (360°), which means
	// once per 360° at a fixed position relative to the crank
	// 
	// First home pulse: 298° (62° BTDC cyl1)
	// Second home pulse: 298° + 360° = 658° (same crank position, second revolution)
	
	float homeAngle1 = 298.0f;  // 62° BTDC cyl1
	float homeAngle2 = 658.0f;  // Same position in second crank revolution (298° + 360°)
	float homePulseWidth = 2.0f;  // Width of home pulse in degrees
	
	s->addEvent720(homeAngle1, TriggerValue::RISE, TriggerWheel::T_SECONDARY);
	s->addEvent720(homeAngle1 + homePulseWidth, TriggerValue::FALL, TriggerWheel::T_SECONDARY);
	s->addEvent720(homeAngle2, TriggerValue::RISE, TriggerWheel::T_SECONDARY);
	s->addEvent720(homeAngle2 + homePulseWidth, TriggerValue::FALL, TriggerWheel::T_SECONDARY);
	
	// Cannot use simple gap detection with 135 evenly-spaced teeth
	// Synchronization relies on home pulse detection combined with cam sensor gating
	s->useOnlyPrimaryForSync = false;  // We need both primary and secondary for sync
	
	// The cam sensor (separate input, not part of trigger waveform) gates which home
	// pulse is used for synchronization, providing unambiguous 720° position
}
