#import "@preview/tablex:0.0.6": tablex, hlinex, vlinex, colspanx
#import "@preview/cetz:0.1.2": canvas, draw, tree
#import "@preview/diagraph:0.1.2": raw-render

#import "@local/typreset:0.1.0": homework

#show: homework.style.with(
  course: "Data Privacy",
  number: "1",
  names: "傅申",
  ids: "PB20000051",
)

#let question = homework.simple_question

#question[K-anonymity]

#let e(content, count) = {content + sub(str(count))}

#let hierarchies(data, spread: 1) = {
  align(center,
    canvas({
      import draw: *
      set-style(content: (padding: .1))
      tree.tree(data, spread: spread)
    })
  )
}

+ The quasi-identifier attributes are *Zip Code*, *Age*, *Salary* and *Nationality*.
+ The *generalization hierarchies* are shown below. Note that the subscripted number is the item count.
  / Zip Code:
    #hierarchies(
      (e("*****", 12),
        (e("130**", 8), e("13053", 4), e("13068", 4)),
        (e("148**", 4), e("14850", 2), e("14853", 2))
      ), spread: 1.2
    )
  / Age:
    #hierarchies(
      (e("0-100", 12),
        (e("<45", 4), e("21", 1), e("23", 1), e("28", 1), e("29", 1), e("30", 1), e("32", 1), e("36", 1), e("37", 1)),
        (e("≥45", 4), e("47", 1), e("49", 1), e("50", 1), e("55", 1))
      )
    )
  / Salary:
    #hierarchies(
      (e([\*\*$k$], 12),
        (e([1\*$k$], 7), e([13$k$], 1), e([15$k$], 1), e([16$k$], 1), e([17$k$], 1), e([18$k$], 1), e([19$k$], 2)),
        (e([2\*$k$], 5), e([20$k$], 1), e([21$k$], 1), e([22$k$], 1), e([25$k$], 2)),
      )
    )
  / Nationality:
    #hierarchies(
      (e("Any", 12),
        e("Russian", 2),
        e("American", 4),
        e("Japanese", 2),
        e("Indian", 3),
        e("Chinese", 1)
      ), spread: 2
    )
  Based on the generalization hierarchies above, a cell-level generalization solution to achieve 2-anonymity can be designed.
  The *released table* is shown below.
  #let shallow(c) = {color.mix((c, 40%), (white))}
  #align(center)[
    #show emph: it => { text(gray, it.body) }
    #tablex(
      columns: 6,
      align: center + horizon,
      auto-lines: false,
      map-rows: (row, cells) => cells.map(c => {
        if c == none or row < 2 {
          c
        } else {
          (..c,
            fill:
              if row < 7 { shallow(blue) }
              else if row < 10 { shallow(red) }
              else if row < 12 { shallow(green) }
              else { shallow(yellow)}
          )
        }
      }),
      hlinex(),
      vlinex(), vlinex(), vlinex(), vlinex(), vlinex(), vlinex(), vlinex(),
      [],   colspanx(4)[Non-Sensitive], [Sensitive],
      hlinex(start: 1),
      [],   [Zip Code],  [Age], [Salary], [Nationality], [Condition],
      hlinex(),
      [_1_],  [13\*\*\*], "<45", [1\*$k$], [Any], [Heart Disease],
      [_3_],  [13\*\*\*], "<45", [1\*$k$], [Any], [Viral Infection],
      [_4_],  [13\*\*\*], "<45", [1\*$k$], [Any], [Viral Infection],
      [_9_],  [13\*\*\*], "<45", [1\*$k$], [Any], [Cancer],
      [_11_], [13\*\*\*], "<45", [1\*$k$], [Any], [Cancer],
      [_10_], [13\*\*\*], "<45", [2\*$k$], [Any], [Cancer],
      [_12_], [13\*\*\*], "<45", [2\*$k$], [Any], [Cancer],
      [_2_],  [13\*\*\*], "<45", [2\*$k$], [Any], [Heart Disease],
      [_6_],  [14\*\*\*], [≥45], [1\*$k$], [Any], [Heart Disease],
      [_7_],  [14\*\*\*], [≥45], [1\*$k$], [Any], [Viral Infection],
      [_5_],  [14\*\*\*], [≥45], [2\*$k$], [Any], [Cancer],
      [_8_],  [14\*\*\*], [≥45], [2\*$k$], [Any], [Viral Infection],
      hlinex(),
    )
  ]
  To calculate the *loss metric* (LM) of this solution, first calculate the losses of each attribute:
  $
    "LM"_"Zip Code"    & = 1 / 12 ((2 - 1)/ (4 - 1) times 8 + (2 - 1)/(4 - 1) times 4) = 1 / 3 \
    "LM"_"Age"         & = 1 / 12 ((44 - 0 + 1)/(100 - 0 + 1) times 8 + (100 - 45 + 1) / (100 - 0 + 1) times 4) = 146 / 303 \
    "LM"_"Salary"      & = 1 / 12 ((6 - 1)/(10 - 1) times 7 + (4 - 1)/(10 - 1) times 5) = 25 / 54 \
    "LM"_"Nationality" & = 1
  $
  The LM for the entrie data set is defined as the sum of the losses for each attribute, which is:
  $
    "LM" = "LM"_"Zip Code" + "LM"_"Age" + "LM"_"Salary" + "LM"_"Nationality" = 12425 / 5454
  $

