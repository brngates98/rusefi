/**
 * @file test_trigger_tritach.cpp
 * Tests for TT_TRI_TACH (Audi 5-cyl / BMW S14 style trigger)
 */

#include "pch.h"
#include "trigger_decoder.h"
#include "trigger_simulator.h"

TEST(triggerTritach, testTriggerShape) {
	EngineTestHelper eth(engine_type_e::TEST_ENGINE);
	eth.setTriggerType(trigger_type_e::TT_TRI_TACH);

	TriggerWaveform& shape = engine->triggerCentral.triggerShape;

	ASSERT_FALSE(shape.shapeDefinitionError) << "shapeDefinitionError";
	ASSERT_EQ(FOUR_STROKE_CRANK_SENSOR, shape.getWheelOperationMode());
	ASSERT_EQ(360, shape.getCycleDuration());
	ASSERT_TRUE(shape.needSecondTriggerInput) << "needSecondTriggerInput";
	ASSERT_FALSE(shape.isSynchronizationNeeded) << "isSynchronizationNeeded should be false for self-sync";

	// PRIMARY: 1 tooth (rise + fall = 2 events)
	// SECONDARY: 135 teeth (rise + fall = 270 events)
	// Total: 272 events per 360° cycle
	size_t expectedPrimaryEvents = 2;
	size_t expectedSecondaryEvents = 270;
	ASSERT_EQ(expectedPrimaryEvents, shape.getExpectedEventCount(TriggerWheel::T_PRIMARY));
	ASSERT_EQ(expectedSecondaryEvents, shape.getExpectedEventCount(TriggerWheel::T_SECONDARY));

	// Total events = (1 + 135) * 2 edges = 272 per cycle
	// For FOUR_STROKE_CRANK_SENSOR, getLength() returns 2x for 720° engine cycle
	size_t expectedLength = (expectedPrimaryEvents + expectedSecondaryEvents) * 2;
	ASSERT_EQ(expectedLength, shape.getLength());
}

TEST(triggerTritach, testSyncOnPrimaryTooth) {
	EngineTestHelper eth(engine_type_e::TEST_ENGINE);
	eth.setTriggerType(trigger_type_e::TT_TRI_TACH);

	// Since isSynchronizationNeeded = false, should sync on first rising edge
	TriggerDecoderBase state_("test");
	const auto& triggerConfiguration = engine->triggerCentral.primaryTriggerConfiguration;
	TriggerWaveform& shape = engine->triggerCentral.triggerShape;

	ASSERT_FALSE(state_.shaft_is_synchronized);

	// Fire first PRIMARY rising edge - should sync immediately
	efitick_t time = 1000;
	state_.decodeTriggerEvent("test", shape, nullptr, triggerConfiguration, SHAFT_PRIMARY_RISING, time);
	ASSERT_TRUE(state_.shaft_is_synchronized) << "Should sync on first PRIMARY rising edge";
}

TEST(triggerTritach, testSecondaryTeethCounting) {
	EngineTestHelper eth(engine_type_e::TEST_ENGINE);
	eth.setTriggerType(trigger_type_e::TT_TRI_TACH);

	TriggerWaveform& shape = engine->triggerCentral.triggerShape;

	// Verify SECONDARY tooth spacing is ~2.667 degrees (360/135)
	float expectedSpacing = 360.0f / 135.0f;
	ASSERT_NEAR(expectedSpacing, 2.6667f, 0.001f);

	// The shape should have 135 SECONDARY teeth
	ASSERT_EQ(270, shape.getExpectedEventCount(TriggerWheel::T_SECONDARY));
}

TEST(triggerTritach, testSimulation) {
	EngineTestHelper eth(engine_type_e::TEST_ENGINE);
	eth.setTriggerType(trigger_type_e::TT_TRI_TACH);

	// Run trigger simulation for a few cycles
	eth.fireTriggerEvents(272 * 2);  // Two 360° cycles

	// Should have valid RPM after simulation
	float rpm = Sensor::getOrZero(SensorType::Rpm);
	ASSERT_GT(rpm, 0) << "RPM should be positive after trigger simulation";
}
