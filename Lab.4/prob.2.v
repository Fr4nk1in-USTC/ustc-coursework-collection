/* 原题
 * module test(
 *     input [4:0] a,
 *     _____________
 * );
 *     always@(*)
 *         b = a;
 * ____________
 */

module test(
    input  [4:0] a,
    output reg [4:0] b 
);
    always@(*)
        b = a;
endmodule