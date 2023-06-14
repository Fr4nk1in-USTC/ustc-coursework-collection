use lab1;

# 查询没有借阅过任何一本 John 所著的图书的读者号和姓名
select ID, name
from Reader
where ID not in (
    select distinct reader_ID
    from Borrow join Book on Borrow.book_ID = Book.ID
    where Book.author = 'John'
    );