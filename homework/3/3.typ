#import "homework.typ": *

#show: homework.with(number: 3)

#question()

+ $F$ 的最小函数依赖集为 ${A -> B, A -> E, B -> C, B -> D, B E -> A}$.
  - 将右边写出单属性并去除重复 FD:
    - $F = {A -> B, A -> C, B -> C, B -> D, A -> E, A B -> C, A C -> D, A C -> E,$
      $B E -> A, B E -> D}$
  - 消去左部冗余属性:
    - $A -> B, A B -> C ==> A -> C$, 因此可去除 $A B -> C$ 中的 $B$.
    - $A -> C, A C -> D ==> A -> D$, 因此可去除 $A C -> D$ 中的 $C$.
    - $A -> C, A C -> E ==> A -> E$, 因此可去除 $A C -> E$ 中的 $C$.
    - $B -> D$, 因此可去除 $B E -> D$ 中的 $E$.
    - $F = {A -> B, A -> C, B -> C, B -> D, A -> E, A -> C, A -> D, A -> E, B E -> A,$
      $B -> D} = {A -> B, A -> C, A -> D, A -> E, B -> C, B -> D, B E -> A}$
  - 消去冗余函数依赖:
    - $A -> C <== A -> B, B -> C$
    - $A -> D <== A -> B, B -> D$
    - $F = {A -> B, A -> E, B -> C, B -> D, B E -> A}$
+ $R$ 的候选码为 $A, B E$.
  - $A -> B, B -> C, B -> D, A -> E in F^+ ==> A -> A B C D E in F^+$,
    并且不存在 $A$ 的真子集 $Y$ 使得 $Y -> U$ 成立.
  - $B E -> A in F^+ ==> B E -> A B C D E in F^+$, 并且不存在 $B E$ 的真子集 $Y$
    使得 $Y -> U$ 成立.
+ 因为非主属性 $C$ 局部函数依赖于 $B E$ (存在 $B -> C$), 所以 $R$ 只属于 1NF.

#question()

+ $F$ 的最小函数依赖集为 ${A -> B, B -> C, A -> D, A -> E, E -> F, A -> G}$.
  - 将右边写出单属性并去除重复 FD:
    - $F = {A -> B, B -> C, A C -> D, A C -> E, E -> F, A B -> E, A C -> G}$
  - 消去左部冗余属性:
    - $A -> B, B -> C, A C -> D ==> A -> D$, 因此可去除 $A C -> D$ 中的 $C$.
    - $A -> B, B -> C, A C -> E ==> A -> E$, 因此可去除 $A C -> E$ 中的 $C$.
    - $A -> B, B -> C, A C -> G ==> A -> G$, 因此可去除 $A C -> G$ 中的 $C$.
    - $A -> B, A B -> E ==> A -> E$, 因此可去除 $A B -> E$ 中的 $B$.
    - $F = {A -> B, B -> C, A -> D, A -> E, E -> F, A -> E, A -> G} = {A -> B, B -> C,$
      $A -> D, A -> E, E -> F, A -> G}$
  - 没有冗余函数依赖.
+ $R$ 的候选码为 $A$.
  - $A -> B, B -> C, A -> D, A -> E, E -> F, A -> G in F^+ ==> A -> A B C D E F G in F^+$,
    并且不存在 $A$ 的真子集 $Y$ 使得 $Y -> U$ 成立.
+ 属于 1NF, 2NF, 因为所有非主属性都完全依赖于 $A$.\
  不属于 3NF, 因为存在 $A -> B, B -> C$ 使得 $C$ 传递依赖于主码 $A$.
+ #set enum(numbering: "1.")
  + 最小 FD 集合 ${A -> B, B -> C, A -> D, A -> E, E -> F, A -> G}$, 没有不在 $F$ 中出现的属性;
  + 对 $F$ 按相同的左部分组: $q = {R_1(A, B, D, E, G), R_2(B, C), R_3(E, F)}$
  + $A$ 是 $R$ 中的主码, $p = q union {R(A)} = {R_1(A, B, D, E, G), R_2(B, C), R_3(E, F)}$

#question()

+ 最小函数依赖集为 $F = {A -> B, A -> E, B -> C, C -> D}$, 候选码为 $A F G$.
  因为 $B, C, D, E$ 均局部函数依赖于 $A F G$ (比如 $A -> B$), 不满足 2NF,
  所以 $R$ 仅仅满足 1NF.
+ #set enum(numbering: "1.")
  + $p = {R}$;
  + $R$ 中的 $C -> D$ 不满足 BCNF 定义, 分解 $R$,\
    $p = {R_1(C, D), R_2(A, B, C, E, F, G)}$.
  + $R_2$ 中的 $B -> C$ 不满足 BCNF 定义, 分解 $R_2$,\
    $p = {R_1(C, D), R_3(B, C), R_4(A, B, E, F, G)}$.
  + $R_4$ 中的 $A -> B$ 不满足 BCNF 定义, 分解 $R_4$,\
    $p = {R_1(C, D), R_3(B, C), R_5(A, B), R_6(A, E, F, G)}$.
  + $R_6$ 中的 $A -> E$ 不满足 BCNF 定义, 分解 $R_6$,\
    $p = {R_1(C, D), R_3(B, C), R_5(A, B), R_7(A, E), R_8(A, F, G)}$.
  + $p$ 中各关系模式都属于 BCNF, 结束.

