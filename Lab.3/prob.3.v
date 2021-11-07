// 4-bit up counter MOD 16
module up_counter_mod16 (
    input clk, rst_n,
    output reg [3:0] cnt
);
    always @(posedge clk or negedge rst_n) begin
        if (rst_n == 0) cnt <= 4'b0;
        else cnt <= cnt + 4'b1;
    end
endmodule