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
    input  CLK100MHZ,
    output reg [3:0] hexplay_data,
    output reg [2:0] hexplay_an
    );
    reg [29:0] cnt;
    integer i;
    always @(posedge CLK100MHZ) begin
        if (cnt == 30'h3fffffff) cnt <= 0;
        else cnt <= cnt + 1;

        if (hexplay_an == 3'h7) hexplay_an <= 0;
        else hexplay_an <= hexplay_an + 1;

        if (i == 7) i <= 0;
        else i <= i + 1;

        hexplay_data <= {3'h0, cnt[i + 22]};
    end
endmodule
