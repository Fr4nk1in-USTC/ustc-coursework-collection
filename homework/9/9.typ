#import "homework.typ": *

#show: homework.with(number: 9)

#set enum(numbering: "(1)")

#question(1)

根据所给数据, 对偶问题为:
$
  min_(bold(alpha))
  &   & 1/2 sum_(i = 1)^N sum_(j = 1)^N alpha_i alpha_j y_i y_j
        (bold(x)_i dot.c bold(x)_j) - sum_(i = 1)^N alpha_i \
  & = & 5/2 alpha_1^2 + 13/2 alpha_2^2 + 9 alpha_3^2 + 5/2 alpha_4^2
        + 13/2 alpha_5^2 + 8 alpha_1 alpha_2 + 9 alpha_1 alpha_3
        - 4 alpha_1 alpha_4 - 7 alpha_1 alpha_5 + 15 alpha_2 alpha_3 \
  &   & - 7 alpha_2 alpha_4 - 12 alpha_2 alpha_5 - 9 alpha_3 alpha_4
        - 15 alpha_3 alpha_5 + 8 alpha_4 alpha_5 
        - (alpha_1 + alpha_2 + alpha_3 + alpha_4 + alpha_5) \
  "s.t."
  &   & alpha_1 + alpha_2 + alpha_3 - alpha_4 - alpha_5 = 0 \
  &   & alpha_i >= 0, i = 1, 2, 3, 4, 5
$
将 $alpha_1 + alpha_2 + alpha_3 - alpha_4 = alpha_5$ 代入目标函数, 得到:
$
  ell(bold(alpha))
  & = 2 alpha_1^2 + alpha_2^2 + 1/2 alpha_3^2 + alpha_4^2 + 2 alpha_1 alpha_2
    - 2 alpha_1 alpha_4 + alpha_2 alpha_3 + alpha_3 alpha_4
    - 2(alpha_1 + alpha_2 + alpha_3) \
  & = 1/2 bold(alpha)^(upright(T))
      mat(
        4,  2, 0, -2;
        2,  2, 1, 0;
        0,  1, 1, 1;
        -2, 0, 1, 2
      ) bold(alpha) - 2 mat(1, 1, 1, 0) bold(alpha)
$
对 $bold(alpha)$ 求导, 得到:
$
  (diff ell)/(diff bold(alpha))
  = mat(
      4,  2, 0, -2;
      2,  2, 1, 0;
      0,  1, 1, 1;
      -2, 0, 1, 2
    ) bold(alpha) - 
    mat(
      2;
      2;
      2;
      0
    )
  = mat(
    4 alpha_1 + 2 alpha_2 - 2 alpha_4 - 2;
    2 alpha_1 + 2 alpha_2 + alpha_3 - 2;
    alpha_2 + alpha_3 + alpha_4 - 2;
    -2 alpha_1 + alpha_3 + 2 alpha_4
  )
$
令导数为 0, 方程无解, 故只能在边界上取到最小值:
- 当 $alpha_1 = 0$ 时, 最小值 $ell(0, 0, 2, 0) = -2$;
- 当 $alpha_1 != 0, alpha_2 = 0$ 时, 最小值 $ell(1/2, 0, 2, 0) = - 5/2$;
- 当 $alpha_1 != 0, alpha_2 != 0, alpha_3 = 0$ 时, 最小值 $ell(1/2, 1/2, 0, 1/2) = -1$;
- 当 $alpha_1 != 0, alpha_2 != 0, alpha_3 != 0, alpha_4 = 0$ 时, 无最小值.

因此, $ell(bold(alpha))$ 在 $bold(alpha)^* = (1/2, 0, 2, 0)^(upright(T))$
时达到最小, 此时 $alpha_5 = alpha_1 + alpha_2 + alpha_3 - alpha_4 = 5/2$,
说明实例点 $bold(x)_1$, $bold(x)_3$, $bold(x)_5$ 为支持向量. 求得最优化问题的解
$bold(w)^*$ 和 $b^*$ 为
$
  bold(w)^* & = 1/2 bold(x)_1 + 2 bold(x)_3 - 5/2 bold(x)_5 = mat(-1; 2) \
  b^*       & = 1 - 1/2 times 5 - 2 times 9 + 5/2 times 7 = -2
$
因此, SVM 的最大间隔分离超平面为
$
  - x^((1)) + 2 x^((2)) - 2 = 0 <=> x^((2)) = 1/2 x^((1)) + 1
$
分类决策函数为
$
  f(bold(x)) = upright(s g n)(- x^((1)) + 2 x^((2)) - 2)
$

由上面的分析, 作出图像如下, 其中
- 圆圈表示正例点, 叉表示负例点;
- 实线表示分离超平面, 虚线表示其间隔边界;
- 支持向量用红色圆圈圈出:

#align(center, image("./tikz/svm/svm.svg", width: 50%))

#question(2)

首先, 有
$
  diff/(diff z) sigma(z)       & = diff/(diff z) 1/(1 + upright(e)^(-z))
                                 = sigma(z) (1 - sigma(z)) \
  diff/(diff z) (1 - sigma(z)) & = diff/(diff z) (upright(e)^(-z))/(1 + upright(e)^(-z))
                                 = upright(e)^(-z) diff/(diff z) sigma(z)
                                 + sigma(z) diff/(diff z) upright(e)^(-z)
                                 = - upright(e)^(-z) sigma^2(z)
                                 = - sigma(z) (1 - sigma(z))
$
记 $g = bold(w) bold(x) + b, f(g) = L_"CE"(bold(w), b)
= - [y log(sigma(g)) + (1 - y) log(1 - sigma(g))]$, 则有
$
  (diff g) / (diff bold(w)) & = bold(x)^(upright(T)) <==> (diff g)/(diff w_j) = x_j \
  diff / (diff g) f(g)
  & = - [y 1/(sigma(g)) diff/(diff g) sigma(g) + (1 - y) 1/(1 - sigma(g)) diff/(diff g) (1 - sigma(g))] \
  & = - [y (1 - sigma(g)) - (1 - y) sigma(g)] \
  & = - y + cancel(y sigma(g)) + sigma(g) - cancel(y sigma(g)) \
  & = sigma(g) - y = sigma(bold(w) bold(x) + b) - y
$
由链式法则, 有
$
  (diff L_"CE")/(diff w_j) = (diff L_"CE")/(diff g) dot.c (diff g)/(diff w_j)
  = (sigma(bold(w) bold(x) + b) - y) x_j
$

