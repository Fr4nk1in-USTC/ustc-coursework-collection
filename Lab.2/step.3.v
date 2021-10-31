// 例1
module test(    //模块名称
    input in,   //输入信号声明
    output out, //输出信号声明
    output out_n);
    //如需要，可在此处声明内部变量
    /*******以下为逻辑描述部分******/
    assign out = in;
    assign out_n = ~in;
    /*******逻辑描述部分结束******/
endmodule //模块名结束关键词

// 例2: 半加器
// 从行为级上描述
module add(
    input a, b,
    output sum, cout);

    assign {cout,sum} = a + b; // 位拼接
endmodule

// 从电路级上描述
module add1(
    input a,b,
    output sum,cout);
    // 两个 assign 是位置无关的
    assign cout = a & b;
    assign sum = a ^ b;
endmodule

// 例3: 全加器
module full_add(
    input a,b,cin,
    output sum,cout);
    wire s,carry1,carry2;
    add add_inst1( // 内部信号声明
        .a (a ),
        .b (b ),
        .sum (s ),
        .cout (carry1));
    add add_inst2(
    .a (s ),
    .b (cin ),
    .sum (sum ),
    .cout (carry2));
    assign cout = carry1 | carry2;
endmodule