# 
# Synthesis run script generated by Vivado
# 

set TIME_start [clock seconds] 
proc create_report { reportName command } {
  set status "."
  append status $reportName ".fail"
  if { [file exists $status] } {
    eval file delete [glob $status]
  }
  send_msg_id runtcl-4 info "Executing : $command"
  set retval [eval catch { $command } msg]
  if { $retval != 0 } {
    set fp [open $status w]
    close $fp
    send_msg_id runtcl-5 warning "$msg"
  }
}
set_param chipscope.maxJobs 2
create_project -in_memory -part xc7a100tcsg324-1

set_param project.singleFileAddWarning.threshold 0
set_param project.compositeFile.enableAutoGeneration 0
set_param synth.vivado.isSynthRun true
set_msg_config -source 4 -id {IP_Flow 19-2162} -severity warning -new_severity info
set_property webtalk.parent_dir D:/Users/Documents/GitHub/COD_lab/lab4/CPU_single_cycle/CPU_single_cycle.cache/wt [current_project]
set_property parent.project_path D:/Users/Documents/GitHub/COD_lab/lab4/CPU_single_cycle/CPU_single_cycle.xpr [current_project]
set_property default_lib xil_defaultlib [current_project]
set_property target_language Verilog [current_project]
set_property ip_output_repo d:/Users/Documents/GitHub/COD_lab/lab4/CPU_single_cycle/CPU_single_cycle.cache/ip [current_project]
set_property ip_cache_permissions {read write} [current_project]
add_files D:/Users/Documents/GitHub/COD_lab/lab4/CPU_single_cycle/CPU_single_cycle.srcs/sources_1/coes/fib_from_mem.coe
add_files D:/Users/Documents/GitHub/COD_lab/lab4/CPU_single_cycle/CPU_single_cycle.srcs/sources_1/coes/data_mem.coe
add_files D:/Users/Documents/GitHub/COD_lab/lab4/CPU_single_cycle/CPU_single_cycle.srcs/sources_1/coes/fib_from_io.coe
add_files D:/Users/Documents/GitHub/COD_lab/lab4/CPU_single_cycle/CPU_single_cycle.srcs/sources_1/coes/test.coe
read_verilog -library xil_defaultlib {
  D:/Users/Documents/GitHub/COD_lab/lab4/CPU_single_cycle/CPU_single_cycle.srcs/sources_1/new/ALU.v
  D:/Users/Documents/GitHub/COD_lab/lab4/CPU_single_cycle/CPU_single_cycle.srcs/sources_1/new/ALU_control.v
  D:/Users/Documents/GitHub/COD_lab/lab4/CPU_single_cycle/CPU_single_cycle.srcs/sources_1/new/CPU.v
  D:/Users/Documents/GitHub/COD_lab/lab4/CPU_single_cycle/CPU_single_cycle.srcs/sources_1/new/Control.v
  D:/Users/Documents/GitHub/COD_lab/lab4/CPU_single_cycle/CPU_single_cycle.srcs/sources_1/new/Imm_Gen_Control.v
  D:/Users/Documents/GitHub/COD_lab/lab4/CPU_single_cycle/CPU_single_cycle.srcs/sources_1/new/PDU.v
  D:/Users/Documents/GitHub/COD_lab/lab4/CPU_single_cycle/CPU_single_cycle.srcs/sources_1/new/Reg_File.v
  D:/Users/Documents/GitHub/COD_lab/lab4/CPU_single_cycle/CPU_single_cycle.srcs/sources_1/new/Main.v
}
read_ip -quiet D:/Users/Documents/GitHub/COD_lab/lab4/CPU_single_cycle/CPU_single_cycle.srcs/sources_1/ip/inst_mem/inst_mem.xci
set_property used_in_implementation false [get_files -all d:/Users/Documents/GitHub/COD_lab/lab4/CPU_single_cycle/CPU_single_cycle.srcs/sources_1/ip/inst_mem/inst_mem_ooc.xdc]

read_ip -quiet D:/Users/Documents/GitHub/COD_lab/lab4/CPU_single_cycle/CPU_single_cycle.srcs/sources_1/ip/data_mem/data_mem.xci
set_property used_in_implementation false [get_files -all d:/Users/Documents/GitHub/COD_lab/lab4/CPU_single_cycle/CPU_single_cycle.srcs/sources_1/ip/data_mem/data_mem_ooc.xdc]

# Mark all dcp files as not used in implementation to prevent them from being
# stitched into the results of this synthesis run. Any black boxes in the
# design are intentionally left as such for best results. Dcp files will be
# stitched into the design at a later time, either when this synthesis run is
# opened, or when it is stitched into a dependent implementation run.
foreach dcp [get_files -quiet -all -filter file_type=="Design\ Checkpoint"] {
  set_property used_in_implementation false $dcp
}
read_xdc D:/Users/Documents/GitHub/COD_lab/lab4/CPU_single_cycle/CPU_single_cycle.srcs/constrs_1/new/FPGA.xdc
set_property used_in_implementation false [get_files D:/Users/Documents/GitHub/COD_lab/lab4/CPU_single_cycle/CPU_single_cycle.srcs/constrs_1/new/FPGA.xdc]

read_xdc dont_touch.xdc
set_property used_in_implementation false [get_files dont_touch.xdc]
set_param ips.enableIPCacheLiteLoad 1
close [open __synthesis_is_running__ w]

synth_design -top main -part xc7a100tcsg324-1


# disable binary constraint mode for synth run checkpoints
set_param constraints.enableBinaryConstraints false
write_checkpoint -force -noxdef main.dcp
create_report "synth_1_synth_report_utilization_0" "report_utilization -file main_utilization_synth.rpt -pb main_utilization_synth.pb"
file delete __synthesis_is_running__
close [open __synthesis_is_complete__ w]
