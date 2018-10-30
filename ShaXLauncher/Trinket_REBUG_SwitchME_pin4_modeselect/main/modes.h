#include <Arduino.h>

//this is the new mode function.
//Values changed are as follows...
//AMOUNT_OF_PAYLOADS - Set number of payloads required. 1 - 8.
//AUTO_INCREASE_PAYLOAD_on - Automatic increase payload when send fails. 1 = on, 0 = off
//FLASH_BEFORE_SEND_on - Flash payload number before attempting to send. 1 = on, 0 = off
//ENABLE_ALL_STRAPS - All straps are enabled / disabled, in all circumstances. 0 = Disabled. 1 = Enabled
//VOLUME_STRAP_ENABLED - Enable vol+ strap - for dual-boot purposes. 1 = enabled, 0 = disabled.
//JOYCON_STRAP_ENABLED - Enable joycon strap - for debug purposes. 1 = enabled, 0 = disabled.

//Modes are cycled through by grounding mode pin
//Mode 1 = Single payload. Straps are enabled. This is essentially standard chainloader
//Mode 2 = 3 payloads. Straps are enabled. Payload indication enabled. As cycling through 8 payloads is hard
//Mode 3 = 8 payloads. Straps are enabled. Payload indication enabled. For those of us with time to spare...
//Mode 4 = 3 payloads. Straps are enabled. Payload indication enabled. **Auto Increasing upon fail
//Mode 5 = 8 payloads. Straps are enabled. Payload indication enabled. **Auto Increasing upon fail
//Mode 6 = Single payload. Straps are disabled. For autoRCM users. Ideal for dongles.
//Mode 7 = 3 payloads. Straps are disabled. For autoRCM users. **Auto Increasing upon fail. Ideal for dongles.
//Mode 8 = 8 payloads. Straps are disabled. For autoRCM users. **Auto Increasing upon fail. Ideal for dongles.
//Mode 9 = 1 payload. Joycon strap dropped only. Auto-increasing. Enables dual-boot via holding vol+ during power-on
//Mode 10 = 3 payloads. Joycon strap dropped only. Auto-increasing. Enables dual-boot via holding vol+ during power-on
//Mode 11 = 8 payloads. Joycon strap dropped only. Auto-increasing. Enables dual-boot via holding vol+ during power-on
void mode_check() {
  if (MODESWITCH_ENABLED == 1){
  if (newmode == 1) {
    AMOUNT_OF_PAYLOADS = 1;
    newpayload = 1;
    AUTO_INCREASE_PAYLOAD_on = 0;
    ENABLE_ALL_STRAPS = 1;
    VOLUME_STRAP_ENABLED = 1;
    JOYCON_STRAP_ENABLED = 1;
    FLASH_BEFORE_SEND_on = 0;
    }
  if (newmode == 2) {
    AMOUNT_OF_PAYLOADS = 3;
    AUTO_INCREASE_PAYLOAD_on = 0;
    ENABLE_ALL_STRAPS = 1;
    VOLUME_STRAP_ENABLED = 1;
    JOYCON_STRAP_ENABLED = 1;
    FLASH_BEFORE_SEND_on = 1;
    }
  if (newmode == 3) {
    AMOUNT_OF_PAYLOADS = 8;
    AUTO_INCREASE_PAYLOAD_on = 0;
    ENABLE_ALL_STRAPS = 1;
    VOLUME_STRAP_ENABLED = 1;
    JOYCON_STRAP_ENABLED = 1;
    FLASH_BEFORE_SEND_on = 1;
    }
  if (newmode == 4) {
    AMOUNT_OF_PAYLOADS = 3;
    AUTO_INCREASE_PAYLOAD_on = 1;
    ENABLE_ALL_STRAPS = 1;
    VOLUME_STRAP_ENABLED = 1;
    JOYCON_STRAP_ENABLED = 1;
    FLASH_BEFORE_SEND_on = 1;
    }
  if (newmode == 5) {
    AMOUNT_OF_PAYLOADS = 8;
    AUTO_INCREASE_PAYLOAD_on = 1;
    ENABLE_ALL_STRAPS = 1;
    VOLUME_STRAP_ENABLED = 1;
    JOYCON_STRAP_ENABLED = 1;
    FLASH_BEFORE_SEND_on = 1;
    }
  if (newmode == 6) {
    AMOUNT_OF_PAYLOADS = 1;
    newpayload = 1;
    AUTO_INCREASE_PAYLOAD_on = 1;
    ENABLE_ALL_STRAPS = 0;
    VOLUME_STRAP_ENABLED = 1;
    JOYCON_STRAP_ENABLED = 1;
    FLASH_BEFORE_SEND_on = 0;
    }    
  if (newmode == 7) {
    AMOUNT_OF_PAYLOADS = 3;
    AUTO_INCREASE_PAYLOAD_on = 1;
    ENABLE_ALL_STRAPS = 0;
    VOLUME_STRAP_ENABLED = 1;
    JOYCON_STRAP_ENABLED = 1;
    FLASH_BEFORE_SEND_on = 1;
    }    
  if (newmode == 8) {
    AMOUNT_OF_PAYLOADS = 8;
    AUTO_INCREASE_PAYLOAD_on = 1;
    ENABLE_ALL_STRAPS = 0;
    VOLUME_STRAP_ENABLED = 1;
    JOYCON_STRAP_ENABLED = 1;
    FLASH_BEFORE_SEND_on = 1;
    }
  if (newmode == 9) {
    AMOUNT_OF_PAYLOADS = 1;
    newpayload = 1;
    AUTO_INCREASE_PAYLOAD_on = 0;
    ENABLE_ALL_STRAPS = 1;
    VOLUME_STRAP_ENABLED = 0;
    JOYCON_STRAP_ENABLED = 1;
    FLASH_BEFORE_SEND_on = 0;
    }      
  if (newmode == 10) {
    AMOUNT_OF_PAYLOADS = 3;
    AUTO_INCREASE_PAYLOAD_on = 1;
    ENABLE_ALL_STRAPS = 1;
    VOLUME_STRAP_ENABLED = 0;
    JOYCON_STRAP_ENABLED = 1;
    FLASH_BEFORE_SEND_on = 1;
    }      
  if (newmode == 11) {
    AMOUNT_OF_PAYLOADS = 8;
    AUTO_INCREASE_PAYLOAD_on = 1;
    ENABLE_ALL_STRAPS = 1;
    VOLUME_STRAP_ENABLED = 0;
    JOYCON_STRAP_ENABLED = 1;
    FLASH_BEFORE_SEND_on = 1;
    }      
  if (newmode > 11) {newmode = 1;}
  return;
  } else {
    //default mode
    AMOUNT_OF_PAYLOADS = 8;
    AUTO_INCREASE_PAYLOAD_on = 1;
    ENABLE_ALL_STRAPS = 1;
    VOLUME_STRAP_ENABLED = 1;
    JOYCON_STRAP_ENABLED = 1;
    FLASH_BEFORE_SEND_on = 1;
  }
}
