`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2021/11/25 13:31:19
// Design Name: 
// Module Name: timer
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


module timer(
    input  clk_100MHz, rst,
    output reg [3:0] hexplay_data,
    output reg [1:0] hexplay_an
    );
    
wire pulse_10Hz, pulse_200Hz;
reg  [23:0] cnt_1;
reg  [18:0] cnt_2;
assign pulse_10Hz = (cnt_1 == 24'h1);
assign pulse_200Hz = (cnt_2 == 19'h1);

always @(posedge clk_100MHz)
begin
    if (rst) cnt_1 <= 0;
    else
    begin
        if (cnt_1 >= 9999999) cnt_1 <= 0;
        else cnt_1 <= cnt_1 + 1;
    end
end

always @(posedge clk_100MHz)
begin
    if (cnt_2 >= 499999) cnt_2 <= 0;
    else cnt_2 <= cnt_2 + 1;
end

reg [3:0] deci_sec, sec, ten_sec, min;
always @(posedge clk_100MHz)
begin
    if (rst)
    begin
        min      <= 4'h1;
        ten_sec  <= 4'h2;
        sec      <= 4'h3;
        deci_sec <= 4'h4;
    end
    else if (pulse_10Hz)
    begin
        if (deci_sec >= 4'h9)
        begin
            if (sec >= 4'h9)
            begin
                if (ten_sec >= 4'h5)
                begin
                    if (min >= 4'h9) min <= 4'h0;
                    else             min <= min + 1;
                    ten_sec <= 4'h0;
                end
                else ten_sec <= ten_sec + 1;
                sec <= 4'h0;
            end
            else sec <= sec + 1;
            deci_sec <= 4'h0;
        end
        else deci_sec <= deci_sec + 1;
    end
end

always @(posedge clk_100MHz)
begin
        begin
            if (hexplay_an >= 2'h3) hexplay_an <= 2'h0;
            else                    hexplay_an <= hexplay_an + 1;
        end
end

always @(clk_100MHz)
begin
    case(hexplay_an)
    2'h0: hexplay_data <= deci_sec;
    2'h1: hexplay_data <= sec;
    2'h2: hexplay_data <= ten_sec;
    2'h3: hexplay_data <= min;
    endcase
end
endmodule
