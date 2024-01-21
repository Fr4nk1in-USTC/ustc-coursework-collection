#import "@preview/tablex:0.0.6": tablex, rowspanx

#import "@local/typreset:0.1.0": homework

#show: homework.style.with(
  course: "Data Privacy",
  number: "3",
  names: "傅申",
  ids: "PB20000051",
)

#let question = homework.complex_question

#set enum(numbering: (..numbers) => {
  if numbers.pos().len() == 1 {
    return strong(numbering("(a)", ..numbers))
  } else {
    return numbering("(1)", numbers.pos().at(-1))
  }
}, full: true)

#show figure: align.with(center)

#show link: text.with(fill: blue)
#show link: underline

#question[
  *(10') Permutation Cipher*
  + (5') Consider the permutation $pi$ on the set $1, 2, dots, 8$ defined as
    follows. Find the inverse permutation $pi^(-1)$.
    #align(
      center,
      table(
        columns: 9,
        align: center + horizon,
        $x$,     [*1*], [*2*], [*3*], [*4*], [*5*], [*6*], [*7*], [*8*],
        $pi(x)$, [*4*], [*1*], [*6*], [*2*], [*7*], [*3*], [*8*], [*5*],
      )
    )
  + (5') Decrypt the following ciphertext encrypted using a permutation cipher
    with the key being the permutation $pi$ from part (a).
    #align(center)[_TGEEMNELNNTDROEOAAHDOETCSHAEIRLM_]
]

+ The inverse permutation is shown below.
  #align(
    center,
    table(
      columns: 9,
      align: center + horizon,
      $x$,          [*1*], [*2*], [*3*], [*4*], [*5*], [*6*], [*7*], [*8*],
      $pi^(-1)(x)$, [*2*], [*4*], [*6*], [*1*], [*8*], [*3*], [*5*], [*7*],
    )
  )
+ If *Columnar Transposition* is used to encrypt the plaintext, the plaintext
  should be written in rows of length $8$ and then the columns are permuted
  according to the permutation $pi$ from part (a). The ciphertext is then
  obtained by reading the columns in order. The grid below shows the process of
  encryption.
  #align(
    center,
    tablex(
      columns: 9,
      align: center + horizon,
      $x$,     [*1*], [*2*], [*3*], [*4*], [*5*], [*6*], [*7*], [*8*],
      $pi(x)$, [*4*], [*1*], [*6*], [*2*], [*7*], [*3*], [*8*], [*5*],
      rowspanx(4)[Plaintext],
      [R], [T], [O], [M], [S], [N], [I], [A],
      [O], [G], [E], [N], [H], [N], [R], [A],
      [E], [E], [T], [E], [A], [T], [L], [H],
      [O], [E], [C], [L], [E], [D], [M], [D]
    )
  )
  Thus, the plaintext is
  #align(center)[_RTOMSNIAOGENHNRAEETEATLHOECLEDMD_.]

  Otherwise, if we *permutate the plaintext block by block*, the plaintext
  should be divided into blocks of length $8$ and then permuted according to the
  permutation $pi$ from part (a). The ciphertext is then obtained by reading the
  blocks in order. To decrypt the ciphertext, first divide the ciphertext into
  blocks of length $8$: _TGEEMNEL NNTDROEO AAHDOETC SHAEIRLM_, and then permute
  each block: _ETNGEELM DNONETOR DAEATHCO ESRHLAMI_. So the plaintext is
  #align(center)[_ETNGEELMDNONETORDAEATHCOESRHLAMI_.]

