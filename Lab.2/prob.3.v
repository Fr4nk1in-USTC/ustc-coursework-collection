module MUX_1bit_2to1 (
    input i0, i1, sel,
    output out
);
    assign out = (~sel & i0) | (sel & i1);
endmodule
