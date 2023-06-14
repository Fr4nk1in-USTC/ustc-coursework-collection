use lab1;

# 查询从没有借过图书也从没有预约过图书的读者号和读者姓名
select ID, name
from Reader
where ID not in (select distinct reader_ID from Borrow) and
      ID not in (select distinct reader_ID from Reserve)