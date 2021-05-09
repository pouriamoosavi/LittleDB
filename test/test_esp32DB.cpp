#include <Arduino.h>
#include "FS.h"
#include <LITTLEFS.h>
#include <unity.h>

#include <LittleDB.h>

void create_db() {
  int8_t res = execQuery("create db test_db");
  TEST_ASSERT( res == RES_OK );
  TEST_ASSERT( LITTLEFS.exists("/test_db") == true );
}

void use_db() {
  int8_t res = execQuery("use db test_db");
  TEST_ASSERT( res == RES_OK );
  TEST_ASSERT( CONNECTED_DB == "test_db" );
}

void create_table() {
  String schem = "id id, age int, ready tinyint, name text, zip int, roomName text";
  schem.trim();
  int8_t res = execQuery("create table test_tbl (" + schem + ")");
  TEST_ASSERT( res == RES_OK );

  String tblPath = "/test_db/test_tbl";
  String schemPath = "/test_db/s.test_tbl";
  TEST_ASSERT( LITTLEFS.exists(tblPath) == true );
  TEST_ASSERT( LITTLEFS.exists(schemPath) == true );

  File schemFile = LITTLEFS.open(schemPath);
  if(!schemFile || schemFile.isDirectory()){
    TEST_ASSERT( false == true ); // generate error in test
  }
  if(!schemFile.available()) {
    TEST_ASSERT( false == true ); // generate error in test
  }

  String schemFromFile = schemFile.readString();
  schemFromFile.trim();
  TEST_ASSERT( schem == schemFromFile );

  schemFile.close();
}

void insert_two_rows() {
  int8_t res = execQuery("insert into test_tbl values (" 
    "1234567890abc, 20, 1, text-1234567890abc, 22313, living room)");
  TEST_ASSERT( res == RES_OK );
  TEST_ASSERT( insertData->len == 61);
  byte rowShouldBe1[] = {
    0x00, 0x3d, 0x00,
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x61, 0x62, 0x63, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x14,
    0x01,
    0x00, 0x14, 0x74, 0x65, 0x78, 0x74, 0x2D, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x61, 0x62, 0x63,
    0x00, 0x00, 0x57, 0x29,
    0x00, 0x0d, 0x6c, 0x69, 0x76, 0x69, 0x6e, 0x67, 0x20, 0x72, 0x6f, 0x6f, 0x6d
  };
  TEST_ASSERT ( memcmp(insertData->bytes, rowShouldBe1, 61) == 0 );

  int8_t res2 = execQuery("insert into test_tbl values ("
    "2, 2147483647, -1, John Doe, 22314, dining room)");
  TEST_ASSERT( res2 == RES_OK );
  TEST_ASSERT( insertData->len == 51);
  byte rowShouldBe2[] = {
    0x00, 0x33, 0x00, 
    0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x7f, 0xff, 0xff, 0xff,
    0xff,
    0x00, 0x0a, 0x4a, 0x6f, 0x68, 0x6e, 0x20, 0x44, 0x6f, 0x65,
    0x00, 0x00, 0x57, 0x2a, 
    0x00, 0x0d, 0x64, 0x69, 0x6e, 0x69, 0x6e, 0x67, 0x20, 0x72, 0x6f, 0x6f, 0x6d
  };
  TEST_ASSERT ( memcmp(insertData->bytes, rowShouldBe2, 51) == 0 );
}

