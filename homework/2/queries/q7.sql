select distinct sno, sname, cno, cname
from (select sno, cno, min(term) as first_term
      from SC
      group by sno, cno
      having count(*) > 1) as multiple_enroll
     natural join SC natural join Student natural join Course
where score < 60 and term > first_term;