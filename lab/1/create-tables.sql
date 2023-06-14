create database if not exists lab1;
use lab1;

create table if not exists Book (
    ID char(8) primary key,
    name varchar(10) not null,
    author varchar(10),
    price float,
    status int check (status in (0, 1, 2)),
    borrow_Times int default 0,
    reserve_Times int default 0
);

create table if not exists Reader (
    ID char(8) primary key,
    name varchar(10),
    age int,
    address varchar(20)
);

create table if not exists Borrow (
    book_ID char(8) references Book(ID),
    reader_ID char(8) references Reader(ID),
    borrow_Date date,
    return_Date date,
    primary key (book_ID, reader_ID, borrow_Date)
);

create table if not exists Reserve (
    book_ID char(8),
    reader_ID char(8),
    reserve_Date date,
    take_Date date,
    primary key (book_ID, reader_ID, reserve_Date),
    check (take_Date >= reserve_Date)
);
