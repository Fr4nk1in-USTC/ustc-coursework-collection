/* 原题
 * module sub_test(
 *     input a,b,
 *     output reg c
 * );
 *     assign c = (a<b)? a : b;
 * endmodule
 * 
 * module test(
 *     input a,b,c,
 *     output o
 * );
 *     reg temp;
 *     sub_test(.a(a),.b(b),temp);
 *     sub_test(temp,c,.c(o));
 * endmodule
 */
// 改正后
module sub_test(
    input  a, b,
    output c);
    assign c = (a<b)? a : b;
endmodule

module test (
    input a, b, c,
    output o);
    wire temp;
    sub_test sub_test1(.a(a),     .b(b), .c(temp));
    sub_test sub_test2(.a(temp),  .b(c), .c(o));
endmodule 