void select_one_row_with_id() {
  int8_t res = execQuery("select from test_tbl where id=1234567890abc");
  TEST_ASSERT( res == RES_OK );

  TEST_ASSERT( selectedRows->rows[0]->len == 61);

  byte rowShouldBe[] = {
    0x00, 0x3d, 0x00,
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x61, 0x62, 0x63, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x14,
    0x01,
    0x00, 0x14, 0x74, 0x65, 0x78, 0x74, 0x2D, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x61, 0x62, 0x63,
    0x00, 0x00, 0x57, 0x29, 
    0x00, 0x0d, 0x6c, 0x69, 0x76, 0x69, 0x6e, 0x67, 0x20, 0x72, 0x6f, 0x6f, 0x6d
  };
  TEST_ASSERT ( memcmp(selectedRows->rows[0]->bytes, rowShouldBe, 61) == 0 );

  byte tblNameShouldBe[] = {
    0x74, 0x65, 0x73, 0x74, 0x5f, 0x74, 0x62, 0x6c
  };
  TEST_ASSERT ( memcmp(selectedRows->rows[0]->tblName, tblNameShouldBe, 8) == 0 );

  String readID = getText(selectedRows->rows[0], "id");
  TEST_ASSERT( readID == "1234567890abc" );

  int32_t readAge = getInt32(selectedRows->rows[0], "age");
  TEST_ASSERT( readAge == 20);

  int8_t readReady = getInt32(selectedRows->rows[0], "ready");
  TEST_ASSERT( readReady == 1 );

  String readName = getText(selectedRows->rows[0], "name");
  TEST_ASSERT( readName == "text-1234567890abc" );

  int32_t readZip = getInt32(selectedRows->rows[0], "zip");
  TEST_ASSERT( readZip == 22313 );

  String readRoomName = getText(selectedRows->rows[0], "roomName");
  TEST_ASSERT( readRoomName == "living room" );
}

void select_one_row_with_name() {
  int8_t res = execQuery("select from test_tbl where name=John Doe");
  TEST_ASSERT( res == RES_OK );

  TEST_ASSERT( selectedRows->rows[0]->len == 51);

  byte rowShouldBe[] = {
    0x00, 0x33, 0x00, 
    0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x7f, 0xff, 0xff, 0xff,
    0xff,
    0x00, 0x0a, 0x4a, 0x6f, 0x68, 0x6e, 0x20, 0x44, 0x6f, 0x65,
    0x00, 0x00, 0x57, 0x2a, 
    0x00, 0x0d, 0x64, 0x69, 0x6e, 0x69, 0x6e, 0x67, 0x20, 0x72, 0x6f, 0x6f, 0x6d
  };
  TEST_ASSERT ( memcmp(selectedRows->rows[0]->bytes, rowShouldBe, 51) == 0 );

  byte tblNameShouldBe[] = {
    0x74, 0x65, 0x73, 0x74, 0x5f, 0x74, 0x62, 0x6c
  };
  TEST_ASSERT ( memcmp(selectedRows->rows[0]->tblName, tblNameShouldBe, 8) == 0 );

  String readID = getText(selectedRows->rows[0], "id");
  TEST_ASSERT( readID == "2" );

  int32_t readAge = getInt32(selectedRows->rows[0], "age");
  TEST_ASSERT( readAge == 2147483647);

  int8_t readReady = getInt32(selectedRows->rows[0], "ready");
  TEST_ASSERT( readReady == -1 );

  String readName = getText(selectedRows->rows[0], "name");
  TEST_ASSERT( readName == "John Doe" );

  int32_t readZip = getInt32(selectedRows->rows[0], "zip");
  TEST_ASSERT( readZip == 22314 );

  String readRoomName = getText(selectedRows->rows[0], "roomName");
  TEST_ASSERT( readRoomName == "dining room" );

  // execQuery("select from test_tbl where id=dfgljfg");
}

void select_one_row_with_age() {
  int8_t res = execQuery("select from test_tbl where age=20");
  TEST_ASSERT( res == RES_OK );

  TEST_ASSERT( selectedRows->rows[0]->len == 61);

  byte rowShouldBe[] = {
    0x00, 0x3d, 0x00,
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x61, 0x62, 0x63, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x14,
    0x01,
    0x00, 0x14, 0x74, 0x65, 0x78, 0x74, 0x2D, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x61, 0x62, 0x63,
    0x00, 0x00, 0x57, 0x29,
    0x00, 0x0d, 0x6c, 0x69, 0x76, 0x69, 0x6e, 0x67, 0x20, 0x72, 0x6f, 0x6f, 0x6d
  };
  TEST_ASSERT ( memcmp(selectedRows->rows[0]->bytes, rowShouldBe, 61) == 0 );
}