#question[L-Diversity]

+ For each $q^*$-block, the sorted sensitive attribute count sequences are all (2, 1, 1), which satisfies $r_1 = 2 < 2(r_2 + r_3) = 4$. Thus, the attributes in the figure meet recursive (2, 2)-diversity.
+ Say there is a $q^*$-block $q^(**)$ merged from $q^*_1$ and $q^*_2$ in table $T$.

  Since $T$ satisfies entropy $ell$-diversity, the entropy of $q^*_1$ and $q^*_2$ is greater than $log(ell)$. Thus,
  $
    "entropy"(q^*_1) = - sum_(s in S) p(q^*_1, s) log(p(q^*_1, s)) >= log(ell) \
    "entropy"(q^*_2) = - sum_(s in S) p(q^*_2, s) log(p(q^*_2, s)) >= log(ell)
  $
  where $display(p(q^*_n, s) = (n(q^*_n, s))/(sum_(s ' in S) n(q^*_n, s')))$.

  Let $display(bold(P)(q^*_n, {s_1, dots.c, s_m}) = mat(p(q^*_n, s_1), dots.c, p(q^*_n, s_m))^"T "), display(f(bold(X)) = - sum_(x in bold(X)) x log(x) )$. Then the entropy of $q^*_n$ equals $f(bold(P)(q^*_n, S))$. Since $f(bold(X))$ is a concave function, which means
  $
    forall alpha in [0, 1], f((1 - alpha)bold(X) + alpha bold(Y)) >= (1 - alpha)f(bold(X)) + alpha f(bold(Y))
  $
  And the distribution of the merged block $q^(**)$ satisfies
  $
    bold(P)(q^(**), S) = n(q^*_1)/(n(q^*_1) + n(q^*_2)) bold(P)(q^*_1, S) + n(q^*_2)/(n(q^*_1) + n(q^*_2)) bold(P)(q^*_2, S)
  $
  Thus,
  $
        & f(bold(P)(q^(**), S)) >= n(q^*_1)/(n(q^*_1) + n(q^*_2)) f(bold(P)(q^*_1, S)) + n(q^*_2)/(n(q^*_1) + n(q^*_2)) f(bold(P)(q^*_2, S)) \
    <=> & "entropy"(q^(**)) >= n(q^*_1)/(n(q^*_1) + n(q^*_2)) "entropy"(q^*_1) + n(q^*_2)/(n(q^*_1) + n(q^*_2)) "entropy"(q^*_2) \
    =>  & "entropy"(q^(**)) >= min("entropy"(q^*_1), "entropy"(q^*_2)) >= log(ell)
  $
  In other words, the merged block $q^(**)$ satisfies entropy $ell$-diversity.

  For any table $T^*$ generalized from $T$, it is always obtained from table $T$ through a finite number of $q^*$-block merges. Thus, the minimal entropy of $T^*$ would never be less than the entropy of $T$, which implies $T^*$ satisfies entropy $ell$-diversity.

#question[T-Clossness]

