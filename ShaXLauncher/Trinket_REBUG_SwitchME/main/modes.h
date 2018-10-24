#include <Arduino.h>

//this is the new mode function.
//modes are cycled through by grounding mode pin

//Mode 1 = Single payload. Straps are enabled. This is essentially standard chainloader
//Mode 2 = 3 payloads. Straps are enabled. Payload indication enabled. As cycling through 8 payloads is hard
//Mode 3 = 8 payloads. Straps are enabled. Payload indication enabled. For those of us with time to spare...
//Mode 4 = 3 payloads. Straps are enabled. Payload indication enabled. **Auto Increasing upon fail
//Mode 5 = 8 payloads. Straps are enabled. Payload indication enabled. **Auto Increasing upon fail
//Mode 6 = Single payload. Straps are disabled. For autoRCM users. Ideal for dongles.
//Mode 7 = 3 payloads. Straps are disabled. For autoRCM users. **Auto Increasing upon fail. Ideal for dongles.
//Mode 8 = 8 payloads. Straps are disabled. For autoRCM users. **Auto Increasing upon fail. Ideal for dongles.

void mode_check() {
  if (newmode == 1) {
    AMOUNT_OF_PAYLOADS = 1;
    AUTO_INCREASE_PAYLOAD_on = 0;
    ENABLE_ALL_STRAPS = 1;
    FLASH_BEFORE_SEND_on = 0;
    }
  if (newmode == 2) {
    AMOUNT_OF_PAYLOADS = 3;
    AUTO_INCREASE_PAYLOAD_on = 0;
    ENABLE_ALL_STRAPS = 1;
    FLASH_BEFORE_SEND_on = 1;
    }
  if (newmode == 3) {
    AMOUNT_OF_PAYLOADS = 8;
    AUTO_INCREASE_PAYLOAD_on = 0;
    ENABLE_ALL_STRAPS = 1;
    FLASH_BEFORE_SEND_on = 1;
    }
  if (newmode == 4) {
    AMOUNT_OF_PAYLOADS = 3;
    AUTO_INCREASE_PAYLOAD_on = 1;
    ENABLE_ALL_STRAPS = 1;
    FLASH_BEFORE_SEND_on = 1;
    }
  if (newmode == 5) {
    AMOUNT_OF_PAYLOADS = 8;
    AUTO_INCREASE_PAYLOAD_on = 1;
    ENABLE_ALL_STRAPS = 1;
    FLASH_BEFORE_SEND_on = 1;
    }
  if (newmode == 6) {
    AMOUNT_OF_PAYLOADS = 1;
    AUTO_INCREASE_PAYLOAD_on = 1;
    ENABLE_ALL_STRAPS = 0;
    FLASH_BEFORE_SEND_on = 0;
    }    
  if (newmode == 7) {
    AMOUNT_OF_PAYLOADS = 3;
    AUTO_INCREASE_PAYLOAD_on = 1;
    ENABLE_ALL_STRAPS = 0;
    FLASH_BEFORE_SEND_on = 1;
    }    
  if (newmode == 8) {
    AMOUNT_OF_PAYLOADS =81;
    AUTO_INCREASE_PAYLOAD_on = 1;
    ENABLE_ALL_STRAPS = 0;
    FLASH_BEFORE_SEND_on = 1;
    }       
  if (newmode > 8) {newmode = 1;}
  return;
}
