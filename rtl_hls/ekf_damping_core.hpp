/**
 * Copyright (c) 2026 Abhishek Singh
 * Lead Author / Principal Investigator: Abhishek Singh (ORCID: 0009-0004-6913-096X)
 * Repository: https://github.com/Abhishek1033ubuntu
 * Description: Pipelined 3-state decoupled float32 EKF, Joseph-form covariance update,
 *              Schmitt-trigger hysteresis latch, and raw-current crowbar comparator.
 */

#ifndef EKF_DAMPING_CORE_HPP
#define EKF_DAMPING_CORE_HPP

#include <cmath>
#include <algorithm>
#include "grid_defense_types.hpp"

inline void step_single_node_ekf(
    NodeState &st,
    const NodeTelemetry &in,
    float i_nom_init,
    float thresh_on,
    float thresh_off,
    float crowbar_limit,
    float &dL_local,
    float &dC_local,
    float &A_est,
    bool &crowbar
) {
    #pragma HLS INLINE
    const float dt   = 0.0001f; // 100 us (10 kHz)
    const float Q[3] = {2e-6f, 2e-6f, 1e-6f};
    const float R[2] = {2e-5f, 2e-5f};

    if (!st.init) {
        st.x[0] = 0.012f;
        st.x[1] = 0.0f;
        st.x[2] = 157.0796327f; // 25 Hz initial modal frequency (rad/s)
        for (int r = 0; r < 3; r++) {
            for (int c = 0; c < 3; c++) {
                st.P[r][c] = (r == c) ? ((r == 2) ? 1e-2f : 1e-4f) : 0.0f;
            }
        }
        for (int i = 0; i < WIN_LEN; i++) {
            st.id_buf[i] = i_nom_init;
            st.iq_buf[i] = 0.2f;
        }
        st.buf_idx        = 0;
        st.damping_active = false;
        st.pwm_cnt        = 0;
        st.pwm_up         = true;
        st.dt_top         = 0;
        st.dt_bot         = 0;
        st.init           = true;
    }

    // 0. Independent Hardware Crowbar Comparator (Taps raw sensor input directly)
    float raw_i_mag = std::sqrt(in.id_meas * in.id_meas + in.iq_meas * in.iq_meas);
    crowbar = (in.valid_frame && (raw_i_mag > crowbar_limit));

    // Update 1-cycle fundamental moving average buffer
    if (in.valid_frame && !crowbar) {
        st.id_buf[st.buf_idx] = in.id_meas;
        st.iq_buf[st.buf_idx] = in.iq_meas;
        st.buf_idx = (st.buf_idx + 1) % WIN_LEN;
    }

    float id_sum = 0.0f, iq_sum = 0.0f;
    for (int i = 0; i < WIN_LEN; i++) {
        #pragma HLS UNROLL factor=16
        id_sum += st.id_buf[i];
        iq_sum += st.iq_buf[i];
    }
    float z_osc[2] = {
        in.id_meas - id_sum * 0.005f,
        in.iq_meas - iq_sum * 0.005f
    };

    // 1. EKF Predict Step (Rotational Sub-Synchronous Modal Plane)
    float is_d = st.x[0], is_q = st.x[1], w = st.x[2];
    float x_pred[3] = {
        is_d - w * is_q * dt,
        is_q + w * is_d * dt,
        w
    };
    float F[3][3] = {
        {1.0f,   -w * dt, -is_q * dt},
        {w * dt,  1.0f,    is_d * dt},
        {0.0f,    0.0f,    1.0f}
    };

    float FP[3][3], P_pred[3][3];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            float s = 0.0f;
            for (int k = 0; k < 3; k++) s += F[i][k] * st.P[k][j];
            FP[i][j] = s;
        }
    }
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            float s = (i == j) ? Q[i] : 0.0f;
            for (int k = 0; k < 3; k++) s += FP[i][k] * F[j][k];
            P_pred[i][j] = s;
        }
    }

    // 2. EKF Update Step (Joseph-Form Covariance Update)
    if (in.valid_frame && !crowbar) {
        float s00 = P_pred[0][0] + R[0], s01 = P_pred[0][1];
        float s10 = P_pred[1][0],        s11 = P_pred[1][1] + R[1];
        float inv_det = 1.0f / (s00 * s11 - s01 * s10 + 1e-12f);
        float si00 =  s11 * inv_det, si01 = -s01 * inv_det;
        float si10 = -s10 * inv_det, si11 =  s00 * inv_det;

        float K[3][2];
        for (int i = 0; i < 3; i++) {
            K[i][0] = P_pred[i][0] * si00 + P_pred[i][1] * si10;
            K[i][1] = P_pred[i][0] * si01 + P_pred[i][1] * si11;
        }

        float e0 = z_osc[0] - x_pred[0], e1 = z_osc[1] - x_pred[1];
        for (int i = 0; i < 3; i++) {
            st.x[i] = x_pred[i] + K[i][0] * e0 + K[i][1] * e1;
        }

        float I_kh[3][3] = {
            {1.0f - K[0][0], -K[0][1],        0.0f},
            {-K[1][0],        1.0f - K[1][1], 0.0f},
            {-K[2][0],       -K[2][1],        1.0f}
        };
        float tmp[3][3];
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                float s = 0.0f;
                for (int k = 0; k < 3; k++) s += I_kh[i][k] * P_pred[k][j];
                tmp[i][j] = s;
            }
        }
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                float s = K[i][0] * R[0] * K[j][0] + K[i][1] * R[1] * K[j][1];
                for (int k = 0; k < 3; k++) s += tmp[i][k] * I_kh[j][k];
                st.P[i][j] = s;
            }
        }
    } else {
        for (int i = 0; i < 3; i++) {
            st.x[i] = x_pred[i];
            for (int j = 0; j < 3; j++) st.P[i][j] = P_pred[i][j];
        }
    }

    // 3. Schmitt-Trigger Hysteresis Latch
    A_est = std::sqrt(st.x[0] * st.x[0] + st.x[1] * st.x[1]);
    if (crowbar) st.damping_active = false;
    else if (A_est > thresh_on)  st.damping_active = true;
    else if (A_est < thresh_off) st.damping_active = false;

    if (st.damping_active) {
        dL_local = -5.50f * st.x[1];
        dC_local =  5.50f * st.x[0];
    } else {
        dL_local = 0.0f;
        dC_local = 0.0f;
    }
}

#endif // EKF_DAMPING_CORE_HPP
