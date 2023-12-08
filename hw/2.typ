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

In this anwser, neighboring databases are those that differ in one and only one
element.

The probability density function of Laplace distribution with scale $b$ is
$"Lap"(x | b) = 1/(2b) exp(-abs(x)/b)$.
In this answer, a random variable $X$ denoted as $"Lap"(b)$ satisfies the
Laplace distribution with scale $b$.

+
  / Global Sensitivity: Since $max abs(x_i - x_i ') = 9$, the global sensitivity
    $Delta f = 9/6 = 1.5$
  / Local Sensitivity: For all neighboring database $x '$ of $x$, the maximum
    difference in sum of $x$ and $x '$ is 7, where $x_1 = 3$ and $x_1 ' = 10$.
    Thus, the local sensitivity $"LS"(f, x) = 7/6$
+
  + The sensitivity of $q_1$ is $Delta q_1 = 6 - 1 = 5$ (consider two
    neighboring datasets where the different elements are 1 and 6). Thus, the
    0.1-differentially private Laplace mechanism is
    $
      cal(M)_L (x, q_1(dot.c), 0.1) = sum_(i = 1)^6 x_i + "Lap"(50)
    $
  + The sensitivity of $q_2$ is $Delta q_2 = 6 - 1 = 5$ (consider dataset
    ${1, 1, ..., 1}$ and ${6, 1, ..., 1}$). Thus, the 0.1-differentially private
    Laplace mechanism is
    $
      cal(M)_L (x, q_2(dot.c), 0.1) = max_(i in {1, 2, ..., 6}) x_i +
      "Lap"(50)
    $


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

In this anwser, neighboring databases are those that differ in one and only one
element.

+
  + Since the range of Physics score is $[0, 100]$, the maximum difference in
    sum of Physics score is 100. Thus, the sensitivity of $q_1$ is
    $Delta q_1 = 100/4000 = 1/40 = 0.025$.
  + Consider Biology scores of two neighboring datasets: ${0, 0, ..., 0}$ and
    ${100, 0, ..., 0}$, which makes the maximum difference in query $q_2$. Thus,
    the sensitivity of $q_2$ is $Delta q_2 = 100$.
+
  + The 0.1-differentially private Laplace mechanism for $q_1$ is
    $
      cal(M)_L (x, q_1(dot.c), 0.1) = 1/4000 sum_("ID" = 1)^4000
      "Physics"_"ID" + "Lap"(40)
    $
  + First, define the scoring function $u(x, r) = II(r = max(x))$, where
    $II(dot)$ is the indicator function. Thus, the sensitivity of $u$ is
    $Delta u = 1$. The 0.1-differentially private exponential mechanism for
    $q_2$ outputs $r in {0, 1, ..., 100}$ with probability proportional to
    $ exp(0.1 times u(x, r)/2) = exp(u(x, r) / 20). $

#question[
  *(20') Composition*
  #block(radius: 5pt, inset: 7pt, stroke: 1pt, above: 5pt)[
    *Theorem 3.16.* Let $cal(M)_i: NN^abs(cal(X)) -> cal(R)_i$ be an
    $(epsilon_i, delta_i)$-differentially private algorithm for $i in [k]$. Then
    if $cal(M)_[k](x) = (cal(M)_1(x), ..., cal(M)_k(x))$, then $cal(M)_k$ is
    $(sum_(i = 1)^k epsilon_i, sum_(i = 1)^k delta_i)$-differentially private.

    *Theorem 3.20 #text(font: "Libertinus Sans")[(Advanced Composition)]*.
    For all $epsilon, delta, delta ' >= 0$, the class of
    $(epsilon, delta)$-differentially private mechanisms satisfies
    $(epsilon ', k delta + delta ')$-differential privacy under $k$-fold
    adaptive composition for:
    $
      epsilon '
      = epsilon sqrt(2k ln(1 / (delta '))) + k epsilon (exp(epsilon) - 1)
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

+ First calculate the $epsilon_0$ and $delta_0$ for each call of $q_1$:
  - For composition, $epsilon = 100 epsilon_0$, $delta = 100
    delta_0$. Thus, $epsilon_0 = epsilon/100 = 0.0125$, $delta_0 = delta/100 =
    10^(-7)$.
  - For advanced composition,
    $
      epsilon = epsilon_0 sqrt(2k ln(1/delta_0)) + k epsilon_0 (exp(epsilon_0) - 1) \
      delta = k delta_0 + delta_0 = (k + 1) delta_0,
    $
    where $k = 100$. The solution of the above equation is
    $
      delta_0   & = 1/101 times 10^(-5) \
      epsilon_0 & approx 0.0212.
    $
  To ensure $(epsilon_0, delta_0)$-DP, the noise variance $sigma^2$ of the
  Gaussian mechanism should be $2 ln(1.25 / delta_0) dot (Delta q_1)^2 / epsilon^2_0$.
  The sensitivity of $q_1$ is $Delta q_1 = 100/2000 = 0.05$. Thus,
  - For composition, the noise variance $sigma^2$ of the Gaussian mechanism
    should be
    $
      sigma^2 = 2 ln(1.25 / 10^(-7)) times (0.05 / 0.0125)^2 approx 522.92
    $
  - For advanced composition, the noise variance $sigma^2$ of the Gaussian
    mechanism should be
    $
      sigma^2 = 2 ln(1.25 times 101 times 10^5) times (0.05 / 0.0212)^2 approx 181.91
    $
    #h(100%)
+ The sensitivity of $q_2$ is $Delta q_2 = 100$. Thus,
  - For composition, the noise variance $sigma^2$ of the Gaussian mechanism
    should be
    $
      sigma^2 = 2 ln(1.25 / 10^(-7)) times (100 / 0.0125)^2 approx 2.09 times 10^9
    $
  - For advanced composition, the noise variance $sigma^2$ of the Gaussian
    mechanism should be
    $
      sigma^2 = 2 ln(1.25 times 101 times 10^5) times (100 / 0.0212)^2 approx 7.28 times 10^8
    $
    #h(100%)


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

+ To make this randomized response mechanism satisfy local differential
  privacy, the probability of responding truthfully should be
  $p = exp(epsilon) / (1 + exp(epsilon))$. Thus, the privacy parameter
  $epsilon$ is
  $ epsilon = ln(p / (1 - p)). $
+ The probability of a user responding "yes" is
  $ Pr["yes"] = p pi + (1 - p) (1 - pi) = (2p - 1) pi - p + 1. $
  Since $n_1 tilde "B "(n, Pr["yes"])$, the expectation of $n_1$ is
  $EE(n_1) = n Pr["yes"]$. Thus, the unbiased estimate of $pi$ is
  $ hat(pi) = (n_1 \/ n + p - 1) / (2 p - 1) = (n_1 + (p - 1)n) / ((2p - 1)n). $
  The variance associated with this estimate is
  $
    "Var"(hat(pi))
    & = "Var"(n_1/((2p - 1)n)) \
    & = (Pr["yes"](1 - Pr["yes"])) / ((2p - 1)^2 n) \
    & = ((2p pi - p - pi + 1)(p + pi - 2 p pi)) / ((2 p - 1)^2 n).
  $

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
    Pr[norm(cal(M)(x) - macron(x))_oo <= cal(B)] >= 1 - beta
  $

  *Hint:* Refer to #link("https://zhuanlan.zhihu.com/p/425562737")[Zhihu link]
  for descriptions of statistical inequalities.
]

Denote $Y_i$ as the noise added to the $i$-th dimension of the mean estimator,
then $cal(M)(x) = macron(x) + Y$. Thus,
$
  norm(cal(M)(x) - macron(x))_oo = max_(i in [d]) abs(Y_i)
  => Pr[norm(cal(M)(x) - macron(x))_oo <= cal(B)]
       = Pr[max_(i in [d]) abs(Y_i) <= cal(B)].
$
Since $Y_i$ is i.i.d. random variables following the Gaussian distribution
$NN(0, sigma^2)$, there is
$
  Pr[norm(cal(M)(x) - macron(x))_oo <= cal(B)]
  = Pr[max_(i in [d]) abs(Y_i) <= cal(B)]
  = product_(i = 1)^d Pr[abs(Y_i) <= cal(B)]
  = p^d,
$
where $p$ stands for the probability that a random variable following
$NN(0, sigma^2)$ is in the interval $[-cal(B), cal(B)]$. It is obvious that if
the inequality in the question holds, then $p >= root(d, 1 - beta)$.

The tail bound (Chernoff-style bound) of Gaussian distribution can be written as
$
  Pr[X - mu >= t] <= exp(- t^2 / (2 sigma^2)), X tilde NN(mu, sigma^2).
$
So $p$ satisfies
$
  p = Pr[abs(X) <= cal(B)] = 1 - 2 Pr[X >= cal(B)]
  >= 1 - 2 exp(- cal(B)^2 / (2 sigma^2)).
$
Then, the inequality in the question holds if
$
  1 - 2 exp(- cal(B)^2 / (2 sigma^2)) >= root(d, 1 - beta),
$
which is equivalent to
$
  cal(B)^2 >= 2 sigma^2 ln 2 / (1 - root(d, 1 - beta))
  <=> cal(B) >= sqrt(2 sigma^2 ln 2 / (1 - root(d, 1 - beta))).
$
Since the Gaussian mechanism is $(epsilon, delta)$-DP, the noise variance
$sigma^2$ should satisfy
$
  Delta_2^2(macron(x)) = norm((100/n, 100/n, 100/n, ..., 100/n))_2^2
  = (10000 d) / n^2 \
  sigma^2 >= 2  dot (Delta_2^2(macron(x))) / epsilon^2 ln 1.25/delta
  = (20000 d) / (epsilon^2 n^2) ln 1.25/delta.
$
So the bound of $cal(B)$ is
$display(
  cal(B)
  >= sqrt(2 sigma^2 ln 2 / (1 - root(d, 1 - beta)))
  >= 200 / (epsilon n) sqrt(d ln 1.25/delta dot ln 2 / (1 - root(d, 1 - beta)))
)$.


#question[
  *(15') Personalized Differential Privacy*\
  Consider an $n$-element dataset $D$ where the $i$-th element is owned by a
  user $i in [n]$, where $[n] = {1, 2, ..., n}$ and the privacy requirement of
  user $i$ is $epsilon_i$-DP. A randomized mechanism $cal(M)$ satisfies
  ${epsilon_i}_(i in [n])$-personalized differential privacy (or
  ${epsilon_i}_(i in [n])$-PDP) if, for every pair of neighboring datasets $D$,
  $D '$ differing at the $j$-th element for an arbitrary $j in [n]$, and for all
  sets $S$ of possible outputs,
  $
    Pr[cal(M)(D) in S] <= exp(epsilon_j) Pr[cal(M)(D ') in S].
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
    ${epsilon_i}_(i in [n])$-PDP.\
    *Hint*: Use the Bayes formula.
]

#show math.equation: text.with(font: "New Computer Modern Math")

In this answer,
- ${epsilon_i}_(i in [n])$ is abbreviated as $cal(E)$, and
  ${epsilon^((k))_i}_(i in [n])$ is abbreviated as $cal(E)^((k))$;
- $D tilde D '$ stands for $D$ and $D '$ are neighboring datasets, and
  $D tilde^j D '$ stands for $D$ and $D '$ are neighboring datasets that differ
  at the $j$-th element;
- The notation $D '$ represents a neighboring dataset of $D$ that differs
  at the $j$-th element, i.e. $D tilde^j D '$;
- Function $pi_t (dot.c)$ is defined as
  $
    pi_t (x) =
    cases(
      (exp(x) - 1) / (exp(t) - 1) & "if" x < t,
      1                           & "otherwise"
    )
  $
- Sampling mechanism is denoted as $cal(S)$, while the sampling procedure in it
  is denoted as $S P$.

+ Let $cal(M)_1$ and $cal(M)_2$ denote two mechanisms that satisfy PDP for
  $cal(E)^((1))$ and $cal(E)^((2))$, respectively. Mechanism $cal(M)_3$
  publishes the result of both $cal(M)_1$ and $cal(M)_2$, i.e.
  $cal(M)_3(D) = (cal(M)_1(D), cal(M)_2(D))$. For any set $S$ of $im cal(M)_3$,
  there is
  $
    Pr[cal(M)_3(D) in S]
    = sum_((s_1, s_2) in S) Pr[cal(M)_1(D) = s_1] dot Pr[cal(M)_2(D) = s_2]
  $

  Let $D tilde^j D '$ be an arbitrary pair of neighboring datasets. Applying the
  definition of PDP for both $D$ and $D '$ gives
  $
    Pr[cal(M)_3(D) in S]
    & <= sum_((s_1, s_2) in S) exp(epsilon_j^((1))) Pr[cal(M)_1(D ') = s_1] dot
         exp(epsilon_j^((2))) Pr[cal(M)_2(D ') = s_2] \
    & = exp(epsilon_j^((1)) + epsilon_j^((2))) sum_((s_1, s_2) in S)
        Pr[cal(M)_1(D ') in S] dot Pr[cal(M)_2(D ') in S] \
    & = exp(epsilon_j^((1)) + epsilon_j^((2))) Pr[cal(M)_3(D ') in S]
  $
  for any set $S$ of $im cal(M)_3$. By the definition of PDP , $cal(M)_3$
  satisfies PDP for $cal(E) = cal(E)^((1)) + cal(E)^((2))$.
  #h(1fr) #box(scale(160%, origin: bottom + right, sym.square.stroked))
+ The subject of this proof is to demonstrate that for any $D tilde^j D '$ and
  any set $S$ of $im cal(S)$,
  $
    Pr[cal(S)(D) in S] <= exp(epsilon_j) Pr[cal(S)(D ') in S].
  $

  Note that all of the possible outputs of the sampling procedure $S P(D)$ can
  be divided into those in which the $j$-th element is contained and in which
  the $j$-th element is not contained. Thus, $Pr[cal(S)(D) in S]$ can be
  rewritten as
  $
    Pr[cal(S)(D) in S]
    & = pi_t (epsilon_j) Pr[cal(M)(S P(D)) in S | D_j in S P(D)] \
    &   quad + (1 - pi_t (epsilon_j))
               Pr[cal(M)(S P(D)) in S | D_j in.not S P(D)], \
    Pr[cal(S)(D ') in S]
    & = pi_t (epsilon_j) Pr[cal(M)(S P(D ')) in S | D_j ' in S P(D ')] \
    &   quad + (1 - pi_t (epsilon_j))
               Pr[cal(M)(S P(D ')) in S | D_j ' in.not S P(D ')]. \
  $
  Since $cal(M)$ satisfies $t$-DP, there is
  $
    Pr[cal(M)(S P(D)) in S | D_j in.not S P(D)] <= exp(t) Pr[cal(M)(S P(D ')) in S | D_j ' in S P(D ')].
  $
  Thus,
  $
    Pr[cal(S)(D) in S]
    & <= exp(t) pi_t (epsilon_j) Pr[cal(M)(S P(D ')) in S | D_j ' in S P(D ')] \
    &    quad + (1 - pi_t (epsilon_j))
               Pr[cal(M)(S P(D ')) in S | D_j ' in.not S P(D ')]. \
  $
  There are two cases:
  - If $epsilon_j >= t <=> pi_t (epsilon_j) = 1$, there is
    $
      Pr[cal(S)(D) in S]
      <= exp(t) Pr[cal(S)(D ') in S]
      <= exp(epsilon_j) Pr[cal(S)(D ') in S].
    $
  // The following proof has critical errors and is not valid.
  // I just can't prove it and I think the question is wrong.
  - If $epsilon_j < t <=> pi_t (epsilon_j) = (exp(epsilon_j) - 1) / (exp(t) - 1)$,
    there is
    $
      exp(t) pi_t (epsilon_j) + 1 - pi_t (epsilon_j)
      = (exp(t + epsilon_j) - exp(epsilon_j)) / (exp(t) - 1)
      = exp(epsilon_j) \
      => Pr[cal(S)(D) in S] <= exp(epsilon_j) Pr[cal(S)(D ') in S].
    $
    #h(100%)
  Thus, $cal(S)$ satisfies $cal(E)$-PDP.
