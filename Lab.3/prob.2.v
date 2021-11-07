// D Flip-Flop with a synchronous set
module d_ff_ss (
    input  clk, st_n, d, 
    output reg q 
);
    always @(posedge clk) begin
        if (st_n == 0) q <= 1'b1;
        else q <= d;
    end
endmodule