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
// 生成 100Hz 时钟信号
assign clk_100hz = (cnt >= 500000);
always @(posedge clk)
begin
    if (rst) cnt <= 0;
    else if (cnt >= 999999) cnt <= 0;
    else cnt <= cnt + 1;
end
// 分时复用显示数字
always @(posedge clk)
begin
    if (clk_100hz) an <= 1'b1;
    else an <= 1'b0;
end
always @(posedge clk)
begin
    if (rst) d <= 4'b0;
    else if (clk_100hz) d <= sw[7:4];
    else d <= sw[3:0];
end
endmodule
