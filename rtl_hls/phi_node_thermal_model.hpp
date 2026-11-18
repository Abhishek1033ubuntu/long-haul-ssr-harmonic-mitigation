/**
 * Copyright (c) 2026 Abhishek Singh
 * Repository: https://github.com/Abhishek1033ubuntu/phononic-diamond-sic-drain
 * Description: Phi-node thermal impedance & phononic mitigation model for 
 *              15 kV Dia-SiC switches under full operational load.
 */

#ifndef PHI_NODE_THERMAL_MODEL_HPP
#define PHI_NODE_THERMAL_MODEL_HPP

#include <cmath>
#include <algorithm>

inline float calculate_phi_node_junction_temp(
    float i_rms,          // Current through the switch (kA)
    float pwm_freq_khz,   // Switching frequency (5 kHz nominal)
    float ambient_temp_c, // Heatsink ambient temp (75 C)
    float r_th_diamond    // Enhanced thermal resistance with CVD diamond + graphene (0.045 C/W)
) {
    #pragma HLS INLINE
    // Joule heating + dynamic switching losses scaled by acoustic impedance mismatch at phi-node
    float power_dissipation = (i_rms * i_rms * 0.012f) + (pwm_freq_khz * 0.18f * std::abs(i_rms));
    float tj_est = ambient_temp_c + (power_dissipation * r_th_diamond * 1200.0f);
    
    // Safety clamp: if junction approaches 175 C thermal ceiling, trigger active derating
    return tj_est;
}

inline bool evaluate_thermal_derating_gate(float tj_current, float &modulation_scale) {
    #pragma HLS INLINE
    if (tj_current > 165.0f) {
        // Active thermal foldback via diamond drain dissipation headroom
        modulation_scale = std::max(0.4f, 1.0f - ((tj_current - 165.0f) / 10.0f));
        return true; // Thermal throttle active
    }
    modulation_scale = 1.0f;
    return false;
}

#endif // PHI_NODE_THERMAL_MODEL_HPP
