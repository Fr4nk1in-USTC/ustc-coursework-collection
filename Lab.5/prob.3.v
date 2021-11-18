module d_ff_r(
    input clk,rst_n,d,
    output reg q
);
always@(posedge clk)
begin
    if(rst_n==0)
        q <= 1'b0;
    else
        q <= d;
end
endmodule

module test_bench();

reg  clk, rst_n, d;
wire q;

d_ff_r d_ff_r(.clk(clk),.rst_n(rst_n),.d(d),.q(q));

initial clk = 0;
always #5 clk = ~clk;

initial
begin
    rst_n = 0;
    #27 rst_n = 1;
    #28 $finish;
end

initial 
begin
    d = 0;
    #13 d = 1;
    #24 d = 0;
    #18 $finish;
end

endmodule

