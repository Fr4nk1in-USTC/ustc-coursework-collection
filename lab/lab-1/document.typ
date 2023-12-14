#import "@preview/lovelace:0.1.0": *
#show: setup-lovelace

#set heading(numbering: "1.")
#set page(height: auto)

#show link: text.with(fill: blue)
#show link: underline

= Privacy Preserving Logistics Regression

== Model: Logistic Regression

$
  hat(bold(y))(bold(x)) = "sigmoid"(bold(x) bold(w)^upright(T) + bold(b))
  => "predict"(bold(x)) = cases(
    hat(bold(y))(bold(x)) >= 0.5: 1,
    hat(bold(y))(bold(x)) < 0.5: 0
  )
$

== General DP-SGD

- Traditional SGD:
  + Compute $nabla L(theta)$ on random sample
  + Update $theta_"new" = theta - eta nabla L(theta)$
- Differential Privacy SGD:
  + Compute $nabla L(theta)$ on random sample
  + Clip and add noise to $nabla L(theta)$
  + Update $theta_"new" = theta - eta (nabla L(theta))$

== PPLR using DP-SGD

#algorithm(
  caption: "Diffential Privacy SGD (Outline)",
  pseudocode(
    no-number,
    [
      #v(-0.9em)
      *Input:*
        - Examples ${x_1, ..., x_N}$
        - Loss function $cal(L) = 1/N sum_i cal(L)(theta, x_i)$
        - Parameters:
          - Learning rate $eta_t$
          - Noise scale $sigma$
          - Group size $L$
          - Gradient Norm Bound $C$
    ],
    [*Initialize* $theta_0$ randomly],
    [*for* $t in [T]$ *do*], ind,
      [Take a random sample $L_t$ with sampling probability $L/N$],
      [
        *Compute Gradient*: For each $i in L_t$, 
        compute $bold(g)_t (x_i) <- nabla_(theta_t) cal(L)(theta_t, x_i)$
      ],
      [
        *Clip Gradient*:
        $macron(bold(g))_t (x_i) <- (bold(g)_t (x_i)) / max(1, norm(bold(g)_t (x_i))_2 \/ C)$
      ],
      [
        *Add Noise*:
        $macron(bold(g))_t <- 1/L (sum_i macron(bold(g))_t (x_i) + cal(N)(0, sigma^2 C^2 bold(I)))$
      ],
      [
        *Descent*:
        $theta_(t+1) <- theta_t - eta_t macron(bold(g))_t$
      ], ded,
    [
      *Output* $theta_T$ and compute the overall privacy cost $(epsilon, delta)$
      using a privacy accounting method.
    ]
  )
)

== Requirements

- (20') 填充实验代码中的缺失部分, 正确实现 DP-SGD 加噪机制
- (20') 验证不同差分隐私预算下对于模型效果的影响
  - 需要根据 $epsilon$ 和 $delta$ 计算出对应的隐私预算;
  - 探究相同的总隐私消耗量下, 不同的迭代轮数对于模型效果的影响
- 实验报告: 说明代码实现方法, 给出不同参数设置下的实验评估结果, 格式为 PDF.

= ElGamal Encryption

== The Algorithm

=== Basics

- ElGamal 加密算法是一种公钥加密算法, 由 Taher ElGamal 于 1985 年提出.
  它提供了一种保护通信隐私的方法, 允许数据在发送方使用接收方的公钥进行加密,
  并由接收方使用其私钥进行解密
- ElGamal 算法的安全性基于计算离散对数的困难性
- Reference:
  - ElGamal, Taher. "A public key cryptosystem and a signature scheme based on
    discrete logarithms." IEEE transactions on information theory 31.4 (1985):
    469-472.
  - #link("https://en.wikipedia.org/wiki/ElGamal_encryption")

=== Procedures

- Key Generation
  - 随机选择一个大素数 $p$ 和原根 $g$, 其中 $p$ 是 $g$ 的生成元
  - 随机选择私钥 $x$, 满足 $0 < x < p - 1$
  - 计算公钥 $y = g^x mod p$
  公钥为 $(p, g, y)$, 私钥为 $x$
- Encryption
  - 将明文消息 $m$ 表示为一个位于 $0$ 和 $p - 1$ 之间的整数
  - 随机选择一个临时私钥 $k$, 使得 $0 < k < p - 1$
  - 计算临时公钥 $c_1 = g^k mod p$
  - 计算临时密文 $c_2 = (y^k m) mod p$
  密文为 $(c_1, c_2)$
- Decryption
  - 利用私钥 $x$ 计算 $c_1$ 的模反演 $s = c_1^x mod p$
  - 计算明文消息 $m = (c_2 dot s^(-1)) mod p$, 其中 $s^(-1)$ 是 $s$ 的模逆元

=== Properties

/ 随机性: ElGamal 加密中使用了随机值, 相同的明文在多次加密时产生不同的密文
/ 乘法同态性: ElGamal 满足乘法同态性. 即两个密文的乘积解密后等于对应明文的乘积

== Requirements

- (30') 实现 `elgamal.py` 代码中缺失的部分函数, 保证加解密功能正确
  - 要求添加代码注释 (参考已有的注释部分)
  - 测试不同 `key_size` 设置下三个阶段的时间开销
  - 加解密的数据量可以酌情设置
- (15') 验证 ElGamal 算法的随机性以及乘法同态性质, 
  对比乘法同态性质运算的时间开销, 即 `time(decrypt([a]*[b]))` 和
  `time(decrypt([a])*decrypt([b]))`, 并给出原因说明.
- (Optional, 15') 在大数据量的场景下, 优化 ElGamal 算法加解密的时间开销.
  - 可考虑的方案: 预计算、批量加密和解密、python 并行计算
  - 请给出方案说明以及方案有效性的证明
- 实验报告: 证明代码有效性以及完成题目要求即可, 格式为 PDF.

