module MUX_1bit_2to1 (
    input i0, i1, sel,
    output out
);
    assign out = (~sel & i0) | (sel & i1);
endmodule

module MUX_1bit_4to1 (
    input a, b, c, d, sel1, sel0,
    output out
);
    wire low_bit_0, low_bit_1;

    MUX_1bit_2to1 Mux0(
        .i0 (a),
        .i1 (c),
        .sel(sel1),
        .out(low_bit_0)
    );
    MUX_1bit_2to1 Mux1(
        .i0 (b),
        .i1 (d),
        .sel(sel1),
        .out(low_bit_1)
    );
    
    MUX_1bit_2to1 Mux2(
        .i0 (low_bit_0),
        .i1 (low_bit_1),
        .sel(sel0),
        .out(out)
    );
endmodule