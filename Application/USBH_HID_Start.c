/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 2003 - 2018     SEGGER Microcontroller GmbH              *
*                                                                    *
*       www.segger.com     Support: support_emusb@segger.com         *
*                                                                    *
**********************************************************************
*                                                                    *
*       emUSB-Host * USB Host stack for embedded applications        *
*                                                                    *
*                                                                    *
*       Please note:                                                 *
*                                                                    *
*       Knowledge of this file may under no circumstances            *
*       be used to write a similar product.                          *
*                                                                    *
*       Thank you for your fairness !                                *
*                                                                    *
**********************************************************************
*                                                                    *
*       emUSB-Host version: V2.15-r13960                             *
*                                                                    *
**********************************************************************
-------------------------- END-OF-HEADER -----------------------------

File    : USBH_HID_Start.c
Purpose : This sample is designed to present emUSBH's capability to
          enumerate Human Interface Devices and handle the input
          data accordingly.
          This sample will try to enumerate a connected mouse or
          keyboard and output the keystrokes or mouse movements
          to the terminal.

Additional information:
  Preparations:
    None.

  Expected behavior:
    When a keyboard is connected the pressed keys will be displayed in the terminal.
    When a mouse is connected the movements and pressed keys will be displayed in the terminal.

  Sample output:
    <...>
    16:512 USBH_Task - _OnDeviceNotification: USB HID device detected interface ID: 1 !
    16:512 USBH_Task - Address   Attrib.   MaxPacketSize   Interval
    16:512 USBH_Task - 0x81      0x03      00008             10

    **** Device added
    27:015 MainTask - Keyboard:  Key s/S                - pressed
    27:132 MainTask - Keyboard:  Key s/S                - released
    27:303 MainTask - Keyboard:  Key e/E                - pressed
    27:384 MainTask - Keyboard:  Key e/E                - released
    27:825 MainTask - Keyboard:  Key g/G                - pressed
    27:888 MainTask - Keyboard:  Key g/G                - released
    28:014 MainTask - Keyboard:  Key g/G                - pressed
    28:095 MainTask - Keyboard:  Key g/G                - released
    28:293 MainTask - Keyboard:  Key e/E                - pressed
    28:383 MainTask - Keyboard:  Key e/E                - released
    28:491 MainTask - Keyboard:  Key r/R                - pressed
    28:581 MainTask - Keyboard:  Key r/R                - released

    <...>

    **** Device added
    42:916 MainTask - Mouse: xRel: 0, yRel: 0, WheelRel: 0, ButtonState: 1
    43:060 MainTask - Mouse: xRel: 0, yRel: 0, WheelRel: 0, ButtonState: 0
    40:078 MainTask - Mouse: xRel: -1, yRel: -3, WheelRel: 0, ButtonState: 0

    <...>
*/

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/
#include "RTOS.h"
#include "BSP.h"
#include "USBH.h"
#include "USBH_HID.h"
#include "SEGGER.h"

/*********************************************************************
*
*       Defines configurable
*
**********************************************************************
*/
#define MAX_DATA_ITEMS        10

/*********************************************************************
*
*       Defines non-configurable
*
**********************************************************************
*/
#define MOUSE_EVENT       (1 << 0)
#define KEYBOARD_EVENT    (1 << 1)

/*********************************************************************
*
*       Local data definitions
*
**********************************************************************
*/
enum {
  TASK_PRIO_APP = 150,
  TASK_PRIO_USBH_MAIN,
  TASK_PRIO_USBH_ISR
};

typedef struct {
  union {
    USBH_HID_KEYBOARD_DATA  Keyboard;
    USBH_HID_MOUSE_DATA     Mouse;
  } Data;
  U8 Event;
}  HID_EVENT;

