use lab1;

# 查询借阅图书数目超过 10 本的读者姓名
select name
from Reader join Borrow on Reader.ID = Borrow.reader_ID
group by Reader.ID
having count(*) > 10;