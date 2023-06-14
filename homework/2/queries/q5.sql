-- 若存在学生重修了通识课, 使用最高分作为其成绩:
select sno, sname
from (select sno, sname, score
        from (select cno, sno, sname from Course, Student
            where type = 2) as full_sc
            natural left outer join
            (select sno, cno, max(score) as score from SC
            group by sno, cno) as max_sc
        group by sno, cno) as filtered_sc
group by sno
having min(score) >= 60 and count(*) = count(score);