+ The EMD between $bold(P)$ and $bold(Q)$ is $D[bold(P), bold(Q)] = min_bold(F)"WORK"(bold(P), bold(Q), bold(F)) = sum_(i = 1)^m sum_(j = 1)^m (|i - j|)/(m - 1) f_(i j)$, where flow $bold(F)$ satisfies
  $
    f_(i j) >= 0\
    r_i = p_i - q_i = sum_(j = 1)^m (f_(i j) - f_(j i))\
    sum_(i = 1)^m sum_(j = 1)^m f_(i j) = 1
  $
  For convience, we only need to consider flows that transport distribution between adjacent elements, since any transportation between further elements can be equivalently decomposed into several transportations between adjacent elements. So $"WORK"(dot)$ can be simplified as
  $"WORK"(bold(P), bold(Q), bold(F)) = 1/(m - 1) sum_(i = 1)^m (f_(i, i - 1) + f_(i, i + 1))$, where the flow $bold(F)$ satisfies
  $
    f_(i j) >= 0, f_(0, *) = f_(*, 0) = f_(m + 1, *) = f_(*, m + 1) = 0 \
    r_i = p_i - q_i = f_(i, i - 1) + f_(i, i + 1) - f_(i - 1, i) - f_(i + 1, i)\
    sum_(i = 1)^m sum_(j = 1)^m f_(i j) = 1
  $
  To minimize $"WORK"(dot)$, it is obivious that one of $f_(i j)$ and $f_(j i)$ is zero. Thus, expanding the sum in $"WORK"$ and pairing each $(f_(i j), f_(j i))$ gives
  $
    min_bold(F) "WORK"(bold(P), bold(Q), bold(F))
    & = min_bold(F) 1/(m - 1)(f_(12) + f_(21) + f_(23) + dots.c + f_(m - 1, m) + f_(m, m - 1))\
    & = min_bold(F) 1/(m - 1) sum_(i = 1)^m (f_(i + 1, i) + f_(i, i + 1))\
    & = min_bold(F) 1/(m - 1) sum_(i = 1)^m abs(f_(i + 1, i) - f_(i, i + 1)) \
    & = min_bold(F) 1/(m - 1) sum_(i = 1)^m abs(sum_(j = 1)^(i) (f_(j, j - 1) + f_(j, j + 1) - f_(j - 1, j) - f_(j + 1, j))) \
    & = 1/(m - 1) sum_(i = 1)^m abs(sum_(j = 1)^(i) r_j) = 1/(m - 1) (abs(r_1) + abs(r_1 + r_2) + dots.c + abs(r_1 + dots.c + r_(m - 1)))
  $
  In other words, $D[bold(P), bold(Q)] = 1/(m - 1) (abs(r_1) + abs(r_1 + r_2) + dots.c + abs(r_1 + dots.c + r_(m - 1))) = 1/(m - 1) sum_(i = 1)^m abs(sum_(j = 1)^i r_j)$.

+ The overall distribution of Salary is $display(bold(Q) = 1/9 mat(1, 1, 1, 1, 1, 1, 1, 1, 1))$, each element represents ${3k, 4k, 5k, 6k, 7k, 8k, 9k, 10k, 11k}$ respectively. For each QI group, calculate the EMD as below.
  - In the first QI group (Zip Code 4767\* and etc.), the distribution is $display(bold(P)_1 = 1/3 mat(0, 1, 1, 1, 0, 0, 0, 0, 0))$. So the EMD is
    $display(D[bold(P)_1, bold(Q)] = 1/8 (1/9 + 1/9 + 3/9 + 5/9 + 4/9 + 3/9 + 2/9 + 1/9) = 5/18)$.
  - In the second QI group (Zip Code 4790\* and etc.), the distribution is $display(bold(P)_1 = 1/3 mat(1, 0, 0, 0, 0, 1, 0, 0, 1))$. So the EMD is
    $display(D[bold(P)_1, bold(Q)] = 1/8 (2/9 + 1/9 + 0 + 1/9 + 2/9 + 0 + 1/9 + 2/9) = 1/8)$.
  - In the third QI group (Zip Code 4760\* and etc.), the distribution is $display(bold(P)_1 = 1/3 mat(0, 0, 0, 0, 1, 0, 1, 1, 0))$. So the EMD is
    $display(D[bold(P)_1, bold(Q)] = 1/8 (1/9 + 2/9 + 3/9 + 4/9 + 2/9 + 3/9 + 1/9 + 1/9) = 17/72)$.
  Therefore, the value of $t$ should be greater than $display(5/18)$, i.e., $t >= display(5/18)$.

#question[Prior and Posterior]

#let prob = $bold(upright(P))$

