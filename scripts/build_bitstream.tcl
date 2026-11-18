open_project -reset zynq_grid_defense_prj
set_top redundant_grid_defense_top

add_files rtl_hls/grid_defense_types.hpp
add_files rtl_hls/ekf_damping_core.hpp
add_files rtl_hls/redundant_grid_defense_top.cpp
add_files -tb rtl_hls/redundant_grid_defense_top.cpp

open_solution -reset "solution_zu9eg_200mhz" -flow_target vivado
set_part {xczu9eg-ffvb1156-2-e}
create_clock -period 5.0 -name default

csim_design -clean
csynth_design
cosim_design -rtl verilog
export_design -format ip_catalog -description "Redundant 4-Node AI L/C Grid Defense Controller" -vendor "grid_rd" -version "1.0"
exit
