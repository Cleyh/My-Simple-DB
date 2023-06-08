# SimpleDBS

-----

This is my programming language design assignment in my first year of university. The code is not mature enough, not functional enough, and still has many bugs, even some of the code is written quite irregularly.

-----

## Introducing

- SimpleDB uses a lot of json format files to store data, making the data format simple and clear, easy to modify, and at the same time simpler in programming. (Although I know this is not standardized, and the data is easy to modify also makes it easy to be accidentally destroyed by the user)

- SimpleDB supports command operation, just enter the command, the database can execute the corresponding operation. And it provides a command api, which can facilitate the user to expand the command more easily.

- SimpleDB supports multiple connections and can, to a certain extent, be able to bring thread conflicts when multiple connections. When a user selects a database, other users can't delete it. (Of course, this piece is not perfect, there are still some apparent thread conflicts not solved)

- SimpleDB takes the measure of delayed loading (please allow me to describe it like this). When you run the database, it does not read out all the data immediately, but first gets the basic information about the database, such as the name, the list it contains, and so on. When the user selects a table in a specific database, only then does the program read the data from the table and perform the operation.

-----
## Command

**Sys command**
The use of these instructions requires the database and list to be selected before the operation can be performed, and the `CLOSE` related instruction needs to be called to save after the operation is completed.

`ADD [add_name] [add_head]` Add a table
`ADD -R [data]/-C [head] [data]` Add a row or column (no data is empty by default) 

`DEL [del_name]` Delete a list
`DEL -R [row]/-C [col]` Delete an entire row or column

`INS -R [row] [data]` Insert a row
`INS -C [col] [head] [data]` Insert a column

`SET [row] [col] [data]` Set a cell of data
`SET -R [row] [data]` Set row data
`SET -C [col] [data] [from] [to]` set the column information from the from row to the to row (if the data length is not enough, then the more is filled as empty), if the to parameter is not set, then it starts from the from, according to the data length.

`GET -DB/-LS` Get database or table information
`GET -R [row]/-C [col]/[row] [col]` Get an entire row/column/cell of data

`SEL -DB [name]/-LS [name]` Select the database/table 
`CLOSE -DB/-LS` Close the database/table
`CLOSE` Close all

`CREATE [name]` Create database
` REMOVE [name]` Delete the database

Example:
`SEL -DB 0`
`ADD student "ID", "NAME", "CLASS", "SCORE"`
`SLE -LS student` (here you can also select by serial number)
`ADD -R "17", "xiaomin", "19c229", "100"`
`ADD -C sex "0", "1", "1", "1", "1", "1"`
`CLOSE`

The above will display the content as a command line and will not send data to the client (a "\n" character will be sent).

**query command**

This type of command is an immediate query operation and does not require database selection or list operation. It is suitable for remote application queries.

`query -get [DB] [LS] -ALL` Query all the data of a table in a library
`query -info` Get all database information, including database name, list of tables



`query -get [DB] [LS] -R [ROW]` look up a row
`query -get [DB] [LS] -Rt [ROW] [ROW]` look up a row to a row
`query -get [DB] [LS] -R [ROW] [ROW] [ROW]` ... Look up a certain row and a certain row and a certain row and...



`query -get [DB] [LS] -C [COW]` look up a column
`query -get [DB] [LS] -Ct [COW] [COW]` look up a column to a column
`query -get [DB] [LS] -C [COW] [COW] [COW]` ... Checking certain columns (same as row checking)



`query -get [DB] [LS] -Q [head] [string]` Find all rows in a list with string data under the head column
`query -where [DB] [LS] -Q [head] [string]` Find the row numbers of all rows in a list with string data under the head column

The above will return the information in json format.

-----

## How to use it?

This project uses some libraries, namely `nlohmann json`, `boost_asio_1_28_0`.
When you try to edit or built this project, you should first reference the `nlohmann json`, `boost_asio_1_28_0` and `boost_1_82_0` libraries.

-----

## File structure

In this repos:
- `cmd.h`, `cmd_factory.h` and its cpp files make up the basic command api.
  `cmd_sys.h`,`cmd_query.h` are two type of command I created.

- `DB_server.h`, `DB_engine.h` and its cpp files are the core files of this software.
  - `DB_engine.h` includes two classes, *DB_LIST* and *DB_engine*:
    - `DB_LIST` is a class for manipulating table, or it is the table itself.
    - `DB_engine` is a class for manipulating *DB_LIST*, and its itself is a single database.
  - `DB_server.h` includes a class, *DB_server*, it manipulates *DB_engine*.

this software has such file structure:
```
root
├─config
|   └─db_list_config.json   //this file was the basic config that software will load it at first
└─db                        /this floder stores the database customer created.
    ├─sdm_basic             
    |   ├─staff_list.txt    
    |   ├─...txt
    |   └─db_config.json    
    ├─sdm_basic_group       //a database customer created.
    |   ├─staff_group.txt   //data file.
    |   ├─...txt
    |   └─db_config.json    //database config.
    └─sdm_fast_query
        ├─tag_list.txt
        └─db_config.json
```

-----

## The data in json format would return

error return:
```
{
    "type":"error",
    "data":[
        ["code","description"]
    ]
}
```

data return:
```
{
    "type":"data",                                  //return type 
    "data":[
        ["name","age","country","car","money"],     //row 0: head of data
        ["xiaomin","20","USA","dianpin","20000"],   //row 1: data 
        ["jack","19","UK","sadsf","1234345"]        //row 2: data 
    ]
}
```
basic format：
```
{
    "tpye":"the data type",
    "data":[
        ["xxx","yyy","zzz","..."],
        ["data1","data2","data3","..."],
        ["data1","data2","data3","..."]
    ]
}
```

-----
## KNOWING BUGS

- Thread conflicts may occur when multiple users connect.
- The program crashes when a user modifies a configuration file or data file, resulting in incorrect formatting, incomplete data, etc. (this bug is easy to fix, it just takes time).
- The code is in a mess
- ...

## FUTHUR FEATURES

- Add a user and permission system that requires a username and password, or Access token, to connect to the database
- Add a command queue and the ability to intelligently adjust for command conflicts between multiple users and execute commands asynchronously
- more...