+
  / Prior and posterior probabilities of $x = 0$ given $R_1(x) = 0$: \
    The *prior* probability of $x = 0$ is $prob[x = 0] = 0.01$. Given $R_1(x) = 0$, the *posterior* probability of $x = 0$ is
    $
      prob[x = 0 | R_1(x) = 0]
      & = (prob[R_1(x) = 0 | x = 0] prob[x = 0]) / prob[R_1(x) = 0] \
      & = (prob[R_1(x) = 0 | x = 0] prob[x = 0]) / (sum_(i = 0)^100 prob[R_1(x) = 0 | x = i] prob[x = i]) \
      & = (0.3 times 0.01) / (0.3 times 0.01 + 100 times 0.007 times 0.0099) \
      & = 100 / 331 approx 0.302
    $
  / Prior and posterior probabilities of $x in [20, 80]$ given $R_2(x) = 0$:\
    The *prior* probability of $x in [20, 80]$ is $prob[x in [20, 80]] = 0.0099 times 61 = 0.6039$. Given $R_2(x) = 0$, the *posterior* probability of $x in [20, 80]$ given $R_2(x) = 0$ is
    $
      prob[x in [20, 80] | R_2(x) = 0]
      & = (prob[R_2(x) = 0 | x in [20, 80]] prob[x in [20, 80]]) / prob[R_2(x) = 0] \
      & = 0 / prob[R_2(x) = 0] \
      & = 0
    $
  / Prior and posterior probabilities of $x = 0$ given $R_3(x) = 0$: \
    The *prior* probability of $x = 0$ is $prob[x = 0] = 0.01$. Given $R_3(x) = 0$, the *posterior* probability of $x = 0$ is
    $
      prob[x = 0 | R_3(x) = 0]
      & = (prob[R_3(x) = 0 | x = 0] prob[x = 0]) / prob[R_3(x) = 0] \
      & = (prob[R_3(x) = 0 | x = 0] prob[x = 0]) / (1/2 sum_(i in [0, 10] union [91, 100]) prob[R_2(x) = 0 | x = i]prob[x = i] + 1/202) \
      & = (1/2 times 1/21 times 0.01 + 1/202) / (1/2 times (20 times 0.0099 + 0.01) times 1/21 + 1/202) \
      & = 11005 / 21004 approx 0.524
    $
+ If we want to preserve better privacy, the method with less information loss is more suitable, because it makes less difference between prior and posterior probability. We can compute the distance (e.g., KL-divergence and *Hellinger Distance*) between prior and (each) posterior probability distribution as
  $
    D_i [bold(P)_"prior", bold(P)_"post"] = sqrt(sum_(X in {{0}, [200, 800], [1, 199] union [801, 1000]}) (sqrt(prob[x in X]) - sqrt(prob[x in X | R_i (x) = 0]))^2 \/ 2) \
    D_1 approx 0.62, quad D_2 approx 0.60, quad D_3 approx 0.22
  $
  So $R_3$ is more suitable.

#question[K-Anonymity in Graphs]

+ After adding edge Tom-Lily, the degree sequence of the graph become ${2, 3, 4, 3, 4, 2}$, making the graph 2-anonymous.
  #align(center, raw-render(engine: "neato", width: 60%,
    ```dot
    graph {
        node[shape=point]
        forcelabels=true
        Tom [xlabel=Tom, pos="0, 0!"]
        Bob [xlabel=Bob, pos="0.40, 1.00!"]
        Ada [shape=plaintext, pos="1.00, 0.75!"]
        A [pos="1.00, 0.5!"]
        Lily [shape=plaintext, pos="2.00, -0.25!"]
        L [pos="2.00, 0!"]
        Lucy [xlabel=Lucy, pos="2.20, 1.00!"]
        Jim [shape=plaintext, pos="3.20, 0.40!"]
        J [pos="2.82, 0.40!"]
        Tom -- Ada:s
        Tom -- Bob
        Bob -- Ada:s
        Ada:s -- Lucy
        Ada:s -- Lily:n
        Lucy -- Lily:n
        Lucy -- Jim:w
        Lily:n -- Jim:w
        Tom -- Lily:n [style=dashed]
      }
      ```
    )
  )
+ After adding edge Lucy-Lily, the the degree sequence of the graph become ${ 2, 2, 4, 4, 4, 2}$, making the graph 3-anonymous.
  #align(center, raw-render(engine: "neato", width: 60%,
      ```dot
      graph {
        node[shape=point]
        forcelabels=true
        Tom [xlabel=Tom, pos="0, 0!"]
        Bob [xlabel=Bob, pos="0.40, 1.00!"]
        Ada [shape=plaintext, pos="1.00, 0.75!"]
        A [pos="1.00, 0.5!"]
        Lily [shape=plaintext, pos="2.00, -0.25!"]
        L [pos="2.00, 0!"]
        Lucy [xlabel=Lucy, pos="2.20, 1.00!"]
        Jim [shape=plaintext, pos="3.20, 0.40!"]
        J [pos="2.82, 0.40!"]
        Tom -- Ada:s
        Tom -- Bob
        Bob -- Ada:s
        Ada:s -- Lucy
        Ada:s -- Lily:n
        Lucy -- Lily:n
        Lucy -- Jim:w
        Lily:n -- Jim:w
        Lucy -- Lily:n [style=dashed]
      }
      ```
    )
  )
+ For the anonymized graph in section (a), the information loss is
  $
    L(G, G'_a) = 1 - 8/9 = 1/9
  $
  For the anonymized graph in section (b), the information loss is
  $
    L(G, G'_b) = 1 - 8/9 = 1/9
  $
