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

File    : USBH_HID_Keyboard.c
Purpose : This sample is designed to present emUSBH's capability to
          enumerate Human Interface Devices and handle the input
          data accordingly.
          This sample will try to enumerate a barcode scanner or
          keyboard and output the scanned or entered message
          to the terminal.
          Message will be shown once the ENTER key was pressed.
          Barcode scanners normally terminate their scan operation
          with an enter key press

Additional information:
  Preparations:
    None.

  Expected behavior:
    When a keyboard is connected the entered message will be displayed in the terminal
    once ENTER was pressed.
    When a barcode scanner is connected the scanned barcode will be displayed in the
    terminal.

  Sample output:
    <...>
    00000001:040 USBH_Task    - **** Device added [0]

    00000006:881 MainTask     - The following message was entered: www.segger.com
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
#include "stm32f4xx_hal.h"

/*********************************************************************
*
*       Defines configurable
*
**********************************************************************
*/
#define MAX_DATA_ITEMS        10
#define KEY_BUFFER_SIZE       10
#define MESSAGE_BUFFER_SIZE   256

/*********************************************************************
*
*       Defines non-configurable
*
**********************************************************************
*/
#define KEY_VALUE_SHIFT_LEFT        0xE1
#define KEY_VALUE_SHIFT_RIGHT       0xE5
#define BEGIN_OF_VALID_KEYS         0x04

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
  U8  Char;
  U8  UpperCaseChar;
} SCANCODE_TO_CHAR;

typedef struct _KEY_BUFFER {
  U8 Code;
  U8 ShiftState;
} KEY_BUFFER;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
//
// This table is identical to the HID US keyboard table
// with the exception that the first four entries have been removed
// (non-visible keys). Any other key that is not on the main field of
// of the keyboard is also removed, (F-Keys, NumPad, Cursors etc.)
//
static const  SCANCODE_TO_CHAR _aScanCode2CharUS[] = 
{
  {'a',    'A'},                     // 0x04: Key a/A
  {'b',    'B'},                     // 0x05: Key b/B
  {'c',    'C'},                     // 0x06: Key c/C
  {'d',    'D'},                     // 0x07: Key d/D
  {'e',    'E'},                     // 0x08: Key e/E
  {'f',    'F'},                     // 0x09: Key f/F
  {'g',    'G'},                     // 0x0A: Key g/G
  {'h',    'H'},                     // 0x0B: Key h/H
  {'i',    'I'},                     // 0x0C: Key i/I
  {'j',    'J'},                     // 0x0D: Key j/J
  {'k',    'K'},                     // 0x0E: Key k/K
  {'l',    'L'},                     // 0x0F: Key l/L
  {'m',    'M'},                     // 0x10: Key m/M
  {'n',    'N'},                     // 0x11: Key n/N
  {'o',    'O'},                     // 0x12: Key o/O
  {'p',    'P'},                     // 0x13: Key p/P
  {'q',    'Q'},                     // 0x14: Key q/Q
  {'r',    'R'},                     // 0x15: Key r/R
  {'s',    'S'},                     // 0x16: Key s/S
  {'t',    'T'},                     // 0x17: Key t/T
  {'u',    'U'},                     // 0x18: Key u/U
  {'v',    'V'},                     // 0x19: Key v/V
  {'w',    'W'},                     // 0x1A: Key w/W
  {'x',    'X'},                     // 0x1B: Key x/X
  {'y',    'Y'},                     // 0x1C: Key y/Y
  {'z',    'Z'},                     // 0x1D: Key z/Z
  {'1',    '!'},                     // 0x1E: Key 1/!
  {'2',    '@'},                     // 0x1F: Key 2/@
  {'3',    '#'},                     // 0x20: Key 3/#
  {'4',    '$'},                     // 0x21: Key 4/$
  {'5',    '%'},                     // 0x22: Key 5/%
  {'6',    '^'},                     // 0x23: Key 6/^
  {'7',    '&'},                     // 0x24: Key 7/&
  {'8',    '*'},                     // 0x25: Key 8/*
  {'9',    '('},                     // 0x26: Key 9/(
  {'0',    ')'},                     // 0x27: Key 0/)
  {'\n',   '\r'},                    // 0x28: Key Return (ENTER)
  {'\x1b', '\x1b'},                  // 0x29: Key ESCAPE
  {'\b',   '\b'},                    // 0x2A: Key DELETE(Backspace)
  {'\t',   '\t'},                    // 0x2B: Key Tab
  {' ',    ' '},                     // 0x2C: Key Spacebar
  {'-',    '_'},                     // 0x2D: Key -/(underscore)
  {'=',    '+'},                     // 0x2E: Key =/+
  {'[',    '{'},                     // 0x2F: Key [/{
  {']',    '}'},                     // 0x30: Key ]/}
  {'\\',   '|'},                     // 0x31: Key \\/|
  {'#',    '~'},                     // 0x32: Key Non-US #/~
  {';',    ':'},                     // 0x33: Key ;/:
  {'\'',   '\"'},                    // 0x34: Key Apostrophe/Quotation mark
  {'`',    '~'},                     // 0x35: Key GraveAccent/Tilde
  {',',    '<'},                     // 0x36: Key,/<
  {'.',    '>'},                     // 0x37: Key ./>
  {'/',    '?'},                     // 0x38: Key //?
};

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static OS_STACKPTR int         _StackMain[1536/sizeof(int)];
static OS_TASK                 _TCBMain;
static OS_STACKPTR int         _StackIsr[1276/sizeof(int)];
static OS_TASK                 _TCBIsr;
static USBH_HID_KEYBOARD_DATA  _aKeyboardData[MAX_DATA_ITEMS];
static OS_MAILBOX              _HIDMailBox;
static U8                      _ShiftActive;
static U8                      _aMessageBuffer[MESSAGE_BUFFER_SIZE];
static KEY_BUFFER              _aKeys[KEY_BUFFER_SIZE];
static int                     _NumKeys;
static int                     _MessageBufferOff;

