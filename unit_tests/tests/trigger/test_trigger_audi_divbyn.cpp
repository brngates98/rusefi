/**
 * @file test_trigger_audi_divbyn.cpp
 * @brief Unit test for Audi DivbyN trigger
 */

#include "pch.h"

TEST(AudiDivbyN, testBasicInitialization) {
	EngineTestHelper eth(engine_type_e::TEST_ENGINE);
	
	// Set trigger type to Audi DivbyN
	eth.setTriggerType(trigger_type_e::TT_AUDI_DIVBYN);
	
	TriggerWaveform *shape = &eth.engine.triggerCentral.triggerShape;
	
	// Verify no shape definition error
	ASSERT_FALSE(shape->shapeDefinitionError) << "Audi DivbyN should initialize without errors";
	
	// Verify flags are set correctly
	ASSERT_TRUE(shape->isSynchronizationNeeded) << "Synchronization should be needed";
	ASSERT_TRUE(shape->needSecondTriggerInput) << "Second trigger input should be needed";
	ASSERT_TRUE(shape->isSecondWheelCam) << "Second wheel should be cam";
	ASSERT_FALSE(shape->useOnlyPrimaryForSync) << "Should not use only primary for sync";
	
	// Verify operation mode is crank sensor
	ASSERT_EQ(FOUR_STROKE_CRANK_SENSOR, shape->getWheelOperationMode()) 
		<< "Should be four stroke crank sensor";
	
	// Verify we have primary channel events
	ASSERT_GT(shape->getExpectedEventCount(TriggerWheel::T_PRIMARY), 0) 
		<< "Primary channel should have events";
	
	// Verify we have secondary channel events
	ASSERT_GT(shape->getExpectedEventCount(TriggerWheel::T_SECONDARY), 0) 
		<< "Secondary channel should have events";
	
	// With defaults (135 teeth, divider 3), we should have 90 virtual teeth
	// Each tooth has rise + fall = 180 events on primary
	// Secondary has 1 window (rise + fall) = 2 events
	// Total length over 720° depends on operation mode multiplier
	
	// Verify event counts are within reasonable limits
	ASSERT_LT(shape->getExpectedEventCount(TriggerWheel::T_PRIMARY), 500) 
		<< "Primary event count should be under limit";
	ASSERT_LT(shape->getExpectedEventCount(TriggerWheel::T_SECONDARY), 10) 
		<< "Secondary event count should be under limit";
	
	// Verify total shape size is reasonable
	ASSERT_GT(shape->getSize(), 0) << "Shape should have events";
	ASSERT_LT(shape->getSize(), 1000) << "Shape size should be under limit";
}

TEST(AudiDivbyN, testVirtualTeethCalculation) {
	EngineTestHelper eth(engine_type_e::TEST_ENGINE);
	
	eth.setTriggerType(trigger_type_e::TT_AUDI_DIVBYN);
	
	TriggerWaveform *shape = &eth.engine.triggerCentral.triggerShape;
	
	// With default values: actualTeeth=135, divider=3
	// virtualTeeth = (135 * 2) / 3 = 90
	// Each virtual tooth has rise + fall = 2 events
	// So primary should have 90 * 2 = 180 events
	
	ASSERT_EQ(180, shape->getExpectedEventCount(TriggerWheel::T_PRIMARY)) 
		<< "Default config should produce 180 primary events";
}
