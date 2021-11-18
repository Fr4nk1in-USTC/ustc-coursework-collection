module test_bench();

initial 
begin
    $dumpfile("prob.2.vcd");
    $dumpvars();
end

reg clk, rst_n, d;

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
