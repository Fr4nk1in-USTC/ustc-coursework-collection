#import "homework.typ": *

#show: homework.with(number: 1)

#question("Sch0.1")

```par
begin
  for 1 <= i <= p par-do
    B[i, 1] = 1
  end for
  for 1 <= i, j <= p par-do
    if B[i, 1] = 1 and A[i] > A[j] then
      B[i, 1] = 0
    end if
  end for
end
```

#question("Ex5.6")

+ 全局读写时间为 $d$, 同步障时间为 $B(p)$.
  #set enum(numbering: (..nums) => "(" + nums.pos().slice(1).map(str).join(".") + ")", full: true)
  + $O(n/p + d)$
  + $B(p)$
  + $ceil(log_B(p(B-1)) + 1) - 1$ 次迭代
    + $O(B d)$
    + $B(p)$
  总共 $O((B d + B(p))log_B p + n/p)$.
+ Barrier 语句确保每个处理器计算完局和并写入 SM 后, 局和才被读取, 避免脏读.

#question("Ex5.7")

+ 忽略传输建立时间, 同步障的时间显然是 $O(L)$ 的.
  #set enum(numbering: (..nums) => "(" + nums.pos().slice(1).map(str).join(".") + ")", full: true)
  + $O(n/p + g)$
  + $O(L)$
  + $ceil(log_d(p(d-1))+1)-1$ 次迭代
    + $O(g(d+1)+d) = O(g d)$
    + $O(L)$
  总共 $O((g d + L)log_d p + n/p)$.
+ 首先 BSP 模型的一个超级步中一个处理器最多可以传送 $h$ 条消息 ($L >= g h$), 
  而在 (3.1) 中, 一个处理器要发送/接收 $d + 1$ 条消息, 因此 $d$ 要满足 $d<=h-1$;
  其次, 考虑到时间性能, 应选择使时间性能尽可能好, 同步障时间尽可能小的 $d<=h-1$
  值.
