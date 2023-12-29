#import "@preview/physica:0.9.1": *
#import "@preview/xarrow:0.2.0": xarrow

#import "@local/typreset:0.1.0": homework

#show: homework.style.with(
  course: "量子计算与机器学习",
  number: "",
  names: "傅申",
  ids: "PB20000051",
  lang: "zh-cn"
)

#let question = homework.complex_question
#let ee = math.upright("e")
#let ii = math.upright("i")

#question(number: "3.1")[
  The _fidelity_ $F$ of two quantum states $ket(psi_1)$ and $ket(psi_2)$ is
  defined by $F equiv abs(braket(psi_1, psi_2))^2$. It is a measure of the
  distance between the two quantum states: We have $0 <= F <= 1$, with $F = 1$
  when $ket(psi_1)$ concides with $ket(psi_2)$ and $F = 0$ when $ket(psi_1)$ and
  $ket(psi_2)$ are orthogonal. Show that $F = cos^2 alpha/2$, with $alpha$ the
  angle between the Bloch vectors corresponding to the quantum states
  $ket(psi_1)$ and $ket(psi_2)$.
]

假设两个量子态 $ket(psi_1)$ 和 $ket(psi_2)$ 在 Bloch 球面上的位置分别为
$(theta_1, phi_1)$ 和 $(theta_2, phi_2)$，即
$
  ket(psi_1) & = cos theta_1/2 ket(0) + ee^(ii phi_1) sin theta_1/2 ket(1)
               = vec(cos theta_1/2, ee^(ii phi_1) sin theta_1/2), \
  ket(psi_1) & = cos theta_2/2 ket(0) + ee^(ii phi_2) sin theta_2/2  ket(1)
               = vec(cos theta_2/2, ee^(ii phi_2) sin theta_2/2). \
$
则这两个量子态的内积为
$
  braket(psi_1, psi_2)
  & = vecrow(cos theta_1/2, ee^(- ii phi_1) sin theta_1/2)
      vec(cos theta_2/2, ee^(ii phi_2) sin theta_2/2) \
  & = cos theta_1/2 cos theta_2/2
      + sin theta_1/2 sin theta_2/2 ee^(ii (phi_2 - phi_1)).
$
因此两个量子态的 fidelity 为
$
  F
  & = abs(braket(psi_1, psi_2))^2 
    = (cos theta_1/2 cos theta_2/2 + sin theta_1/2 sin theta_2/2 cos(phi_2 - phi_1))^2
      + (sin theta_1/2 sin theta_2/2 sin(phi_2 - phi_1))^2 \
  & = cos^2 theta_1/2 cos^2 theta_2/2 + sin^2 theta_1/2 sin^2 theta_2/2
      + 2 cos theta_1/2 cos theta_2/2 sin theta_1/2 sin theta_2/2 cos(phi_2 - phi_1) \
  & = 1/2 (1 + cos theta_1 cos theta_2 + sin theta_1 sin theta_2 cos(phi_2 - phi_1)).
$
而两个量子态在 Bloch 球面上的夹角 $alpha$ 即为向量
$vecrow(sin theta_1 cos phi_1, sin theta_1 sin phi_1, cos theta_1)^TT$ 和
$vecrow(sin theta_2 cos phi_2, sin theta_2 sin phi_2, cos theta_2)^TT$
之间的夹角，有
$
  cos alpha
  & = vecrow(sin theta_1 cos phi_1, sin theta_1 sin phi_1, cos theta_1)
      vec(sin theta_2 cos phi_2, sin theta_2 sin phi_2, cos theta_2) \
  & = sin theta_1 sin theta_2 (cos phi_1 cos phi_2 + sin phi_1 sin phi_2)
      + cos theta_1 cos theta_2 \
  & = cos theta_1 cos theta_2 + sin theta_1 sin theta_2 cos(phi_2 - phi_1) \
  & = 2 F - 1.
$
因此，$F = 1/2 (1 + cos alpha) = cos^2 alpha/2$。

#question(number: "3.2")[
  Show that the unitary operator moving the state parametrized on the Bloch
  sphere by the angles $(theta_1, phi_1)$ into the state $(theta_2, phi_2)$ is
  given by
  $
    R_z (pi/2 + phi_2) H R_z (theta_2 - theta_1) H R_z (- pi/2 - phi_1).
  $

  The _phase-shift gate_ is defined as
  $
    R_z (delta) = mat(delim: "[", 1, 0; 0, ee^(ii delta)).
  $
]
角度 $(theta_1, phi_1)$ 对应的量子态为
$
  ket(psi_1)
  = cos theta_1/2 ket(0) + ee^(ii phi_1) sin theta_1/2 ket(1)
  = vec(cos theta_1/2, ee^(ii phi_1) sin theta_1/2).
$
经过给出的各个酉操作后，量子态依次变为：
$
  ket(psi_1) = vec(cos theta_1/2, ee^(ii phi_1) sin theta_1/2)
  & xarrow(R_z (- pi/2 - phi_1)) vec(cos theta_1/2, -ii sin theta_1/2)
    xarrow(H) 1/sqrt(2) vec(ee^(-ii theta_1 / 2), ee^(ii theta_1 / 2))
    xarrow(R_z (theta_2 - theta_1))
      1/sqrt(2) ee^(-ii theta_1 / 2) vec(1, ee^(ii theta_2)) \
  & xarrow(H)
      1/2 ee^(-ii theta_1 / 2) vec(1 + ee^(ii theta_2), 1 - ee^(ii theta_2))
    xarrow(R_z (pi/2 + phi_2))
      1/2 ee^(-ii theta_1 / 2) vec(1 + ee^(ii theta_2), ee^(pi/2 + phi_2)(1 - ee^(ii theta_2))) \
