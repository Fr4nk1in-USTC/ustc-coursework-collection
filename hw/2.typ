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

#let question = homework.complex_question

#set enum(numbering: (..numbers) => {
  if numbers.pos().len() == 1 {
    return strong(numbering("(a)", ..numbers))
  } else {
    return numbering("1.", numbers.pos().at(-1))
  }
}, full: true)
#show figure: align.with(center)
#show link: text.with(fill: blue)
#show link: underline

#question[
  *(15') Laplace Mechanism* \
  + *(5')* Given the function $f(x) = 1/6 sum_(i = 1)^6 x_i$, where
    $x_i in {1, 2, ..., 10}$ for $i in {1, 2, ..., 6}$, compute the global
    sensitivity and local sensitivity when $x = {3, 5, 4, 5, 6, 7}$.
  + *(10')* Given a database $x$ where each element is in ${1, 2, 3, 4, 5, 6}$,
    design $epsilon$-differentially private Laplace mechanisms corresponding to
    the following queries, where $epsilon$ = 0.1:
    + $q_1(x) = sum_(i = 1)^6 x_i$
    + $q_2(x) = max_(i in {1, 2, ..., 6}) x_i$
#v(3pt)
]


#question[
  *(15') Exponential mechanism*
  #figure(
    tablex(
      columns: 8,
      align: center + horizon,
      auto-lines: false,
      hlinex(),
      [*ID*], [*sex*], [*Chinese*], [*Mathematics*], [*English*], [*Physics*],
      [*Chemistry*], [*Biology*],
      hlinex(),
         1,   "Male", 96, 58, 80, 53, 56, 100,
         2,   "Male", 60, 63, 77, 50, 59, 75,
         3, "Female", 83, 86, 98, 69, 80, 100,
      colspanx(8)[...],
      4000, "Female", 86, 83, 98, 87, 82, 92,
      hlinex(),
    ),
    caption: "Scores of students in School A",
    kind: "table",
    supplement: "Table"
  )<table-1>

  @table-1 records the scores of students in School A in the final exam. We
  need to help the teacher query the database while protecting the privacy of
  students' scores. The domain of this database is
  ${"Male", "Female"} times {0, 1, 2, ..., 100}^6$. Answer the following
  questions.
  + *(5')* What is the sensitivity of the following queries:
    + $q_1(x) = 1/4000 sum_("ID" = 1)^4000 "Physics"_"ID"$
    + $q_2(x) = max_("ID" in {1, 2, ..., 4000}) "Biology"_"ID"$
  + *(10')* Design $epsilon$-differentially privacy mechanisms  corresponding to
    the two queries in (a), where $epsilon$ = 0.1. (Using Laplace mechanism for
    $q_1$ and exponential mechanism for $q_2$)
]

#question[
  *(20') Composition*
  #block(radius: 5pt, inset: 7pt, stroke: 1pt, above: 5pt)[
    *Theorem 3.16.* Let $cal(M)_i: NN^abs(cal(X)) -> cal(R)_i$ be an
    $(epsilon_i, delta_i)$-differentially private algorithm for $i in [k]$. Then
    if $cal(M)_[k](x) = (cal(M)_1(x), ..., cal(M)_k(x))$, then $cal(M)_k$ is
    $(sum_(i = 1)^k epsilon_i, sum_(i = 1)^k delta_i)$-differentially private.

    *Theorem 3.20 #text(font: "Libertinus Sans")[(Advanced Composition)]*.
    For all $epsilon, delta, delta' >= 0$, the class of
    $(epsilon, delta)$-differentially private mechanisms satisfies
    $(epsilon', k delta + delta')$-differential privacy under $k$-fold adaptive
    composition for:
    $
      epsilon' = epsilon sqrt(2k ln(1/delta')) + k epsilon (epsilon^epsilon - 1)
    $
  ]
  + *(10')* Given a database $x = {x_1, x_2, ..., x_2000}$ where
    $x_i in {0, 1, 2, ..., 100}$ for each $i$ and privacy parameters
    $(epsilon, delta) = (1.25, 10^(-5))$, apply the Gaussian mechanism to
    protect 100 calls to the query $q_1(x) = 1/2000 sum_(i = 1)^2000 x_i$.
    Determine the noise variances $sigma^2$ of the Gaussian mechanism to ensure
    $(epsilon, delta)$-DP based on the composition and advanced composition
    theorems, respectively.
  + *(10')* Determine the noise variances $sigma^2$ of the Gaussian mechanism
    to protect 100 calls to the query
    $q_2(x) = max_(i in {1, 2, ..., 2000}) x_i$ to ensure $(1.25, 10^(-5))$-DP
    based on the composition and advanced composition theorems, respectively,
    where $x$ is the database in (a).
]

#question[
  *(25') Randomized Response for Local DP*\
  Consider a population of $n$ users, where the true proportion of males is
  denoted as $pi$. Our objective is to gather statistics on the proportion of
  males, prompting a sensitive question: "Are you male?" Each user responds
  with either a yes or no, but due to privacy concerns, they refrain from
  directly disclosing their true gender. Instead, they employ a biased coin
  with a probability of landing heads denoted as $p$, and tails as $1 - p$.
  When the coin is tossed, a truthful response is given if heads appear, while
  the opposite response is provided if tails come up.
  + *(10')* Demonstrate that the aforementioned randomized response adheres to
    local differential privacy and determine the corresponding privacy
    parameter, $epsilon$.
  + *(15')*  Employing the perturbation method outlined above to aggregate
    responses from the $n$ users yields a statistical estimate for the number of
    males. Assuming the count of "yes" responses is $n_1$, construct an unbiased
    estimate $pi$ for based on $n, n_1, p$. Calculate the variance associated
    with this estimate.
]

#question[
  *(10') Accuracy Guarantee of DP*\
  Consider the application of an $(epsilon, delta)$-differentially private
  Gaussian mechanism denoted by $cal(M)$ to protect the mean estimator
  $macron(x) = 1/n sum_(i = 1)^n x_i$ of a $d$-dimensional input database $x$,
  where $x_i in {0, 1, ..., 100}^d$ for each $i$. Let $cal(M)(x)$ represent the
  output of this Gaussian mechanism. Utilize both the tail bound and the union
  bound to derive the $L_oo$-norm error bound of $cal(M)$, denoted by
  $norm(cal(M)(x) - macron(x))_oo$, ensuring a probability of at least $1 -
  beta$. Specifically, solve for the bound $cal(B)$ such that
  $
    "Pr"[norm(cal(M)(x) - macron(x))_oo <= cal(B)] >= 1 - beta
  $

  *Hint:* Refer to #link("https://zhuanlan.zhihu.com/p/425562737")[Zhihu link]
  for descriptions of statistical inequalities.
]

#question[
  *(15') Personalized Differential Privacy*\
  Consider an $n$-element dataset $D$ where the $i$-th element is owned by a
  user $i in [n]$, where $[n] = {1, 2, ..., n}$ and the privacy requirement of
  user $i$ is $epsilon_i$-DP. A randomized mechanism $cal(M)$ satisfies
  ${epsilon_i}_(i in [n])$-personalized differential privacy (or
  ${epsilon_i}_(i in [n])$-PDP) if, for every pair of neighboring datasets $D$,
  $D'$ differing at the $j$-th element for an arbitrary $j in [n]$, and for all
  sets $S$ of possible outputs,
  $
    "Pr"[cal(M)(D) in S] <= exp(epsilon_j) Pr[cal(M)(D') in S].
  $
  + *(5')* Prove the composition theorem of PDP: if a mechanism is
    ${epsilon_i^((1))}_(i in [n])$-PDP and another is ${epsilon_i^((2))}_(i in
    [n])$-PDP, then publishing the result of both is ${epsilon_i^((1)) +
    epsilon_i^((2))}_(i in [n])$-PDP.
  + *(10')* Given a dataset $D$ and a privacy requirement set
    ${epsilon_i}_(i in [n])$, the _Sample mechanism_ works as follows:
    #set enum(numbering: "1)", full: false)
    + We pick an arbitrary threshold value $t > 0$;
    + We sample a subset $D_S subset D$ where the probability that the $i$-th
      element of $D$ is contained in $D_S$ equals
      $(exp(epsilon_i) - 1)/(exp(t) - 1)$ if $epsilon_i < t$ and 1 otherwise;
    + We output $cal(M)(D_S)$, where $cal(M)$ is a $t$-differentially private
      mechanism.
    Prove that the Sample mechanism with any $t > 0$ is
    ${epsilon_i}(i in [n])$-PDP.\
    *Hint*: Use the Bayes formula.
]
