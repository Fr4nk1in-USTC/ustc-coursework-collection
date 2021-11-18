module test_bench();
initial 
begin
    $dumpfile("prob.1.vcd");
    $dumpvars();
end

reg [1:0] a, b;

initial 
begin
    a = 1;
    #200 a = 0;
    #200 $finish;
end

initial 
begin
    b = 0;
    #100 b = 1;
    #175 b = 0;
    #75  b = 1;
    #50  $finish;
end
endmodule