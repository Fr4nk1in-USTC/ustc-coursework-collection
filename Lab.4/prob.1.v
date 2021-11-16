/* 原题
 * module test(
 *    input  a,
 *    output b
 * )
 *    if(a) b = i'b0;
 *    else b = 1'b1;
 * endmodule
 */
// 改正后
module test(
    input  a,
    output reg b
);
    always @(*) begin
        if (a) b = 1'b0;
        else   b = 1'b1;
    end
endmodule