void select_one_row_with_ready() {
  int8_t res = execQuery("select from test_tbl where ready=-1");
  TEST_ASSERT( res == RES_OK );

  TEST_ASSERT( selectedRows->rows[0]->len == 51);

  byte rowShouldBe[] = {
    0x00, 0x33, 0x00, 
    0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x7f, 0xff, 0xff, 0xff,
    0xff,
    0x00, 0x0a, 0x4a, 0x6f, 0x68, 0x6e, 0x20, 0x44, 0x6f, 0x65,
    0x00, 0x00, 0x57, 0x2a, 
    0x00, 0x0d, 0x64, 0x69, 0x6e, 0x69, 0x6e, 0x67, 0x20, 0x72, 0x6f, 0x6f, 0x6d
  };
  TEST_ASSERT ( memcmp(selectedRows->rows[0]->bytes, rowShouldBe, 51) == 0 );
}

void select_one_row_with_zip() {
  int8_t res = execQuery("select from test_tbl where zip=22313");
  TEST_ASSERT( res == RES_OK );

  TEST_ASSERT( selectedRows->rows[0]->len == 61);

  byte rowShouldBe[] = {
    0x00, 0x3d, 0x00,
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x61, 0x62, 0x63, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x14,
    0x01,
    0x00, 0x14, 0x74, 0x65, 0x78, 0x74, 0x2D, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x61, 0x62, 0x63,
    0x00, 0x00, 0x57, 0x29,
    0x00, 0x0d, 0x6c, 0x69, 0x76, 0x69, 0x6e, 0x67, 0x20, 0x72, 0x6f, 0x6f, 0x6d
  };
  TEST_ASSERT ( memcmp(selectedRows->rows[0]->bytes, rowShouldBe, 61) == 0 );
}

void select_one_row_with_roomName() {
  int8_t res = execQuery("select from test_tbl where roomName=dining room");
  TEST_ASSERT( res == RES_OK );

  TEST_ASSERT( selectedRows->rows[0]->len == 51);

  byte rowShouldBe[] = {
    0x00, 0x33, 0x00, 
    0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x7f, 0xff, 0xff, 0xff,
    0xff,
    0x00, 0x0a, 0x4a, 0x6f, 0x68, 0x6e, 0x20, 0x44, 0x6f, 0x65,
    0x00, 0x00, 0x57, 0x2a, 
    0x00, 0x0d, 0x64, 0x69, 0x6e, 0x69, 0x6e, 0x67, 0x20, 0x72, 0x6f, 0x6f, 0x6d
  };
  TEST_ASSERT ( memcmp(selectedRows->rows[0]->bytes, rowShouldBe, 51) == 0 );
}

void update_one_row() {
  int8_t res = execQuery("update test_tbl set name=updated where id=1234567890abc");
  TEST_ASSERT( res == RES_OK );

  TEST_ASSERT( selectedRows->rows[0]->len == 61);
  byte selectShouldBe[] = {
    0x00, 0x3d, 0x00,
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x61, 0x62, 0x63, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x14,
    0x01,
    0x00, 0x14, 0x74, 0x65, 0x78, 0x74, 0x2D, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x61, 0x62, 0x63,
    0x00, 0x00, 0x57, 0x29,
    0x00, 0x0d, 0x6c, 0x69, 0x76, 0x69, 0x6e, 0x67, 0x20, 0x72, 0x6f, 0x6f, 0x6d
  };
  TEST_ASSERT ( memcmp(selectedRows->rows[0]->bytes, selectShouldBe, 61) == 0 );

  TEST_ASSERT( insertData->len == 50);
  byte insertShouldBe[] = {
    0x00, 0x32, 0x00,
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x61, 0x62, 0x63, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x14,
    0x01,
    0x00, 0x09, 0x75, 0x70, 0x64, 0x61, 0x74, 0x65, 0x64,
    0x00, 0x00, 0x57, 0x29,
    0x00, 0x0d, 0x6c, 0x69, 0x76, 0x69, 0x6e, 0x67, 0x20, 0x72, 0x6f, 0x6f, 0x6d
  };
  TEST_ASSERT ( memcmp(insertData->bytes, insertShouldBe, 50) == 0 );

  int8_t selectRes = execQuery("select from test_tbl where id=1234567890abc");
  TEST_ASSERT( selectRes == RES_OK );
  TEST_ASSERT( selectedRows->rows[0]->len == 50);
  byte rowShouldBe[] = {
    0x00, 0x32, 0x00,
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x61, 0x62, 0x63, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x14,
    0x01,
    0x00, 0x09, 0x75, 0x70, 0x64, 0x61, 0x74, 0x65, 0x64,
    0x00, 0x00, 0x57, 0x29,
    0x00, 0x0d, 0x6c, 0x69, 0x76, 0x69, 0x6e, 0x67, 0x20, 0x72, 0x6f, 0x6f, 0x6d
  };
  TEST_ASSERT ( memcmp(selectedRows->rows[0]->bytes, rowShouldBe, 50) == 0 );
}

