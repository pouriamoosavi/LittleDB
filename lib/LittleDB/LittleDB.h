#ifndef LITTLE_DB_H
#define LITTLE_DB_H 

#include <Arduino.h>
#include "FS.h"
#include <LITTLEFS.h>

// dbs
const String CREATE_DB = "create db ";
const String DROP_DB = "drop db ";
const String CONNECT_DB = "use db ";

// tables
const String CREATE_TBL = "create table ";
const String DROP_TBL = "drop table ";
const String ALTER_TBL = "alter table ";

const String ALTER_TBL_ADD = "add column ";
const String ALTER_TBL_ALTER = "alter column ";
const String ALTER_TBL_DROP = "drop column ";

// rows
const String SELECT = "select from ";
const String UPDATE = "update ";
const String DELETE = "delete from ";
const String INSERT = "insert into ";

// others
const String FORMAT_FS = "drop all dbs and format fs ";
const String COMPACT = "compact table ";

// database and tables config
extern String CONNECTED_DB;

const String prefix = "/"; // must have / at the end
const int dbNameLimit = 31;
const int tblNameLimit = 29;

// cel types and length
const String CELL_TYPE_ID = "id";
const uint8_t CELL_TYPE_ID_LEN = 16;
const String CELL_TYPE_INT = "int";
const uint8_t CELL_TYPE_INT_LEN = 4;
const String CELL_TYPE_TINYINT = "tinyint";
const uint8_t CELL_TYPE_TINYINT_LEN = 1;
const String CELL_TYPE_TEXT = "text";

// responses
const int8_t RES_OK = 0;
const int8_t RES_EMPTY = -4;
const int8_t RES_SYSTEM_ERR = -1;
const int8_t RES_USER_ERR = -2;
const int8_t RES_NOT_IMPLEMENTED = -3;

// options
const uint8_t OPT_DEFAULT = 0b00000000;
const uint8_t OPT_DELETED = 0b00000001;

// structs for select and insert
struct InsertData_t {
  uint16_t usedLen;
  uint16_t len;
  byte bytes[];
};
extern InsertData_t* insertData;

struct SelectData_t {
  uint16_t len;
  char tblName[tblNameLimit];
  byte bytes[];
};
extern SelectData_t* selectData;

// work with files ===============================
void listDir(fs::FS &fs, String dirname, uint8_t levels);
int8_t createDir(fs::FS &fs, String path);
int8_t dropDir(fs::FS &fs, String path);
int8_t createFile(fs::FS &fs, String path);
int8_t writeFile(fs::FS &fs, String path, String message);
int8_t appendInsertData(fs::FS &fs, String path);
int8_t deleteFile(fs::FS &fs, String path);
int8_t renameFile(fs::FS &fs, String from, String to);
// prints ===============================
void printArray(byte *input, short len);
void printInsertData();
void printSelectData();
// geters ===============================
String getText(SelectData_t* selectData, String colName);
int32_t getInt32(SelectData_t* selectData, String colName);
// executes ===============================
int8_t deleteRowWithID(File tblFile, String id);
int8_t findRowWithID(File tblFile, String tblName, String id);
int8_t insertDataToBytes(File schemFile, String tblName, String queryValues);
int8_t updateRowWithID(File schemFile, File tblFile, String tblName, String colName, String colNewValue, String idValue);
int8_t compactTable(File oldTblFile, String newTblPath);
// dbs ===============================
int8_t execCreateDB(String query);
int8_t execDropDB(String query);
int8_t execConnectToDB(String query);
// tables ===============================
int8_t execCreateTbl(String query);
int8_t execDropTbl(String query);
int8_t execAlterTbl(String query);
// rows ===============================
int8_t execSelect(String query);
int8_t execUpdate(String query);
int8_t execDelete(String query);
int8_t execInsert(String query);
// others ===============================
int8_t execCompact(String query);
int8_t execFormat(String query);
int8_t execQuery(String query);

#endif