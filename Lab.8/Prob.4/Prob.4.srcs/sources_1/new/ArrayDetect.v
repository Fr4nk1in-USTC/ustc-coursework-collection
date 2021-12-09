`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2021/12/08 21:08:23
// Design Name: 
// Module Name: array_detect
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


module array_detect(
    input clk_100mhz,
    input btn,
    input sw,
    output reg [3:0] hexplay_data,
    output reg [2:0] hexplay_an
    );
// generate a 400 Hz pulse
wire pulse_400hz;
reg [18:0] pulse_cnt;

assign pulse_400hz = (pulse_cnt == 19'h00001);
always @(posedge clk_100mhz)
begin
    if (pulse_cnt >= 19'h3d090) pulse_cnt <= 19'h00000;
    else pulse_cnt <= pulse_cnt + 19'h00001;
end

// get btn signal edge
wire btn_edge;
signal_edge getBtnEdge(.clk(clk_100mhz),
                       .signal(btn),
                       .signal_edge(btn_edge));

// input array process
reg [3:0] input_array;
initial input_array = 4'h0;
always @(posedge clk_100mhz)
begin
     if (btn_edge) input_array <= {input_array[2:0], sw};
end
 
// FSM
parameter STATE_0 = 2'b00;
parameter STATE_1 = 2'b01;
parameter STATE_2 = 2'b10;
parameter STATE_3 = 2'b11;

reg [1:0] curr_state, next_state;
reg [3:0] cnt;
initial cnt <= 4'h0;
// Part 1
always @(*) 
begin
    if (sw) 
    begin
        case(curr_state)
            STATE_0: next_state = STATE_1;
            STATE_1: next_state = STATE_2;
            STATE_2: next_state = STATE_2;
            STATE_3: next_state = STATE_0;
            default: next_state = STATE_0;
        endcase
    end
    else
    begin
        case(curr_state)
            STATE_0: next_state = STATE_0;
            STATE_1: next_state = STATE_0;
            STATE_2: next_state = STATE_3;
            STATE_3: next_state = STATE_0;
            default: next_state = STATE_0;
        endcase
    end
end
// Part 2
always @(posedge clk_100mhz)
    if (btn_edge) curr_state <= next_state;
// Part 3
always @(posedge clk_100mhz)
begin
    if (btn_edge) 
    begin
        if (curr_state == STATE_3 && sw == 0)
        begin
            if (cnt >= 4'hf) cnt <= 4'h0;
            else cnt <= cnt + 4'h1;
        end 
    end
end

// Segplay
always @(posedge clk_100mhz)
begin
    if (pulse_400hz)
    begin
        if (hexplay_an >= 3'h7)
            hexplay_an <= 3'h0;
        else if (hexplay_an == 3'h0 ||
                 hexplay_an == 3'h5)
             hexplay_an <= hexplay_an + 3'h2;
        else hexplay_an <= hexplay_an + 3'h1;
    end
end

always @(posedge clk_100mhz)
begin
    case (hexplay_an)
        3'h0: hexplay_data <= cnt;
        3'h2: hexplay_data <= {3'b000, input_array[0]};
        3'h3: hexplay_data <= {3'b000, input_array[1]};
        3'h4: hexplay_data <= {3'b000, input_array[2]};
        3'h5: hexplay_data <= {3'b000, input_array[3]};
        3'h7: hexplay_data <= {2'b00, curr_state};
        default: hexplay_data <= 4'h0;
    endcase
end
endmodule
