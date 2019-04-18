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

File    : SEGGER_snprintf.c
Purpose : Replacement for big snprintf() from standard library.
Revision: $Rev: 12754 $
*/

#include <stdlib.h>
#include <stdarg.h>
#include "SEGGER.h"

/*********************************************************************
*
*       Prototypes
*
**********************************************************************
*/

static void _StoreChar    (SEGGER_BUFFER_DESC* pBufferDesc, SEGGER_SNPRINTF_CONTEXT* pContext, char c);
static int  _PrintUnsigned(SEGGER_BUFFER_DESC* pBufferDesc, SEGGER_SNPRINTF_CONTEXT* pContext, U32 v, unsigned Base, char Flags, int Width, int Precision);
static int  _PrintInt     (SEGGER_BUFFER_DESC* pBufferDesc, SEGGER_SNPRINTF_CONTEXT* pContext, I32 v, unsigned Base, char Flags, int Width, int Precision);

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static const char _aV2C[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

static const SEGGER_PRINTF_API _Api = {
  _StoreChar,
  _PrintUnsigned,
  _PrintInt
};

static SEGGER_PRINTF_FORMATTER* _pFirstPrintfFormatter;

/*********************************************************************
*
*       Local functions
*
**********************************************************************
*/

#if defined (_WIN32) && defined (_MSC_VER)
  #if _MSC_VER < 1800
  //
  // On Windows va_copy is implemented from VisualStudio 2013 onwards
  // For previous version we have to implement it here.
  // Code was copied from VisualStudio 2013 implementation.
  //
  #define va_copy(Dest, Src)  _vacopy(&(Dest), Src)
  /*********************************************************************
  *
  *       _vacopy()
  */
  static void _vacopy(va_list *pap, va_list ap) {
    *pap = ap;
  }
  #endif
#endif

/*********************************************************************
*
*       _StoreChar()
*
*  Function description
*    Stores one character into the buffer and preserves one byte
*    in buffer to be able to terminate the string at a later time.
*
*  Parameters
*    pBufferDesc: Output buffer descriptor.
*    pContext   : Management context. Can be NULL.
*    c          : Character to store in buffer.
*/
static void _StoreChar(SEGGER_BUFFER_DESC* pBufferDesc, SEGGER_SNPRINTF_CONTEXT* pContext, char c) {
  int Cnt;

  Cnt = pBufferDesc->Cnt;
  if ((Cnt + 1) < pBufferDesc->BufferSize) {
    *(pBufferDesc->pBuffer + Cnt) = c;
    Cnt++;
    pBufferDesc->Cnt = Cnt;  // Write back here as it might be used in pfFlush callback.
    if (pContext != NULL) {
      if (pContext->pfFlush != NULL) {
        //
        // Check if we have used the last free byte in buffer
        // excluding one byte for string termination.
        // If it was the last byte execute the flush callback
        // if we have one.
        //
        if ((Cnt + 1) == pBufferDesc->BufferSize) {
          pContext->pfFlush(pContext);
        }
      }
    }
  } else {
    Cnt++;
    pBufferDesc->Cnt = Cnt;  // Write back here as it might be used in pfFlush callback.
  }
}

/*********************************************************************
*
*       _Terminate()
*
*  Function description
*    Terminates the buffer with NULL. The buffer is automatically
*    flushed, if required.
*
*  Parameters
*    pBufferDesc: Output buffer descriptor.
*    pContext   : Management context. Can be NULL.
*/
static void _Terminate(SEGGER_BUFFER_DESC* pBufferDesc, SEGGER_SNPRINTF_CONTEXT* pContext) {
  int Cnt;

  Cnt = pBufferDesc->Cnt;
  //
  // Add trailing 0 to string.
  //
  if ((Cnt + 1) > pBufferDesc->BufferSize) {
    Cnt = pBufferDesc->BufferSize - 1;
  }
  *(pBufferDesc->pBuffer + Cnt) = 0;
  //
  // Execute flush callback in case the buffer is not full
  // and not empty which means we have characters in the
  // buffer that might have not been seen by the application
  // in case the flush callback is available.
  //
  if (pContext != NULL) {
    if (pContext->pfFlush != NULL) {
      if ((Cnt != 0) && ((Cnt + 1) < pBufferDesc->BufferSize)) {
        pContext->pfFlush(pContext);
      }
    }
  }
}

/*********************************************************************
*
*       _PrintUnsigned()
*
*  Function description
*    Stores an unsigned value into the string buffer.
*
*  Parameters
*    pBufferDesc: Output buffer descriptor.
*    pContext   : Management context. Can be NULL.
*    v          : Value to store into string buffer.
*    Base       : Numeric base to use (base of 10 for decimal).
*    Flags      : Flags Byte, see SEGGER_PRINTF_FLAG_* definitions.
*    Precision  : Number of digits (characters until next delimiter)
*                 of the value to store.
*
*  Return value
*    Number of characters (without termination) that would have been
*    stored if the buffer had been large enough.
*/
static int _PrintUnsigned(SEGGER_BUFFER_DESC* pBufferDesc, SEGGER_SNPRINTF_CONTEXT* pContext, U32 v, unsigned Base, char Flags, int Width, int Precision) {
  U32  Div;
  U32  Max;
  int  Digit;
  int  Stored;
  char Sign;
  char NoPrec;

  Stored = 0;
  Div = 1;
  Max = 0xFFFFFFFF / Base;
  if (Precision == -1) {
    NoPrec = 1;
  } else {
    NoPrec = 0;
  }
  //
  // Compute Div.
  // Loop until Div has the value of the highest digit required.
  // Example: If the output is 345 (Base 10), loop 2 times until Div is 100.
  //
  while (Div < Max) {
    if (Div * Base > v) {
      break;
    }
    Div *= Base;
    Width--;
    Precision--;
  }
  //
  // Output leading chars and sign. If leading chars are '0', the sign is printed at first.
  // Otherwise the sign is printed behind the leading chars. The number of digits is reduced
  // by 1, if a sign is going to be printed out.
  //
  Sign = 0;
  //
  // Handle sign
  //
  if (Flags & SEGGER_PRINTF_FLAG_NEGATIVE) {
    Sign = '-';
  } else {
    if (Flags & SEGGER_PRINTF_FLAG_SIGNFORCE) {
      Sign = '+';
    } else if(Flags & SEGGER_PRINTF_FLAG_SIGNSPACE) {
      Sign = ' ';
    }
  }
  if (Sign) {
    Width--; // Only applies to 'space' padding
  }
  //
  // The zero padding specifier should be ignored, if precision is given.
  //
  if (Flags & SEGGER_PRINTF_FLAG_ZEROPAD) {
    if (NoPrec) {
      Precision = Width;
      Width = -1;
    }
  }
  //
  // Apply 'space' padding
  //
  if (!(Flags & SEGGER_PRINTF_FLAG_ADJLEFT)) {
    while (Width > 1 && Width > Precision) {
      _StoreChar(pBufferDesc, pContext, (char)(Flags & SEGGER_PRINTF_FLAG_ZEROPAD ? '0' : ' '));
      Stored++;
      Width--;
    }
  }
  //
  // Output sign
  //
  if (Sign) {
    _StoreChar(pBufferDesc, pContext, Sign);
    Stored++;
  }
  //
  // Apply zero padding
  //
  while (Precision > 1) {
    _StoreChar(pBufferDesc, pContext, '0');
    Stored++;
    Precision--;
    Width--;
  }
  //
  // Output digits.
  //
  do {
    Digit = v / Div;
    v -= Digit * Div;
    Div /= Base;
    _StoreChar(pBufferDesc, pContext, _aV2C[Digit]);
    Stored++;
  } while (Div > 0);
  //
  // Right padding
  //
  if (Flags & SEGGER_PRINTF_FLAG_ADJLEFT) {
    while (Width > 1) {
      _StoreChar(pBufferDesc, pContext, ' ');
      Stored++;
      Width--;
    }
  }
  return Stored;
}

/*********************************************************************
*
*       _PrintInt()
*
*  Function description
*    Stores an unsigned value into the string buffer.
*
*  Parameters
*    pBufferDesc: Output buffer descriptor.
*    pContext   : Management context. Can be NULL.
*    v          : Value to store into string buffer.
*    Base       : Numeric base to use (base of 10 for decimal).
*    Flags      : Flags Byte, see SEGGER_PRINTF_FLAG_* definitions.
*    Precision  : Number of digits (characters until next delimiter)
*                 of the value to store.
*
*  Return value
*    Number of characters (without termination) that would have been
*    stored if the buffer had been large enough.
*/
static int _PrintInt(SEGGER_BUFFER_DESC* pBufferDesc, SEGGER_SNPRINTF_CONTEXT* pContext, I32 v, unsigned Base, char Flags, int Width, int Precision) {
  //
  // Handle the sign by converting v to a positive value
  // and sending the sign as character to _PrintUnsigned().
  //
  if (v < 0) {
    return _PrintUnsigned(pBufferDesc, pContext, -v, Base, (char)(Flags | SEGGER_PRINTF_FLAG_NEGATIVE), Width, Precision);
  } else {
    return _PrintUnsigned(pBufferDesc, pContext,  v, Base, Flags, Width, Precision);
  }
}

/*********************************************************************
*
*       _vsnprintf()
*
*  Function description
*    Replaces placeholders in a formatted string and stores the
*    output into the given buffer.
*
*  Parameters
*    pBufferDesc: Output buffer descriptor.
*    pContext   : Management context. Can be NULL.
*    sFormat    : Formatted string that might contain placeholders.
*    ParamList  : Variable arguments list to fill placeholders.
*
*  Return value
*    Number of characters (without termination) that would have been
*    stored if the buffer had been large enough.
*/
static int _vsnprintf(SEGGER_BUFFER_DESC* pBufferDesc, SEGGER_SNPRINTF_CONTEXT* pContext, const char* sFormat, va_list ParamList) {
  SEGGER_PRINTF_FORMATTER* pFormatter;
  char    Flags;
  int     Width;
  int     Precision;
  I32     v;
  char    c;
  va_list z;

  //
  // We need to make a copy of the va_list, since the internal type of the va_list
  // varies in different architectures. In case of va_list is an array type, the
  // formatter expects a pointer to the first element of the array, but &ParamList
  // is a pointer to the array which results in a compiler warning. However, creating
  // a pointer to the local variable z always results in the correct type for the
  // formatter function.
  //
  // The Cppcheck utility reports that z is used before being initialized via
  // va_start(). This is wrong since z is a copy of ParamList that is initialized
  // in the calling function. For this reason we suppress this check here.
  // cppcheck-suppress va_list_usedBeforeStarted
  //
#if defined(__va_copy)
  va_copy(z, ParamList);
#else
  z = ParamList;
#endif
  //
  // Execute flush callback in case the buffer was already full.
  //
  if (pContext != NULL) {
    if (pContext->pfFlush != NULL) {
      if ((pBufferDesc->Cnt + 1) == pBufferDesc->BufferSize) {
        pContext->pfFlush(pContext);
      }
    }
  }
  do {
    c = *sFormat++;
    if (c == 0) {
      break;
    }
    if (c == '%') {
      //
      // Parse flags.
      //
      Flags = 0;
      do {
        c = *sFormat++;
        if (c == '-') {
          Flags |= SEGGER_PRINTF_FLAG_ADJLEFT;
          continue;
        }
        if (c == '+') {
          Flags |= SEGGER_PRINTF_FLAG_SIGNFORCE;
          continue;
        }
        if (c == ' ') {
          Flags |= SEGGER_PRINTF_FLAG_SIGNSPACE;
          continue;
        }
        if (c == '#') {
          Flags |= SEGGER_PRINTF_FLAG_PRECEED;
          continue;
        }
        if (c == '0') {
          Flags |= SEGGER_PRINTF_FLAG_ZEROPAD;
          continue;
        }
        break;
      } while (1);
      //
      // Parse width specifier.
      //
      if ((c >= '0') && (c <= '9')) {
        Width = 0;
        do {
          Width = (Width * 10) + (c - '0');
          c = *sFormat++;
        } while ((c >= '0') && (c <= '9'));
      } else {
        Width = -1;
      }
      //
      // Filter out trailing dot.
      //
      if (c == '.') {
        c = *sFormat++;
        //
        // Parse precision specifier.
        //
        Precision = 0;
        if ((c >= '0') && (c <= '9')) {
          do {
            Precision = (Precision * 10) + (c - '0');
            c = *sFormat++;
          } while ((c >= '0') && (c <= '9'));
        } else if (c == '*') {
          //
          // Precision is given in an argument.
          //
          Precision = va_arg(z, int);
          c         = *sFormat++;
        }
      } else {
        Precision = -1;
      }
      //
      // Parse qualifiers.
      //
      do {
        if (c == 'l' || c == 'h') {
          c = *sFormat++;
          continue;
        }
        break;
      } while (1);
      //
      // Handle different types.
      //
      switch (c) {
      case '_':
        c = *sFormat++;
        goto Custom;
      case '%':
        _StoreChar(pBufferDesc, pContext, '%');
        break;
      case 'c': {
        char c0;

        c0 = (char)va_arg(z, int);
        _StoreChar(pBufferDesc, pContext, c0);
        break;
      }
      case 'd':
        v = va_arg(z, int);
        _PrintInt(pBufferDesc, pContext, v, 10, Flags, Width, Precision);
        break;
      case 'u':
        v = va_arg(z, int);
        _PrintUnsigned(pBufferDesc, pContext, v, 10, Flags, Width, Precision);
        break;
      case 'x':
      case 'X':
        v = va_arg(z, int);
        _PrintUnsigned(pBufferDesc, pContext, v, 16, Flags, Width, Precision);
        break;
      case 's': {
          const char* s;

          s = va_arg(z, const char*);
          if(s) {
            do {
              c = *s++;
              if (c == 0) {
                break;
              }
              if (Precision != -1) {
                if (Precision-- == 0) {
                  break;
                }
              }
              _StoreChar(pBufferDesc, pContext, c);
            } while (1);
          }
          break;
        }
      case 'p':
        v = va_arg(z, int);
        _PrintUnsigned(pBufferDesc, pContext, v, 16, Flags, -1, 8);
        break;
      default:
Custom:
        pFormatter = _pFirstPrintfFormatter;
        while (pFormatter) {
          if (pFormatter->Specifier == c) {
            pFormatter->pfFormatter(pBufferDesc, pContext, &_Api, &z, Flags, Width, Precision);
            break;
          }
          pFormatter = pFormatter->pNext;
        }
      }
    } else {
      _StoreChar(pBufferDesc, pContext, c);
    }
  } while (1);
  _Terminate(pBufferDesc, pContext);
#if defined(__va_copy)
  va_end(z);
#endif
  return pBufferDesc->Cnt;
}

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*       SEGGER_StoreChar()
*
*  Function description
*    Stores one character into the buffer and preserves one byte
*    in buffer to be able to terminate the string at a later time.
*
*  Parameters
*    pBufferDesc: Output buffer descriptor.
*    c          : Character to store in buffer.
*/
void SEGGER_StoreChar(SEGGER_BUFFER_DESC* pBufferDesc, char c) {
  _StoreChar(pBufferDesc, NULL, c);
}

/*********************************************************************
*
*       SEGGER_PrintUnsigned()
*
*  Function description
*    Stores an unsigned value into the string buffer.
*
*  Parameters
*    pBufferDesc: Output buffer descriptor.
*    v          : Value to store into string buffer.
*    Base       : Numeric base to use (base of 10 for decimal).
*    Precision  : Number of digits (characters until next delimiter)
*                 of the value to store.
*/
void SEGGER_PrintUnsigned(SEGGER_BUFFER_DESC* pBufferDesc, U32 v, unsigned Base, int Precision) {
  _PrintUnsigned(pBufferDesc, NULL, v, Base, SEGGER_PRINTF_FLAG_ZEROPAD, -1, Precision);
  _Terminate(pBufferDesc, NULL);
}

/*********************************************************************
*
*       SEGGER_PrintInt()
*
*  Function description
*    Stores an unsigned value into the string buffer.
*
*  Parameters
*    pBufferDesc: Output buffer descriptor.
*    v          : Value to store into string buffer.
*    Base       : Numeric base to use (base of 10 for decimal).
*    Precision  : Number of digits (characters until next delimiter)
*                 of the value to store.
*/
void SEGGER_PrintInt(SEGGER_BUFFER_DESC* pBufferDesc, I32 v, unsigned Base, int Precision) {
  _PrintInt(pBufferDesc, NULL, v, Base, SEGGER_PRINTF_FLAG_ZEROPAD, -1, Precision);
  _Terminate(pBufferDesc, NULL);
}

/*********************************************************************
*
*       SEGGER_PRINTF_AddFormatter()
*
*  Function description
*    Adds a hook with a custom formatter to SEGGER_PRINTF.
*
*  Parameters
*    pFormatter : Pointer to a structure of SEGGER_PRINTF_FORMATTER
*    pfFormatter: Formatter function.
*    c          : Format specifier.
*/
int SEGGER_PRINTF_AddFormatter(SEGGER_PRINTF_FORMATTER* pFormatter, SEGGER_pFormatter pfFormatter, char c) {
  SEGGER_PRINTF_FORMATTER* p;

  //
  // Make sure the formatter to add is not already hooked in.
  //
  p = _pFirstPrintfFormatter;
  while (p != NULL) {
    if (p->Specifier == c) {
      return -1;
    }
    p = p->pNext;
  }
  //
  // Add the new hook to the front.
  //
  pFormatter->Specifier   = c;
  pFormatter->pfFormatter = pfFormatter;
  pFormatter->pNext       = _pFirstPrintfFormatter;
  _pFirstPrintfFormatter  = pFormatter;
  return 0;
}

/*********************************************************************
*
*       SEGGER_snprintf()
*
*  Function description
*    Replaces placeholders in a formatted string and stores the
*    output into the given buffer.
*
*  Parameters
*    pBuffer   : Output buffer.
*    BufferSize: Size of output buffer.
*    sFormat   : Formatted string that might contain placeholders.
*    ...       : Variable arguments list to fill placeholders.
*
*  Return value
*    Number of characters (without termination) that would have been
*    stored if the buffer had been large enough.
*/
int SEGGER_snprintf(char* pBuffer, int BufferSize, const char* sFormat, ...) {
  SEGGER_BUFFER_DESC BufferDesc;
  va_list            ParamList;
  int                r;

  BufferDesc.pBuffer     = pBuffer;
  BufferDesc.BufferSize  = BufferSize;
  BufferDesc.Cnt         = 0;
  va_start(ParamList, sFormat);
  r = _vsnprintf(&BufferDesc, NULL, sFormat, ParamList);
  va_end(ParamList);
  return r;
}

/*********************************************************************
*
*       SEGGER_vsnprintf()
*
*  Function description
*    Replaces placeholders in a formatted string and stores the
*    output into the given buffer. Uses a prepared parameter list.
*
*  Parameters
*    pBuffer   : Output buffer.
*    BufferSize: Size of output buffer.
*    sFormat   : Formatted string that might contain placeholders.
*    ParamList : Variable arguments list to fill placeholders.
*
*  Return value
*    Number of characters (without termination) that would have been
*    stored if the buffer had been large enough.
*/
int SEGGER_vsnprintf(char* pBuffer, int BufferSize, const char* sFormat, va_list ParamList) {
  SEGGER_BUFFER_DESC BufferDesc;

  BufferDesc.pBuffer     = pBuffer;
  BufferDesc.BufferSize  = BufferSize;
  BufferDesc.Cnt         = 0;
  return _vsnprintf(&BufferDesc, NULL, sFormat, ParamList);
}

/*********************************************************************
*
*       SEGGER_vsnprintfEx()
*
*  Function description
*    Replaces placeholders in a formatted string and stores the
*    output into the given buffer descriptor. Uses a prepared
*    parameter list.
*
*  Parameters
*    pContext : Management context. Can be NULL.
*    sFormat  : Formatted string that might contain placeholders.
*    ParamList: Variable arguments list to fill placeholders.
*
*  Return value
*    Number of characters (without termination) that would have been
*    stored if the buffer had been large enough.
*
*  Additional information
*    Allows a more flexible handling of vsnprintf() by using a buffer
*    descriptor instead of just an output buffer. By providing a
*    flush callback the output does not have to stop once the buffer
*    is full but can be restarted by handling the content currently
*    in the output buffer and resetting it back to empty to store
*    more. The normal [SEGGER_]vsnprintf() will stop storing characters
*    once the buffer is full and all further characters to store are
*    simply discarded.
*/
int SEGGER_vsnprintfEx(SEGGER_SNPRINTF_CONTEXT* pContext, const char* sFormat, va_list ParamList) {
  return _vsnprintf(pContext->pBufferDesc, pContext, sFormat, ParamList);
}

/*************************** End of file ****************************/
