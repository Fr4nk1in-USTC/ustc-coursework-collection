#import "homework.typ": *

#show: homework.with(number: 7)

#set enum(numbering: "a.")

#question("14.12")

+ #set enum(numbering: "(i)")
  + 是错误的, 因为它表明在给定 $M_1$ 和 $M_2$ 的情况下, $N$ 的分布与 $F_1$ 和
    $F_2$ 无关, 但是 $N$ 的分布显然与 $F_1$ 和 $F_2$ 有关.
  + 是正确的.
  + 是正确的.
+ (ii) 更好, 因为它的网络结构更简单.
+ $bold(P)(M_1 | N)$ 可以表示为:
  $
    bold(P)(M_1 | N) & = bold(P)(M_1 | N, F_1)bold(P)(F_1 | N)
                      + bold(P)(M_1 | N, not F_1)bold(P)(not F_1 | N) \
                    & = bold(P)(M_1 | N, F_1)bold(P)(F_1)
                      + bold(P)(M_1 | N, not F_1)bold(P)(not F_1)
  $
  因为这里 $N in {1, 2, 3}$, 所以若望远镜出现对焦不准确的情况,
  科学家会一颗恒星都观测不到, 发生的概率为 $f$. 同时还有 $e$ 的概率出现 1
  颗恒星的误差 (可能多观测也可能少观测), 因此, $bold(P)(M_1 | N)$
  的条件概率表为:
  #align(center, table(
    columns: (auto, auto, auto, auto, auto, auto),
    align: center + horizon,
    $ N $, $P(M_1 = 0)$, $P(M_1 = 1)$,  $P(M_1 = 2)$,  $P(M_1 = 3)$,  $P(M_1 = 4)$, 
    $1$,   $e(1-f) + f$, $(1-2e)(1-f)$, $e(1 - f)$,    $0$,           $0$,
    $2$,   $f$,          $e(1-f)$,      $(1-2e)(1-f)$, $e(1-f)$,      $0$,
    $3$,   $f$,          $0$,           $e(1-f)$,      $(1-2e)(1-f)$, $e(1-f)$,
  ))
+ 恒星数 $N$ 可能满足: $N = 2$; $N = 4$; $N >= 6$.
+ 考虑 $N$ 的三种可能情况, 计算 $bold(P)(M_1 = 1, M_2 = 3 | N)$ 如下:
  $
    p_2     & = bold(P)(M_1 = 1, M_2 = 3 | N = 2)  & = e^2(1 - f)^2 \
    p_4     & = bold(P)(M_1 = 1, M_2 = 3 | N = 4)  & <= e f(1 - f)  \
    p_(<=6) & = bold(P)(M_1 = 1, M_2 = 3 | N >= 6) & <= f^2
  $
  因为 $f < e$, 所以 $p_2 > p_4 > p_(>=6)$, $N = 2$ 是最可能的恒星数目.

#question("14.13")

设归一化常数为 $alpha$, 则:
$
  bold(P)(N | M_1 = 2, M_2 = 2)
  & = alpha sum_(F_1, F_2) bold(P)(F_1, F_2, N, M_1, M_2) \
  & = alpha sum_(F_1, F_2) bold(P)(F_1) bold(P)(F_2) bold(P)(N)
      bold(P)(M_1 = 2 | F_1, N) bold(P)(M_2 = 2 | F_2, N)
$
在题设条件下, 若望远镜对焦不准确, 则 $M_i$ 不可能为 2, 因此唯一可能的情况为
$F_1 = F_2 = "f"$, 所以
$
  bold(P)(N | M_1 = 2, M_2 = 2)
  = alpha (1 - f)^2 angle.l p_1 e^2, p_2 (1 - 2e)^2, p_3 e^2 angle.r
  = alpha' angle.l p_1 e^2, p_2 (1 - 2e)^2, p_3 e^2 angle.r
$
