use lab1;

# 设计一个存储过程 borrowBook, 当读者借书时调用该存储过程完成借书处理
#   同一天不允许同一个读者重复借阅同一本读书
#   如果该图书存在预约记录，而当前借阅者没有预约，则不允许借阅
#   一个读者最多只能借阅 3 本图书，意味着如果读者已经借阅了 3 本图书并且未归还则不允许再借书
#   如果借阅者已经预约了该图书，则允许借阅，但要求借阅完成后删除借阅者对该图书的预约记录
#   借阅成功后图书表中的 times 加 1
#   借阅成功后修改 status
drop procedure if exists borrowBook;
create procedure borrowBook(in rid char(8), in bid char(8), out state int)
    modifies sql data
begin
    declare reader_exists, book_exists bool default false;
    declare book_status int default 0;
    declare reader_last_borrow_date date default null;
    declare book_reserved_by_reader bool default false;
    declare books_reader_borrowed int default 0;

    # s:
    #   0: the reader is able to borrow the book without reservation
    #   1: the reader is able to borrow the book with reservation
    #   2: reader not found
    #   3: book not found
    #   4: the book is not available
    #   5: the reader has already borrowed the book today
    #   6: the reader has no reservation for the book but the book is reserved
    #   7: the reader has already borrowed 3 books
    #   8: SQL warning
    #   9: SQL exception
    declare s int default 0;

    declare continue handler for sqlwarning set s = 8;
    declare continue handler for sqlexception set s = 9;

    start transaction;

    select exists(select * from Reader where ID = rid) into reader_exists;
    select exists(select * from Book where ID = bid) into book_exists;

    if not reader_exists then
        set s = 2;
    elseif not book_exists then
        set s = 3;
    else
        select status from Book where ID = bid into book_status;

        select max(borrow_date)
        from Borrow
        where reader_ID = rid and book_ID = bid
        into reader_last_borrow_date;

        select exists(
            select *
            from Reserve
            where reader_ID = rid and book_ID = bid
        ) into book_reserved_by_reader;

        select count(*)
        from Borrow
        where reader_ID = rid and return_date is null
        into books_reader_borrowed;

        if reader_last_borrow_date = curdate() then
            set s = 5;
        elseif books_reader_borrowed >= 3 then
            set s = 7;
        elseif book_status = 0 then # not reserved or borrowed
            set s = 0;
        elseif book_status = 1 then # borrowed
            set s = 4;
        elseif book_status = 2 then # reserved
            if book_reserved_by_reader then
                set s = 1;
            else
                set s = 6;
            end if;
        end if;
    end if;

    if s < 2 then
        insert into Borrow values (bid, rid, curdate(), null);
        update Book set status = 1 where ID = bid;
        update Book set borrow_Times = borrow_Times + 1 where ID = bid;
        if s = 1 then
            delete from Reserve where book_ID = bid and reader_ID = rid;
        end if;
    end if;

    if s < 2 then
        case s
            when 0 then set state = 0;
                        set @info = concat('Book ', bid,
                                           ' borrowed by reader ', rid,
                                           ' without reservation.');
            when 1 then set state = 1;
                        set @info = concat('Book ', bid,
                                           ' borrowed by reader ', rid,
                                           ' with reservation.');
            end case;
        commit;
    else
        case s
            when 2 then set state = 2;
                        set @info = concat('Reader ', rid, ' not found.');
            when 3 then set state = 3;
                        set @info = concat('Book ', bid, ' not found.');
            when 4 then set state = 4;
                        set @info = concat('Book ', bid,
                                           ' is not available now.');
            when 5 then set state = 5;
                        set @info = concat('Reader ', rid,
                                           ' has already borrowed book ', bid,
                                           ' today.');
            when 6 then set state = 6;
                        set @info = concat('Reader ', rid,
                                           ' has no reservation for book ',
                                           bid, ' but the book is reserved.');
            when 7 then set state = 7;
                        set @info = concat('Reader ', rid,
                                           ' has already borrowed 3 books.');
            when 8 then set state = 8;
                        set @info = concat('SQL warning.');
            when 9 then set state = 9;
                        set @info = concat('SQL exception.');
            end case;
        rollback;
    end if;
end;