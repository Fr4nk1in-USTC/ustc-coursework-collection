`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2021/11/18 09:43:39
// Design Name: 
// Module Name: counter_30bit
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


module counter_30bit(
    input  clk, rst,
    output [7:0] led
    );
    reg [29:0] count;
    assign led = count[29:22];
    always @(posedge clk or posedge rst) begin
        if (rst || count == 30'h3fffffff)
            count <= 30'h00000000;
        else
            count <= count + 1;
    end
endmodule
