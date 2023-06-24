use lab3;

create table if not exists Teacher (
    id varchar(5) not null,
    name varchar(255) not null,
    sex int not null check (sex in (1, 2)),
    title int not null check (title <= 11 and title >= 1),
    primary key (id)
);

create table if not exists Paper (
    id int not null auto_increment,
    title varchar(255) not null,
    source varchar(255) not null,
    publish_date date not null,
    type int not null check (type in (1, 2, 3, 4)),
    level int not null check (level in (1, 2, 3, 4, 5, 6)),
    primary key (id)
);

create table if not exists Project (
    id varchar(255) not null,
    name varchar(255) not null,
    source varchar(255) not null,
    type int not null check (type in (1, 2, 3, 4, 5)),
    fund float not null check (fund >= 0),
    start_year int not null check (start_year >= 0),
    end_year int not null,
    primary key (id),
    check (start_year <= end_year)
);

create table if not exists Course (
    id varchar(255) not null,
    name varchar(255) not null,
    credit int not null check (credit >= 0),
    type int not null check (type in (1, 2)),
    primary key (id)
);

create table if not exists Teacher_Paper (
    teacher_id varchar(5) not null,
    paper_id int not null,
    `rank` int check (`rank` >= 1),
    is_corresponding_author boolean not null,
    primary key (teacher_id, paper_id),
    foreign key (teacher_id) references Teacher (id),
    foreign key (paper_id) references Paper (id)
);

create table if not exists Teacher_Project (
    teacher_id varchar(5) not null,
    project_id varchar(255) not null,
    `rank` int check (`rank` >= 1),
    fund float not null check (fund >= 0),
    primary key (teacher_id, project_id),
    foreign key (teacher_id) references Teacher (id),
    foreign key (project_id) references Project (id)
);

create table if not exists Teacher_Course (
    teacher_id varchar(5) not null,
    course_id varchar(255) not null,
    term int not null check (term in (1, 2, 3)),
    credit int not null check (credit >= 0),
    primary key (teacher_id, course_id),
    foreign key (teacher_id) references Teacher (id),
    foreign key (course_id) references Course (id)
);
