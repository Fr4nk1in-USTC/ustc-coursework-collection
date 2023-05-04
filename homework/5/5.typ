#import "homework.typ": *

#show: homework.with(number: 4)

#set enum(numbering: "a.")

#question("7.13")

+ 因为 $P => Q ident not P or Q$, 而由 De Morgan 律, 
  $not(P_1 and P_2 and ... and P_m) ident (not P_1 or not P_2 or ... or not P_m)$
  所以
  $ (not P_1 or not P_2 or ... or not P_m or Q)
    ident not(P_1 and P_2 and ... and P_m) or Q
    ident (P_1 and P_2 and ... and P_m) => Q $
+ 一个文字要么为真要么为假, 那些为假的文字可以表示为 $not P_1, not P_2, ... , not P_m$,
  为真的文字可以表示为 $Q_1, Q_2, ... , Q_n$. 那么子句就可以表示为
  $ not P_1 or not P_2 or ... or not P_m or Q_1 or Q_2 or ... or Q_n
    ident (P_1 and P_2 and ... and P_m) => (Q_1 or Q_2 or ... or Q_n) $
+ 对一系列文字 $p_i, q_i, r_i, s_i$, 其中有 $p_j = q_k$, 用 b. 中结论推广全归结规则得到:
  $ (p_1 and ... and p_j and ... and p_n_1 => r_1 or ... or r_n_2,
    s_1 and ... and s_n_3 => q_1 or ... or q_k or ... or q_n_4 ) /
    (p_1 and ... and p_(j - 1) and p_(j + 1) and p_n_1 and s_1 and s_n_3 =>
    r_1 or ... or r_n_2 or q_1 or ... or q_(k - 1) or q_(k + 1) or ... or q_n_4) $

#question("证明前向链接算法的完备性")
#show emph: text.with(font: "New Computer Modern")
假设前向链接算法到达了不动点, 考察 _inferred_ 表的最终状态,
参与推理过程的每个符号都被赋值了 _true_/_false_ 值. 将 _inferred_
表看作一个模型 _m_, 则在原始 _KB_ 中的每个确定子句在该模型中都为真. 
(为了证明这一点, 使用反证法, 假设某个子句 $a_1 and ... and a_n => b$
在此模型下为假, 则 $a_1 and ... and a_n$ 为真, $b$ 为假,
但这与算法已经到达了不动点这一假设相矛盾) 因此 _m_ 是 _KB_ 的一个模型,
如果 $K B |= q$, 则 $q$ 在 _KB_ 的所有模型 (包括 _m_) 中都为真, 即在 _inferred_
表中为真, 也就被前向链接算法推断出来了.
