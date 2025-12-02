/**
 * @file trigger_emulator_algo.h
 *
 * @date Mar 3, 2014
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once

// Support emulation for all trigger input channels
#define NUM_EMULATOR_CHANNELS TRIGGER_INPUT_PIN_COUNT

class PwmConfig;
class MultiChannelStateSequence;

void initTriggerEmulator();
void startTriggerEmulatorPins();
void stopTriggerEmulatorPins();
void setTriggerEmulatorRPM(int value);
void onConfigurationChangeRpmEmulatorCallback(engine_configuration_s *previousConfiguration);

// Start & stop trigger emulation
void enableTriggerStimulator(bool incGlobalConfiguration = true);
void enableExternalTriggerStimulator();
void disableTriggerStimulator();

class TriggerEmulatorHelper {
public:
    TriggerEmulatorHelper();
	void handleEmulatorCallback(int channel, const MultiChannelStateSequence& mcss, int stateIndex);
};

int getPreviousIndex(const int currentIndex, const int size);
bool needEvent(const int currentIndex, const MultiChannelStateSequence& mcss, int channelIndex);

