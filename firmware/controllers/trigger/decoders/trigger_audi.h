/*
 * @file trigger_audi.h
 *
 * @date Dec 2, 2024
 * @author rusEFI Team
 */

#pragma once

class TriggerWaveform;

// TT_AUDI_5_CYL - Audi 5-cylinder trigger
// Primary: 135-tooth wheel on starter ring
// Secondary: Home pin on flywheel (fires at 62° BTDC cyl1 and 134° BTDC cyl5)
// Cam: Hall sensor used for 720° sync gating
void configureAudi5Cyl(TriggerWaveform *s);
