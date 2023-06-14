-- 学生表
Create Table Student(
    sno Varchar(10) Primary Key,
    sname Varchar(50) Not Null,
    gender Varchar(1) Check (gender in ('M', 'F')),
    birthdate Date
);
-- 课程表
Create Table Course(
   cno Varchar(10) Primary Key,
   cname Varchar(50) Not Null,
   type Int Default 0 Check (type >= 0 and type <= 3),
   credit Decimal
);
-- 选课表
Create Table SC(
   sno Varchar(10),
   cno Varchar(10),
   score Decimal Check (score >= 0 and score <= 100),
   term Int Check (term >= 1 and term <= 8),
   Primary Key(sno, cno, term),
   Foreign Key(sno) References Student(sno),
   Foreign Key(cno) References Course(cno)
);
