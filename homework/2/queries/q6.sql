select cno, cname, type, avg_score, fails / selected as fail_rate
from (select cno, cname, type, avg(score) as avg_score, count(score) as selected
      from Course
               natural join SC
      group by cno) as course_stats
     natural left outer join
     (select cno, count(*) as fails
      from Course
               natural join SC
      where score < 60
      group by cno) as fail_courses
order by (case type
              when 2 then 0
              when 0 then 1
              when 1 then 2
              when 3 then 3
          end);