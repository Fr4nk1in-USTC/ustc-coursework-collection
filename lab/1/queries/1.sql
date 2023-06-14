use lab1;

# 查询读者 Rose 借过的读书（包括已还和未还）的图书号、书名和借期
select Book.ID, Book.name, Borrow.borrow_date
from Book join Borrow on Book.ID = Borrow.book_ID
          join Reader on Borrow.reader_ID = Reader.ID
where Reader.name = 'Rose';