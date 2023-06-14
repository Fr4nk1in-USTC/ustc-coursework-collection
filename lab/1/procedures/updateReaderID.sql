use lab1;

# 设计一个存储过程 updateReaderID, 实现对读者表的 ID 的修改
drop procedure if exists updateReaderID;
create procedure updateReaderID(in old_id char(8), in new_id char(8),
                                out state int)
    modifies sql data
begin
    declare old_id_exists, new_id_exists bool default false;

    # s:
    #   0: able to update
    #   1: old_id not found
    #   2: new_id exists
    #   3: SQL warning
    #   4: SQL exception
    declare s int default 0;
    declare continue handler for sqlwarning set s = 3;
    declare continue handler for sqlexception set s = 4;

    start transaction;
    select exists(select * from Reader where ID = old_id) into old_id_exists;
    select exists(select * from Reader where ID = new_id) into new_id_exists;

    if s = 0 then
        if not old_id_exists then
            set s = 1;
        elseif new_id_exists then
            set s = 2;
        else
            update Reader set ID = new_id where ID = old_id;
            update Borrow set reader_ID = new_id where reader_ID = old_id;
            update Reserve set reader_ID = new_id where reader_ID = old_id;
        end if;
    end if;

    if s = 0 then
        set state = 0;
        set @info = 'Update success';
        commit;
    else
        case s
            when 1 then set state = 1;
                        set @info = concat('Reader ', old_id, ' not found');
            when 2 then set state = 2;
                        set @info = concat('Reader ', new_id,
                                           ' already exists');
            when 3 then set state = 3;
                        set @info = 'SQL warning';
            when 4 then set state = 4;
                        set @info = 'SQL exception';
            end case;
        rollback;
    end if;
end;
