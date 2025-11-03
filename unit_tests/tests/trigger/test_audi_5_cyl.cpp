#include "pch.h"

TEST(trigger, audi5Cyl) {
	EngineTestHelper eth(engine_type_e::TEST_CRANK_ENGINE);
	
	eth.setTriggerType(trigger_type_e::TT_AUDI_5_CYL);
	
	TriggerWaveform *shape = &eth.engine.triggerCentral.triggerShape;
	
	// Verify the trigger shape was initialized correctly
	ASSERT_FALSE(shape->shapeDefinitionError) << "Trigger should initialize without errors";
	
	// Should have two trigger inputs: 135 teeth primary + crankhome secondary
	ASSERT_TRUE(shape->needSecondTriggerInput) << "Audi trigger requires second input";
	
	// The second wheel is not a cam (both are crank-based, cam-HALL masks the secondary)
	ASSERT_FALSE(shape->isSecondWheelCam) << "Both signals are crank-based";
	
	// Should have 135 teeth on primary wheel = 270 edges (rise + fall for each tooth)
	// Plus 2 edges for the secondary crankhome signal
	int expectedEdges = (135 * 2) + 2; // 270 + 2 = 272 edges
	ASSERT_EQ(expectedEdges, shape->getLength()) << "Should have 272 total edges (135 teeth * 2 + crankhome * 2)";
	
	// TDC position should be set (720° - 62° + 62° = 720°, wrapped to position)
	ASSERT_GT(shape->tdcPosition, 0) << "TDC position should be defined";
	
	// Test that the trigger can sync and run
	eth.smartFireTriggerEvents2(/*count*/400, /*delay*/ 5);
	
	// Should be able to calculate RPM after sufficient events
	float rpm = Sensor::getOrZero(SensorType::Rpm);
	ASSERT_GT(rpm, 0) << "Should calculate RPM after sync";
}