$
而
$
1/2 ee^(-ii theta_1 / 2)
vec(1 + ee^(ii theta_2), ee^(pi/2 + phi_2)(1 - ee^(ii theta_2)))
& = 1/2 ee^(ii (theta_2 - theta_1))
    vec(ee^(-ii theta_2 / 2) + ee^(ii theta_2 / 2),
        ii ee^(phi_2)(ee^(-ii theta_2 / 2) - ee^(ii theta_2 / 2))) \
& = ee^(ii (theta_2 - theta_1) / 2)
    vec(cos theta_2 / 2, ee^(ii phi_2) sin theta_2 / 2) \
& = ee^(ii (theta_2 - theta_1) / 2) ket(psi_2) \
& = ket(psi_2). \
$
其中 $ket(psi_2)$ 是角度 $(theta_2, phi_2)$ 对应的量子态。综上所述，
$R_z (pi/2 + phi_2) H R_z (theta_2 - theta_1) H R_z (- pi/2 - phi_1)$
是将角度 $(theta_1, phi_1)$ 对应的量子态变为角度 $(theta_2, phi_2)$ 的酉操作。

#question(number: "4.1")[
  证明贝尔态 $ket(Phi^+) = 1/sqrt(2) (ket(00) + ket(11))$ 可以等效表达为
  $ket(Phi^+) = 1/sqrt(2) (ket(a a) + ket(b b))$，其中 $ket(a)$ 和 $ket(b)$
  是任意一组正交归一基。
]

因为 $ket(a)$ 和 $ket(b)$ 是任意一组正交归一基，所以可以将 $ket(0)$ 和 $ket(1)$
表示为
$
  ket(0) = alpha ket(a) + beta ket(b), \
  ket(1) = - beta ket(a) + alpha ket(b).
$
其中 $alpha^2 + beta^2 = 1$。因此，$ket(Phi^+)$ 可以表示为
$
  ket(Phi^+)
  & = 1/sqrt(2) (ket(00) + ket(11)) \
  & = 1/sqrt(2) ((alpha ket(a) + beta ket(b)) times.circle (alpha ket(a) + beta ket(b))
                + (- beta ket(a) + alpha ket(b)) times.circle (- beta ket(a) + alpha ket(b))) \
  & = 1/sqrt(2) ((alpha^2 + beta^2) ket(a a) + (alpha^2 + beta^2) ket(b b)) \
  & = 1/sqrt(2) (ket(a a) + ket(b b)).
$
命题得证。

#question(number: "5.1")[
  Let $ket(x)$ be a basis state of $n$ qubits. Prove that
  $ H^(times.circle n) ket(x) = (sum_z (-1)^(x dot.c z) ket(z)) / sqrt(2^n) $
  where $x dot.c z$ is the bitwise inner product of $x$ and $z$, modulo 2, and
  the sum is over all $z in {0, 1}^n$.
]

如下所示，有
$
  H^(times.circle n) ket(x)
  & = H ket(x_1) times.circle H ket(x_2) times.circle dots.c times.circle H ket(x_n) \
  & = 1/sqrt(2) (ket(0) + (-1)^(x_1) ket(1)) times.circle
      1/sqrt(2) (ket(0) + (-1)^(x_2) ket(1)) times.circle
      dots.c times.circle
      1/sqrt(2) (ket(0) + (-1)^(x_n) ket(1)) times.circle \
  & = 1/sqrt(2^n) sum_(z_1 in {0, 1}) (-1)^(x_1 z_1) ket(z) times.circle
                                      (ket(0) + (-1)^(x_2) ket(1)) times.circle
                                      dots.c times.circle
                                      (ket(0) + (-1)^(x_n) ket(1)) \
  & = 1/sqrt(2^n)
      sum_(z_1 in {0, 1})
      sum_(z_2 in {0, 1}) (-1)^(x_1 z_1) ket(z_1) times.circle
                          (-1)^(x_2 z_2) ket(z_2) times.circle
                          (ket(0) + (-1)^(x_3) ket(1)) times.circle
                          dots.c times.circle
                          (ket(0) + (-1)^(x_n) ket(1)) times.circle \
  & = dots.c \
  & = 1/sqrt(2^n)
      sum_(z_1 in {0, 1})
      sum_(z_2 in {0, 1})
      dots.c
      sum_(z_n in {0, 1}) (-1)^(x_1 z_1) ket(z_1) times.circle
                          (-1)^(x_2 z_2) ket(z_2) times.circle
                          dots.c times.circle
                          (-1)^(x_n z_n) ket(z_n) \
  & = 1/sqrt(2^n)
      sum_(z_1, z_2, dots.c, z_n in {0, 1})
      (-1)^(x_1 z_1 + x_2 z_2 + dots.c + x_n z_n) ket(z_1 z_2 dots.c z_n) \
  & = 1/sqrt(2^n) sum_(z in {0, 1}^n) (-1)^(x dot.c z) ket(z).
$
即 $H^(times.circle n) ket(x) = (sum_z (-1)^(x dot.c z) ket(z)) / sqrt(2^n)$，
命题得证。
