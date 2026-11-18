"""
Copyright (c) 2026 Abhishek Singh
Lead Author / Principal Investigator: Abhishek Singh (ORCID: 0009-0004-6913-096X)
AI Computational & Multi-Physics Support: Team Gemini (Google DeepMind)
Description: Automated GCC compilation and verification runner for the 4-node
             redundant FPGA grid defense HLS kernel.
"""

import subprocess
import sys
import os

CPP_SRC = os.path.join("rtl_hls", "redundant_grid_defense_top.cpp")
BIN_OUT = "grid_defense_verify_bin"

def main():
    if not os.path.exists(CPP_SRC):
        print(f"[ERROR] Source file not found: {CPP_SRC}")
        sys.exit(1)

    print(f"[BUILD] Compiling {CPP_SRC} with g++ -O3 -std=c++17...")
    build_cmd = ["g++", "-O3", "-std=c++17", "-Wno-unknown-pragmas", CPP_SRC, "-o", BIN_OUT]
    res_build = subprocess.run(build_cmd, capture_output=True, text=True)
    if res_build.returncode != 0:
        print("[BUILD FAILED]\n", res_build.stderr)
        sys.exit(res_build.returncode)

    print("[RUN] Executing 15,000-cycle multi-contingency & hitless redundancy testbench...")
    res_run = subprocess.run([f"./{BIN_OUT}"], capture_output=True, text=True)
    print(res_run.stdout.strip())

    if os.path.exists(BIN_OUT):
        os.remove(BIN_OUT)

    if res_run.returncode != 0:
        print("[VERIFICATION FAILED]")
        sys.exit(res_run.returncode)

    print("[SUCCESS] All SSCI damping, multi-node decoupling, PRP redundancy, and crowbar checks passed.")

if __name__ == "__main__":
    main()
