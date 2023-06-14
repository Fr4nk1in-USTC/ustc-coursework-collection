use lab1;

# Forbidden operations

## Old reader ID doesn't exist
call updateReaderID('r0', 'r25', @state);
select @state, @info;

## New reader ID already exists
call updateReaderID('r1', 'r2', @state);
select @state, @info;

# Allowed operations
call updateReaderID('r1', 'r24', @state);
select @state, @info;

call updateReaderID('r24', 'r1', @state);