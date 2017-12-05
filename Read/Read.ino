#include <SPI.h>
#include <MFRC522.h>
#include "pitches.h"

#define FIRST 0
#define LAST 1

#define RST_PIN         9
#define SS_FIRST_PIN    8
#define SS_LAST_PIN     14
#define SPEAKER_PIN     15  

#define READERS         2
byte ssPins[] = {SS_FIRST_PIN, SS_LAST_PIN};

//char boxWord[] = "とけい";
byte firstAndLastChars[] = {0, 1};

#define BLOCK_ADDR      4
#define TRAILER_BROCK   7

// notes in the melody:
int melody[] = {
  NOTE_C4, NOTE_G3, 0, 0, 0, 0, 0, 0
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

MFRC522 mfrc522[READERS];   // Create MFRC522 instance.

MFRC522::MIFARE_Key key;

void setup()
{
    Serial.begin(9600); // Initialize serial communications with the PC
    while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
    SPI.begin();        // Init SPI bus
  
    for (byte i = 0; i < 6; i++)
    {
       key.keyByte[i] = 0xFF;
    }
  
    for (uint8_t reader = 0; reader < READERS; reader++)
    {
      mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN); // Init each MFRC522 card
      Serial.print(F("Reader "));
      Serial.print(reader);
      Serial.print(F(": "));
      mfrc522[reader].PCD_DumpVersionToSerial();
    }
    pinMode(SPEAKER_PIN, OUTPUT);
    Serial.print(F("--- First Char : ")); Serial.print(firstAndLastChars[FIRST]); Serial.print(F(", Last Char: ")); Serial.print(firstAndLastChars[LAST]); Serial.println(F(" ---")); 
}

/*
void set_chars()
{
  char first_char = box_word[0];
  Serial.println(first_char);
  char last_char = box_word[sizeof(box_word)-1];
  Serial.println(last_char);
  char hiragana[45] =
  {
    'あ', 'い', 'う', 'え', 'お',
    'か', 'き', 'く', 'け', 'こ',
    'さ', 'し', 'す', 'せ', 'そ',
    'た', 'ち', 'つ', 'て', 'と',
    'な', 'に', 'ぬ', 'ね', 'の',
    'は', 'ひ', 'ふ', 'へ', 'ほ',
    'ま', 'み', 'む', 'め', 'も',
    'や', 'ゆ', 'よ',
    'ら', 'り', 'る', 'れ', 'ろ',
    'わ', 'ん'
  };
  int hiragana_count = sizeof(hiragana) / sizeof(hiragana[0]);
  
  for (int idx = 0; idx < hiragana_count; idx++)
  {
    if (first_char == hiragana[idx])
    {
      firstAndLastChars[FIRST] = byte(hiragana[idx]);
    }
    else if (last_char == hiragana[idx])
    {
      firstAndLastChars[LAST] = byte(hiragana[idx]);
    }
  }
  Serial.print(F("FIRST: ")); Serial.print(firstAndLastChars[FIRST]); Serial.print(F(", LAST: ")); Serial.println(firstAndLastChars[LAST]); 
}
*/

void loop()
{
  for (uint8_t reader = 0; reader < READERS; reader++)
  {
      if (mfrc522[reader].PICC_IsNewCardPresent() && mfrc522[reader].PICC_ReadCardSerial())
      {
        show_info(reader);      
        if (!authenticate(reader)) break;
        judge(firstAndLastChars[reader], read_word(reader));
        
        // Halt PICC
        mfrc522[reader].PICC_HaltA();
        // Stop encryption on PCD
        mfrc522[reader].PCD_StopCrypto1();
      }
  }
}

void dump_byte_array(byte *buffer, byte bufferSize)
{
  for (byte i = 0; i < bufferSize; i++)
  {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}

void show_info(uint8_t reader)
{
    Serial.print(F("Reader "));Serial.print(reader); Serial.print(F(": Card UID:"));
    dump_byte_array(mfrc522[reader].uid.uidByte, mfrc522[reader].uid.size);
    Serial.println();
      
     /* 
      Serial.print(F("PICC type: "));
      MFRC522::PICC_Type piccType = mfrc522[reader].PICC_GetType(mfrc522[reader].uid.sak);
      Serial.println(mfrc522[reader].PICC_GetTypeName(piccType));
     */
}

byte read_word(uint8_t reader)
{
    MFRC522::StatusCode status;
    byte buffer[18];
    byte size = sizeof(buffer);
    
    status = (MFRC522::StatusCode) mfrc522[reader].MIFARE_Read(BLOCK_ADDR, buffer, &size);
    if (status != MFRC522::STATUS_OK)
    {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522[reader].GetStatusCodeName(status));
    }
    Serial.print(F("--- Read Word :")); Serial.print(buffer[0], DEC); Serial.println(F(" ---")) ;
      
    return buffer[0];
}

bool authenticate(uint8_t reader)
{
    MFRC522::StatusCode status;

    status = (MFRC522::StatusCode) mfrc522[reader].PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, TRAILER_BROCK, &key, &(mfrc522[reader].uid));
    if (status != MFRC522::STATUS_OK)
    {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522[reader].GetStatusCodeName(status));
        return false;
    }
    return true;
}

void judge(byte charA, byte charB)
{
  bool result = (charA == charB);
  if (result) Serial.println(F("*** OK ***"));
  else        Serial.println(F("xxx FAIL xxx"));
  play(result);
}

void play(bool result)
{
  if (result == true)
  {
   tone(SPEAKER_PIN, NOTE_A6, 100);
    delay(100);
    tone(SPEAKER_PIN, NOTE_F6, 100);
    delay(100);
    tone(SPEAKER_PIN, NOTE_A6, 100);
    delay(100);
    tone(SPEAKER_PIN, NOTE_F6, 500);
    delay(500);
  }
  else
  {
    tone(SPEAKER_PIN, NOTE_CS3, 100);
    delay(120);
    tone(SPEAKER_PIN, NOTE_CS3, 500);
    delay(500);
  }
}