#question[
  *(20') Perfect Secrecy*
  + *(10')* Let $n$ be a positive integer. An $n$-th order Latin square is an
    $n times n$ matrix $L$ such that each of the $n$ integers $1, 2, dots, n$
    appears exactly once in each row and each column of $L$. The pollowing is an
    example of a Latin square of order 3:
    $ mat(delim: #none, 1, 2, 3; 3, 1, 2; 2, 3, 1) $
    For any $n$-th order Latin square $L$, we can define a related encryption
    scheme. Let $cal(M) = cal(C) = cal(K) = {1, 2, dots, n}$. For $1 <= i <= n$,
    the encryption rule $e_i$ is defined as $e_i (j) = L(i, j)$ (thus, each row
    provides an encryption rule). Prove that if the key is chosen uniformly at
    random, the Latin square cipher has perfect secrecy.
  + *(10')* Prove that if a cipher has perfect secrecy and
    $abs(cal(M)) = abs(cal(C)) = abs(cal(K))$, then each ciphertext is
    equiprobable.
]

+ Accroding to Shannon's theorem, the Latin square cipher has perfect secrecy
  because
  + Each key is chosen with equal probability.
  + Knowing $j$, there is only one key that encrypt $j$ to a $L(i, j)$, because
    each number appears only once on a row.
+ Since the cipher has perfect secrecy, each key is chosen with equal
  probability $1 \/ abs(cal(K))$. For every $m in cal(M)$ and $c in cal(C)$,
  there is a unique $k in cal(K)$ such that $e_k (m) = c$. Thus, the probability
  of $c$ is
  $
    Pr[c]
    = sum_(m in cal(M)) Pr[m] Pr[c | m]
    = sum_(m in cal(M)) 1 / abs(cal(M)) Pr[e_k (m) = c | m]
    = sum_(m in cal(M)) 1 / abs(cal(M)) 1 / abs(cal(K))
    = 1 / abs(cal(K)) = 1 / abs(cal(C)).
  $
  So each ciphertext is equiprobable.

#question[
  *(25') RSA* \
  Assuming that Bob uses RSA and selects two _large_ prime numbers $p = 101$ and
  $q = 113$:
  + *(5')* How many possible public keys Bob can choose?
  + *(10')* Assuming that Bob uses a public encryption key $e = 3533$. Alice
    sends Bob a message $M = 9726$. What will be the ciphertext received by Bob?
    Show the detailed procedure that Bob decrypts the received ciphertext.
  + *(10')* Let $n = p q$ be a product of two distinct primes. Show that if
    $phi.alt(n)$ and $n$ are known, then it is possible to compute $p$ and $q$
    in polynomial time. _Hint: Derive a quadratic equation (over the integers)
    in the unknown p._
]

$ n = p q = 11413, phi.alt(n) = (p - 1)(q - 1) = 11200 $
+ The possible public keys are the integers in $(1, 11200)$ that are coprime to
  $11200$, so there are $phi.alt(11200)$ such integers. Since
  $11200 = 2^6 times 5^2 times 7$, there are
  $ phi.alt(11200) = 11200 (1 - 1/2) (1 - 1/5) (1 - 1/7) = 3840 $
  possible public keys.
+ The private key $d$ should satisfy $d equiv e^(-1) (mod phi.alt(n))$. Since
  $e = 3533$ and $phi.alt(n) = 11200$, it can be computed that $d = 6597$.

  Given the plaintext $m = 9726$, the ciphertext Bob received is
  $ c = m^e mod n = 9726^3533 mod 11413 = 5761. $

  To decrypt the ciphertext, Bob computes
  $ m = c^d mod n = 5761^6597 mod 11413 = 9726, $ which is the plaintext.
+ Since $n = p q$ is a product of two distinct primes,
  $phi.alt(n) = (p - 1)(q - 1) = n - p - q + 1$.
  Thus, we have
  $
    cases(
      p q   & = n,
      p + q & = n + 1 - phi.alt(n),
    ),
  $
  Accroding to Vieta's formulas, $p$ and $q$ are the roots of equation
  $x^2 - (n + 1 - phi.alt(n)) x + n = 0$. So $p$ and $q$ can be computed by
  solving the equation, which can be done in polynomial time:
  $
    p, q = ((n + 1 - phi.alt(n)) plus.minus sqrt((n + 1 - phi.alt(n))^2 - 4n)) / 2
  $

#let oplus = math.plus.circle
#question[
  *(20') Multi-Party Computation*
  + *(10') Paillier Encryption*. Assuming Alice employs the Paillier encryption
    scheme with the prime numbers $p = 11$ and $q = 17$, along with a randomly
    chosen value of $r = 83$ and $g = n + 1$. Alice transmits a message
    $M = 175$ to Bob. What ciphertext will Bob receive? Additionally, please
    prove the Homomorphic addition property of Paillier:
    $ op("Decrypt")((c_1 dot.c c_2) mod n^2) = m_1 + m_2 $
  + *(10') Secret Sharing*. We define a 2-out-of-3 secret sharing scheme as
    follows. In order to share a bit $v$, the dealer chooses three random bits
    $x_1, x_2, x_3 in {0, 1}$ under the constraint that
    $x_1 oplus x_2 oplus x_3 = 0$. Then:
    - $P_1$'s share is the pair $(x_1, a_1)$ where $a_1 = x_3 oplus v$.
    - $P_2$'s share is the pair $(x_2, a_2)$ where $a_2 = x_1 oplus v$.
    - $P_3$'s share is the pair $(x_3, a_3)$ where $a_3 = x_2 oplus v$.
    Let $(x_1, a_1), (x_2, a_2), (x_3, a_3)$ be a secret sharing of $v_1$, and
    let $(y_1, b_1), (y_2, b_2), (y_3, b_3)$ be a secret sharing of $v_2$. Try
    to explain that no communication is needed in order to compute a secret
    sharing of $v_1 oplus v_2$. ($oplus$ means XOR.)
]

+ _(Encryption)_ First, run the key generation procedure as follows:
  + Pick $p = 11$ and $q = 17$.
  + Compute $n = 11 times 17 = 187$.
  + Compute $lambda = op("lcm")(p - 1, q - 1) = op("lcm")(10, 16) = 80$.
  + Pick $g = n + 1 = 188$ is picked.
  + Compute $mu = (L(g^lambda mod n^2))^(-1) mod n = 180$
  Thus, the public key is $(n, g) = (187, 188)$ and the private key is
  $(lambda, mu) = (80, 180)$. Given the plaintext $M = 175$, the ciphertext Bob
  received is
  $ C = g^M r^n mod n^2 = (188^175 times 83^187) mod 187^2 = 23911. $

  #let Decrypt = math.op("Decrypt")
  #let Encrypt = math.op("Encrypt")
  _(Proof of Homomorphic addition property)_ For two arbitrary plaintext
  $m_1, m_2$, the ciphertexts are
  $
    c_1 = g^m_1 r_1^n mod n^2, c_2 = g^m_2 r_2^n mod n^2
    => c_1 dot.c c_2 = g^(m_1 + m_2) (r_1 r_2)^n mod n^2.
  $
  Thus, the product is decrypted as
  $
    Decrypt(c_1 dot.c c_2 mod n^2)
    & = Decrypt(g^(m_1 + m_2) (r_1 r_2)^n mod n^2) \
    & = Decrypt(g^(m_1 + m_2) r_*^n mod n^2) \
    & = m_1 + m_2.
  $
  So the Homomorphic addition property of Paillier holds.
+ For each $P_i$, simply compute $(x_i oplus y_i, a_i oplus b_i)$ would form a
  secret sharing of $v_1 oplus v_2$. It does not require any communication. The
  result secret sharing is shown as follows:
  #align(
    center,
    table(
      columns: 2,
      align: center + horizon,
      [*Player*], [*Computed Secret Sharing*],
      $P_1$, $(x_1 oplus y_1, (x_3 oplus y_3) oplus (v_1 oplus v_2))$,
      $P_2$, $(x_2 oplus y_2, (x_1 oplus y_1) oplus (v_1 oplus v_2))$,
      $P_3$, $(x_3 oplus y_3, (x_2 oplus y_2) oplus (v_1 oplus v_2))$,
    )
  )
  Since $(x_3 oplus y_3) oplus (x_1 oplus y_1) oplus (x_2 oplus y_2)
  = (x_1 oplus x_2 oplus x_3) oplus (y_1 oplus y_2 oplus y_3)
  = 0 oplus 0 = 0$, the computed secret sharing is valid.

#question[
  *(25') Computational Security*
  + *(5')* Explain the difference between _Interchangeable_ and _Indistinguishable_.
  + *(10')* Which of the following are negligible functions in $lambda$? Justify
    your answers.
    $
      1/(2^(lambda \/ 2)) quad
      1/(2^(log(lambda^2))) quad
      1/(lambda^(log lambda)) quad
      1/(lambda^2) quad
      1/(2^(log lambda)^2) quad
      1/((log lambda)^2) quad
      1/(lambda^(1 \/ lambda)) quad
      1/(sqrt(lambda)) quad
      1/(2^(sqrt(lambda)))
    $
  + *(10')* Suppose $f$ and $g$ are negligible.
    + Show that $f + g$ is negligible.
    + Show that $f times g$ is negligible.
    + Give an example $f$ and $g$ which are both negligible, but where
      $f(lambda) \/ g(lambda)$ is not negligible.
]
+ The definition of these two terms are
  / Interchangeable: $cal(L)_1$ and $cal(L)_2$ are interchangeable if for all
    programs $cal(A)$ that output a single bit,
    $Pr[cal(A) lozenge.small cal(L)_1 => 1] = Pr[cal(A) lozenge.small cal(L)_2 => 1]$.
  / Indistinguishable: $cal(L)_1$ and $cal(L)_2$ are indistinguishable if for
    all polynomial-time programs $cal(A)$ that output a single bit,
    $Pr[cal(A) lozenge.small cal(L)_1 => 1] - Pr[cal(A) lozenge.small cal(L)_2 => 1]$
    is negligible.
  So the difference is that, interchangeable is a stronger condition than
  indistinguishable. There is no program $cal(A)$ that can distinguish two
  interchangeable libraries, but there may exist a (non-polynomial-time) program
  $cal(A)$ that can distinguish two indistinguishable libraries.
+ 
  - $1/(2^(lambda \/ 2))$ is negligible, because
    $2^(lambda \/ 2) = (sqrt(2))^lambda$ is exponential.

  - $1/(2^(log(lambda^2)))$ is not negligible, because
    $2^(log(lambda^2)) = lambda^(2 / (log_2 e))$ is a lower order infinity than
    some polynomial (e.g. $lambda^2$).

  - $1/(lambda^(log lambda))$ is negligible, because for any finite order $n$,
    there exists $lambda_0 >= exp(n)$ such that $forall lambda > lambda_0$,
    $lambda^(log lambda) > lambda^n$, proving that $lambda^(log lambda)$ is
    a higher order infinity than any polynomial.

  - $1/(lambda^2)$ is obviously not negligible, because $lambda^2$ is a
    polynomial.

  - $1/(2^(log lambda)^2)$ is negligible. Since
    $2^((log lambda)^2) = lambda^(2 / (log_2 e) log lambda)$, for any finite
    order $n$, there exists $lambda_0 >= exp(n log_2 e \/ 2)$ such that
    $forall lambda > lambda_0$, $lambda^(2 / (log_2 e) log lambda) > lambda^n$,
    proving that $2^((log lambda)^2)$ is a higher order infinity than any
    polynomial.

  - $1/((log lambda)^2)$ is obviously not negligible, because
    $log(lambda)^2 < lambda^2$ for $lambda$ large enough.

  - $1/(lambda^(1 \/ lambda))$ is obviously not negligible, because
    $lambda^(1 \/ lambda) < lambda$ for $lambda > 1$.

  - $1/(sqrt(lambda))$ is obviously not negligible, because
    $sqrt(lambda) < lambda$ for $lambda > 1$.

  - $1/(2^(sqrt(lambda)))$ is negligible, because for any $k in NN$,
    $
      lim_(lambda -> +oo) lambda^k / 2^(sqrt(lambda))
      = lim_(lambda -> +oo) exp(k log lambda - sqrt(lambda) log 2)
      = exp(-oo)
      = 0.
    $
    So $2^(sqrt(lambda))$ is a higher order infinity than any polynomial.
+ 
  + Since $2 max(f, g) > f + g$, $P(lambda) times (2 max(f, g)) > P(lambda) (f + g)$.
    And there is
    $
      lim_(lambda -> +oo) P(lambda) times (2 max(f, g))
      = 2 lim_(lambda -> +oo) P(lambda) max(f, g)
      = 0.
    $
    So
    $
      lim_(lambda -> +oo) P(lambda) (f + g) = 0.
    $
    In other words, $f + g$ is negligible.
  + By definition,
    $
      lim_(lambda -> +oo) P(lambda) times (f times g)
      = lim_(lambda -> +oo) (P(lambda) times f) times g
      = 0 times 0 = 0.
    $
    shows that $f times g$ is negligible.
  + For example, $f(lambda) = exp(-lambda)$ and
    $g(lambda) = lambda exp(-lambda)$ are both negligible, but
    $f(lambda) \/ g(lambda) = 1 \/ lambda$ is not negligible.
