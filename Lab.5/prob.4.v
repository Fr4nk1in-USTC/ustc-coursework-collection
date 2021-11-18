module decoder_3to8(
    input  [3:0] in,
    output reg a, b, c, d, e, f, g, h
);
always @(*) 
begin
    a = 0;
    b = 0;
    c = 0;
    d = 0;
    e = 0;
    f = 0;
    g = 0;
    h = 0;
    case(in)
        3'b000: a = 1;
        3'b001: b = 1;
        3'b010: c = 1;
        3'b011: d = 1;
        3'b100: e = 1;
        3'b101: f = 1;
        3'b110: g = 1;
        3'b111: h = 1;
        default :;
    endcase
end
endmodule

module test_bench();
reg  [3:0] in;
wire a, b, c, d, e, f, g, h;
decoder_3to8 decoder(.in(in), .a(a), .b(b), .c(c), .d(d), .e(e), .f(f), .g(g), .h(h));

initial 
begin
    in = 3'b000;
    repeat(7)
    #10 in = in + 1;
end
endmodule