#import "homework.typ": *

#show: homework.with(number: 6)

#set enum(numbering: "a.")

#question("8.24")

词汇表如下:
- $"Student"(s)$: $s$ 是学生.
- $"Term"(t)$: $s$ 是学期号.
- $"Take"(s, t, c)$: 学生 $s$ 在 $t$ 学期上了课程 $c$.
- $"Pass"(s, t, c)$: 学生 $s$ 在 $t$ 学期通过了课程 $c$.
- $"Score"(s, t, c)$: 学生 $s$ 在 $t$ 学期课程 $c$ 上的成绩.
- $F$ 和 $G$: 法语课和希腊语课.
- $"Person"(p)$: $p$ 是人.
- $"Insurance"(i)$: $i$ 是保险.
- $"Agent"(a)$: $a$ 是代理.
- $"Expensive"(i)$: $i$ 是昂贵的.
- $"Insured(p)"$: $p$ 有被投保.
- $"Smart"(p)$: $p$ 是聪明的.
- $"Buy"(c, s, i)$: $p$ 从 $s$ 处购买了 $i$.
- $"Sell"(s, c, i)$: $s$ 把 $i$ 卖给了 $c$.
- $"Barber"(b)$: $b$ 是理发师.
- $"Man"(m)$: $m$ 是男人.
- $"InTown"(p)$: $p$ 在镇上.
- $"Shaves"(b, p)$: $b$ 给 $p$ 刮胡子.
- $"Born"(p, c)$: $p$ 出生在 $c$ 国.
- $"Citizen"(p, c, r)$: $p$ 因 $r$ 而成为 $c$ 国公民.
- $"Resident"(p, c)$: $p$ 是 $c$ 国永久居住者.
- $"Parent"(p, c)$: $p$ 是 $c$ 的父母.
- $U K$: 英国.
- $B$: 出生
- $D$: 血统.
- $"Politician"(p)$: $p$ 是政治家.
- $"Time"(t)$: $t$ 是时间.
- $"Fool"(p, r, t)$: $p$ 在 $t$ 时刻愚弄 $r$ .

+ $exists s "Student"(s) and "Take"(s, "2001春", F)$
+ $forall s, t "Student"(s) and "Term"(t) and "Take"(s, t, F) => "Pass"(s, t, F)$
+ $exists s "Student"(s) and "Take"(s, "2001春", G) and (forall s' space
  s' != s => not "Take"(s', "2001春", G))$
+ $forall t "Term"(t) => (exists s_G "Student"(s_G) and "Take"(s_G, t, G) and $
  $quad (forall s_F "Student"(s_F) and "Take"(s_F, t, F) => ("Score"(s_G, t, G) > "Score"(s_F, t, F))))$
+ $forall p "Person"(p) and
  (exists s, i "Person"(s) and "Insurance"(i) and "Buy"(p, s, i)) => "Smart"(p)$
+ $forall c, s, i "Person"(c) and "Person"(s) and "Insurance"(i) and
  "Expensive"(i) => not "Buy"(c, s, i)$
+ $exists a "Agent"(a) and (forall p, i "Person"(p) and "Insurance"(i) and
  "Sell"(a, p, i) => not "Insured"(p))$
+ $exists b "Barber"(b) and "InTown"(b) and
  (forall m "Man"(m) and not "Shave"(m, m) => "Shave"(b, m))$
+ $forall p "Person"(p) and "Born"(p, U K) and$
  $(forall p' "Parent"(p', p) =>
    ((exists r "Citizen"(p', U K, r)) or "Resident"(p', U K)))$\
  $=> "Citizen"(p, U K, B)$
+ $forall p "Person"(p) and not "Born"(p, U K) and
  (exists p' "Parent"(p', p) and "Citizen"(p', U K, B))$
  $=> "Citizen"(p, U K, B)$
+ $forall p "Politician"(p) =>$\
  $quad (exists p' "Person"(p') and (forall t "Time"(t) => "Fool"(p, p', t))) and$\
  $quad (exists t "Time"(t) and (forall p' "Person"(p') => "Fool"(p, p', t))) and$\
  $quad not (forall t "Time"(t) => (forall p' "Person"(p') => "Fool"(p, p', t)))$

#question("8.17")

#set enum(numbering: "1.")

存在以下的问题:
+ 它能够证明 $"Adjacent"([1, 1], [1, 2])$ 但不能证明
  $"Adjacent"([1, 2], [1, 1])$.
+ 它不能证明 $not "Adjacent"([1, 1], [1, 3])$.
+ 它不适用于边界.

#set enum(numbering: "a.")

#question("9.3")

b. 和 c. 都是合法的. a. 不合法, 因为其给 $x$ 赋予了一个已经存在的变量名 Everest.

#question("9.4")

+ ${x \/ A, y \/ B, z \/ B}$
+ 不存在.
+ ${y \/ "John", x \/ "John"}$
+ 不存在.

#question("9.6")

+ $"Horse"(x) => "Mammal"(x)$\
  $"Cow"(x) => "Mammal"(x)$\
  $"Pig"(x) => "Mammal"(x)$
+ $"Offspring"(x, y) and "Horse"(y) => "Horse"(x)$
+ $"Horse"("Bluebeard")$
+ $"Parent"("Bluebeard", "Charlie")$
+ $"Offspring"(x, y) => "Parent"(y, x)$\
  $"Parent"(x, y) => "Offspring"(y, x)$
+ $"Mammal"(x) => "Parent"(F(x), x)$ ($F(dot.c)$ 是 Skolem 函数)

#question("9.13")

+ 如下@proof-tree, 其中 $"Offspring"("Bluebeard", y)$ 和 $"Parent"(y, "Bluebeard")$
  之间的死循环导致证明的其余部分不可达.
+ 证明树中出现了死循环. $"Offspring"(x, y) => "Parent"(y, x)$ 和
  $"Parent"(x, y) => "Offspring"(y, x)$ 这两个语句导致从
  $"Offspring"(x, y) and "Horse"(y) => "Horse"(x)$ 反向推理
  $"Horse"("Bluebeard")$ 之后出现无限循环.
+ 一个, ${h \/ "Charlie"}$.
#figure(
  image("images/9-13.png"),
  caption: [反向链接算法为 $exists h "Horse"(h)$ 生成的证明树],
) <proof-tree>
