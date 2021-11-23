module test_bench();

reg  a, b;

initial 
begin
    $dumpfile("prob.1.vcd");
    $dumpvars();
end

initial #400 $finish;

initial 
begin
    a = 1'b1;
    #200 a = 1'b0;
end

initial 
begin
    b = 1'b0;
    #100 b = 1'b1;
    #175 b = 1'b0;
    #75  b = 1'b1;
end
endmodule