#import "homework.typ": *

#show: homework.with(number: 3)

#question(6.5)

同时使用三种方法进行求解, 红色代表赋值:

#let choose = text.with(red)
#table(
  columns: (auto, auto, auto, auto, auto, auto, auto, auto, auto),
  align: (x, y) => if (y == 0 or x == 8) { center } else { left } + horizon,
  $F$,         $T$,          $U$,          $W$,          $R$,          $O$,          $C_1$, $C_2$, $C_3$,
  `123456789`, `0123456789`, `0123456789`, `0123456789`, `0123456789`, `0123456789`, `01`,  `01`,  `1`,
  `1        `, `     56789`, `0123456789`, `0123456789`, `0123456789`, `0123456789`, `01`,  `01`,  choose(`1`),
  choose(`1`), `     56789`, `0 23456789`, `0 23456789`, `0 23456789`, `0 23456789`, `01`,  `01`,  `1`,
  `1        `, `     56789`, `0 2 4 6 8 `, `0 23456789`, `0   4 6 8 `, `0 234     `, choose(`0`),  `01`,  `1`,
  `1        `, `     567  `, `0   4 6 8 `, `0 234     `, `0   4 6 8 `, `0 2 4     `, `0 `,  choose(`0`),  `1`,
  `1        `, `       7  `, `0     6 8 `, `0 23      `, `        8 `, choose(`    4     `), `0 `,  `0 `,  `1`,
  `1        `, `       7  `, `0     6   `, `0 23      `, choose(`        8 `), `    4     `, `0 `,  `0 `,  `1`,
  `1        `, choose(`       7  `), `0     6   `, `0 23      `, `        8 `, `    4     `, `0 `,  `0 `,  `1`,
  `1        `, `       7  `, choose(`      6   `), `   3      `, `        8 `, `    4     `, `0 `,  `0 `,  `1`,
  `1        `, `       7  `, `      6   `, choose(`   3      `), `        8 `, `    4     `, `0 `,  `0 `,  `1`,
  `1        `, `       7  `, `      6   `, `   3      `, `        8 `, `    4     `, `0 `,  `0 `,  `1`,
)

得到的解为: $T W O = 734$, $F O U R = 1468$ ($734 + 734 = 1468$)

#question(6.11)

AC-3 算法执行过程如下, 灰色表示弹出的弧 (如果弹出的弧没有改变值域, 则继续弹出,
直至值域改变, 即可能有多个灰色的弧, 但只有最后一个改变了值域),
蓝色表示新插入的弧:

#let pop = text.with(gray)
#let push = text.with(blue)
#show table: par.with(justify: false)

#table(
  columns: (auto, auto, auto, auto, auto, auto, auto, auto),
  align: center + horizon,
  [*队列*], [WA], [NT], [Q], [SA], [NSW], [V], [T],
  [(SA, WA), (SA, NT), (SA, Q), (SA, NSW), (SA, V), (WA, NT), (NT, Q), (Q, NSW),
   (NSW, V)],
  `G`, `RGB`, `RGB`, `RGB`, `RGB`, `R`, `RGB`,
  [#pop[(SA, WA)], (SA, NT), (SA, Q), (SA, NSW), (SA, V), (WA, NT), (NT, Q),
   (Q, NSW), (NSW, V), #push[(SA, NT), (SA, Q), (SA, NSW), (SA, V)]],
  `G`, `RGB`, `RGB`, `R B`, `RGB`, `R`, `RGB`,
  [#pop[(SA, NT), (SA, Q), (SA, NSW), (SA, V)], (WA, NT), (NT, Q),
   (Q, NSW), (NSW, V), (SA, NT), (SA, Q), (SA, NSW), (SA, V),
   #push[(SA, WA), (SA, NT), (SA, Q), (SA, NSW)]],
  `G`, `RGB`, `RGB`, `  B`, `RGB`, `R`, `RGB`,
  [#pop[(WA, NT)], (NT, Q), (Q, NSW), (NSW, V), (SA, NT), (SA, Q), (SA, NSW),
   (SA, V), (SA, WA), (SA, NT), (SA, Q), (SA, NSW),
   #push[(WA, SA), (WA, NT)]],
  `G`, `R B`, `RGB`, `  B`, `RGB`, `R`, `RGB`,
  [#pop[(NT, Q), (Q, NSW), (NSW, V)], (SA, NT), (SA, Q), (SA, NSW),
   (SA, V), (SA, WA), (SA, NT), (SA, Q), (SA, NSW), (WA, SA), (WA, NT),
   #push[(NSW, Q), (NSW, SA)]],
  `G`, `R B`, `RGB`, `  B`, ` GB`, `R`, `RGB`,
  [#pop[(SA, NT)], (SA, Q), (SA, NSW), (SA, V), (SA, WA), (SA, NT), (SA, Q),
   (SA, NSW), (WA, SA), (WA, NT), (NSW, Q), (NSW, SA),
   #push[(NT, WA), (NT, Q)]],
  `G`, [`R  `$$], `RGB`, `  B`, ` GB`, `R`, `RGB`,
  [#pop[(SA, Q)], (SA, NSW), (SA, V), (SA, WA), (SA, NT), (SA, Q), (SA, NSW),
   (WA, SA), (WA, NT), (NSW, Q), (NSW, SA), (NT, WA), (NT, Q),
   #push[(Q, NT), (Q, NSW)]],
  `G`, [`R  `$$], [`RG `$$], `  B`, ` GB`, `R`, `RGB`,
  [#pop[(SA, NSW)], (SA, V), (SA, WA), (SA, NT), (SA, Q), (SA, NSW), (WA, SA),
   (WA, NT), (NSW, Q), (NSW, SA), (NT, WA), (NT, Q), (Q, NT), (Q, NSW),
   #push[(NSW, Q), (NSW, V)]],
  `G`, [`R  `$$], [`RG `$$], `  B`, `G`, `R`, `RGB`,
  [#pop[(SA, V), (SA, WA), (SA, NT), (SA, Q), (SA, NSW), (WA, SA), (WA, NT),
   (NSW, Q), (NSW, SA), (NT, WA), (NT, Q), (Q, NT)], (Q, NSW), (NSW, Q),
   (NSW, V), #push[(Q, SA), (Q, NSW)]],
  `G`, [`R  `$$], `G`, `  B`, `G`, `R`, `RGB`,
  [#pop[(Q, NSW)], (NSW, Q), (NSW, V), (Q, SA), (Q, V),
   #push[(Q, SA), (Q, NT)]],
  `G`, [`R  `$$], ``, `  B`, `G`, `R`, `RGB`,
)

注意到 Q 的值域变为空, AC-3 算法返回 `false`, 说明部分赋值
${"WA"="green", "V "="red"}$ 的不相容.

#question(6.12)

在树结构的 CSP 中, 当一条弧被 AC-3 算法处理后, 它将不会再被重新插入到队列中,
而检验一条弧的相容性可以在 $O(d^2)$ 内完成, 因此 AC-3 算法求解树结构 CSP
在最坏情况下的复杂度为 $O(c d^2)$, 其中 $c$ 为弧的个数, $d$
为变量值域元素个数的最大值.
