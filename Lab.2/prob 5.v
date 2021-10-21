module priority_encoder_8bit (
    input i7, i6, i5, i4, i3, i2, i1, i0,
    output y2, y1, y0
);
    assign y2 = i7 | i6 | i5 | i4;
    assign y1 = i7 | i6 | (~i5 & ~i4 & i3) | (~i5 & ~i4 & i2);
    assign y0 = i7 | (~i6 & i5) | (~i6 & ~i4 & i3) | (~i6 & ~i4 & ~i2 & i1);
endmodule