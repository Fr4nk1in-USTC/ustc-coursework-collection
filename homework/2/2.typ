#import "homework.typ": *

#show: homework.with(number: 2)

#question(4.1)

如下, 结点表示为 "Name$(f^g_h)$", 蓝色的结点为拓展的结点.

#show emph: it => {
  text(blue, it.body)
}
+ _Lugoj$(244^0_244)$_
+ _Mehadia$(311^70_241)$_, Timisoara$(440^111_329)$
+ _Lugoj$(384^140_244)$_, Drobeta$(387^145_242)$, Timisoara$(440^111_329)$
+ _Drobeta$(387^145_242)$_, Timisoara$(440^111_329)$, Mehadia$(451^210_241)$,
  Timisoara$(580^251_329)$
+ _Craiova$(425^265_160)$_, Timisoara$(440^111_329)$, Mehadia$(451^210_241)$,
  Mehadia$(461^220_241)$, Timisoara$(580^251_329)$
+ _Timisoara$(440^111_329)$_, Mehadia$(451^210_241)$, Mehadia$(461^220_241)$,
  Pitesti$(503^403_100)$, Timisoara$(580^251_329)$,
  Rimnicu Vilcea$(604^411_193)$, Drobeta$(627^385_242)$
+ _Mehadia$(451^210_241)$_, Mehadia$(461^220_241)$, Lugoj$(466^222_244)$,
  Pitesti$(503^403_100)$, Timisoara$(580^251_329)$, Arad$(595^229_366)$,
  Rimnicu Vilcea$(604^411_193)$, Drobeta$(627^385_242)$
+ _Mehadia$(461^220_241)$_, Lugoj$(466^222_244)$, Pitesti$(503^403_100)$,
  Lugoj$(524^280_244)$, Drobeta$(527^285_242)$, Timisoara$(580^251_329)$,
  Arad$(595^229_366)$, Rimnicu Vilcea$(604^411_193)$, Drobeta$(627^385_242)$
+ _Lugoj$(466^222_244)$_, Pitesti$(503^403_100)$, Lugoj$(524^280_244)$,
  Drobeta$(527^285_242)$, Lugoj$(534^290_244)$, Drobeta$(537^295_242)$,
  Timisoara$(580^251_329)$, Arad$(595^229_366)$, Rimnicu Vilcea$(604^411_193)$,
  Drobeta$(627^385_242)$
+ _Pitesti$(503^403_100)$_, Lugoj$(524^280_244)$, Drobeta$(527^285_242)$,
  Mehadia$(533^292_241)$, Lugoj$(534^290_244)$, Drobeta$(537^295_242)$,
  Timisoara$(580^251_329)$, Arad$(595^229_366)$, Rimnicu Vilcea$(604^411_193)$,
  Drobeta$(627^385_242)$, Timisoara$(662^333_329)$
+ _Bucharest$(504^504_0)$_, Lugoj$(524^280_244)$, Drobeta$(527^285_242)$,
  Mehadia$(533^292_241)$, Lugoj$(534^290_244)$, Drobeta$(537^295_242)$,
  Timisoara$(580^251_329)$, Arad$(595^229_366)$, Rimnicu Vilcea$(604^411_193)$,
  Drobeta$(627^385_242)$, Timisoara$(662^333_329)$,
  Rimnicu Vilcea$(693^500_193)$, Craiova$(701^541_160)$

最后得到的解为: Lugoj $->$ Mehadia $->$ Drobeta $->$ Craiova $->$ Pitesti $->$
Bucharest, 总代价 504.

#question(4.2)

算法中 $w$ 取 $0 <= w <= 1$ 时能保证其最优: 当 $w = 0$ 时, 算法对应一致代价搜索,
它是最优的; 当 $0 < w <= 1$ 时, $f(n) = (2 - w) [g(n) + w/(2 - w) h(n)]$, 相当于
$h'(n) = w/(2 - w) h(n) <= h(n)$ 的 A\* 搜索, 因为 $h$ 是可采纳的, 所以
$h' <= h$ 也是可采纳的, 算法最优.

/ $w = 0$: $f(n) = 2g(n)$, 这个算法是一致代价搜索;
/ $w = 1$: $f(n) = g(n) + h(n)$, 这个算法是 A\* 搜索;
/ $w = 2$: $f(n) = 2h(n)$, 这个算法是贪婪最佳优先搜索.

#question(4.6)


#grid(
  columns: (auto, auto),
  gutter: 10pt,
  [
    使用 $h_1$ (不在位的棋子数) 与 $h_2$ (所有棋子到其目标位置的曼哈顿距离和)
    的和 $h_3 = h_1 + h_2$ 作为启发函数. 它在八数码游戏中有时会估计过高,
    比如对右边上图的状态, 其值为 $h_3 = 29$, 大于它的最优解路径为 25 步.
    并且对于右边下图, 其最优解路径为 25 步, 但是使用 $h_3$ 作为启发函数的 A\*
    算法给出的解为 27 步, 非最优解. 

    下面证明题中命题: 设 A\* 算法使用的启发函数 $h$ 满足 $h(n) <= h^*(n) + c$,
    其中 $h^*(n)$ 是 $n$ 的最优解路径的代价. 设存在一个非最优解 $G$ 满足
    $g(G) > C^* + c$. 考虑路径上的任何一个结点 $n$, 都有
  ],
  [
    #table(
      columns: (auto, auto, auto),
      [4], [7], [3],
      [5], [8], [6],
      [2], [], [1]
    )
    #table(
      columns: (auto, auto, auto),
      [2], [7], [8],
      [6], [5], [4],
      [1], [ ], [3]
    )
  ]
)
$
  f(n) & = g(n) + h(n) \
       & <= g(n) + h^*(n) + c \
       & <= C^* + c \
       & < g(G) \
$

因此 $G$ 不会在找到解之前被扩展到, 即不会成为算法的解.

#question(4.7)

使用数学归纳法, 设 $k$ 是当前结点 $n$ 到最优解结点 $n_g$ 的所需的步数.

+ 当 $k = 1$ 时, 显然有 $h(n) <= c(n, a, n_g) = h^*(n)$, $h(n)$ 是可采纳的.
+ 设当 $k = i$ 时有 $h(n') <= h^*(n')$, 则当 $k = i + 1$ 时, #v(-0.5em)
  $
  h(n) <= c(n, a, n') + h(n') = c(n, a, n') + h^*(n') <= h^*(n)
  $
  #v(-0.5em)
  $h(n)$ 是可采纳的.

因此一致的启发式都是可采纳的.

#grid(
  columns: (1fr, auto),
  [
    对于右图中的问题, 可以给出一个启发函数如下:
    $
      h(upright(A)) = 4 \
      h(upright(B)) = 2 \
      h(upright(G)) = 0
    $
    这个启发函数是可采纳的, 但是它不是一致的, 因为 
    $
      h(A) = 4 > c(A, a, B) + h(B) = 3
    $
  ],
  image("images/example.svg", width: 100pt)
)