void delete_one_row() {
  int8_t res = execQuery("delete from test_tbl where id=1234567890abc");
  TEST_ASSERT( res == RES_OK );

  int8_t selectRes = execQuery("select from test_tbl where id=1234567890abc");
  TEST_ASSERT( selectRes == RES_EMPTY );
  TEST_ASSERT( selectedRows->rowsLen == 0);
}

void insert_select_multiple_rows() {
  String query;
  int count = 5;
  int i;
  for(i=0; i< count; i++) {
    query = "insert into test_tbl values (";
    query.concat(i);
    query.concat(", 2147483648, -1, Hello ");
    query.concat(i);
    query.concat(", ");
    query.concat(i);
    query.concat(", insert select multiple rows )");
    execQuery(query);
  }

  execQuery("select from test_tbl where roomName=insert select multiple rows");
  TEST_ASSERT( selectedRows->rowsLen == count );

  for(i=0; i< count; i++) {
    String readRoomName = getText(selectedRows->rows[i], "roomName");
    TEST_ASSERT( readRoomName == "insert select multiple rows" );

    int32_t readZip = getInt32(selectedRows->rows[i], "zip");
    TEST_ASSERT( readZip == i );
  }
}

void select_insert_no_memory_leak() {
  Serial.println("Please wait...");

  execQuery("select from test_tbl where age=0"); // clear current selectedRows

  uint32_t beforHeapSize = ESP.getFreeHeap();
  String query;
  int i;
  for(i=0;i< 50; i++){
    query = "insert into test_tbl values (";
    query.concat(i);
    query.concat(", 2147483648, -1, Hello, ");
    query.concat(i);
    query.concat(", living room)");
    execQuery(query);
    execQuery("select from test_tbl where age=2147483648");
  }
  execQuery("select from test_tbl where age=0");

  uint8_t insertDataAndCodeLength = 92 + 4;
  uint32_t afterHeapSize = ESP.getFreeHeap();
  TEST_ASSERT( beforHeapSize - afterHeapSize == insertDataAndCodeLength );
}

void drop_table() {
  int8_t res = execQuery("drop table test_tbl");
  TEST_ASSERT( res == RES_OK );

  String tblPath = "/test_db/test_tbl";
  String schemPath = "/test_db/s.test_tbl";
  TEST_ASSERT( LITTLEFS.exists(tblPath) == false );
  TEST_ASSERT( LITTLEFS.exists(schemPath) == false );
}

void drop_db() {
  int8_t res = execQuery("drop db test_db");
  TEST_ASSERT( res == RES_OK );

  TEST_ASSERT( LITTLEFS.exists("/test_db") == false );
}

void setup() {
  delay(2000); // DONT DELETE THIS

  Serial.begin(115200);
  LITTLEFS.begin(true);
  
  //LITTLEFS.format();

  UNITY_BEGIN();

  RUN_TEST(create_db);
  RUN_TEST(use_db);
  RUN_TEST(create_table);
  RUN_TEST(insert_two_rows);
  RUN_TEST(select_one_row_with_id);
  RUN_TEST(select_one_row_with_name);
  RUN_TEST(select_one_row_with_age);
  RUN_TEST(select_one_row_with_ready);
  RUN_TEST(select_one_row_with_zip);
  RUN_TEST(select_one_row_with_roomName);
  RUN_TEST(update_one_row);
  RUN_TEST(delete_one_row);
  RUN_TEST(insert_select_multiple_rows);
  RUN_TEST(select_insert_no_memory_leak);
  RUN_TEST(drop_table);
  RUN_TEST(drop_db);

  Serial.println(uxTaskGetStackHighWaterMark(NULL) );

  UNITY_END();
}

void loop() {

}