/*********************************************************************
*
*       Static Code
*
**********************************************************************
*/


/*********************************************************************
*
*       _AddChar2Buffer
*/
static void _AddChar2Buffer(U8 Code, U8 ShiftActive) 
{
  U8 Char;
  if ((Code > (SEGGER_COUNTOF(_aScanCode2CharUS) + BEGIN_OF_VALID_KEYS)) || (Code < BEGIN_OF_VALID_KEYS)) 
  {
    return;
  }
  Code -= BEGIN_OF_VALID_KEYS;
  if (ShiftActive) 
  {
    Char = _aScanCode2CharUS[Code].UpperCaseChar;
  } 
  else
  {
    Char = _aScanCode2CharUS[Code].Char;
  }
  if (Char == '\b') 
  {
    if (_MessageBufferOff > 0) {
      _MessageBufferOff--;
    }
    return;
  }
  if (Char != '\0') {
    if ((Char == '\r') || (Char == '\n') || (_MessageBufferOff == (sizeof(_aMessageBuffer)-1))) {
      _aMessageBuffer[_MessageBufferOff] = 0;
      USBH_Logf_Application("The following message was entered: %s", _aMessageBuffer);
      //
      // After output, reset buffer.
      //
      _MessageBufferOff = 0;
      _aMessageBuffer[0] = 0;
      return;
    }
    _aMessageBuffer[_MessageBufferOff++] = Char;
  }
}

/*********************************************************************
*
*       _ScanCodeOperation
*/
static void _ScanCodeOperation(unsigned Code, unsigned State) {
  //
  // Check whether one of the 'Shift'-Key was pressed
  //
  if (Code == KEY_VALUE_SHIFT_LEFT || Code == KEY_VALUE_SHIFT_RIGHT) 
  {
    _ShiftActive = State;
    return;
  }
  //
  // Store the value into key buffer,
  // the once the key is released we will store the
  // key into the message buffer
  //
  if (State) 
  {
    _aKeys[_NumKeys].Code = Code;
    _aKeys[_NumKeys++].ShiftState = _ShiftActive;
    //
    // If we reached end of the buffer, go back to the begining
    //
    if (_NumKeys >= KEY_BUFFER_SIZE) {
      _NumKeys = 0;
    }
  } 
  else 
  {
    int i;
    for (i = 0; i < KEY_BUFFER_SIZE; i++) {
      if (Code == _aKeys[i].Code) {
        _AddChar2Buffer(Code, _aKeys[i].ShiftState);
        _aKeys[i].Code = 0;
        _aKeys[i].ShiftState = 0;
      }
    }
  }
}

