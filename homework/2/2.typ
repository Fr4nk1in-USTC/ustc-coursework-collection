#import "homework.typ": *

#show: homework.with(number: 2)

#question()

外模式对应视图, 模式对应基本表, 内模式对应存储文件.

/ 外模式 (视图): `create view` / `drop view`;
/ 模式 (基本表): `create table` / `alter table` / `drop table`;
/ 内模式 (文件): `create index` / `drop index`.

#question()

#set enum(numbering: "(1)")
+ ```sql
  -- 学生表
  create table Student(
      sno varchar(10) primary key,
      sname varchar(50) not null,
      gender varchar(1) check (gender in ('m', 'f')),
      birthdate date
  );
  -- 课程表
  create table Course(
    cno varchar(10) primary key,
    cname varchar(50) not null,
    type int default 0 check (type >= 0 and type <= 3),
    credit decimal
  );
  -- 选课表
  create table SC(
    sno varchar(10),
    cno varchar(10),
    score decimal check (score >= 0 and score <= 100),
    term int check (term >= 1 and term <= 8),
    primary key(sno, cno, term),
    foreign key(sno) references Student(sno),
    foreign key(cno) references Course(cno)
  );
  ```
+ #set enum(numbering: "1.1)")
  + ```sql
    select birthdate
    from Student
    where sname = '张三';
    ```
  + ```sql
    select sno, sname, gender
    from Student
    where sname like '李%';
    ```
  + ```sql
    select distinct sno, sname
    from SC natural join Course natural join Student
    where score is null and type = 0
    order by sno;
    ```
  + ```sql
    select sname
    from SC natural join Course natural join Student
    where type = 0
    group by sno
    having sum(credit) > 16 and avg(score) >= 75;
    ```
  + ```sql
    -- 若存在学生重修了通识课, 使用最高分作为其成绩:
    select sno, sname
    from (select sno, sname, score
          from (select cno, sno, sname
                from Course, Student
                where type = 2) as full_sc
               natural left outer join
               (select sno, cno, max(score) as score
                from SC
                group by sno, cno) as max_sc
          group by sno, cno) as filtered_sc
    group by sno
    having min(score) >= 60 and count(*) = count(score);
    ```
  + ```sql
    select cno, cname, type, avg_score, fails / selected as fail_rate
    from (select cno, cname, type, avg(score) as avg_score,
                 count(score) as selected
          from Course natural join SC
          group by cno) as course_stats
         natural left outer join
         (select cno, count(*) as fails
          from Course natural join SC
          where score < 60
          group by cno) as fail_courses
    order by (case type
                  when 2 then 0
                  when 0 then 1
                  when 1 then 2
                  when 3 then 3
              end);
    ```
  + ```sql
    select distinct sno, sname, cno, cname
    from (select sno, cno, min(term) as first_term
          from SC
          group by sno, cno
          having count(*) > 1) as multiple_enroll
        natural join SC natural join Student natural join Course
    where score < 60 and term > first_term
    ```

#question()

```sql
select D
from (select *
      from (select * from R where p) as R_p
           natural join
           (select * from S where m) as S_m
      where q) as R_pS_m
     natural join
     T
```