typedef struct {
  U16 KeyCode;
  const char * sDesc;
} SCANCODE_TO_DESC;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static const  SCANCODE_TO_DESC _aScanCode2StringTable[] = {
  { 0x00, "Reserved/(no event indicated)"},
  { 0x01, "Key ErrorRollOver            "},
  { 0x02, "Key POSTFail                 "},
  { 0x03, "Key ErrorUndefined           "},
  { 0x04, "Key a/A                      "},
  { 0x05, "Key b/B                      "},
  { 0x06, "Key c/C                      "},
  { 0x07, "Key d/D                      "},
  { 0x08, "Key e/E                      "},
  { 0x09, "Key f/F                      "},
  { 0x0A, "Key g/G                      "},
  { 0x0B, "Key h/H                      "},
  { 0x0C, "Key i/I                      "},
  { 0x0D, "Key j/J                      "},
  { 0x0E, "Key k/K                      "},
  { 0x0F, "Key l/L                      "},
  { 0x10, "Key m/M                      "},
  { 0x11, "Key n/N                      "},
  { 0x12, "Key o/O                      "},
  { 0x13, "Key p/P                      "},
  { 0x14, "Key q/Q                      "},
  { 0x15, "Key r/R                      "},
  { 0x16, "Key s/S                      "},
  { 0x17, "Key t/T                      "},
  { 0x18, "Key u/U                      "},
  { 0x19, "Key v/V                      "},
  { 0x1A, "Key w/W                      "},
  { 0x1B, "Key x/X                      "},
  { 0x1C, "Key y/Y                      "},
  { 0x1D, "Key z/Z                      "},
  { 0x1E, "Key 1/!                      "},
  { 0x1F, "Key 2/@                      "},
  { 0x20, "Key 3/#                      "},
  { 0x21, "Key 4/$                      "},
  { 0x22, "Key 5/%                      "},
  { 0x23, "Key 6/^                      "},
  { 0x24, "Key 7/&                      "},
  { 0x25, "Key 8/*                      "},
  { 0x26, "Key 9/(                      "},
  { 0x27, "Key 0/)                      "},
  { 0x28, "Key Return (ENTER)           "},
  { 0x29, "Key ESCAPE                   "},
  { 0x2A, "Key DELETE(Backspace)        "},
  { 0x2B, "Key Tab                      "},
  { 0x2C, "Key Spacebar                 "},
  { 0x2D, "Key -/(underscore)           "},
  { 0x2E, "Key =/+                      "},
  { 0x2F, "Key [/{                      "},
  { 0x30, "Key ]/}                      "},
  { 0x31, "Key \\/|                     "},
  { 0x32, "Key Non-US #/~               "},
  { 0x33, "Key ;/:                      "},
  { 0x34, "Key Apostrophe/Quotation mark"},
  { 0x35, "Key GraveAccent/Tilde        "},
  { 0x36, "Key,/<                       "},
  { 0x37, "Key ./>                      "},
  { 0x38, "Key //?                      "},
  { 0x39, "Key Caps Lock                "},
  { 0x3A, "Key F1                       "},
  { 0x3B, "Key F2                       "},
  { 0x3C, "Key F3                       "},
  { 0x3D, "Key F4                       "},
  { 0x3E, "Key F5                       "},
  { 0x3F, "Key F6                       "},
  { 0x40, "Key F7                       "},
  { 0x41, "Key F8                       "},
  { 0x42, "Key F9                       "},
  { 0x43, "Key F10                      "},
  { 0x44, "Key F11                      "},
  { 0x45, "Key F12                      "},
  { 0x46, "Key PrintScreen              "},
  { 0x47, "Key Scroll Lock              "},
  { 0x48, "Key Pause                    "},
  { 0x49, "Key Insert                   "},
  { 0x4A, "Key Home                     "},
  { 0x4B, "Key PageUp                   "},
  { 0x4C, "Key Delete Forward           "},
  { 0x4D, "Key End                      "},
  { 0x4E, "Key PageDown                 "},
  { 0x4F, "Key RightArrow               "},
  { 0x50, "Key LeftArrow                "},
  { 0x51, "Key DownArrow                "},
  { 0x52, "Key UpArrow                  "},
  { 0x53, "Keypad NumLock/Clear         "},
  { 0x54, "Keypad /                     "},
  { 0x55, "Keypad *                     "},
  { 0x56, "Keypad -                     "},
  { 0x57, "Keypad +                     "},
  { 0x58, "Keypad ENTER                 "},
  { 0x59, "Keypad 1/End                 "},
  { 0x5A, "Keypad 2/Down Arrow          "},
  { 0x5B, "Keypad 3/PageDn              "},
  { 0x5C, "Keypad 4/Left Arrow          "},
  { 0x5D, "Keypad 5                     "},
  { 0x5E, "Keypad 6/Right Arrow         "},
  { 0x5F, "Keypad 7/Home                "},
  { 0x60, "Keypad 8/Up Arrow            "},
  { 0x61, "Keypad 9/PageUp              "},
  { 0x62, "Keypad 0/Insert              "},
  { 0x63, "Keypad ./Delete              "},
  { 0x64, "Key Non-US \\/|              "},
  { 0x65, "Key Application              "},
  { 0x66, "Key Power                    "},
  { 0x67, "Keypad =                     "},
  { 0x68, "Key F13                      "},
  { 0x69, "Key F14                      "},
  { 0x6A, "Key F15                      "},
  { 0x6B, "Key F16                      "},
  { 0x6C, "Key F17                      "},
  { 0x6D, "Key F18                      "},
  { 0x6E, "Key F19                      "},
  { 0x6F, "Key F20                      "},
  { 0x70, "Key F21                      "},
  { 0x71, "Key F22                      "},
  { 0x72, "Key F23                      "},
  { 0x73, "Key F24                      "},
  { 0x74, "Key Execute                  "},
  { 0x75, "Key Help                     "},
  { 0x76, "Key Menu                     "},
  { 0x77, "Key Select                   "},
  { 0x78, "Key Stop                     "},
  { 0x79, "Key Again                    "},
  { 0x7A, "Key Undo                     "},
  { 0x7B, "Key Cut                      "},
  { 0x7C, "Key Copy                     "},
  { 0x7D, "Key Paste                    "},
  { 0x7E, "Key Find                     "},
  { 0x7F, "Key Mute                     "},
  { 0x80, "Key Volume Up                "},
  { 0x81, "Key Volume Down              "},
  { 0x82, "Key Locking CapsLock         "},
  { 0x83, "Key Locking NumLock          "},
  { 0x84, "Key Locking ScrollLock       "},
  { 0x85, "Keypad Comma                 "},
  { 0x86, "Keypad Equal Sign            "},
  { 0x87, "Key International1           "},
  { 0x88, "Key International2           "},
  { 0x89, "Key International3           "},
  { 0x8A, "Key International4           "},
  { 0x8B, "Key International5           "},
  { 0x8C, "Key International6           "},
  { 0x8D, "Key International7           "},
  { 0x8E, "Key International8           "},
  { 0x8F, "Key International9           "},
  { 0x90, "Key LANG1                    "},
  { 0x91, "Key LANG2                    "},
  { 0x92, "Key LANG3                    "},
  { 0x93, "Key LANG4                    "},
  { 0x94, "Key LANG5                    "},
  { 0x95, "Key LANG6                    "},
  { 0x96, "Key LANG7                    "},
  { 0x97, "Key LANG8                    "},
  { 0x98, "Key LANG9                    "},
  { 0x99, "Key Alternate Erase          "},
  { 0x9A, "Key SysReq/Attention         "},
  { 0x9B, "Key Cancel                   "},
  { 0x9C, "Key Clear                    "},
  { 0x9D, "Key Prior                    "},
  { 0x9E, "Key Return                   "},
  { 0x9F, "Key Separator                "},
  { 0xA0, "Key Out                      "},
  { 0xA1, "Key Oper                     "},
  { 0xA2, "Key Clear/Again              "},
  { 0xA3, "Key CrSel/Props              "},
  { 0xA4, "Key ExSel                    "},
  { 0xB0, "Keypad 00                    "},
  { 0xB1, "Keypad 000                   "},
  { 0xB2, "Thousands Separator          "},
  { 0xB3, "Decimal Separator            "},
  { 0xB4, "Currency Unit                "},
  { 0xB5, "Currency Sub-unit            "},
  { 0xB6, "Keypad (                     "},
  { 0xB7, "Keypad )                     "},
  { 0xB8, "Keypad {                     "},
  { 0xB9, "Keypad }                     "},
  { 0xBA, "Keypad Tab                   "},
  { 0xBB, "Keypad Backspace             "},
  { 0xBC, "Keypad A                     "},
  { 0xBD, "Keypad B                     "},
  { 0xBE, "Keypad C                     "},
  { 0xBF, "Keypad D                     "},
  { 0xC0, "Keypad E                     "},
  { 0xC1, "Keypad F                     "},
  { 0xC2, "Keypad XOR                   "},
  { 0xC3, "Keypad ^                     "},
  { 0xC4, "Keypad %                     "},
  { 0xC5, "Keypad <                     "},
  { 0xC6, "Keypad >                     "},
  { 0xC7, "Keypad &                     "},
  { 0xC8, "Keypad &&                    "},
  { 0xC9, "Keypad |                     "},
  { 0xCA, "Keypad ||                    "},
  { 0xCB, "Keypad :                     "},
  { 0xCC, "Keypad #                     "},
  { 0xCD, "Keypad Space                 "},
  { 0xCE, "Keypad @                     "},
  { 0xCF, "Keypad !                     "},
  { 0xD0, "Keypad Memory Store          "},
  { 0xD1, "Keypad Memory Recall         "},
  { 0xD2, "Keypad Memory Clear          "},
  { 0xD3, "Keypad Memory Add            "},
  { 0xD4, "Keypad Memory Subtract       "},
  { 0xD5, "Keypad Memory Multiply       "},
  { 0xD6, "Keypad Memory Divide         "},
  { 0xD7, "Keypad +/-                   "},
  { 0xD8, "Keypad Clear                 "},
  { 0xD9, "Keypad Clear Entry           "},
  { 0xDA, "Keypad Binary                "},
  { 0xDB, "Keypad Octal                 "},
  { 0xDC, "Keypad Decimal               "},
  { 0xDD, "Keypad Hexadecimal           "},
  { 0xE0, "Key LeftControl              "},
  { 0xE1, "Key LeftShift                "},
  { 0xE2, "Key LeftAlt                  "},
  { 0xE3, "Key Left GUI                 "},
  { 0xE4, "Key RightControl             "},
  { 0xE5, "Key RightShift               "},
  { 0xE6, "Key RightAlt                 "},
  { 0xE7, "Key Right GUI                "}
};

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static OS_STACKPTR int _StackMain[1536/sizeof(int)];
static OS_TASK         _TCBMain;
static OS_STACKPTR int _StackIsr[1276/sizeof(int)];
static OS_TASK         _TCBIsr;
static HID_EVENT       _aHIDEvents[MAX_DATA_ITEMS];
static OS_MAILBOX      _HIDMailBox;

