/*
 * @file trigger_vw.cpp
 *
 * @date Aug 25, 2018
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#include "pch.h"
#include "trigger_vw.h"
#include "trigger_universal.h"

void setSkodaFavorit(TriggerWaveform *s) {
        s->initialize(FOUR_STROKE_CRANK_SENSOR, SyncEdge::Rise);

	s->addEvent360(133, TriggerValue::RISE, TriggerWheel::T_PRIMARY);
	s->addEvent360(177, TriggerValue::FALL, TriggerWheel::T_PRIMARY);

	s->addEvent360(180, TriggerValue::RISE, TriggerWheel::T_PRIMARY);
	s->addEvent360(183, TriggerValue::FALL, TriggerWheel::T_PRIMARY);

	s->addEvent360(313, TriggerValue::RISE, TriggerWheel::T_PRIMARY);
	s->addEvent360(360, TriggerValue::FALL, TriggerWheel::T_PRIMARY);

	s->tdcPosition = 180 + 47;
        s->setTriggerSynchronizationGap3(/*gapIndex*/0, 2, 4);
}

void configureAudi135CamHallWindow(TriggerWaveform *s) {
        s->initialize(FOUR_STROKE_CRANK_SENSOR, SyncEdge::RiseOnly);

        s->shapeWithoutTdc = true;

        // 135 evenly spaced teeth on the flywheel
        addSkippedToothTriggerEvents(TriggerWheel::T_PRIMARY, s, 135, /* skipped */ 0, 0.5, 0,
                        getEngineCycle(s->getWheelOperationMode()), NO_LEFT_FILTER, NO_RIGHT_FILTER);

        // Cam hall window masks the earlier reference pulse, leaving a single sync at 134 BTDC on cylinder 5
        constexpr angle_t camWindowSync = 432 - 134;
        constexpr angle_t camWindowWidth = 5;

        s->isSecondWheelCam = true;
        s->addEvent720(camWindowSync, TriggerValue::RISE, TriggerWheel::T_SECONDARY);
        s->addEvent720(camWindowSync + camWindowWidth, TriggerValue::FALL, TriggerWheel::T_SECONDARY);
}

void setVwConfiguration(TriggerWaveform *s) {
	s->initialize(FOUR_STROKE_CRANK_SENSOR, SyncEdge::RiseOnly);

	int totalTeethCount = 60;
	int skippedCount = 2;

	float engineCycle = FOUR_STROKE_ENGINE_CYCLE;
	float toothWidth = 0.5;

	addSkippedToothTriggerEvents(TriggerWheel::T_PRIMARY, s, 60, 2, toothWidth, 0, engineCycle,
			NO_LEFT_FILTER, 690);

	float angleDown = engineCycle / totalTeethCount * (totalTeethCount - skippedCount - 1 + (1 - toothWidth) );
	s->addEventClamped(0 + angleDown + 12, TriggerValue::RISE, TriggerWheel::T_PRIMARY, NO_LEFT_FILTER, NO_RIGHT_FILTER);
	s->addEventClamped(0 + engineCycle, TriggerValue::FALL, TriggerWheel::T_PRIMARY, NO_LEFT_FILTER, NO_RIGHT_FILTER);

	s->setTriggerSynchronizationGap2(1.6, 4);
	s->setSecondTriggerSynchronizationGap(1); // this gap is not required to synch on perfect signal but is needed to handle to reject cranking transition noise
	s->setThirdTriggerSynchronizationGap(1);
}
