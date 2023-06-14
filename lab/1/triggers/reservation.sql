use lab1;

drop trigger if exists new_reservation;
create trigger new_reservation after insert on Reserve for each row
begin
    declare old_status int default 0;
    declare old_reserve_times int default 0;

    select status
    from Book
    where ID = new.book_ID
    into old_status;

    select reserve_Times
    from Book
    where ID = new.book_ID
    into old_reserve_times;

    if old_status != 1 then
        update Book
        set status = 2
        where ID = new.book_ID;
    end if;

    update Book
    set reserve_Times = old_reserve_times + 1
    where ID = new.book_ID;
end;

drop trigger if exists cancel_reservation;
create trigger cancel_reservation after delete on Reserve for each row
begin
    declare old_reserve_times int default 0;
    declare old_status, new_status int default 0;

    select reserve_Times
    from Book
    where ID = old.book_ID
    into old_reserve_times;

    select status
    from Book
    where ID = old.book_ID
    into old_status;

    if old_status = 1 then
        set new_status = 1;
    elseif old_reserve_times <= 1 then
        set new_status = 0;
    else
        set new_status = 2;
    end if;

    update Book
    set status = new_status, reserve_Times = old_reserve_times - 1
    where ID = old.book_ID;
end;