/*********************************************************************
*
*       _OnKeyboardChange
*
*  Function description
*    Callback, called from the USBH task when a keyboard event occurs.
*/
static void _OnKeyboardChange(USBH_HID_KEYBOARD_DATA  * pKeyData) 
{
  USBH_HID_KEYBOARD_DATA KeyboardData;

  KeyboardData = *pKeyData;
  USBH_Logf_Application("**** receive KB  code [%d] Value=[%d] InterfaceID[%d]", KeyboardData.Code,KeyboardData.Value,KeyboardData.InterfaceID);
  OS_PutMailCond(&_HIDMailBox, &KeyboardData);
}

/*********************************************************************
*
*       _OnDevNotify
*
*  Function description
*    Callback, called when a device is added or removed.
*    Call in the context of the USBH_Task.
*    The functionality in this routine should not block!
*/
static void _OnDevNotify(void * pContext, U8 DevIndex, USBH_DEVICE_EVENT Event) 
{
  (void)pContext;
  switch (Event) 
  {
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
  USBH_HID_KEYBOARD_DATA KeyboardData;

  //SEGGER_RTT_printf(0, " RCC->CFGR  0x%x\n",RCC_CFGR_SW);    
  printf("  RCC_CFGR_SWS 0x%x\n",RCC_CFGR_SWS); 
  printf("  tmp 0x%x\n",RCC->CFGR & RCC_CFGR_SWS); 
  printf("  SystemCoreClock %d\n",SystemCoreClock); 
  printf("  RCC_PLLCFGR_PLLN %d\n",RCC_PLLCFGR_PLLN); 
  printf("  HSE_VALUE %d\n",HSE_VALUE); 
   
  USBH_Init();

  printf("  ---------------\n"); 
   
  OS_SetPriority(OS_GetTaskID(), TASK_PRIO_APP);                                       // This task has the lowest prio for real-time application.
                                                                                       // Tasks using emUSB-Host API should always have a lower priority than emUSB-Host main and ISR tasks.
  OS_CREATETASK(&_TCBMain, "USBH_Task", USBH_Task, TASK_PRIO_USBH_MAIN, _StackMain);   // Start USBH main task
  OS_CREATETASK(&_TCBIsr, "USBH_isr", USBH_ISRTask, TASK_PRIO_USBH_ISR, _StackIsr);    // Start USBH ISR task
  USBH_HID_Init();
  USBH_HID_SetOnKeyboardStateChange(_OnKeyboardChange);
  //
  // Some USB host controllers lack the support for multiple USB transfers at once.
  // Only one operation at a time is allowed.
  // This the case for the KINETIS FS driver with the Kinetis USBOTG (USB0) controller.
  // If you are using a different controller you can comment the following line to
  // enable LED update functionality for keyboards.
  //
  USBH_HID_ConfigureAllowLEDUpdate(0);
  USBH_HID_RegisterNotification(_OnDevNotify, NULL);
  //
  // Create mailbox to store the HID events
  //
  OS_CREATEMB(&_HIDMailBox, sizeof(USBH_HID_KEYBOARD_DATA), MAX_DATA_ITEMS, &_aKeyboardData);

  while (1) 
  {
    BSP_ToggleLED(1);
    
    // Get data from the mailbox, print information according to the event type.
    
    OS_GetMail(&_HIDMailBox, &KeyboardData);
    //_Add
    _ScanCodeOperation(KeyboardData.Code, KeyboardData.Value);
  }
}
/*************************** End of file ****************************/
