#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         3
#define SS_PIN          10

#define SECTOR          1
#define BLOCK_ADDR      4
#define TRAILER_BROCK   7
byte writeData = 1;
byte dataBlock[16];

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
MFRC522::MIFARE_Key key;

void setup()
{
    Serial.begin(9600); // Initialize serial communications with the PC
    while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
    SPI.begin();        // Init SPI bus
    mfrc522.PCD_Init(); // Init MFRC522 card

    for (byte i = 0; i < 6; i++)
    {
        key.keyByte[i] = 0xFF;
    }  
}

void loop()
{
    if (!mfrc522.PICC_IsNewCardPresent()) return;

    if (!mfrc522.PICC_ReadCardSerial()) return;

    show_info();
    data_set();
    if (authenticate())
    {
      read_data();
      write_data();
      check_data();
    }
    
    // Halt PICC
    mfrc522.PICC_HaltA();
    // Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();
}

void dump_byte_array(byte *buffer, byte bufferSize)
{
    for (byte i = 0; i < bufferSize; i++)
    {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}

void show_info()
{
    Serial.print(F("Card UID:"));
    dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size); Serial.println();
    Serial.print(F("PICC type: "));
    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    Serial.println(mfrc522.PICC_GetTypeName(piccType));
    if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
        &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
        &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K)
        {
          Serial.println(F("This sample only works with MIFARE Classic cards."));
          return;
        }
}

void data_set()
{
    for (byte i = 0; i < 16; i++)
      {
        if (i == 0) dataBlock[i] = writeData;
        else dataBlock[i] = 0x00;
      }
}

bool authenticate()
{
    MFRC522::StatusCode status;

    status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, TRAILER_BROCK, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK)
    {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return false;
    }
    return true;
}

void read_data()
{
    MFRC522::StatusCode status;
    byte buffer[18];
    byte size = sizeof(buffer);
    
    Serial.print(F("--- Reading data from block ")); Serial.print(BLOCK_ADDR); Serial.println(F(" ... ---"));
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(BLOCK_ADDR, buffer, &size);
    if (status != MFRC522::STATUS_OK)
    {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }
    Serial.print(F("--- Data in block ")); Serial.print(BLOCK_ADDR); Serial.println(F(": ---"));
    dump_byte_array(buffer, 16);
    Serial.println();
}

void write_data()
{
    MFRC522::StatusCode status;
    
    Serial.print(F("--- Writing data into block ")); Serial.print(BLOCK_ADDR); Serial.println(F(" ... ---"));
    dump_byte_array(dataBlock, 16); Serial.println();
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(BLOCK_ADDR, dataBlock, 16);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Write() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }
    Serial.println();
}

void check_data()
{
    MFRC522::StatusCode status;
    byte buffer[18];
    byte size = sizeof(buffer);
    
    Serial.print(F("--- Reading data from block ")); Serial.print(BLOCK_ADDR); Serial.println(F(" ... ---"));
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(BLOCK_ADDR, buffer, &size);
    if (status != MFRC522::STATUS_OK)
    {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }
  
    Serial.println(F("--- Checking result... ---"));
    byte count = 0;
    for (byte i = 0; i < 16; i++)
    {
        if (buffer[i] == dataBlock[i]) count++;
        else
        {
          Serial.println(F("xxx Faile xxx"));
          Serial.print(F("ReadData: ")); dump_byte_array(buffer, 16); Serial.println();
          Serial.print(F("WriteData: ")); dump_byte_array(dataBlock, 16); Serial.println();
        }
    }
    Serial.println(F("*** OK ***"));
}

