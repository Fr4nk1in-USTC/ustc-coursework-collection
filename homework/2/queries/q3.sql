select distinct sno, sname
from SC natural join Course natural join Student
where score is null and type = 0
order by sno;
