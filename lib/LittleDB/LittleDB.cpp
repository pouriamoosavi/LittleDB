#include <Arduino.h>
#include "FS.h"
#include <LITTLEFS.h>

#include "LittleDB.h"

String CONNECTED_DB;
InsertData_t* insertData;
SelectData_t* selectData;

// work with files
void listDir(fs::FS &fs, String dirname, uint8_t levels) {
  Serial.print("Listing directory:");
  Serial.println(dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

int8_t createDir(fs::FS &fs, String path) {
  if(fs.mkdir(path)) {
    return RES_OK;
  } else {
    return RES_SYSTEM_ERR;
  }
}

int8_t dropDir(fs::FS &fs, String path) {
  if(fs.rmdir(path)) {
    return RES_OK;
  } else {
    return RES_SYSTEM_ERR;
  }
}

int8_t createFile(fs::FS &fs, String path) {
  File file = fs.open(path, FILE_WRITE);
  if(!file){
    file.close();
    return RES_SYSTEM_ERR;
  } else {
    file.close();
    return RES_OK;
  }
}

int8_t writeFile(fs::FS &fs, String path, String message){
  File file = fs.open(path, FILE_WRITE);
  if(!file){
    return RES_SYSTEM_ERR;
  }

  int result;
  if(file.print(message)){
    result = RES_OK;
  } else {
    result = RES_SYSTEM_ERR;
  }
  file.close();
  return result;
}

int8_t appendInsertData(fs::FS &fs, String path) {
  File file = fs.open(path, FILE_APPEND);
  if(!file){
    return RES_SYSTEM_ERR;
  }

  int result;
  if(file.write(insertData->bytes, insertData->len)){
    result = RES_OK;
  } else {
    result = RES_SYSTEM_ERR;
  }
  file.close();
  return result;
}

int8_t deleteFile(fs::FS &fs, String path){
  if(fs.remove(path)){
    return RES_OK;
  } else {
    return RES_SYSTEM_ERR;
  }
}

int8_t renameFile(fs::FS &fs, String from, String to){
  if (fs.rename(from, to)) {
    return RES_OK;
  } else {
    return RES_SYSTEM_ERR;
  }
}

void printArray(byte *input, short len){
  short i;
  for(i=0; i< len; i++){
    if(input[i]<16) Serial.print(0);
    Serial.print(input[i], HEX);
    Serial.print(" ");
  }
  Serial.println("z");
}

void printInsertData() {
  short i;
  for(i=0; i< insertData->len; i++){
    if(insertData->bytes[i]<16) Serial.print(0);
    Serial.print(insertData->bytes[i], HEX);
    Serial.print(" ");
  }
  Serial.println("z");
}

void printSelectData() {
  short i;
  for(i=0; i< selectData->len; i++){
    if(selectData->bytes[i]<16) Serial.print(0);
    Serial.print(selectData->bytes[i], HEX);
    Serial.print(" ");
  }
  Serial.println("z");
}

String getText(SelectData_t* selectData, String colName) {
  String tblNameStr = String(selectData->tblName);
  if(!tblNameStr || tblNameStr.length() == 0) {
    return "";
  }
  
  String schemPath = prefix + CONNECTED_DB + "/s." + tblNameStr;
  File schemFile = LITTLEFS.open(schemPath);
  if(!schemFile || schemFile.isDirectory()){
    return "";
  }

  uint16_t len = selectData->len;
  uint16_t i=3;
  while(i< len) {
    if(!schemFile.available()) {
      return "";
    }

    // read schem until , which seprates two columns
    String colFromSchem = schemFile.readStringUntil(',');
    colFromSchem.trim();

    String colNameFromSchem = colFromSchem.substring(0, colFromSchem.indexOf(' '));
    colNameFromSchem.trim();

    String colTypeFromSchem = colFromSchem.substring(colFromSchem.indexOf(' '));
    colTypeFromSchem.trim();

    if(colTypeFromSchem == CELL_TYPE_INT) {
      i += CELL_TYPE_INT_LEN;
      continue;
    } else if (colTypeFromSchem == CELL_TYPE_TINYINT) {
      i += CELL_TYPE_TINYINT_LEN;
      continue;
    }

    int cellLength=0;
    if (colTypeFromSchem == CELL_TYPE_ID) {
      cellLength = CELL_TYPE_ID_LEN;
      
    } else if(colTypeFromSchem == CELL_TYPE_TEXT) {
      uint16_t thisCellLength = selectData->bytes[i] << 8 | selectData->bytes[i+1];
      i += 2;
      cellLength = thisCellLength - 2; // 2 bytes text length
    }

    if(colNameFromSchem == colName) {
      byte outputBytes[cellLength];
      memcpy(outputBytes, &selectData->bytes[i], cellLength); // first 2 bytes are length
      String result = "";
      for(int a=0; a< cellLength; a++) {
        if(outputBytes[a] == 0) break;
        result += (char)outputBytes[a];
      }
      //String result = String((char*)outputBytes); // does not work for some reason.
      return result;
    } else {
       i+=cellLength; 
    }
  }

  return "";
}

int32_t getInt32(SelectData_t* selectData, String colName) {
  String tblNameStr = String(selectData->tblName);
  if(!tblNameStr || tblNameStr.length() == 0) {
    return 0;
  }
  
  String schemPath = prefix + CONNECTED_DB + "/s." + tblNameStr;
  File schemFile = LITTLEFS.open(schemPath);
  if(!schemFile || schemFile.isDirectory()){
    return 0;
  }

  uint16_t len = selectData->len;
  uint16_t i=3;
  while(i< len) {
    if(!schemFile.available()) {
      return 0;
    }

    // read schem until , which seprates two columns
    String colFromSchem = schemFile.readStringUntil(',');
    colFromSchem.trim();

    String colNameFromSchem = colFromSchem.substring(0, colFromSchem.indexOf(' '));
    colNameFromSchem.trim();

    String colTypeFromSchem = colFromSchem.substring(colFromSchem.indexOf(' '));
    colTypeFromSchem.trim();

    if(colTypeFromSchem == CELL_TYPE_ID) {
      i += CELL_TYPE_ID_LEN;
      continue;
    } else if (colTypeFromSchem == CELL_TYPE_TEXT) {
      uint16_t thisCellLength = selectData->bytes[i] << 8 | selectData->bytes[i+1];
      i += thisCellLength;
      continue;
    }

    int cellLength=0;
    if (colTypeFromSchem == CELL_TYPE_INT) {
      cellLength = CELL_TYPE_INT_LEN;

    } else if(colTypeFromSchem == CELL_TYPE_TINYINT) {
      cellLength = CELL_TYPE_TINYINT_LEN;
    }

    if(colNameFromSchem == colName) {
      int32_t output = 0;
      for(int j=0; j< cellLength; j++){
        output = (output << (j*8)) | selectData->bytes[i];
        i++;
      }
      
      return output;
    } else {
       i+=cellLength; 
    }
  }

  return 0;
}

int8_t readRowIntoSelectData(
  File tblFile, 
  String tblName, 
  uint16_t rowLen, 
  uint16_t distanceFromRowStart
) {
  uint32_t selectDataMemLen = rowLen + sizeof(selectData->tblName) + sizeof(selectData->len); // bytes length + other fixed length attributes.
  selectData = (SelectData_t*)realloc(selectData, selectDataMemLen); // bytes length + 2 len 
  if(selectData == NULL) {
    return RES_SYSTEM_ERR;
  }
  tblName.toCharArray(selectData->tblName, tblName.length()+1);
  selectData->len = rowLen;
  tblFile.seek(-distanceFromRowStart, SeekCur); // back to the start of the row
  tblFile.read(selectData->bytes, rowLen);
  return RES_OK;
}

bool valEqualRead(File tblFile, String val, String valType) {
  if(valType == CELL_TYPE_TEXT)  {
    uint16_t valLen = val.length()+2;
    byte readLenArr[2];
    tblFile.read(readLenArr, 2);
    uint16_t readLen = readLenArr[0] << 8 ^ readLenArr[1];
    if(valLen != readLen) {
      return false;
    }

    // from here readLen == valLen
    byte readArr[readLen];
    tblFile.read(readArr, readLen);
    byte valArr[readLen];
    val.getBytes(valArr, readLen);
    int equal = memcmp(readArr, valArr, readLen);
    return equal == 0;

  } else if(valType == CELL_TYPE_INT){
    int32_t valInt = val.toInt();
    byte valArr[CELL_TYPE_INT_LEN];
    valArr[0] = (valInt >> 24) & 0xFF;
    valArr[1] = (valInt >> 16) & 0xFF;
    valArr[2] = (valInt >> 8) & 0xFF;
    valArr[3] = valInt & 0xFF;

    byte readArr[CELL_TYPE_INT_LEN];
    tblFile.read(readArr, CELL_TYPE_INT_LEN);
    int equal = memcmp(readArr, valArr, CELL_TYPE_INT_LEN);
    return equal == 0;

  } else if (valType == CELL_TYPE_TINYINT) {
    int8_t valInt = val.toInt();
    byte valArr[] = {valInt};
    byte readArr[CELL_TYPE_TINYINT_LEN];
    tblFile.read(readArr, CELL_TYPE_TINYINT_LEN);
    int equal = memcmp(readArr, valArr, CELL_TYPE_TINYINT_LEN);
    return equal == 0;

  }

  return false;
}

int8_t findRowWithAnyField(
  File schemFile, 
  File tblFile, 
  String tblName, 
  String colName, 
  String val
) {
  schemFile.seek(0, SeekSet);
  tblFile.seek(0, SeekSet);

  tblFileLoop: while(tblFile.available()) {
    uint16_t distanceFromRowStart = 0;
    byte rowLenArr[2];
    tblFile.read(rowLenArr, 2); // read length from start of the file
    distanceFromRowStart += 2;
    uint16_t rowLen = rowLenArr[0] << 8 ^ rowLenArr[1];

    byte option[1];
    tblFile.read(option, 1); // read option after length
    distanceFromRowStart += 1;
    if(option[0] & OPT_DELETED) { // this line is deleted
      tblFile.seek(rowLen-distanceFromRowStart, SeekCur); // go to next row
      goto tblFileLoop;
    }

    while(schemFile.available()) {
      // read schem until , which seprates two columns
      String colFromSchem = schemFile.readStringUntil(',');
      colFromSchem.trim();

      // read until space which is column name
      String colNameFromSchem = colFromSchem.substring(0, colFromSchem.indexOf(' '));
      colNameFromSchem.trim();

      // get from space until end of string which should be column type
      String colTypeFromSchem = colFromSchem.substring(colFromSchem.indexOf(' '));
      colTypeFromSchem.trim();

      if(colNameFromSchem == colName) {
        // this is the field, so compare values
        bool found = valEqualRead(tblFile, val, colTypeFromSchem);
        if(!found) {
          tblFile.seek(rowLen-distanceFromRowStart, SeekCur); // go to next row
          goto tblFileLoop;

        } else {
          // Hooray!
          return readRowIntoSelectData(tblFile, tblName, rowLen, distanceFromRowStart);
        }
      } else {
        if(colTypeFromSchem == CELL_TYPE_ID) {
          tblFile.seek(CELL_TYPE_ID_LEN, SeekCur); // go to next column
          distanceFromRowStart += CELL_TYPE_ID_LEN;

        } else if (colTypeFromSchem == CELL_TYPE_INT) {
          tblFile.seek(CELL_TYPE_INT_LEN, SeekCur); // go to next column
          distanceFromRowStart += CELL_TYPE_INT_LEN;
          
        } else if (colTypeFromSchem == CELL_TYPE_TINYINT) {
          tblFile.seek(CELL_TYPE_TINYINT_LEN, SeekCur); // go to next column
          distanceFromRowStart += CELL_TYPE_TINYINT_LEN;
          
        } else if (colTypeFromSchem == CELL_TYPE_TEXT) {
          byte colLenArr[2];
          tblFile.read(colLenArr, 2);
          uint16_t colLen = colLenArr[0] << 8 ^ colLenArr[1];
          tblFile.seek(colLen, SeekCur); // go to next column
          distanceFromRowStart += colLen;
        }
      }
    }
    // we are at the end of schemFile and we SHOULD be at the end of this row in tblFile
  }

  return RES_EMPTY;
}

int8_t deleteRowWithID(File tblFile, String id) {
  tblFile.seek(0, SeekSet); // make sure the pointer is at start of the file
  while(tblFile.available()) {
    int distanceFromRowStart = 0;
    byte rowLenArr[2];
    tblFile.read(rowLenArr, 2); // read length from start of the file
    distanceFromRowStart += 2;
    uint16_t rowLen = rowLenArr[0] << 8 ^ rowLenArr[1];

    byte option[1];
    tblFile.read(option, 1); // read option after length
    distanceFromRowStart += 1;
    if(option[0] & OPT_DELETED) { // this line is deleted
      tblFile.seek(rowLen-distanceFromRowStart, SeekCur); // go to next row
      continue;
    }

    byte fileIDArr[CELL_TYPE_ID_LEN];
    tblFile.read(fileIDArr, CELL_TYPE_ID_LEN);
    distanceFromRowStart += CELL_TYPE_ID_LEN;

    byte inputIDArr[CELL_TYPE_ID_LEN];
    memset(inputIDArr, 0, CELL_TYPE_ID_LEN);
    id.getBytes(inputIDArr, id.length()+1);

    int equal = memcmp(fileIDArr, inputIDArr, CELL_TYPE_ID_LEN);
    if(equal == 0){
      tblFile.seek(-distanceFromRowStart+2, SeekCur); // back to the option byte which is after len
      byte option[1];
      tblFile.read(option, 1);
      option[0] |= OPT_DELETED;
      tblFile.seek(-1, SeekCur);
      tblFile.write((uint8_t*) &option, 1);
      
      return RES_OK;
    }
    
    tblFile.seek(rowLen-distanceFromRowStart, SeekCur); // go to next row
    //return -5;
    continue;
  }
  return RES_EMPTY;
}

int8_t findRowWithID(File tblFile, String tblName, String id) {
  tblFile.seek(0, SeekSet); // make sure the pointer is at start of the file
  while(tblFile.available()) {
    uint16_t distanceFromRowStart = 0;
    byte rowLenArr[2];
    tblFile.read(rowLenArr, 2); // read length from start of the file
    distanceFromRowStart += 2;
    uint16_t rowLen = rowLenArr[0] << 8 ^ rowLenArr[1];

    byte option[1];
    tblFile.read(option, 1); // read option after length
    distanceFromRowStart += 1;
    if(option[0] & OPT_DELETED) { // this line is deleted
      tblFile.seek(rowLen-distanceFromRowStart, SeekCur); // go to next row
      continue;
    }

    byte fileIDArr[CELL_TYPE_ID_LEN];
    tblFile.read(fileIDArr, CELL_TYPE_ID_LEN);
    distanceFromRowStart += CELL_TYPE_ID_LEN;
    byte inputIDArr[CELL_TYPE_ID_LEN];
    memset(inputIDArr, 0, CELL_TYPE_ID_LEN);
    id.getBytes(inputIDArr, id.length()+1);

    int equal = memcmp(fileIDArr, inputIDArr, CELL_TYPE_ID_LEN);
    if(equal == 0) {
      return readRowIntoSelectData(tblFile, tblName, rowLen, distanceFromRowStart);
    }

    tblFile.seek(rowLen-distanceFromRowStart, SeekCur); // go to next row
    continue;
  }

  selectData = (SelectData_t*)realloc(selectData, 2); // 2 for len
  selectData->len = 0;
  return RES_EMPTY; 
}

int8_t insertDataToBytes(File schemFile, String tblName, String queryValues) {
  insertData = (InsertData_t*)realloc(insertData, 3+4); // 2 bytes length + 1 byte options + 2 byte len + 2 byte usedLen;
  insertData->len = 3;
  insertData->usedLen = 3;

  // first read schema, then find that column in query
  int valueStartIndex=0;
  while(schemFile.available()) {
    // read schem until , which seprates two columns
    String colFromSchem = schemFile.readStringUntil(',');
    colFromSchem.trim();
    // get from space until end of string which should be column type
    String colType = colFromSchem.substring(colFromSchem.indexOf(' '));
    colType.trim();

    // find start and end index for this value in query
    int valueEndIndex = queryValues.indexOf(",", valueStartIndex);

    // substring this value from query
    String value = queryValues.substring(valueStartIndex, valueEndIndex);
    value.trim();
    valueStartIndex = valueEndIndex+1;
    
    if(value.length() == 0) {
      break; //while(schemFile.available())
    }

    if(colType == CELL_TYPE_ID) {
      insertData->len += CELL_TYPE_ID_LEN;
      insertData = (InsertData_t*)realloc(insertData, insertData->len+4); // length of bytes + 2 byte len + 2 byte usedLen;
      int i, j=0;
      for(i=insertData->usedLen; i< insertData->len; i++){
        insertData->bytes[i] = value.charAt(j);
        j++;
      }
      
    } else if (colType == CELL_TYPE_INT) {
      insertData->len += CELL_TYPE_INT_LEN;
      insertData = (InsertData_t*)realloc(insertData, insertData->len+4); // length of bytes + 2 byte len + 2 byte usedLen;
      int32_t valueInt = value.toInt();
      int i, j=CELL_TYPE_INT_LEN-1;
      for(i=insertData->usedLen; i< insertData->len; i++){
        insertData->bytes[i] = (valueInt >> (j*8)) & 0xFF;
        j--;
      }
      
    } else if (colType == CELL_TYPE_TINYINT) {
      insertData->len += CELL_TYPE_TINYINT_LEN;
      insertData = (InsertData_t*)realloc(insertData, insertData->len+4); // length of bytes + 2 byte len + 2 byte usedLen;
      int8_t valueInt = value.toInt();
      insertData->bytes[insertData->usedLen] = valueInt;
      
    } else if (colType == CELL_TYPE_TEXT) {
      uint16_t rowLength = value.length()+2; // 2 bytes for text length
      insertData->len += rowLength;
      insertData = (InsertData_t*)realloc(insertData, insertData->len+4); // length of bytes + 2 byte len + 2 byte usedLen;
      insertData->bytes[insertData->usedLen+1] = (rowLength >> 0) & 0xFF; // length 1
      insertData->bytes[insertData->usedLen] = (rowLength >> 8) & 0xFF; // length 2
      insertData->usedLen = insertData->usedLen+2; // 2 bytes for text length
      int i, j=0;
      for(i=insertData->usedLen; i< insertData->len; i++){
        insertData->bytes[i] = value.charAt(j);
        j++;
      }
    }

    // after filling insertData->bytes completely, set insertData->usedLen;
    insertData->usedLen = insertData->len;
  } //end of while(schemFile.available())

  insertData->bytes[1] = (insertData->len >> 0) & 0xFF; // length 1
  insertData->bytes[0] = (insertData->len >> 8) & 0xFF; // length 2
  insertData->bytes[2] = OPT_DEFAULT; // options

  return RES_OK;
}

/**
 * 1- Find row with this id and fill selectData.
 * 2- Mark this row as deleted.
 * 3- Read schema and find changed column.
 * 4- Copy unchanged data befor changed column, copy column changed value, copy all unchanged data after changed column; 
 * all into insertedData
 * 5- Write new insertedData into file.
 */
int8_t updateRowWithID(
  File schemFile, 
  File tblFile, 
  String tblName, 
  String colName, 
  String colNewValue, 
  String idValue
) {
  schemFile.seek(0, SeekSet); // make sure the pointer is at start of the file
  tblFile.seek(0, SeekSet); // make sure the pointer is at start of the file

  uint8_t findResult = findRowWithID(tblFile, tblName, idValue);
  if(findResult != RES_OK) {
    return findResult;
  }

  uint8_t deleteResult = deleteRowWithID(tblFile, idValue);
  if(deleteResult != RES_OK) {
    return deleteResult;
  }
  tblFile.close();

  uint16_t changeStartIndex = 3;
  uint16_t changeEndIndex = 3;
  String changeColType = "";
  
  while(schemFile.available()) {
    // read schem until , which seprates two columns
    String colFromSchem = schemFile.readStringUntil(',');
    colFromSchem.trim();

    // read until space which is column name
    String colNameFromSchem = colFromSchem.substring(0, colFromSchem.indexOf(' '));
    colNameFromSchem.trim();

    // get from space until end of string which should be column type
    String colTypeFromSchem = colFromSchem.substring(colFromSchem.indexOf(' '));
    colTypeFromSchem.trim();

    changeStartIndex = changeEndIndex;
    
    if(colTypeFromSchem == CELL_TYPE_ID) {
      changeEndIndex += CELL_TYPE_ID_LEN;
      
    } else if (colTypeFromSchem == CELL_TYPE_INT) {
      changeEndIndex += CELL_TYPE_INT_LEN;
      
    } else if (colTypeFromSchem == CELL_TYPE_TINYINT) {
      changeEndIndex += CELL_TYPE_TINYINT_LEN;
      
    } else if (colTypeFromSchem == CELL_TYPE_TEXT) {
      String text = getText(selectData, colNameFromSchem);
      changeEndIndex += text.length() + 2; // 2 bytes for text length
    }

    if(colNameFromSchem == colName) {
      changeColType = colTypeFromSchem;
      break;
    }
  }
  schemFile.close();

  // 2 bytes len key + 2 bytes usedLen key + (3 bytes len and option, colStartIndex bytes befor any change);
  insertData = (InsertData_t*)realloc(insertData, 4 + 3 + changeStartIndex);
  insertData->len = changeStartIndex;
  insertData->usedLen = changeStartIndex;

  // copy all unchanged columns BEFOR changeColumn, into insertedData from selectData
  memcpy(&insertData->bytes[3], &selectData->bytes[3], changeStartIndex);

  if(changeColType == CELL_TYPE_ID) {
    insertData->len += CELL_TYPE_ID_LEN;
    insertData = (InsertData_t*)realloc(insertData, insertData->len+4); // length of bytes + 2 byte len + 2 byte usedLen;
    int i, j=0;
    for(i=insertData->usedLen; i< insertData->len; i++){
      insertData->bytes[i] = colNewValue.charAt(j);
      j++;
    }
    
  } else if (changeColType == CELL_TYPE_INT) {
    insertData->len += CELL_TYPE_INT_LEN;
    insertData = (InsertData_t*)realloc(insertData, insertData->len+4); // length of bytes + 2 byte len + 2 byte usedLen;
    int32_t valueInt = colNewValue.toInt();
    int i, j=CELL_TYPE_INT_LEN-1;
    for(i=insertData->usedLen; i< insertData->len; i++){
      insertData->bytes[i] = (valueInt >> (j*8)) & 0xFF;
      j--;
    }

  } else if (changeColType == CELL_TYPE_TINYINT) {
    insertData->len += CELL_TYPE_TINYINT_LEN;
    insertData = (InsertData_t*)realloc(insertData, insertData->len+4); // length of bytes + 2 byte len + 2 byte usedLen;
    int8_t valueInt = colNewValue.toInt();
    insertData->bytes[insertData->usedLen] = valueInt;
    
  } else if (changeColType == CELL_TYPE_TEXT) {
    uint16_t textCellLength = colNewValue.length()+2; // 2 bytes for text length
    insertData->len += textCellLength;
    insertData = (InsertData_t*)realloc(insertData, insertData->len+4); // length of bytes + 2 byte len + 2 byte usedLen;
    insertData->bytes[insertData->usedLen+1] = (textCellLength >> 0) & 0xFF; // length 1
    insertData->bytes[insertData->usedLen] = (textCellLength >> 8) & 0xFF; // length 2
    insertData->usedLen = insertData->usedLen+2; // 2 bytes for text length
    int i, j=0;
    for(i=insertData->usedLen; i< insertData->len; i++){
      insertData->bytes[i] = colNewValue.charAt(j);
      j++;
    }
  }

  insertData->usedLen = insertData->len;

  uint16_t remainingLengthAfterChange = selectData->len - changeEndIndex;
  if(remainingLengthAfterChange != 0) {
    insertData->len += remainingLengthAfterChange;
    insertData = (InsertData_t*)realloc(insertData, insertData->len+4); // length of bytes + 2 byte len + 2 byte usedLen;

    // copy all unchanged columns AFTER changeColumn, into insertedData from selectData
    memcpy(&insertData->bytes[insertData->usedLen], &selectData->bytes[changeEndIndex], remainingLengthAfterChange);

    insertData->usedLen = insertData->len;
  }
  
  insertData->bytes[1] = (insertData->len >> 0) & 0xFF; // length 1
  insertData->bytes[0] = (insertData->len >> 8) & 0xFF; // length 2
  insertData->bytes[2] = OPT_DEFAULT; // options

  String tblPath = prefix + CONNECTED_DB + "/" + tblName;
  int result = appendInsertData(LITTLEFS, tblPath);
  
  return result;
}

int8_t compactTable(File oldTblFile, String newTblPath) {
  while(oldTblFile.available()) {
    byte rowLenArr[2];
    byte rowOptionArr[1];
    oldTblFile.readBytes((char*)rowLenArr, 2);
    oldTblFile.readBytes((char*)rowOptionArr, 1);
    uint16_t rowLen = rowLenArr[0] << 8 ^ rowLenArr[1];

    if(rowOptionArr[0] & OPT_DELETED) { // this row is deleted
      oldTblFile.seek((rowLen-3), SeekCur); // -3 because we are already at position 3 in this row
      continue;
    }

    oldTblFile.seek(-3, SeekCur); // return to the beginning of this row
    insertData = (InsertData_t*)realloc(insertData, rowLen + 2 + 2); // + 2 byte len + 2 byte usedLen;
    insertData->len = rowLen;
    oldTblFile.readBytes((char*)insertData->bytes, rowLen);
    insertData->usedLen = rowLen;

    uint8_t appendResult = appendInsertData(LITTLEFS, newTblPath);
    if(appendResult != RES_OK) {
      return appendResult;
    }
  }
  return RES_OK;
}
// dbs =================================
int8_t execCreateDB(String query) {
  String dbName = query.substring(9);
  dbName.trim();
  if(dbName.length() > dbNameLimit) {
    return RES_USER_ERR;
  }
  return createDir(LITTLEFS, prefix + dbName);
}

int8_t execDropDB(String query) {
  String dbName = query.substring(7);
  dbName.trim();
  return dropDir(LITTLEFS, prefix + dbName);
}

int8_t execConnectToDB(String query) {
  String dbName = query.substring(6);
  dbName.trim();
  if(LITTLEFS.exists(prefix + dbName)) {
    CONNECTED_DB = dbName;
    return 0;
  } else {
    return RES_USER_ERR;
  }  
}

// tables =================================
int8_t execCreateTbl(String query) {
  int pOpenIndex = query.indexOf("(");
  String tblName = query.substring(12, pOpenIndex);
  tblName.trim();
  
  if(tblName.length() > tblNameLimit) {
    return RES_USER_ERR;
  }
  if(tblName.indexOf(".") > -1) {
    return RES_USER_ERR;
  }

  int tblResult = createFile(LITTLEFS, prefix + CONNECTED_DB + "/" + tblName);
  if(tblResult != RES_OK) return tblResult;

  int pCloseIndex = query.indexOf(")");
  String schem = query.substring(pOpenIndex+1, pCloseIndex);

  int schemCreateResult = createFile(LITTLEFS, prefix + CONNECTED_DB + "/s." + tblName);
  if(schemCreateResult != RES_OK) return schemCreateResult;
  
  int schemResult = writeFile(LITTLEFS, prefix + CONNECTED_DB + "/s." + tblName, schem);
  return schemResult;
}

int8_t execDropTbl(String query) {
  String tblName = query.substring(10);
  tblName.trim();

  int8_t tblResult = deleteFile(LITTLEFS, prefix + CONNECTED_DB + "/" + tblName);
  if(tblResult != RES_OK) return tblResult;

  int8_t schemResult = deleteFile(LITTLEFS, prefix + CONNECTED_DB + "/s." + tblName);
  return schemResult;
}

int8_t execAlterTbl(String query) {
  return RES_NOT_IMPLEMENTED;
}

// rows =================================
int8_t execSelect(String query) {
  int whereIndex = query.indexOf("where");
  String tblName = query.substring(SELECT_LEN, whereIndex);
  tblName.trim();

  int equalIndex = query.indexOf("=", whereIndex);

  String fieldName = query.substring(whereIndex+1, equalIndex);
  fieldName.trim();

  String fieldVal = query.substring(equalIndex+1);
  fieldVal.trim();

  String tblPath = prefix + CONNECTED_DB + "/" + tblName;
  File tblFile = LITTLEFS.open(tblPath);
  if(!tblFile || tblFile.isDirectory()){
    return RES_SYSTEM_ERR;
  }

  String schemPath = prefix + CONNECTED_DB + "/s." + tblName;
  File schemFile = LITTLEFS.open(schemPath);
  if(!schemFile || schemFile.isDirectory()){
    return RES_SYSTEM_ERR;
  }
  String colTypeFromSchem = "";
  while(schemFile.available()) {
    // read schem until , which seprates two columns
    String colFromSchem = schemFile.readStringUntil(',');
    colFromSchem.trim();

    // read until space which is column name
    String colNameFromSchem = colFromSchem.substring(0, colFromSchem.indexOf(' '));
    colNameFromSchem.trim();

    if(colNameFromSchem == fieldName) {
      colTypeFromSchem = colFromSchem.substring(colFromSchem.indexOf(' '));
      colTypeFromSchem.trim();
    }
  }

  int8_t result = RES_USER_ERR;
  if(colTypeFromSchem.length() == 0) {
    return result;
  }

  if(colTypeFromSchem == CELL_TYPE_ID) {
    result = findRowWithID(tblFile, tblName, fieldVal);
  } else {
    result = findRowWithAnyField(schemFile, tblFile, tblName, fieldName, fieldVal);
  }

  schemFile.close();
  tblFile.close();
  return result;
}

int8_t execUpdate(String query) {
  int setIndex = query.indexOf("set");
  String tblName = query.substring(6, setIndex);
  tblName.trim();
  
  int whereIndex = query.indexOf("where");
  String value = query.substring(setIndex+3, whereIndex);
  value.trim();

  int colEqualIndex = value.indexOf("=");
  String colName = value.substring(0, colEqualIndex);
  colName.trim();
  String colNewValue = value.substring(colEqualIndex+1);
  colNewValue.trim();

  String schemPath = prefix + CONNECTED_DB + "/s." + tblName;
  File schemFile = LITTLEFS.open(schemPath);
  if(!schemFile || schemFile.isDirectory()){
    return RES_SYSTEM_ERR;
  }

  String tblPath = prefix + CONNECTED_DB + "/" + tblName;
  File tblFile = LITTLEFS.open(tblPath, "r+");
  if(!tblFile || tblFile.isDirectory()){
    return RES_SYSTEM_ERR;
  }

  int condEqualIndex = query.indexOf("=", whereIndex);
  String idValue = query.substring(condEqualIndex+1);
  idValue.trim();

  int result = updateRowWithID(schemFile, tblFile, tblName, colName, colNewValue, idValue);

  return result;
}

int8_t execDelete(String query) {
  int whereIndex = query.indexOf("where");
  String tblName = query.substring(DELETE_LEN, whereIndex);
  tblName.trim();

  int equalIndex = query.indexOf("=", whereIndex);
  String idValue = query.substring(equalIndex+1);
  idValue.trim();

  String tblPath = prefix + CONNECTED_DB + "/" + tblName;
  File tblFile = LITTLEFS.open(tblPath, "r+");
  if(!tblFile || tblFile.isDirectory()){
    return RES_SYSTEM_ERR;
  }

  int8_t result = deleteRowWithID(tblFile, idValue);

  tblFile.close();
  return result;
}

int8_t execInsert(String query) {
  int valuesIndex = query.indexOf("values");
  String tblName = query.substring(INSERT_LEN, valuesIndex);
  tblName.trim();

  String schemPath = prefix + CONNECTED_DB + "/s." + tblName;
  File schemFile = LITTLEFS.open(schemPath);
  if(!schemFile || schemFile.isDirectory()){
    return RES_SYSTEM_ERR;
  }
  
  int pOpenIndex = query.indexOf("(");
  int pCloseIndex = query.indexOf(")");
  String values = query.substring(pOpenIndex+1, pCloseIndex);
  values.trim();

  insertDataToBytes(schemFile, tblName, values);
  schemFile.close();
  String tblPath = prefix + CONNECTED_DB + "/" + tblName;
  int result = appendInsertData(LITTLEFS, tblPath);
  return result;
}

// others =================================
int8_t execCompact(String query) {
  String tblName = query.substring(14);
  tblName.trim();

  String oldTblPath = prefix + CONNECTED_DB + "/" + tblName;
  File oldTblFile = LITTLEFS.open(oldTblPath);
  if(!oldTblFile || oldTblFile.isDirectory()){
    return RES_SYSTEM_ERR;
  }

  String newTablePath = prefix + CONNECTED_DB + "/c." + tblName;
  int8_t newTblCreateResult = createFile(LITTLEFS, newTablePath);
  if(newTblCreateResult != RES_OK) {
    return newTblCreateResult;
  }

  int8_t result = compactTable(oldTblFile, newTablePath);
  oldTblFile.close();
  if(result != RES_OK) {
    return result;
  }

  result = deleteFile(LITTLEFS, oldTblPath);
  if(result != RES_OK) {
    return result;
  }

  result = renameFile(LITTLEFS, newTablePath, oldTblPath);
  return result;
}

int8_t execFormat(String query) {
  if(LITTLEFS.format()) {
    return RES_OK;
  } else {
    return RES_SYSTEM_ERR;
  }
}

int8_t execQuery(String query) {
  query.trim();
  if (query.startsWith(CREATE_DB)) {
    return execCreateDB(query);

  } else if (query.startsWith(DROP_DB)) {
    return execDropDB(query);

  } else if (query.startsWith(CONNECT_DB)) {
    return execConnectToDB(query);

  } else if (query == FORMAT_FS) {
    return execFormat(query);
    
  } else {
    if(CONNECTED_DB.length() == 0) {
      return RES_USER_ERR;
    }
    
    if (query.startsWith(CREATE_TBL)) {
      return execCreateTbl(query);
      
    } else if (query.startsWith(DROP_TBL)) {
      return execDropTbl(query);

    } else if (query.startsWith(ALTER_TBL)) {
      return execAlterTbl(query);
      
    } else if (query.startsWith(SELECT)) {
      return execSelect(query);

    } else if (query.startsWith(UPDATE)) {
      return execUpdate(query);

    } else if (query.startsWith(DELETE)) {
      return execDelete(query);
  
    } else if (query.startsWith(INSERT)) {
      return execInsert(query);

    } else if (query.startsWith(COMPACT)) {
      return execCompact(query);
    }
  }

  return RES_NOT_IMPLEMENTED;
}