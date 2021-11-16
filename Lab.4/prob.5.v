/* 原题
 * module sub_test(
 *     input a, b
 * );
 *     output o;
 *     assign o = a + b;
 * endmodule
 * module test(
 *     input a,b,
 *     output c
 * );
 *     always@(*)
 *     begin
 *         sub_test sub_test(a,b,c);
 *     end
 * endmodule
 */
// 改正后
module sub_test(
    input  a, b,
    output o);
    assign o = a + b;
endmodule

module test(
    input  a, b,
    output c);
    sub_test sub_test(a, b, c);
endmodule