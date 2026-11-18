/**
 * Copyright (c) 2026 Abhishek Singh
 * Lead Author / Principal Investigator: Abhishek Singh (ORCID: 0009-0004-6913-096X)
 * Repository: https://github.com/Abhishek1033ubuntu
 * Description: Hardware data structures, per-unit scaling parameters, and 4-node
 *              corridor decoupling matrix for AMD/Xilinx Zynq UltraScale+ HLS.
 */

#ifndef GRID_DEFENSE_TYPES_HPP
#define GRID_DEFENSE_TYPES_HPP

#include <cstdint>

#define NUM_NODES 4
#define WIN_LEN   200  // 1-cycle (20 ms @ 10 kHz) 50 Hz fundamental rejection window

struct NodeTelemetry {
    float    vd_meas;          // Per-unit or scaled d-axis bus voltage
    float    vq_meas;          // Per-unit or scaled q-axis bus voltage
    float    id_meas;          // Per-unit or scaled d-axis line current
    float    iq_meas;          // Per-unit or scaled q-axis line current
    float    grid_phase_rad;   // Synchronized PLL phase angle [0, 2*pi)
    uint64_t ptp_timestamp_ns; // IEEE 1588 PTPv2 nanosecond timestamp
    bool     valid_frame;      // TSN / PRP frame validity flag
};

struct NodeHardwareOutputs {
    float delta_L_coord;       // Coordinated inductive modulation command (pu)
    float delta_C_coord;       // Coordinated capacitive modulation command (pu)
    float est_A_sso;           // Estimated sub-synchronous oscillation amplitude
    float alpha_deg;           // TCSR optical firing angle (degrees)
    float pwm_duty;            // VSC optical PWM duty cycle [0.05, 0.95]
    bool  tcsr_fire_pos;       // Positive half-cycle optical trigger
    bool  tcsr_fire_neg;       // Negative half-cycle optical trigger
    bool  vsc_top_gate;        // Top Dia-SiC switch optical gate
    bool  vsc_bot_gate;        // Complementary bottom Dia-SiC gate (2.0 us dead-time)
    bool  bypass_crowbar_cmd;  // 5 ns hardware fault bypass crowbar trigger
    bool  ptp_sync_ok;         // IEEE 1588 PTP clock skew status (<= 5.0 us)
};

struct RedundantSystemStatus {
    bool active_is_primary;      // True = Board A owns optical bus; False = Board B
    bool lan_a_healthy;          // Primary IEC 62439-3 PRP ring status
    bool lan_b_healthy;          // Diverse backup IEC 62439-3 PRP ring status
    bool maintenance_safe_ok;    // Cabinet SAFE-TO-SERVICE hardware interlock LED
    bool failover_event_latched; // Diagnostic latch for automatic watchdog failover
};

struct NodeState {
    float    x[3];               // [is_d, is_q, omega_sso]
    float    P[3][3];            // 3x3 Joseph-form error covariance matrix
    float    id_buf[WIN_LEN];    // Sliding fundamental rejection buffer (d-axis)
    float    iq_buf[WIN_LEN];    // Sliding fundamental rejection buffer (q-axis)
    int      buf_idx;
    bool     init;
    bool     damping_active;     // Schmitt-trigger hysteresis latch state
    uint32_t pwm_cnt;
    bool     pwm_up;
    uint32_t dt_top;
    uint32_t dt_bot;
};

// Cross-Node Inverse Sensitivity Matrix (cancels traveling-wave reflections across substations)
static const float W_DECOUPLE[NUM_NODES][NUM_NODES] = {
    { 1.00f, -0.18f,  0.04f, -0.01f},
    {-0.18f,  1.04f, -0.18f,  0.04f},
    { 0.04f, -0.18f,  1.04f, -0.18f},
    {-0.01f,  0.04f, -0.18f,  1.00f}
};

#endif // GRID_DEFENSE_TYPES_HPP