/*********************************************************************
*
*       Static Code
*
**********************************************************************
*/

/*********************************************************************
*
*       _ScanCode2String
*
*  Function description
*    Converts a given keyboard scan code to the key description.
*/
static const char * _ScanCode2String(unsigned Code) {
  unsigned i;

  for (i = 0; i < SEGGER_COUNTOF(_aScanCode2StringTable); i++) {
    if (Code == _aScanCode2StringTable[i].KeyCode) {
      return  _aScanCode2StringTable[i].sDesc;
    }
  }
  return "Reserved or unknown code      ";
}

/*********************************************************************
*
*       _OnMouseChange
*
*  Function description
*    Callback, called from the USBH task when a mouse event occurs.
*/
static void _OnMouseChange(USBH_HID_MOUSE_DATA  * pMouseData) {
  HID_EVENT  HidEvent;

  HidEvent.Event = MOUSE_EVENT;
  HidEvent.Data.Mouse = *pMouseData;
  OS_PutMailCond(&_HIDMailBox, &HidEvent);
}

/*********************************************************************
*
*       _OnKeyboardChange
*
*  Function description
*    Callback, called from the USBH task when a keyboard event occurs.
*/
static void _OnKeyboardChange(USBH_HID_KEYBOARD_DATA  * pKeyData) {
  HID_EVENT  HidEvent;

  HidEvent.Event         = KEYBOARD_EVENT;
  HidEvent.Data.Keyboard = *pKeyData;
  OS_PutMailCond(&_HIDMailBox, &HidEvent);
}

