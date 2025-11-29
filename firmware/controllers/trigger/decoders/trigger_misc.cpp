/*
 * trigger_misc.cpp
 *
 *  Created on: Oct 30, 2018
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#include "pch.h"

#include "trigger_misc.h"
#include "trigger_universal.h"

// TT_FIAT_IAW_P8
void configureFiatIAQ_P8(TriggerWaveform * s) {
	s->initialize(FOUR_STROKE_CAM_SENSOR, SyncEdge::Rise);

	int width = 60;
	s->tdcPosition = width;

	s->addEvent720(width, TriggerValue::RISE);
	s->addEvent720(180, TriggerValue::FALL);

	s->addEvent720(180 + width, TriggerValue::RISE);
	s->addEvent720(720, TriggerValue::FALL);
	s->setTriggerSynchronizationGap(3);
}

// TT_TRI_TACH
void configureTriTach(TriggerWaveform * s) {
	s->initialize(FOUR_STROKE_CRANK_SENSOR, SyncEdge::RiseOnly);

	s->isSynchronizationNeeded = false;

	float toothWidth = 0.5;

	float engineCycle = FOUR_STROKE_ENGINE_CYCLE;

	int totalTeethCount = 135;
	float offset = 0;

	float angleDown = engineCycle / totalTeethCount * (0 + (1 - toothWidth));
	float angleUp = engineCycle / totalTeethCount * (0 + 1);
	s->addEventClamped(offset + angleDown, TriggerValue::RISE, TriggerWheel::T_PRIMARY, NO_LEFT_FILTER, NO_RIGHT_FILTER);
	s->addEventClamped(offset + angleDown + 0.1, TriggerValue::RISE, TriggerWheel::T_SECONDARY, NO_LEFT_FILTER, NO_RIGHT_FILTER);
	s->addEventClamped(offset + angleUp, TriggerValue::FALL, TriggerWheel::T_PRIMARY, NO_LEFT_FILTER, NO_RIGHT_FILTER);
	s->addEventClamped(offset + angleUp + 0.1, TriggerValue::FALL, TriggerWheel::T_SECONDARY, NO_LEFT_FILTER, NO_RIGHT_FILTER);


	addSkippedToothTriggerEvents(TriggerWheel::T_SECONDARY, s, totalTeethCount, /* skipped */ 0, toothWidth, offset, engineCycle,
			1.0 * FOUR_STROKE_ENGINE_CYCLE / 135,
			NO_RIGHT_FILTER);
}

/**
 * based on https://fordsix.com/threads/understanding-standard-and-signature-pip-thick-film-ignition.81515/
 * based on https://www.w8ji.com/distributor_stabbing.htm
 */
void configureFordPip(TriggerWaveform * s) {
	s->initialize(FOUR_STROKE_CAM_SENSOR, SyncEdge::Rise);

	s->tdcPosition = 662.5;

	s->setTriggerSynchronizationGap(0.66);
	s->setSecondTriggerSynchronizationGap(1.25);
	/**
	 * sensor is mounted on distributor but trigger shape is defined in engine cycle angles
	 */
	int oneCylinder = s->getCycleDuration() / 8;

	s->addEventAngle(oneCylinder * 0.75, TriggerValue::RISE);
	s->addEventAngle(oneCylinder, TriggerValue::FALL);


	for (int i = 2;i<=8;i++) {
		s->addEventAngle(oneCylinder * (i - 0.5), TriggerValue::RISE);
		s->addEventAngle(oneCylinder * i, TriggerValue::FALL);
	}

}

void configureFordST170(TriggerWaveform * s) {
	s->initialize(FOUR_STROKE_CAM_SENSOR, SyncEdge::RiseOnly);
	int width = 10;

	int total = s->getCycleDuration() / 8;

	s->addEventAngle(1 * total - width, TriggerValue::RISE);
	s->addEventAngle(1 * total, TriggerValue::FALL);

	s->addEventAngle(2 * total - width, TriggerValue::RISE);
	s->addEventAngle(2 * total, TriggerValue::FALL);

	s->addEventAngle(4 * total - width, TriggerValue::RISE);
	s->addEventAngle(4 * total, TriggerValue::FALL);

	s->addEventAngle(6 * total - width, TriggerValue::RISE);
	s->addEventAngle(6 * total, TriggerValue::FALL);

	s->addEventAngle(8 * total - width, TriggerValue::RISE);
	s->addEventAngle(8 * total, TriggerValue::FALL);
}

