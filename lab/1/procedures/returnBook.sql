use lab1;

drop procedure if exists returnBook;
create procedure returnBook(in rid char(8), in bid char(8), out state int)
    modifies sql data
begin
    declare reader_exists, book_exists, borrow_exists bool default false;
    declare book_reserved bool default false;
    declare new_status int default 0;

    # s:
    #     0: able to return
    #     1: reader not found
    #     2: book not found
    #     3: the reader has not borrowed this book
    #     4: SQL warning
    #     5: SQL exception
    declare s int default 0;
    declare continue handler for sqlwarning set s = 4;
    declare continue handler for sqlexception set s = 5;

    start transaction;

    select exists(select * from Reader where ID = rid) into reader_exists;
    select exists(select * from Book where ID = bid) into book_exists;

    if not reader_exists then
        set s = 1;
    elseif not book_exists then
        set s = 2;
    else
        select exists(select *
                      from Borrow
                      where reader_ID = rid
                        and book_ID = bid
                        and return_Date is null)
        into borrow_exists;
        if borrow_exists then
            select exists(select *
                          from Reserve
                          where reader_ID = rid
                            and book_ID = bid)
            into book_reserved;
            if book_reserved then
                set new_status = 2;
            else
                set new_status = 0;
            end if;
        else
            set s = 3;
        end if;
    end if;

    if s = 0 then
        update Borrow
        set return_Date = curdate()
        where reader_ID = rid
          and book_ID = bid
          and return_Date is null;
        update Book
        set status = new_status
        where ID = bid;
    end if;

    if s = 0 then
        set state = 0;
        set @info = concat('Reader ', rid, ' returned book ', bid, '.');
        commit;
    else
        case s
            when 1 then set state = 1;
                        set @info = concat('Reader ', rid, ' not found.');
            when 2 then set state = 2;
                        set @info = concat('Book ', bid, ' not found.');
            when 3 then set state = 3;
                        set @info = concat('Reader ', rid,
                                           ' has not borrowed Book ', bid,
                                           '.');
            when 4 then set state = 4;
                        set @info = 'SQL warning.';
            when 5 then set state = 5;
                        set @info = 'SQL exception.';
            end case;
        rollback;
    end if;
end;