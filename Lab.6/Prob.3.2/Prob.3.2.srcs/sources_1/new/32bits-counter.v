`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2021/11/18 15:44:53
// Design Name: 
// Module Name: counter_32bit
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


module counter_32bit(
    input  clk, rst,
    output [7:0] led
    );
    reg [31:0] count;
    assign led = count[31:24];
    always @(posedge clk or posedge rst) begin
        if (rst || count == 32'hffffffff)
            count <= 32'h00000000;
        else
            count <= count + 1;
    end
endmodule