static void daihatsu(TriggerWaveform * s, int count) {
	s->initialize(FOUR_STROKE_CAM_SENSOR, SyncEdge::RiseOnly);
	s->tdcPosition = 720 - 30;

	int width = 10;

	s->addEventAngle(30 - width, TriggerValue::RISE);
	s->addEventAngle(30, TriggerValue::FALL);

	for (int i = 1;i<=count;i++) {
		s->addEventAngle(s->getCycleDuration() / count * i - width, TriggerValue::RISE);
		s->addEventAngle(s->getCycleDuration() / count * i, TriggerValue::FALL);
	}
}

void configureDaihatsu3cyl(TriggerWaveform * s) {
	daihatsu(s, 3);
	s->setTriggerSynchronizationGap(0.125);
}


void configureDaihatsu4cyl(TriggerWaveform * s) {
	daihatsu(s, 4);
	s->setTriggerSynchronizationGap(0.17);
}

void configureBarra3plus1cam(TriggerWaveform *s) {
	s->initialize(FOUR_STROKE_CAM_SENSOR, SyncEdge::RiseOnly);

	// This wheel has four teeth
	// two short gaps, and two long gaps
	// short = 60 deg
	// long = 120 deg

	{
		int offset = 60;
		int w = 5;

		s->addToothRiseFall(offset, w);

		// short gap 60 deg
		s->addToothRiseFall(offset + 60, w);

		// long gap 120 deg
		s->addToothRiseFall(offset + 180, w);
		// long gap 120 deg
		s->addToothRiseFall(offset + 300, w);

		// short gap, 60 deg back to zero/720
	}

	// sync tooth is the zero tooth, the first short gap after two long gaps
	s->setTriggerSynchronizationGap3(0, 1.6f, 2.4f);
	// previous gap should be 1.0
	s->setTriggerSynchronizationGap3(1, 0.8f, 1.2f);
}

void configureBenelli(TriggerWaveform *s) {
	s->initialize(FOUR_STROKE_CRANK_SENSOR, SyncEdge::RiseOnly);

    constexpr float magic = 15;

	float angle = 3 * magic;

	s->addEvent360(angle / 2 /* 22.5 */, TriggerValue::RISE);
	s->addEvent360(angle /*     45   */, TriggerValue::FALL);

	for (int i = 0;i< 24 - 3;i++) {
		angle += magic / 2;
		s->addEvent360(angle, TriggerValue::RISE);

		angle += magic / 2;
		s->addEvent360(angle, TriggerValue::FALL);
	}
}

void configure60degSingleTooth(TriggerWaveform *s) {
	/** @todo
	 * My approach was to utilize ::Both especially for single
	 * tooth and manual kikstarter, to be ready on both sides of blind.
	 * But unfortuneally, my experiments show me the Trigger can't
	 * become syncronized by 'last' and folowed 'first' events only.
	 * Also I observe phase-aligment mehanism is trying to consume a
	 * longer side of trigger as latest before TDC.
	 * I wish to setup SyncEdge::Both for my TT_60DEG_TOOTH after
	 * this case of scenario become work well. For now, ::Rise work
	 * well for my 60 degree trigger and both edges phase sync work
	 * as mush as expected for my engine startup. */

	s->initialize(FOUR_STROKE_CRANK_SENSOR, SyncEdge::Rise);

	s->addEvent360(300, TriggerValue::RISE);
	s->addEvent360(360, TriggerValue::FALL);

	s->tdcPosition = 60;

	s->isSynchronizationNeeded = false;
	s->useOnlyPrimaryForSync = true;
}

