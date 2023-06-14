use lab1;

# Forbidden operations

## Try to borrow a reserved book without reservation
call borrowBook('r1', 'b10', @state);
select @state, @info;

## A reader who has borrowed 3 books tries to borrow another book
call borrowBook('r23', 'b7', @state);
select @state, @info;

## Try to borrow an unavailable book
call borrowBook('r1', 'b1', @state);
select @state, @info;

# Allowed operations

## Borrow a book without reservation
call borrowBook('r1', 'b7', @state);
select @state, @info;

## Borrow a book with reservation
call borrowBook('r20', 'b10', @state);
select @state, @info;