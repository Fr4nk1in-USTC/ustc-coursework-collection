use lab1;

# 查询被借阅次数最多的作者（注意一个作者可能写了多本书）
select author
from Book
group by author
order by sum(borrow_Times) desc
limit 1;