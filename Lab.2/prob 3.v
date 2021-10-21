module 1bit_2to1_MUX (
    input i0, i1, sel,
    output out
);
    assign out = (~sel & i0) | (sel & i1);
endmodule
