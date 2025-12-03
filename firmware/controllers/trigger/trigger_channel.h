/**
 * @file	trigger_channel.h
 * @brief	Trigger channel definitions for multi-input trigger systems
 *
 * @date Dec 3, 2024
 * @author rusEFI Team
 */

#pragma once

/**
 * @brief Trigger input channel enumeration
 * 
 * Defines the different physical trigger inputs that can be used.
 * - CRANK: Primary crankshaft position sensor (T_PRIMARY)
 * - HOME: Index/home pulse for multi-tooth systems (T_TERTIARY)
 * - CAM_SYNC: Camshaft synchronization signal (T_SECONDARY)
 * - CAM_EXHAUST: Exhaust camshaft signal (future expansion)
 */
enum class TriggerChannel : uint8_t {
	CRANK = 0,        // T_PRIMARY - Main crankshaft position
	CAM_SYNC = 1,     // T_SECONDARY - Cam position/phase sync
	HOME = 2,         // T_TERTIARY - Index/home pulse
	CAM_EXHAUST = 3,  // Future: Exhaust cam (for engines with separate intake/exhaust cam sensors)
};

// Backwards compatibility aliases
#define T_PRIMARY   TriggerChannel::CRANK
#define T_SECONDARY TriggerChannel::CAM_SYNC
#define T_TERTIARY  TriggerChannel::HOME
