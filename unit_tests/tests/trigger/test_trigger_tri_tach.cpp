/*
 * test_trigger_tri_tach.cpp
 *
 * Unit tests for TT_TRI_TACH (Audi 5-cylinder 135+1) trigger
 */

#include "pch.h"
#include "trigger_decoder.h"
#include "trigger_simulator.h"

TEST(TriggerTriTach, VerifyShape) {
	EngineTestHelper eth(engine_type_e::TEST_ENGINE);

	engineConfiguration->trigger.type = trigger_type_e::TT_TRI_TACH;

	TriggerWaveform *shape = &engine->triggerCentral.triggerShape;

	// Reinitialize with TriTach trigger
	shape->initializeTriggerWaveform(
		FOUR_STROKE_CRANK_SENSOR,
		engineConfiguration->trigger,
		true
	);

	// Verify we have the correct number of events
	// 135 primary teeth + 2 secondary tooth events (rise + fall) = 137
	EXPECT_EQ(137, shape->getSize());

	// Verify getLength() doubles it for 720° engine cycle
	EXPECT_EQ(274, shape->getLength());

	// Verify we're under the PWM_PHASE_MAX_COUNT limit
	EXPECT_LT(shape->getSize(), PWM_PHASE_MAX_COUNT);

	// Verify configuration flags
	EXPECT_TRUE(shape->useOnlyRisingEdges);
	EXPECT_TRUE(shape->isSynchronizationNeeded);
	EXPECT_TRUE(shape->needSecondTriggerInput);

	// Verify TDC position is set
	EXPECT_FLOAT_EQ(62.0f, shape->tdcPosition);
}

TEST(TriggerTriTach, VerifyToothSpacing) {
	EngineTestHelper eth(engine_type_e::TEST_ENGINE);

	engineConfiguration->trigger.type = trigger_type_e::TT_TRI_TACH;

	TriggerWaveform *shape = &engine->triggerCentral.triggerShape;

	shape->initializeTriggerWaveform(
		FOUR_STROKE_CRANK_SENSOR,
		engineConfiguration->trigger,
		true
	);

	// Expected tooth angle = 360 / 135 = 2.6667 degrees
	float expectedToothAngle = 360.0f / 135.0f;

	// Check first few teeth are evenly spaced
	float firstTooth = shape->getSwitchAngle(0);
	float secondTooth = shape->getSwitchAngle(1);

	float spacing = secondTooth - firstTooth;
	EXPECT_NEAR(expectedToothAngle, spacing, 0.01f);
}

TEST(TriggerTriTach, VerifySyncToothPosition) {
	EngineTestHelper eth(engine_type_e::TEST_ENGINE);

	engineConfiguration->trigger.type = trigger_type_e::TT_TRI_TACH;

	TriggerWaveform *shape = &engine->triggerCentral.triggerShape;

	shape->initializeTriggerWaveform(
		FOUR_STROKE_CRANK_SENSOR,
		engineConfiguration->trigger,
		true
	);

	// Verify expected event counts per channel
	// With useOnlyRisingEdges=true, only RISE events are counted
	// T_PRIMARY should have 135 events (RISE only)
	// T_SECONDARY should have 1 event (RISE only)
	EXPECT_EQ(135, shape->getExpectedEventCount(TriggerWheel::T_PRIMARY));
	EXPECT_EQ(1, shape->getExpectedEventCount(TriggerWheel::T_SECONDARY));
}
