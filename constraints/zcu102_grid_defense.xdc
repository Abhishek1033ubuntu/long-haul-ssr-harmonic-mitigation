# 200 MHz Differential System Clock (ZCU102 Si570)
set_property PACKAGE_PIN AL8 [get_ports sys_clk_p]
set_property IOSTANDARD LVDS [get_ports sys_clk_p]
create_clock -period 5.000 -name sys_clk [get_ports sys_clk_p]

# Dual-Redundant IEC 62439-3 PRP Optical SFP+ Cages (LAN-A & LAN-B)
set_property PACKAGE_PIN C4 [get_ports sfp_lan_a_rx_p]
set_property PACKAGE_PIN B4 [get_ports sfp_lan_a_tx_p]
set_property PACKAGE_PIN E4 [get_ports sfp_lan_b_rx_p]
set_property PACKAGE_PIN D4 [get_ports sfp_lan_b_tx_p]

# FMC HPC1 Optical Gate-Drive Outputs (Galvanically Isolated POF Transmitters)
set_property -dict {PACKAGE_PIN D8  IOSTANDARD LVCMOS18 SLEW FAST} [get_ports {tcsr_fire_pos[0]}]
set_property -dict {PACKAGE_PIN D9  IOSTANDARD LVCMOS18 SLEW FAST} [get_ports {tcsr_fire_neg[0]}]
set_property -dict {PACKAGE_PIN G6  IOSTANDARD LVCMOS18 SLEW FAST} [get_ports {vsc_top_gate[0]}]
set_property -dict {PACKAGE_PIN G7  IOSTANDARD LVCMOS18 SLEW FAST} [get_ports {vsc_bot_gate[0]}]
set_property -dict {PACKAGE_PIN H4  IOSTANDARD LVCMOS18 SLEW FAST} [get_ports {bypass_crowbar_cmd[0]}]

# Cabinet Maintenance Key-Switches & SAFE-TO-SERVICE Interlock LED
set_property -dict {PACKAGE_PIN AN14 IOSTANDARD LVCMOS33 PULLDOWN true} [get_ports maint_switch_pull_a]
set_property -dict {PACKAGE_PIN AP14 IOSTANDARD LVCMOS33 PULLDOWN true} [get_ports maint_switch_pull_b]
set_property -dict {PACKAGE_PIN AG14 IOSTANDARD LVCMOS33}               [get_ports maintenance_safe_ok]