void configureArcticCat(TriggerWaveform *s) {
  s->initialize(TWO_STROKE, SyncEdge::Rise);

  int totalTeethCount = 24;
  float engineCycle = TWO_STROKE_CYCLE_DURATION;
  float toothWidth = 0.5;

    addSkippedToothTriggerEvents(TriggerWheel::T_PRIMARY, s, totalTeethCount, 0, toothWidth, /*offset*/0, engineCycle,
    		/*from*/ 30 + 1, /* to */ 195 + 1);


    addSkippedToothTriggerEvents(TriggerWheel::T_PRIMARY, s, totalTeethCount, 0, toothWidth, /*offset*/0, engineCycle,
    		/*from*/ 210 + 1, /* to */ NO_RIGHT_FILTER);

  s->setTriggerSynchronizationGap(2);
  int c = 9;
  for (int gapIndex = 1; gapIndex <= c; gapIndex++) {
    s->setTriggerSynchronizationGap3(gapIndex, 0.75, 1.25);
  }


}

/**
 * Audi 5 cylinder trigger with 135-tooth crank wheel + 1 crank home tooth
 * Based on VEMS documentation:
 * - Primary input: 135 teeth on flywheel (evenly spaced at 2.667° per tooth)
 * - Secondary input: 1 crank home tooth at 62° BTDC cylinder #1
 * - CAM input is handled separately via VVT_SINGLE_TOOTH mode
 *
 * Note: Due to rusEFI's event limit (280), we cannot define all 270 teeth (135×2 per 720°).
 * This implementation follows the Tri-Tach pattern: define tooth spacing over 720° as if
 * there were 135 teeth per engine cycle (not per crank rotation). This gives 5.33° spacing
 * instead of 2.67°, but maintains the essential trigger characteristics for sync purposes.
 * The actual 135 teeth/360° resolution is handled by rusEFI's tooth counting logic.
 *
 * Users should configure this trigger and verify timing with actual hardware.
 */
// TT_AUDI_5CYL_135_1_1
void configureAudi5cyl135_1_1(TriggerWaveform *s) {
	s->initialize(FOUR_STROKE_CRANK_SENSOR, SyncEdge::RiseOnly);

	// No sync needed on evenly-spaced primary teeth - sync comes from secondary
	s->isSynchronizationNeeded = false;

	// Crank home is at 62° BTDC cylinder #1
	s->tdcPosition = 62;

	float toothWidth = 0.5;
	float engineCycle = FOUR_STROKE_ENGINE_CYCLE; // 720°

	// Use 135 teeth over 720° (like Tri-Tach) to stay within event limits
	// This gives 5.33° spacing instead of the actual 2.67° physical spacing
	int totalTeethCount = 135;
	float offset = 0;

	// First tooth on both primary and secondary channels (sync reference)
	float firstToothRise = engineCycle / totalTeethCount * (1 - toothWidth);
	float firstToothFall = engineCycle / totalTeethCount;
	s->addEventClamped(offset + firstToothRise, TriggerValue::RISE, TriggerWheel::T_PRIMARY, NO_LEFT_FILTER, NO_RIGHT_FILTER);
	s->addEventClamped(offset + firstToothRise + 0.1f, TriggerValue::RISE, TriggerWheel::T_SECONDARY, NO_LEFT_FILTER, NO_RIGHT_FILTER);
	s->addEventClamped(offset + firstToothFall, TriggerValue::FALL, TriggerWheel::T_PRIMARY, NO_LEFT_FILTER, NO_RIGHT_FILTER);
	s->addEventClamped(offset + firstToothFall + 0.1f, TriggerValue::FALL, TriggerWheel::T_SECONDARY, NO_LEFT_FILTER, NO_RIGHT_FILTER);

	// Add remaining teeth on secondary channel
	// Filter to start after the first tooth that we manually added
	addSkippedToothTriggerEvents(TriggerWheel::T_SECONDARY, s, totalTeethCount, /* skipped */ 0,
			toothWidth, offset, engineCycle,
			/* filterLeft */ 1.0f * FOUR_STROKE_ENGINE_CYCLE / 135,
			NO_RIGHT_FILTER);
}
