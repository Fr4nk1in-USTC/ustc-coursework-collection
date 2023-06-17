#import "homework.typ": *

#show: homework.with(number: 8)

#set enum(numbering: "(1)")

#question(1)

以如下方式构造决策树. 每一层都只对同一个属性进行划分,
子结点对应该属性所有可能的取值, 依次遍历所有的属性,
这样得到的树的每个叶结点都与一个属性取值一一对应.
去掉训练集中不存在的分支后, 由于训练集中没有冲突数据,
所以每个叶结点都只对应一个类别, 该类别就是训练集中对应样本的类别.
这样就得到了与训练集一致的决策树.

#question(2)

+ 因为数据中不同的特征存在的误差程度是不同的,
  所以规范化项应该对不同的特征有不同的权重, 因此选择
  $bold(w)^(upright(T)) bold(D) bold(w)$. $bold(D)$
  的对角元素体现了对应特征的数据中存在的误差程度, $bold(D)_(i i)$ 越大,
  说明特征 $i$ 的误差越大.
+ 令 $ell = (bold(X)bold(w) - bold(y))^(upright(T)) (bold(X)bold(w) - bold(y))
  + lambda bold(w)^(upright(T)) bold(D) bold(w)$, 则
  $
    (diff ell) / (diff bold(w))
    = 2 bold(X)^(upright(T))(bold(X)bold(w) - bold(y))
      + 2 lambda bold(D) bold(w)
    = 0
    => (bold(X)^(upright(T)) bold(X) + lambda bold(D)) bold(w)
        = bold(X)^(upright(T)) bold(y)
  $
  得到闭式解
  $
    bold(w)^* = (bold(X)^(upright(T)) bold(X) + lambda bold(D))^(-1)
    bold(X)^(upright(T)) bold(y)
  $

#question(3)

+ $forall (i, j), K_(i, j) = K(bold(x)_i, bold(x)_j) = phi(bold(x)_i) dot.c
    phi(bold(x)_j) = phi(bold(x)_j) dot.c phi(bold(x)_i) =
    K(bold(x)_j, bold(x)_i) = K_(j, i)$, 因此 $K$ 是对称矩阵.
+ 记 $Phi = (phi(bold(x)_1), dots.c, phi(bold(x)_n))^(upright(T))$, 则
  $K = Phi dot.c Phi^(upright(T))$, 因此, $forall bold(z) in RR^n$, 有
  $
    bold(z)^(upright(T)) K bold(z) = bold(z)^(upright(T)) Phi dot.c
    Phi^(upright(T)) bold(z) = (bold(z)^(upright(T)) Phi)^2 >= 0
  $
  所以 $K$ 是半正定矩阵.

#question(4)

K-means 算法一定会收敛. 考虑 K-means 算法的目标函数 $ell = sum_(i = 1)^k
sum_(bold(x) in C_i) abs(bold(x) - bold(mu)_i)^2$ 和算法的主要两步:
+ 第一步中, 固定了 $bold(mu)$ 而优化 $C$, 使得 $ell$ 减小;
+ 第二步中, 固定了 $C$ 而优化 $bold(mu)$, 使得 $ell$ 减小.
即执行这两步后, $ell$ 一定减小, 且 $ell$ 有下界 $0$. 在 K-means 算法执行过程中,
目标函数值单调递减且有下界, $ell$ 一定会收敛, 即 K-means 算法一定会收敛.