/*********************************************************************
*
*       _KeyState2String
*
*  Function description
*    Converts a given key state to a string.
*/
static const char * _KeyState2String(unsigned State) {
   return State == 1 ? "pressed" : "released";
}

/*********************************************************************
*
*       _OnDevNotify
*
*  Function description
*    Callback, called when a device is added or removed.
*    Called in the context of the USBH_Task.
*    The functionality in this routine should not block!
*/
static void _OnDevNotify(void * pContext, U8 DevIndex, USBH_DEVICE_EVENT Event) {
  (void)pContext;
  switch (Event) {
  case USBH_DEVICE_EVENT_ADD:
    USBH_Logf_Application("**** Device added [%d]", DevIndex);
    break;
  case USBH_DEVICE_EVENT_REMOVE:
    USBH_Logf_Application("**** Device removed [%d]", DevIndex);
    break;
  default:;   // Should never happen
  }

}
/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       MainTask
*/
#ifdef __cplusplus
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif
void MainTask(void);
#ifdef __cplusplus
}
#endif
void MainTask(void) 
{
  HID_EVENT  HidEvent;

  USBH_Init();
  OS_SetPriority(OS_GetTaskID(), TASK_PRIO_APP);                                       // This task has the lowest prio for real-time application.
                                                                                       // Tasks using emUSB-Host API should always have a lower priority than emUSB-Host main and ISR tasks.
  OS_CREATETASK(&_TCBMain, "USBH_Task", USBH_Task, TASK_PRIO_USBH_MAIN, _StackMain);   // Start USBH main task
  OS_CREATETASK(&_TCBIsr, "USBH_isr", USBH_ISRTask, TASK_PRIO_USBH_ISR, _StackIsr);    // Start USBH ISR task

  USBH_HID_Init();
  USBH_HID_SetOnMouseStateChange(_OnMouseChange);
  USBH_HID_SetOnKeyboardStateChange(_OnKeyboardChange);
  USBH_HID_RegisterNotification(_OnDevNotify, NULL);
  //
  // Some USB host controllers lack the support for multiple USB transfers at once.
  // Only one operation at a time is allowed.
  // This the case for the KINETIS FS driver with the Kinetis USBOTG (USB0) controller.
  // If you are using a different controller you can comment the following line to
  // enable LED update functionality for keyboards.
  //
  USBH_HID_ConfigureAllowLEDUpdate(0);
  //
  // Create mailbox to store the HID events
  //
  OS_CREATEMB(&_HIDMailBox, sizeof(HID_EVENT), MAX_DATA_ITEMS, &_aHIDEvents);

  while (1) 
  {
    BSP_ToggleLED(1);
    //
    // Get data from the mailbox, print information according to the event type.
    //
    OS_GetMail(&_HIDMailBox, &HidEvent);

    if ((HidEvent.Event & (MOUSE_EVENT)) == MOUSE_EVENT) 
    {
      HidEvent.Event &= ~(MOUSE_EVENT);
      USBH_Logf_Application("Mouse: xRel: %d, yRel: %d, WheelRel: %d, ButtonState: %d",
                            HidEvent.Data.Mouse.xChange, HidEvent.Data.Mouse.yChange, HidEvent.Data.Mouse.WheelChange, HidEvent.Data.Mouse.ButtonState);
    } 
    else if ((HidEvent.Event & (KEYBOARD_EVENT)) == KEYBOARD_EVENT) 
    {
      HidEvent.Event &= ~(KEYBOARD_EVENT);
      USBH_Logf_Application("Keyboard:  %s - %s", _ScanCode2String(HidEvent.Data.Keyboard.Code), _KeyState2String(HidEvent.Data.Keyboard.Value));
    }
  }
}

/*************************** End of file ****************************/
