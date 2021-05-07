# Little Sql like DB for ESP32 boards
A little Sql like database, which runs on esp32. This library works with esp32 file system with help of LittleFS which is a great library to manage fs in embed boards and esp. 

## Queries
Execute a query by calling `execQuery` function with one of these queries as input. `execQuery` will return a number which indicates the result of operation (refer to [Responses](#Responses)).
- #### CREATE_DB = `create db [DB_NAME]`
  Creates a database with this name, Databases are directories inside root file system.

- #### DROP_DB `drop db [DB_NAME]` _(Caution)_
  Drop a database (Delete it's directory).

- #### CONNECT_DB `use db [DB_NAME]`
  Connect to a DB,Must be called before all bellow queries

- #### CREATE_TBL `create table [TABLE_NAME] ([SCHEMA])`
  Create a table with this schema. Schema must have only valid column types. Tables are files inside DB directory.

- #### DROP_TBL `drop table [TABLE_NAME]`  _(Caution)_
  Drop a table (Delete it's file).

- #### ALTER_TBL `alter table [TABLE_NAME]` _(Not yet implemented)_
  Not yet implemented.

- #### SELECT `select from [TABLE_NAME] where [COLUMN_NAME]=[CONDITION]`
  Selects one row and write result into memory. A global variable with name [selectData](#selectData) and type `SelectData_t` will be created. It will be declared with corresponding row. Although it is possible to select base on any field, selecting by `id` is more optimal base on both memory and time.

- #### UPDATE `update [TABLE_NAME] set [COLUMN_NAME]=[VALUE] where id=[CONDITION]`
  Update one row and set a new value for one column. It tries to cast value into corresponding column's type. Because of write limit on esp memories, this query perform one delete and one insert underneath. For now it is not possible to update base on other fields than `id`

- #### DELETE `delete from [TABLE_NAME] where id=[CONDITION]`
  Delete one row. Because of write limit on esp memories this query just set one bit, which then will be use to detect if this row is deleted. For now it is not possible to delete base on other fields than `id`

- #### INSERT `insert into [TABLE_NAME] values ([VALUE_1], [VALUE_2], [VALUE_3], ...)`
  Insert one row into table. A global variable with name [insertData](#insertData) and type `InsertData_t` will be created. it will be declared with corresponding row. This query tries to cast values into corresponding column's types. values must be in the same order as schema.

- #### FORMAT_FS `drop all dbs and format fs ` _(Caution)_
  Delete everything and format fs.

- #### COMPACT `compact table [TABLE_NAME]`
  As we mentioned delete query just mark deleted rows as deleted. This query copy all table rows except deleted rows into another table with the same name. This operation makes a lot of write into fs and it is better to be avoid.

- #### getText(selectDate: SelectData_t, columnName: String | char*): String
  Get a text base on column name from selectedData and returns it.

- #### getInt32(selectDate: SelectData_t, columnName: String | char*): int32_t
  Get an 32 bit int base on column name from selectedData and returns it.

## Responses
| Response Number |    Response Code    |                                         Description                                         |
|---------------|-------------------|-------------------------------------------------------------------------------------------|
| 0               | RES_OK              | Everything works fine. (e.g: if you are selecting [selectData](#selectData) is now initialized with selected row.)                                                                |
| -1              | RES_SYSTEM_ERR      | There is an error that isn't user fault. (e.g: failed to allocate memory to variable)       |
| -2              | RES_USER_ERR        | There is an error that is user's fault. (e.g: wrong syntax)                                 |
| -3              | RES_NOT_IMPLEMENTED | This function does not still work.                                                         |
| -4              | RES_EMPTY           | There is no result for this operation. (e.g: `delete` or `select` criteria does not match any row) |

## Schema types
|    Name   |   Type   |  Length  |                 Example                |                    Limitation                    |
|---------|--------|--------|--------------------------------------|------------------------------------------------|
| `id`      | `String` | 16 byte  | "407bc45f-e10a-4175-ad4f-940df30ee87c" | String between 1 to 16 characters                |
| `int`     | `Number` | 4 byte   | 158                                    | Integer between -2,147,483,647 and 2,147,483,647 |
| `tinyint` | `Number` | 1 byte   | 1                                      | Integer between -128 and 128                     |
| `text`    | `String` | variable | "Hello World with any length you wish" | String between 0 to 65,536 characters            |

## Global Variables
### selectData: SelectData_t
```
uint16_t len;
char tblName[29];
byte bytes[];
```
Will be overwrite with next `select` and `update`

### insertData: InsertData_t
```
uint16_t usedLen;
uint16_t len;
byte bytes[];
```
Will be overwrite with next `insert` and `update`

## Notices and Considerations
- Table names length can't be more than 29 character.
- Database names length can't be more than 31 character.
- Rows length can't be more than 65,536 bytes (64 KB or 65,536 characters in total). 
- There is only one `selectData` and one `insertData`, so they will overwrite in next operation. Refer to [selectData](#selectData) and [insertData](#insertData) for more information.

## Examples and tests
Refer to [test](test) folder.

## Run Tests
### In Arduino IDE
- Clone project from this repo.
- Extract it.
- Move files from lib, test and src directories into one arduino project.
- Rename main.cpp into YOUR_PROJECT_NAME.ino (in Arduino IDE, .ino file and project must have the same name).
- In Arduino IDE Open File > Preferences
Add `http://arduino.esp8266.com/stable/package_esp8266com_index.json, https://dl.espressif.com/dl/package_esp32_index.json` Into `Additional Board Manager URLs`.
- In Arduino IDE Open Tools > Board > Board Manager...
Search for `esp32` and install the `esp32` package by `Espressif Systems`.
- In Arduino IDE  Open Tools > Manage libraries
Search for `littlefs` and change `Topic` to `data Storage`
- Connect your esp board and run test.

### In Platform.&#46;IO
- Clone project from this repo.
- Extract it.
- Open containing folder with VSCode with Platform.&#46;IO installed.
- Add esp32 library from Platform.&#46;IO library manager. (Wemos lolin32).
- Clone <a href="https://github.com/lorol/LITTLEFS">LITTLEFS project</a> from git and put it inside PROJECT_DIR/.pio/libdeps/lolin32/- LittleFS_esp32.  (I couldn't run project after adding LITTLEFS from Platform.&#46;IO library manager)
- Connect your esp board and run test.

## Usage
To use this library in your own project you need to add `/lib/LittleDB` directory to your project. Usually this can be done by:
- Copy `/lib/LittleDB` where your IDE (Arduino IDE, VSCode, etc) can see.
- include it into your code `#include <LittleDB.h>`.
- Now you can call `execQuery` function with your desired queries.

## Dependencies
- LittleFS
