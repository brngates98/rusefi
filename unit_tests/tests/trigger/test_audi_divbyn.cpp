/**
 * @file	test_audi_divbyn.cpp
 *
 * Unit tests for Audi DivbyN trigger
 *
 * @date Dec 3, 2024
 * @author Andrey Belomutskiy, (c) 2012-2024
 */

#include "pch.h"

#include "trigger_decoder.h"

TEST(trigger, testAudiDivbyNBasic) {
	EngineTestHelper eth(engine_type_e::TEST_ENGINE);
	
	// Configure Audi DivbyN trigger with default parameters
	engineConfiguration->trigger.type = trigger_type_e::TT_AUDI_DIVBYN;
	engineConfiguration->trigger.audiActualTeeth = 135;
	engineConfiguration->trigger.audiTriggerDivider = 3;
	engineConfiguration->trigger.audiFirstTriggerTooth = 0;
	engineConfiguration->trigger.audiTdcAfterTrigger = 58.0f;
	engineConfiguration->trigger.audiToothAngularWidth = 8.0f;
	
	eth.setTriggerType(trigger_type_e::TT_AUDI_DIVBYN);
	
	TriggerWaveform *shape = &engine->triggerCentral.triggerShape;
	
	// Verify basic configuration
	ASSERT_FALSE(shape->shapeDefinitionError) << "Shape should not have errors";
	
	// Calculate expected virtual teeth: (135 * 2) / 3 = 90
	int expectedVirtualTeeth = (135 * 2) / 3;
	ASSERT_EQ(90, expectedVirtualTeeth) << "Virtual teeth calculation";
	
	// Each tooth creates 2 events (rise + fall), so total events = 90 * 2 = 180
	int expectedEvents = expectedVirtualTeeth * 2;
	ASSERT_EQ(expectedEvents, shape->getSize()) << "Total trigger events";
	
	// Verify TDC position
	ASSERT_FLOAT_EQ(58.0f, shape->tdcPosition) << "TDC position";
	
	// Verify that synchronization is needed (multi-tooth trigger)
	ASSERT_TRUE(shape->isSynchronizationNeeded) << "Synchronization should be needed";
	
	// Verify second input needed for cam sensor
	ASSERT_TRUE(shape->needSecondTriggerInput) << "Second trigger input for cam sensor";
	ASSERT_TRUE(shape->isSecondWheelCam) << "Second wheel is cam";
}

TEST(trigger, testAudiDivbyNVirtualToothCalculation) {
	EngineTestHelper eth(engine_type_e::TEST_ENGINE);
	
	// Test with different divider values
	struct TestCase {
		uint8_t actualTeeth;
		uint8_t divider;
		int expectedVirtualTeeth;
	};
	
	TestCase testCases[] = {
		{135, 3, 90},   // Default case: (135 * 2) / 3 = 90
		{135, 5, 54},   // (135 * 2) / 5 = 54
		{120, 2, 120},  // (120 * 2) / 2 = 120
		{60, 2, 60},    // (60 * 2) / 2 = 60
	};
	
	for (const auto& testCase : testCases) {
		engineConfiguration->trigger.type = trigger_type_e::TT_AUDI_DIVBYN;
		engineConfiguration->trigger.audiActualTeeth = testCase.actualTeeth;
		engineConfiguration->trigger.audiTriggerDivider = testCase.divider;
		engineConfiguration->trigger.audiFirstTriggerTooth = 0;
		engineConfiguration->trigger.audiTdcAfterTrigger = 58.0f;
		engineConfiguration->trigger.audiToothAngularWidth = 8.0f;
		
		eth.setTriggerType(trigger_type_e::TT_AUDI_DIVBYN);
		
		TriggerWaveform *shape = &engine->triggerCentral.triggerShape;
		
		// Each tooth creates 2 events (rise + fall)
		int expectedEvents = testCase.expectedVirtualTeeth * 2;
		ASSERT_EQ(expectedEvents, shape->getSize()) 
			<< "Events for teeth=" << (int)testCase.actualTeeth 
			<< " divider=" << (int)testCase.divider;
	}
}

TEST(trigger, testAudiDivbyNToothSpacing) {
	EngineTestHelper eth(engine_type_e::TEST_ENGINE);
	
	// Configure with default parameters
	engineConfiguration->trigger.type = trigger_type_e::TT_AUDI_DIVBYN;
	engineConfiguration->trigger.audiActualTeeth = 135;
	engineConfiguration->trigger.audiTriggerDivider = 3;
	engineConfiguration->trigger.audiFirstTriggerTooth = 0;
	engineConfiguration->trigger.audiTdcAfterTrigger = 58.0f;
	engineConfiguration->trigger.audiToothAngularWidth = 8.0f;
	
	eth.setTriggerType(trigger_type_e::TT_AUDI_DIVBYN);
	
	TriggerWaveform *shape = &engine->triggerCentral.triggerShape;
	
	// Virtual teeth = 90, so spacing should be 720 / 90 = 8 degrees
	float expectedSpacing = 720.0f / 90.0f;
	ASSERT_FLOAT_EQ(8.0f, expectedSpacing) << "Tooth spacing";
	
	// Verify the angle of first few events
	// Event 0: tooth 0 rise at 0 degrees
	// Event 1: tooth 0 fall at 8 degrees (tooth width)
	// Event 2: tooth 1 rise at 8 degrees (next tooth spacing)
	// Event 3: tooth 1 fall at 16 degrees
	
	ASSERT_FLOAT_EQ(0.0f, shape->getSwitchAngle(0)) << "First tooth rise";
	ASSERT_FLOAT_EQ(8.0f, shape->getSwitchAngle(1)) << "First tooth fall";
	ASSERT_FLOAT_EQ(8.0f, shape->getSwitchAngle(2)) << "Second tooth rise";
	ASSERT_FLOAT_EQ(16.0f, shape->getSwitchAngle(3)) << "Second tooth fall";
}

TEST(trigger, testAudiDivbyNInvalidParameters) {
	EngineTestHelper eth(engine_type_e::TEST_ENGINE);
	
	// Test with zero teeth (invalid)
	engineConfiguration->trigger.type = trigger_type_e::TT_AUDI_DIVBYN;
	engineConfiguration->trigger.audiActualTeeth = 0;
	engineConfiguration->trigger.audiTriggerDivider = 3;
	engineConfiguration->trigger.audiFirstTriggerTooth = 0;
	engineConfiguration->trigger.audiTdcAfterTrigger = 58.0f;
	engineConfiguration->trigger.audiToothAngularWidth = 8.0f;
	
	eth.setTriggerType(trigger_type_e::TT_AUDI_DIVBYN);
	
	TriggerWaveform *shape = &engine->triggerCentral.triggerShape;
	ASSERT_TRUE(shape->shapeDefinitionError) << "Should have error with zero teeth";
	
	// Test with zero divider (invalid)
	engineConfiguration->trigger.audiActualTeeth = 135;
	engineConfiguration->trigger.audiTriggerDivider = 0;
	
	eth.setTriggerType(trigger_type_e::TT_AUDI_DIVBYN);
	
	shape = &engine->triggerCentral.triggerShape;
	ASSERT_TRUE(shape->shapeDefinitionError) << "Should have error with zero divider";
}
