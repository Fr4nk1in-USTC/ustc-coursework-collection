use lab1;

# 查询 2022 年借阅图书数目排名前 10 名的读者号、姓名以及借阅图书数
select reader_ID, name, count(*) as book_num
from Reader join Borrow on Reader.ID = Borrow.reader_ID
where year(Borrow.borrow_date) = 2022
group by reader_ID
order by book_num desc
limit 10;