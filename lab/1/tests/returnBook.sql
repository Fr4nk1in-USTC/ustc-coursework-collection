use lab1;

# Forbidden operations

## The reader has not borrowed the book
call returnBook('r1', 'b1', @state);
select @state, @info;

# Allowed operations

call returnBook('r14', 'b1', @state);
select  @state, @info;