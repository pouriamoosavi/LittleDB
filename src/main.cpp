#include <Arduino.h>
#include "FS.h"
#include <LITTLEFS.h>

#include "littleDB.h"

void setup() {
  Serial.begin(115200);
  LITTLEFS.begin(true);
  delay(1000);
  return;
  delay(4000);
  
/*
  LITTLEFS.format();

  int cd = execQuery("create db test");
  Serial.print("cd: ");
  Serial.println(cd);
*/
  
  int ud = execQuery("use db test");
  Serial.print("ud: ");
  Serial.println(ud);
  //delay(5000);

/*
  int ct = execQuery("create table test_tbl (id id, age int, ready tinyint, name text)");
  Serial.print("ct: ");
  Serial.println(ct);
  
  String schemPath = prefix + CONNECTED_DB + "/s.test_tbl";
  File schemFile = LITTLEFS.open(schemPath);
  while(schemFile.available()){
    Serial.write(schemFile.read());
  }
  Serial.println();

  //listDir(LITTLEFS, "/", 2);

  int8_t it = execQuery("insert into test_tbl values (1234567890abcde, 10, 1, text-1234567890abcde)");
  Serial.print("it: ");
  Serial.println(it);

  int8_t it1 = execQuery("insert into test_tbl values (1234567890abc, 20, 1, text-1234567890abc)");
  Serial.print("it1: ");
  Serial.println(it1);

  int8_t it2 = execQuery("insert into test_tbl values (1234567, 8, 30, text-1234567)");
  Serial.print("it2: ");
  Serial.println(it2);
  
  int8_t it3 = execQuery("insert into test_tbl values (abcdef, 40, 1, text-abcdef)");
  Serial.print("it3: ");
  Serial.println(it3);
  */
  /*
  String tblPath = prefix + CONNECTED_DB + "/test_tbl";
  File tblFile = LITTLEFS.open(tblPath);
  while(tblFile.available()){
    Serial.print(tblFile.read(), HEX);
    Serial.print(" ");
  }
  Serial.println();
  */

/*
  int8_t it1 = execQuery("insert into test_tbl values (1234567890abc, 8, abc3)");
  Serial.print("insertABD before delete: ");
  Serial.println(it1);
*/


// test update
/*
  int8_t beforUpdate = execQuery("select from test_tbl where id=1234567");
  Serial.print("selectABC beforUpdate: ");
  Serial.println(beforUpdate);
  printArray(selectData->bytes, selectData->len);
  
  // uint32_t buAge = getInt32(selectData, "age");
  // Serial.print("buAge ");
  // Serial.println(buAge);

  int8_t update = execQuery("update test_tbl set name=name where id=1234567");
  Serial.print("selectABC update: ");
  Serial.println(update);

  int8_t afterUpdate = execQuery("select from test_tbl where id=1234567");
  Serial.print("selectABC afterUpdate: ");
  Serial.println(afterUpdate);
  printArray(selectData->bytes, selectData->len);
  
  // uint32_t auAge = getInt32(selectData, "age");
  // Serial.print("auAge ");
  // Serial.println(auAge);

*/
  listDir(LITTLEFS, "/", 2);
/*
// test delete
  int8_t sf = execQuery("select from test_tbl where id=1234567");
  Serial.print("selectABC befor delete: ");
  Serial.println(sf);
  Serial.println(selectData->len);
  printArray(selectData->bytes, selectData->len);

  // String id = getText(selectData, "id");
  // Serial.println(id);

  // uint32_t age = getInt32(selectData, "age");
  // Serial.print("age ");
  // Serial.println(age);

  // uint32_t ready = getInt32(selectData, "ready");
  // Serial.print("ready ");
  // Serial.println(ready);

  // String name = getText(selectData, "name");
  // Serial.println(name);

  int8_t df = execQuery("delete from test_tbl where id=1234567");
  Serial.print("deleteABC: ");
  Serial.println(df);

  int8_t sf1 = execQuery("select from test_tbl where id=1234567");
  Serial.print("selectABC after delete: ");
  Serial.println(sf1);
  printArray(selectData->bytes, selectData->len);
*/
}

void loop() {

}


