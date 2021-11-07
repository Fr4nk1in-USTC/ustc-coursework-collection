// 4-bit down counter MOD 10
module down_counter_mod10(
    input clk, rst_n,
    output reg [3:0] cnt
);
    always @(posedge clk or negedge rst_n) begin
        if (rst_n == 0) cnt <= 4'b1001;
        else begin
            if (cnt == 4'b0) cnt <= 4'b1001;
            else cnt <= cnt - 1;
        end
    end
endmodule