// Step 4
// D Flip-Flop
module d_ff (
    input clk, d,
    output reg q 
);
    always@(posedge clk)  q <= d;
endmodule
// D Flip-Flop with a synchronous reset
module d_ff_sr (
    input  clk, rst_n, d,
    output reg q 
);
    always @(posedge clk) begin
        if (rst_n == 0) q <= 1'b0;
        else q <= d;
    end
endmodule

// D Flip-Flop with a synchronous reset
module d_ff_ar (
    input  clk, rst_n, d,
    output reg q
);
    always @(posedge clk or negedge rst_n) begin
        if (rst_n == 0) q <= 1'b0;
        else q <= d;
    end
endmodule

// Step 5
// 4-bit register
module reg_4bit (
    input  clk, rst_n,
    input  [3:0] D_in,
    output reg [3:0] D_out 
);
    always @(posedge clk) begin
        if (rst_n == 0) D_out <= 4'b0; // D_out <= 4'b0011;
        else D_out <= D_in;
    end
endmodule

// Step 6
// 4-bit counter MOD16
module counter_4bit (
    input  clk, rst_n, 
    output reg [3:0] cnt 
);
    always @(posedge clk ) begin
        if (rst_n == 0) cnt <= 4'b0;
        else cnt <= cnt + 4'b1;
    end
endmodule