module test(
    input [7:0] a,b,
    output [7:0] c,d,e,f,g,h,i,j,k
);
    assign c = a & b;
    assign d = a | b;
    assign e = a ^ b;
    assign f = ~a;
    assign g = {a[3:0],b[3:0]};
    assign h = a >> 3;
    assign i = &b;
    assign j = (a > b) ? a : b;
    assign k = a - b;
endmodule

/* a = 8'b0011_0011, b = 8'b1111_0000 时:
 * c = 8'b0011_0000
 * d = 8'b1111_0011
 * e = 8'b1100_0011
 * f = 8'b1100_1100
 * g = 8'b0011_0000
 * h = 8'b0000_0110
 * i = 8'b0000_0000
 * j = 8'b1111_0000
 * k = 8'b0100_0011
 */

module test_bench;
    reg [7:0] a, b;
    wire [7:0] c, d, e, f, g, h, i, j, k;
    test test(a, b, c, d, e, f, g, h, i, j, k);
    initial begin
        a = 8'b0011_0011;
        b = 8'b1111_0000;
        #5 $display("a = %b, b = %b", a, b);
        #5 $display("c = %b, d = %b", c, d);
        #5 $display("e = %b, f = %b", e, f);
        #5 $display("g = %b, h = %b", g, h);
        #5 $display("i = %b, j = %b", i, j);
        #5 $display("k = %b", k);
    end
endmodule