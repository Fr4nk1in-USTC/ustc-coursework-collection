use lab1;

# 创建一个读者借书信息的视图,该视图包含读者号、姓名、所借图书号、图书名和借期
create or replace view reader_book as
select reader_ID, Reader.name as reader_name, book_ID, Book.name as book_name, borrow_date
from Borrow join Reader on Borrow.reader_ID = Reader.ID
            join Book on Borrow.book_ID = Book.ID;

# 并使用该视图查询最近一年所有读者的读者号以及所借阅的不同图书数
select reader_ID, count(distinct book_ID) as book_num
from reader_book
where date(borrow_date) between date_sub(curdate(), interval 1 year) and curdate()
group by reader_ID;