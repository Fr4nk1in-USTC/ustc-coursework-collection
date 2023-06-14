select sname
from SC natural join Course natural join Student
where type = 0
group by sno
having sum(credit) > 16 and avg(score) >= 75;