/*
 * @file test_trigger_audi_divbyn.cpp
 *
 * @date Dec 3, 2024
 * @author Copilot
 */

#include "pch.h"

TEST(trigger, testAudiDivbyNBasic) {
	EngineTestHelper eth(engine_type_e::TEST_ENGINE);
	
	// Configure the trigger
	engineConfiguration->trigger.type = trigger_type_e::TT_AUDI_DIVBYN;
	engineConfiguration->audiActualTeeth = 135;
	engineConfiguration->audiTriggerDivider = 3;
	engineConfiguration->audiTdcAfterTrigger = 58;
	engineConfiguration->audiCrankPinBTDC = 58;
	
	TriggerWaveform *shape = &engine->triggerCentral.triggerShape;
	shape->initializeTriggerWaveform(FOUR_STROKE_CRANK_SENSOR, engineConfiguration->trigger, true);
	
	// Verify no shape definition error
	ASSERT_FALSE(shape->shapeDefinitionError) << "Should not have shape definition error";
	
	// Verify flags are set correctly
	ASSERT_TRUE(shape->isSynchronizationNeeded) << "Should need synchronization";
	ASSERT_TRUE(shape->needSecondTriggerInput) << "Should need second trigger input";
	ASSERT_FALSE(shape->isSecondWheelCam) << "Secondary should be crank-home, not cam";
	ASSERT_FALSE(shape->useOnlyPrimaryForSync) << "Should use both channels for sync";
	
	// Verify TDC position
	ASSERT_EQ(58, shape->tdcPosition) << "TDC position should be 58 degrees";
	
	// Verify virtual teeth calculation: (135 * 2) / 3 = 90 teeth
	// Each tooth has rise and fall, so 90 * 2 = 180 primary events
	// Plus 2 crank-home pulses with rise and fall each = 4 secondary events
	// Total expected events should be reasonable
	int totalEvents = shape->getLength();
	ASSERT_GT(totalEvents, 0) << "Should have events";
	ASSERT_LT(totalEvents, 200) << "Should not exceed event limit";
}

TEST(trigger, testAudiDivbyNDefaults) {
	EngineTestHelper eth(engine_type_e::TEST_ENGINE);
	
	// Test with default values (zeros should use defaults)
	engineConfiguration->trigger.type = trigger_type_e::TT_AUDI_DIVBYN;
	engineConfiguration->audiActualTeeth = 0;  // Should default to 135
	engineConfiguration->audiTriggerDivider = 0;  // Should default to 3
	engineConfiguration->audiTdcAfterTrigger = 0;  // Should default to 58
	engineConfiguration->audiCrankPinBTDC = 0;  // Should default to 58
	
	TriggerWaveform *shape = &engine->triggerCentral.triggerShape;
	shape->initializeTriggerWaveform(FOUR_STROKE_CRANK_SENSOR, engineConfiguration->trigger, true);
	
	// Should work with defaults
	ASSERT_FALSE(shape->shapeDefinitionError) << "Should work with default values";
	ASSERT_TRUE(shape->isSynchronizationNeeded);
}

TEST(trigger, testAudiDivbyNInvalidParams) {
	EngineTestHelper eth(engine_type_e::TEST_ENGINE);
	
	// Test with invalid divider
	engineConfiguration->trigger.type = trigger_type_e::TT_AUDI_DIVBYN;
	engineConfiguration->audiActualTeeth = 10;
	engineConfiguration->audiTriggerDivider = 0;  // Will use default
	
	TriggerWaveform *shape = &engine->triggerCentral.triggerShape;
	shape->initializeTriggerWaveform(FOUR_STROKE_CRANK_SENSOR, engineConfiguration->trigger, true);
	
	// Should handle invalid params gracefully with defaults
	ASSERT_FALSE(shape->shapeDefinitionError) << "Should use defaults for zero divider";
}

TEST(trigger, testAudiDivbyNVirtualTeeth) {
	EngineTestHelper eth(engine_type_e::TEST_ENGINE);
	
	// Test different virtual teeth configurations
	engineConfiguration->trigger.type = trigger_type_e::TT_AUDI_DIVBYN;
	
	// Test case: 60 actual teeth, divider 2 -> 60 virtual teeth
	engineConfiguration->audiActualTeeth = 60;
	engineConfiguration->audiTriggerDivider = 2;
	engineConfiguration->audiTdcAfterTrigger = 60;
	engineConfiguration->audiCrankPinBTDC = 60;
	
	TriggerWaveform *shape = &engine->triggerCentral.triggerShape;
	shape->initializeTriggerWaveform(FOUR_STROKE_CRANK_SENSOR, engineConfiguration->trigger, true);
	
	ASSERT_FALSE(shape->shapeDefinitionError) << "Should work with 60/2 configuration";
	ASSERT_TRUE(shape->isSynchronizationNeeded);
	
	// Test case: ensure minimum teeth requirement
	engineConfiguration->audiActualTeeth = 4;
	engineConfiguration->audiTriggerDivider = 2;
	
	shape->initializeTriggerWaveform(FOUR_STROKE_CRANK_SENSOR, engineConfiguration->trigger, true);
	ASSERT_FALSE(shape->shapeDefinitionError) << "Should work with 4/2 configuration (4 virtual teeth)";
}
