#include "pch.h"
#include "trigger_decoder.h"

// Test basic DivbyN with default 135/3 configuration
TEST(AudiDivbyN, BasicConfiguration) {
    EngineTestHelper eth(engine_type_e::TEST_ENGINE);
    
    engineConfiguration->audiActualTeeth = 135;
    engineConfiguration->audiTriggerDivider = 3;
    engineConfiguration->audiTdcAfterTrigger = 58.0f;
    
    eth.setTriggerType(trigger_type_e::TT_AUDI_DIVBYN);
    
    // Verify no shape definition error
    ASSERT_FALSE(engine->triggerCentral.triggerShape.shapeDefinitionError);
    
    // Verify event count is under limit (252)
    ASSERT_LT(engine->triggerCentral.triggerShape.getSize(), 252);
}

// Test different divider values
TEST(AudiDivbyN, Divider2) {
    EngineTestHelper eth(engine_type_e::TEST_ENGINE);
    
    engineConfiguration->audiActualTeeth = 135;
    engineConfiguration->audiTriggerDivider = 2;  // (135*2)/2 = 135 virtual teeth
    
    eth.setTriggerType(trigger_type_e::TT_AUDI_DIVBYN);
    
    ASSERT_FALSE(engine->triggerCentral.triggerShape.shapeDefinitionError);
}

// Test 136-tooth wheel for V8/even cylinder engines
TEST(AudiDivbyN, EvenCylinder136Tooth) {
    EngineTestHelper eth(engine_type_e::TEST_ENGINE);
    
    engineConfiguration->audiActualTeeth = 136;
    engineConfiguration->audiTriggerDivider = 4;  // (136*2)/4 = 68 virtual teeth
    
    eth.setTriggerType(trigger_type_e::TT_AUDI_DIVBYN);
    
    ASSERT_FALSE(engine->triggerCentral.triggerShape.shapeDefinitionError);
}

// Test synchronization behavior
TEST(AudiDivbyN, Synchronization) {
    EngineTestHelper eth(engine_type_e::TEST_ENGINE);
    
    engineConfiguration->audiActualTeeth = 135;
    engineConfiguration->audiTriggerDivider = 3;
    
    eth.setTriggerType(trigger_type_e::TT_AUDI_DIVBYN);
    
    ASSERT_EQ(0, round(Sensor::getOrZero(SensorType::Rpm)));
    
    // Fire trigger events to achieve sync
    eth.smartFireTriggerEvents2(/*count*/100, /*delay*/ 5);
    
    // Should achieve synchronization
    EXPECT_TRUE(engine->triggerCentral.triggerState.getShaftSynchronized());
}

// Test no errors with clean signal over multiple revolutions
TEST(AudiDivbyN, NoTriggerErrors) {
    EngineTestHelper eth(engine_type_e::TEST_ENGINE);
    
    engineConfiguration->audiActualTeeth = 135;
    engineConfiguration->audiTriggerDivider = 3;
    
    eth.setTriggerType(trigger_type_e::TT_AUDI_DIVBYN);
    
    // Run for 10 complete engine cycles
    for (int i = 0; i < 10; i++) {
        eth.smartFireTriggerEvents2(/*count*/90, /*delay*/ 5);
    }
    
    // Should have zero trigger errors
    EXPECT_EQ(0u, engine->triggerCentral.triggerState.totalTriggerErrorCounter);
}
