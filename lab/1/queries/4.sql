use lab1;

# 查询目前借阅未还的书名中包含 “MySQL” 的的图书号和书名
select book_ID, name
from Book join Borrow on Book.ID = Borrow.book_ID
where Borrow.return_Date is null and
      Book.name like '%MySQL%';