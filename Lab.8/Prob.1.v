module test(
    input clk, rst,
    output led
);
parameter STATE_0 = 2'h0;
parameter STATE_1 = 2'h1;
parameter STATE_2 = 2'h2;
parameter STATE_3 = 2'h3;
reg [1:0] curr_state, next_state;
// FSM Part 1
always @(*) case (curr_state)
    STATE_0: next_state = STATE_1;
    STATE_1: next_state = STATE_2;
    STATE_2: next_state = STATE_3;
    STATE_3: next_state = STATE_0;
    default: next_state = STATE_0;
endcase
// FSM Part 2
always @(posedge clk or posedge rst) begin
    if (rst) curr_state <= STATE_0;
    else curr_state <= next_state;
end
// FSM Part 3
assign led = (curr_state == STATE_3);
endmodule
