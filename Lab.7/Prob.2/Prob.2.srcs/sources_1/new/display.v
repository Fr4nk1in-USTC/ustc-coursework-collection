`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2021/11/25 02:46:34
// Design Name: 
// Module Name: display
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


module display(
    input  clk, rst,
    input  [7:0] sw,
    output reg an,
    output reg [3:0] d
    );
reg [19:0] cnt;
wire clk_100hz;
assign clk_100hz = (cnt >= 500000);
always @(posedge clk)
begin
    if (rst) cnt <= 0;
    else
    begin
        if (cnt >= 1000000) cnt <= 0;
        else cnt <= cnt + 1;
    end
end
always @(posedge clk)
begin
    if (rst)
    begin
        d <= 4'h0;
        an <= 1'b0;
    end
    else
    begin
        if (clk_100hz) 
        begin
            an <= 1'b1;
            d <= sw[7:4];
        end
        else
        begin
            an <= 1'b0;
            d <= sw[3:0];
        end
    end
end
endmodule
