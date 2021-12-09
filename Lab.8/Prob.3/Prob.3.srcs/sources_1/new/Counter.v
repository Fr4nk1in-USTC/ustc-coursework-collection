`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2021/12/08 19:50:04
// Design Name: 
// Module Name: counter
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


module counter(
    input clk_100mhz,
    input sw,
    input btn,
    input rst,
    output reg hexplay_an,
    output reg [3:0] hexplay_data
    );
// generate a 100 hz clock
wire clk_100hz;
reg [19:0] clk_cnt;
assign clk_100hz = (clk_cnt >= 500000);
always @(posedge clk_100mhz)
begin
    if (clk_cnt >= 1000000) clk_cnt = 0;
    else clk_cnt = clk_cnt + 20'h00001;
end
// get the edge of btn signal
wire btn_edge;
signal_edge getBtnEdge(.clk(clk_100mhz), 
                       .signal(btn), 
                       .signal_edge(btn_edge));
// counter
reg [7:0] cnt;
always @(posedge clk_100mhz)
begin
    if (rst) cnt <= 8'h1f;
    else if (btn_edge)
    begin
        if (sw)
        begin
            if (cnt >= 8'hff) cnt <= 8'h00;
            else cnt <= cnt + 8'h01;
        end
        else
        begin
            if (cnt == 8'h00) cnt <= 8'hff;
            else cnt <= cnt - 8'h01;
        end
    end
end
// segplay
always @(posedge clk_100mhz)
begin
    if (clk_100hz) 
    begin
        hexplay_an = 1'b1;
        hexplay_data = cnt[7:4];
    end
    else
    begin
        hexplay_an = 1'b0;
        hexplay_data = cnt[3:0];
    end
end
endmodule
