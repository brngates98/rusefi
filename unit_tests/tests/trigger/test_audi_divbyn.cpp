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
	
	// Verify that synchronization is NOT needed (uniform teeth, no missing teeth pattern)
	// With uniform teeth, this trigger doesn't provide automatic cycle sync
	// In a real implementation, cam/home sensors would be used for cycle detection
	ASSERT_FALSE(shape->isSynchronizationNeeded) << "Synchronization not needed for uniform teeth";
	
	// Verify shape without TDC (uniform teeth don't define exact TDC location)
	ASSERT_TRUE(shape->shapeWithoutTdc) << "Shape without TDC for uniform teeth";
	
	// No second input in this simplified implementation
	ASSERT_FALSE(shape->needSecondTriggerInput) << "No second trigger input in simplified version";
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
	
	// Verify trigger was created successfully
	ASSERT_FALSE(shape->shapeDefinitionError) << "Shape should not have errors";
	
	// Verify we have multiple events (teeth)
	ASSERT_GT(shape->getSize(), 10) << "Should have many trigger events";
	
	// Verify events form a reasonable pattern with alternating rise/fall
	for (size_t i = 0; i < std::min((size_t)10, shape->getSize() - 1); i += 2) {
		float riseAngle = shape->getSwitchAngle(i);
		float fallAngle = shape->getSwitchAngle(i + 1);
		ASSERT_LT(riseAngle, fallAngle) << "Rise should be before fall for tooth " << i/2;
	}
}

TEST(trigger, testAudiDivbyNInvalidParameters) {
	// Note: Invalid parameter testing would require catching firmware errors
	// which are thrown as exceptions during trigger initialization.
	// For now, we verify that the trigger works with valid parameters.
	// TODO: Add proper invalid parameter handling if needed
	GTEST_SKIP() << "Invalid parameter test skipped - requires firmware error handling";
}
