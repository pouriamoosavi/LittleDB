#include <Arduino.h>
#include "FS.h"
#include <LITTLEFS.h>

#include <LittleDB.h>

void setup() {
  Serial.begin(115200);
  LITTLEFS.begin(true);
  delay(1000);
  
  execQuery("create db test1");
  execQuery("use db test1");
  
  String schem = "id id, age int, ready tinyint, name text";
  schem.trim();
  execQuery("create table test_tbl (" + schem + ")");

  int8_t it = execQuery("insert into test_tbl values (1, 10, 1, text-1)");
  Serial.print("it: ");
  Serial.println(it);

  it = execQuery("insert into test_tbl values (2, 10, 1, text-2)");
  Serial.print("it: ");
  Serial.println(it);

  listDir(LITTLEFS, "/", 2);

  execQuery("update test_tbl set name=text-12 where id=1");
  execQuery("update test_tbl set name=text-13 where id=1");

  listDir(LITTLEFS, "/", 2);

  int8_t ct = execQuery("compact table test_tbl");
  Serial.print("ct: ");
  Serial.println(ct);

  listDir(LITTLEFS, "/", 2);
/*
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
}

void loop() {

}