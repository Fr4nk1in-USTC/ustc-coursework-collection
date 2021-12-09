`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2021/12/08 20:48:37
// Design Name: 
// Module Name: signal_edge
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module signal_edge(
    input clk,
    input signal,
    output signal_edge
    );
reg signal_r1, signal_r2;
always @(posedge clk) signal_r1 <= signal;
always @(posedge clk) signal_r2 <= signal_r1;
assign signal_edge = signal_r1 & (~signal_r2);
endmodule
