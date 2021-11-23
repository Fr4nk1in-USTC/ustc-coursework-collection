module test_bench();

reg clk, rst_n, d;

initial 
begin
    $dumpfile("prob.2.vcd");
    $dumpvars();
end

initial #55 $finish;

initial clk = 1'b0;
always #5 clk = ~clk;

initial
begin
    rst_n = 1'b0;
    #27 rst_n = 1'b1;
end

initial 
begin
    d = 1'b0;
    #13 d = 1'b1;
    #24 d = 1'b0;
end
endmodule
