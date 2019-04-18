/*********************************************************************
*                     SEGGER Microcontroller GmbH                    *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 1995 - 2018 SEGGER Microcontroller GmbH                  *
*                                                                    *
*       Internet: segger.com  Support: support_embos@segger.com      *
*                                                                    *
**********************************************************************
*                                                                    *
*       embOS * Real time operating system for microcontrollers      *
*                                                                    *
*       Please note:                                                 *
*                                                                    *
*       Knowledge of this file may under no circumstances            *
*       be used to write a similar product or a real-time            *
*       operating system for in-house use.                           *
*                                                                    *
*       Thank you for your fairness !                                *
*                                                                    *
**********************************************************************
*                                                                    *
*       OS version: 5.02a                                            *
*                                                                    *
**********************************************************************

----------------------------------------------------------------------
File    : embOSPluginSES.js
Purpose : Script for thread windows for embOS and SEGGER Embedded Studio
--------  END-OF-HEADER  ---------------------------------------------
*/

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

/**** Configurable **************************************************/
var StackCheckLimit = 8192;

/**** ARM Register indices, constant ********************************/
var arm_r0  =  0;
var arm_r1  =  1;
var arm_r2  =  2;
var arm_r3  =  3;
var arm_r4  =  4;
var arm_r5  =  5;
var arm_r6  =  6;
var arm_r7  =  7;
var arm_r8  =  8;
var arm_r9  =  9;
var arm_r10 = 10;
var arm_r11 = 11;
var arm_r12 = 12;
var arm_sp  = 13;
var arm_lr  = 14;
var arm_pc  = 15;
var arm_psr = 16;

//
// embOS preserves an additional [pseudo] register
// (exec location with ARM and a modified LR with Cortex-M)
//
var arm_exec = 15;

/**** RISC-V Register indices, constant ********************************/
var riscv_pc  =  0;
var riscv_ra  =  1;
var riscv_sp  =  2;
var riscv_gp  =  3;
var riscv_tp  =  4;
var riscv_t0  =  5;
var riscv_t1  =  6;
var riscv_t2  =  7;
var riscv_s0  =  8;
var riscv_s1  =  9;
var riscv_a0  = 10;
var riscv_a1  = 11;
var riscv_a2  = 12;
var riscv_a3  = 13;
var riscv_a4  = 14;
var riscv_a5  = 15;
var riscv_a6  = 16;
var riscv_a7  = 17;
var riscv_s2  = 18;
var riscv_s3  = 19;
var riscv_s4  = 20;
var riscv_s5  = 21;
var riscv_s6  = 22;
var riscv_s7  = 23;
var riscv_s8  = 24;
var riscv_s9  = 25;
var riscv_s10 = 26;
var riscv_s11 = 27;
var riscv_t3  = 28;
var riscv_t4  = 29;
var riscv_t5  = 30;
var riscv_t6  = 31;

/**** embOS task states, constant ***********************************/
var TS_READY            = (0x00 << 3);  // ready
var TS_WAIT_EVENT       = (0x01 << 3);  // waiting for task event
var TS_WAIT_MUTEX       = (0x02 << 3);  // waiting for mutexes
var TS_WAIT_ANY         = (0x03 << 3);  // Waiting for any reason
var TS_WAIT_SEMA        = (0x04 << 3);  // Waiting for semaphore
var TS_WAIT_MEMPOOL     = (0x05 << 3);  // Waiting for memory pool since V4.30
var TS_WAIT_MEMPOOL_old = (0x05 << 5);  // Waiting for memory pool pre V4.30
var TS_WAIT_QNE         = (0x06 << 3);  // Waiting for message in queue
var TS_WAIT_MBNF        = (0x07 << 3);  // Waiting for space in mailbox
var TS_WAIT_MBNE        = (0x08 << 3);  // Waiting for message in mailbox
var TS_WAIT_EVENTOBJ    = (0x09 << 3);  // Waiting for event object
var TS_WAIT_QNF         = (0x0A << 3);  // Waiting for space in queue

var TS_MASK_SUSPEND_CNT = (0x03 << 0);  // Mask for task suspension count
var TS_MASK_TIMEOUT     = (0x01 << 2);  // Mask for task timeout period
var TS_MASK_STATE       = (0xF8 << 0);  // Mask for task state

/**** embOS list names, constant ***********************************/
var Timers              = "Timers";
var Mailboxes           = "Mailboxes";
var Queues              = "Queues";
var Mutexes             = "Mutexes";
var Semaphores          = "Semaphores";
var MemoryPools         = "Memory Pools";
var EventObjects        = "Event Objects";
var WatchDogs           = "Watchdogs";
var SystemInformation   = "System Information";

/*********************************************************************
*
*       Local functions
*
**********************************************************************
*/
/*********************************************************************
*
*       GetObjectName()
*
*  Parameters
*    Address: Object address
*
*  Functions description:
*    Returns an object name
*/
function GetObjectName(Address) {
  var p;
  var pObjID;

  p = Debug.evaluate("OS_pObjNameRoot");
  if (p == undefined || p == 0) {
    return "";
  }
  pObjID = Debug.evaluate("(*(OS_OBJNAME*)" + p + ").pOSObjID");
  while (pObjID != Address) {
    p = Debug.evaluate("(OS_OBJNAME*)(*(OS_OBJNAME*)" + p + ").pNext");
    if (p != undefined && p != 0) {
      pObjID = Debug.evaluate("(*(OS_OBJNAME*)" + p + ").pOSObjID");
    } else {
      return "";
    }
  }
  return Debug.evaluate("(char*)(*(OS_OBJNAME*)" + p + ").sName");
}

/*********************************************************************
*
*       GetTaskName()
*
*  Parameters
*    Address: Task address
*
*  Functions description:
*    Returns an object name
*/
function GetTaskName(Address) {
  var Name;

  Name = Debug.evaluate("(char*)(*(OS_TASK*)" + Address + ").Name");
  if (Name == undefined || Name == "") {
    Name = "n.a.";
  } else if (Name == 0) {
    Name = "";
  }
  return Name;
}

/*********************************************************************
*
*       GetWaitingTasks()
*
*  Functions description:
*    Used to retrieve a string with a list of tasks that are waiting
*    for the specified object.
*
*  Parameters
*    pObject: Address of the object which the tasks returned are waiting.
*/
function GetWaitingTasks(pObject) {
  var WaitingTasks = "";

  for (var TaskIterator = Debug.evaluate("OS_Global.pTask"); (TaskIterator != undefined) && (TaskIterator != 0); TaskIterator = Debug.evaluate("(OS_TASK*)(*(OS_TASK*)" + TaskIterator + ").pNext")) {
    var pWaitObject = Debug.evaluate("(OS_WAIT_OBJ_STRUCT*)(*(OS_WAIT_LIST*)(*(OS_TASK*)" + TaskIterator + ").pWaitList).pWaitObj");
    if (pWaitObject == pObject) {
      WaitingTasks += "0x" + TaskIterator.toString(16).toUpperCase() + " (" + GetTaskName(TaskIterator) + "), ";
    }
  }
  WaitingTasks = WaitingTasks.substring(0, WaitingTasks.length - 2);
  return WaitingTasks;
}

/*********************************************************************
*
*       UpdateTimers()
*
*  Functions description:
*    Called from the update() function in order to retrieve timer
*    information and update the timers list.
*
*  Parameters
*    Window: Object used to access the plug in window.
*/
function UpdateTimers(Window) {
  var Hook;
  var RemainingTime;
  var Timeout;
  var Period;

  for (var pTimer = Debug.evaluate("OS_Global.pTimer"); (pTimer != undefined) && (pTimer != 0); pTimer = Debug.evaluate("(*(OS_TIMER*)" + pTimer + ").pNext")) {
    var Timer = Debug.evaluate("*(OS_TIMER*)" + pTimer);
    //
    // Read information from timer structure. Only running timers can be listed.
    //
    Hook          = Timer.Hook;
    Timeout       = Timer.Time;
    RemainingTime = Timeout - Debug.evaluate("OS_Global.Time");
    Period        = Timer.Period;
    //
    // Update information in the plug in window
    //
    Window.add2(Timers,
                "0x" + pTimer.toString(16).toUpperCase(),
                GetObjectName(pTimer),
                "0x" + Hook.toString(16).toUpperCase() + " (" + Debug.getfunction(Hook) + ")",
                RemainingTime + " (" + Timeout + ")",
                Period);
  }
}

/*********************************************************************
*
*       UpdateMailboxes()
*
*  Functions description:
*    Called from the update() function in order to retrieve mailbox
*    information and update the mailbox list.
*
*  Parameters
*    Window: Object used to access the plug in window.
*/
function UpdateMailboxes(Window) {
  var InUse;
  var Messages;
  var MaxMessages;
  var MessageSize;
  var pBuffer;
  var WaitingTasks;

  for (var pMailbox = Debug.evaluate("OS_pMailboxRoot"); (pMailbox != undefined) && (pMailbox != 0); pMailbox = Debug.evaluate("(OS_MAILBOX*)(*(OS_MAILBOX*)" + pMailbox + ").pNext")) {
    var MailBox = Debug.evaluate("*(OS_MAILBOX*)" + pMailbox);
    //
    // Read information from mailbox structure
    //
    InUse       = MailBox.InUse;
    Messages    = MailBox.nofMsg;
    MaxMessages = MailBox.maxMsg;
    MessageSize = MailBox.sizeofMsg;
    pBuffer     = Debug.evaluate("(unsigned int*)(*(OS_MAILBOX*)" + pMailbox + ").pData"); //MailBox.pData;
    //
    // Retrieve waiting tasks
    //
    WaitingTasks = GetWaitingTasks(pMailbox);
    //
    // Update information in the plug in window
    //
    Window.add2(Mailboxes,
                "0x" + pMailbox.toString(16).toUpperCase(),
                GetObjectName(pMailbox),
                Messages + "/" + MaxMessages,
                MessageSize,
                "0x" + pBuffer.toString(16).toUpperCase(),
                WaitingTasks,
                InUse ? "True" : "False");
  }
}

/*********************************************************************
*
*       UpdateQueues()
*
*  Functions description:
*    Called from the update() function in order to retrieve queue
*    information and update the queues list.
*
*  Parameters
*    Window: Object used to access the plug in window.
*/
function UpdateQueues(Window) {
  var Messages;
  var pBuffer;
  var BufferSize;
  var WaitingTasks;

  for (var pQueue = Debug.evaluate("OS_pQRoot"); (pQueue != undefined) && (pQueue != 0); pQueue = Debug.evaluate("(*(OS_Q*)" + pQueue + ").pNext")) {
    var Queue = Debug.evaluate("*(OS_Q*)" + pQueue);
    //
    // Read information from queue structure
    //
    Messages   = Queue.MsgCnt;
    pBuffer    = Queue.pData;
    BufferSize = Queue.Size;
    //
    // Retrieve waiting tasks
    //
    WaitingTasks = GetWaitingTasks(pQueue);
    //
    // Update information in the plug in window
    //
    Window.add2(Queues,
                "0x" + pQueue.toString(16).toUpperCase(),
                GetObjectName(pQueue),
                Messages,
                "0x" + pBuffer.toString(16).toUpperCase(),
                BufferSize,
                WaitingTasks);
  }
}

/*********************************************************************
*
*       UpdateMutexes()
*
*  Functions description:
*    Called from the update() function in order to retrieve mutexes
*    information and update the resource semaphores list.
*
*  Parameters
*    Window: Object used to access the plug in window.
*/
function UpdateMutexes(Window) {
  var Owner;
  var UseCounter;
  var WaitingTasks;
  var Cast;
  var Root;

  if (Debug.evaluate("OS_pRSemaRoot") == undefined) {
    Cast = "(OS_MUTEX*)";
    Root = "OS_pMutexRoot";
  } else {
    Cast = "(OS_RSEMA*)";
    Root = "OS_pRSemaRoot";
  }

  for (var pMutex = Debug.evaluate(Root); (pMutex != undefined) && (pMutex != 0); pMutex = Debug.evaluate("(*" + Cast + pMutex + ").pNext")) {
    var Mutex = Debug.evaluate("*" + Cast + pMutex);
    //
    // Read information from queue structure
    //
    UseCounter = Mutex.UseCnt;
    if (UseCounter != 0) {
      Owner = Mutex.pTask;
    } else {
      Owner = 0;
    }
    //
    // Retrieve waiting tasks
    //
    WaitingTasks = GetWaitingTasks(pMutex);
    //
    // Update information in the plug in window
    //
    Window.add2(Mutexes,
                "0x" + pMutex.toString(16).toUpperCase(),
                GetObjectName(pMutex),
                Owner ? "0x" + Owner.toString(16).toUpperCase() + " (" + GetTaskName(Owner) + ")" : "",
                UseCounter,
                WaitingTasks);
  }
}

/*********************************************************************
*
*       UpdateSemaphores()
*
*  Functions description:
*    Called from the update() function in order to retrieve semaphores
*    information and update the semaphores list.
*
*  Parameters
*    Window: Object used to access the plug in window.
*/
function UpdateSemaphores(Window) {
  var Count;
  var WaitingTasks;
  var Cast;
  var Root;

  if (Debug.evaluate("OS_pCSemaRoot") == undefined) {
    Cast = "(OS_SEMAPHORE*)";
    Root = "OS_pSemaRoot";
  } else {
    Cast = "(OS_CSEMA*)";
    Root = "OS_pCSemaRoot";
  }

  for (var pSema = Debug.evaluate(Root); (pSema != undefined) && (pSema != 0); pSema = Debug.evaluate("(*" + Cast + pSema + ").pNext")) {
    //
    // Read information from queue structure
    //
    Count = Debug.evaluate("(*" + Cast + pSema + ").Cnt");
    //
    // Retrieve waiting tasks
    //
    WaitingTasks = GetWaitingTasks(pSema);
    //
    // Update information in the plug in window
    //
    Window.add2(Semaphores,
                "0x" + pSema.toString(16).toUpperCase(),
                GetObjectName(pSema),
                Count,
                WaitingTasks);
  }
}

/*********************************************************************
*
*       UpdateMemoryPools()
*
*  Functions description:
*    Called from the update() function in order to retrieve memory
*    pool information and update the memory pools list.
*
*  Parameters
*    Window: Object used to access the plug in window.
*/
function UpdateMemoryPools(Window) {
  var NumFreeBlocks;
  var NumBlocks;
  var BlockSize;
  var MaxUsage;
  var pBuffer;
  var WaitingTasks;

  for (var pMemPool = Debug.evaluate("OS_pMEMFRoot"); (pMemPool != undefined) && (pMemPool != 0); pMemPool = Debug.evaluate("(*(OS_MEMF*)" + pMemPool + ").pNext")) {
    var MemPool = Debug.evaluate("*(OS_MEMF*)" + pMemPool);
    //
    // Read information from queue structure
    //
    NumFreeBlocks = MemPool.NumFreeBlocks;
    NumBlocks     = MemPool.NumBlocks;
    BlockSize     = MemPool.BlockSize;
    MaxUsage      = MemPool.MaxUsed;
    pBuffer       = MemPool.pPool;
    //
    // Retrieve waiting tasks
    //
    WaitingTasks = GetWaitingTasks(pMemPool);
    //
    // Update information in the plug in window
    //
    Window.add2(MemoryPools,
                "0x" + pMemPool.toString(16).toUpperCase(),
                GetObjectName(pMemPool),
                NumFreeBlocks + "/" + NumBlocks,
                BlockSize,
                MaxUsage,
                "0x" + pBuffer.toString(16).toUpperCase(),
                WaitingTasks);
  }
}

/*********************************************************************
*
*       UpdateEventObjects()
*
*  Functions description:
*    Called from the update() function in order to retrieve event
*    object information and update the event objects list.
*
*  Parameters
*    Window: Object used to access the plug in window.
*/
function UpdateEventObjects(Window) {
  var Signaled;
  var ResetMode;
  var MaskMode;
  var WaitingTasks;

  var aResetModes = ["Semiauto", "Manual", "Auto"];
  var aMaskModes  = ["OR Logic", "AND Logic"];

  for (var pEvent = Debug.evaluate("OS_pEventRoot"); (pEvent != undefined) && (pEvent != 0); pEvent = Debug.evaluate("(*(OS_EVENT*)" + pEvent + ").pNext")) {
    //
    // Read information from queue structure
    //
    Signaled  = Debug.evaluate("     (*(OS_EVENT*)" + pEvent + ").Signaled");
    ResetMode = Debug.evaluate("(int)(*(OS_EVENT*)" + pEvent + ").ResetMode");
    MaskMode  = Debug.evaluate("(int)(*(OS_EVENT*)" + pEvent + ").MaskMode");
    //
    // Retrieve waiting tasks
    //
    WaitingTasks = GetWaitingTasks(pEvent);
    //
    // Update information in the plug in window
    //
    Window.add2(EventObjects,
                "0x" + pEvent.toString(16).toUpperCase(),
                GetObjectName(pEvent),
                "0x" + Signaled.toString(16).toUpperCase(),
                aResetModes["" + ResetMode],
                aMaskModes[MaskMode],
                WaitingTasks);
  }
}

/*********************************************************************
*
*       UpdateWatchDogs()
*
*  Functions description:
*    Called from the update() function in order to retrieve watchdog
*    information and update the watchdogs list.
*
*  Parameters
*    Window: Object used to access the plug in window.
*/
function UpdateWatchDogs(Window) {
  var TimeDex;
  var Period;
  var GlobalTime;

  GlobalTime = Debug.evaluate("OS_Global.Time");
  for (var pWatchdog = Debug.evaluate("OS_pWDRoot"); (pWatchdog != undefined) && (pWatchdog != 0); pWatchdog = Debug.evaluate("(*(OS_WD*)" + pWatchdog + ").pNext")) {
    var Watchdog = Debug.evaluate("*(OS_WD*)" + pWatchdog);
    //
    // Read information from queue structure
    //
    TimeDex = Watchdog.TimeDex;
    Period  = Watchdog.Period;
    //
    // Retrieve waiting tasks
    //
    WaitingTasks = GetWaitingTasks(pWatchdog);
    //
    // Update information in the plug in window
    //
    Window.add2(WatchDogs,
                "0x" + pWatchdog.toString(16).toUpperCase(),
                GetObjectName(pWatchdog),
                (TimeDex - GlobalTime) + " (" + TimeDex + ")",
                Period,
                WaitingTasks);
  }
}

/*********************************************************************
*
*       UpdateSystemInformation()
*
*  Functions description:
*    Called from the update() function in order to retrieve watchdog
*    information and update the watchdogs list.
*
*  Parameters
*    Window: Object used to access the plug in window.
*/
function UpdateSystemInformation(Window) {
  var Status;
  var SystemTime;
  var pCurrentTask;
  var ActiveTask;
  var Libmode;
  var VersionNum;
  var Version;
  var OS_Global;

  var aFunctions = ["OS_CreateTask_XR", "OS_CreateTask_R", "OS_CreateTask_S",
                    "OS_CreateTask_SP", "OS_CreateTask_D", "OS_CreateTask_DP",
                    "OS_CreateTask_DT", "OS_CreateTask_SAFE",
                    "OS_TASK_Create_XR", "OS_TASK_Create_R", "OS_TASK_Create_S",
                    "OS_TASK_Create_SP", "OS_TASK_Create_D", "OS_TASK_Create_DP",
                    "OS_TASK_Create_DT", "OS_TASK_Create_SAFE"];

  var aLibmodes = ["Extreme Release (XR)",
                   "Release (R)",
                   "Stackcheck (S)",
                   "Stackcheck + Profiling (SP)",
                   "Debug (D)",
                   "Debug + Profiling (DP)",
                   "Debug + Trace + Profiling (DT)",
                   "Safe (SAFE)"];

  //
  // Read information
  //
  OS_Global    = Debug.evaluate("OS_Global");
  Status       = Debug.evaluate("OS_Status");
  SystemTime   = OS_Global.Time;
  pCurrentTask = OS_Global.pCurrentTask;
  pActiveTask  = OS_Global.pActiveTask;
  VersionNum   = Debug.evaluate("OS_Version");
  //
  // Check Libmode
  //
  for (var i = 0; i < aFunctions.length; ++i) {
    Libmode = Debug.evaluate(aFunctions[i]);
    if (Libmode != undefined) {
      Libmode = aLibmodes[i % 8];
      break;
    }
  }
  //
  // Compute information
  //
  if (pCurrentTask == "0") {
    pCurrentTask = "None"
  } else {
    pCurrentTask = "0x" + pCurrentTask.toString(16).toUpperCase() + " (" + GetTaskName(pCurrentTask) + ")";
  }
  if (pActiveTask == "0") {
    pActiveTask = "None"
  } else {
    pActiveTask = "0x" + pActiveTask.toString(16).toUpperCase() + " (" + GetTaskName(pActiveTask) + ")";
  }
  if (VersionNum == undefined) {
    Version = "n.a.";
  } else {
    Version = Math.floor((VersionNum / 10000)) + "." + (""+(Math.floor(VersionNum / 100))).substring(1, 3);
    if ((VersionNum = VersionNum % 100) != 0) {
      Version = Version
              + (((VersionNum % 25) != 0) ? String.fromCharCode(96 + (VersionNum % 25)) : ".")
              + (((VersionNum / 25) >= 1) ? Math.floor(VersionNum / 25) : "");
    }
  }
  //
  // Update information in the plug in window
  //
  Window.add2(SystemInformation, "System Status", (Status == "OS_OK") ? "O.K." : Status);
  Window.add2(SystemInformation, "System Time",   SystemTime);
  Window.add2(SystemInformation, "Current Task",  pCurrentTask);
  Window.add2(SystemInformation, "Active Task",   pActiveTask);
  Window.add2(SystemInformation, "embOS Build",   Libmode);
  Window.add2(SystemInformation, "embOS Version", Version);
}

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*       getregs()
*
*  Functions description:
*    If a thread is selected in the threads window, this function returns
*    an array containing the registers r0-r12, sp, lr, pc, and psr.
*
*  Parameters
*    x: Pointer to task control block
*/
function getregs(x) {
  if (Debug.evaluate("SkipSaveMainContextRISCV")) {  // This is for embOS for RISCV
    var aRegs       = new Array(32);
    var Interrupted = 0;
    var PreviousPC;
    var VectoredMode;

    aRegs[riscv_sp]   = Debug.evaluate("((OS_TASK*)" + x + ")->pStack");
    aRegs[riscv_sp]  += 4;  // "pop" Counters
    aRegs[riscv_s0]   = TargetInterface.peekWord(aRegs[riscv_sp]);
    aRegs[riscv_sp]  += 4;  // "pop" s0
    aRegs[riscv_s1]   = TargetInterface.peekWord(aRegs[riscv_sp]);
    aRegs[riscv_sp]  += 4;  // "pop" s1
    aRegs[riscv_s2]   = TargetInterface.peekWord(aRegs[riscv_sp]);
    aRegs[riscv_sp]  += 4;  // "pop" s2
    aRegs[riscv_s3]   = TargetInterface.peekWord(aRegs[riscv_sp]);
    aRegs[riscv_sp]  += 4;  // "pop" s3
    aRegs[riscv_s4]   = TargetInterface.peekWord(aRegs[riscv_sp]);
    aRegs[riscv_sp]  += 4;  // "pop" s4
    aRegs[riscv_s5]   = TargetInterface.peekWord(aRegs[riscv_sp]);
    aRegs[riscv_sp]  += 4;  // "pop" s5
    aRegs[riscv_s6]   = TargetInterface.peekWord(aRegs[riscv_sp]);
    aRegs[riscv_sp]  += 4;  // "pop" s6
    aRegs[riscv_s7]   = TargetInterface.peekWord(aRegs[riscv_sp]);
    aRegs[riscv_sp]  += 4;  // "pop" s7
    aRegs[riscv_s8]   = TargetInterface.peekWord(aRegs[riscv_sp]);
    aRegs[riscv_sp]  += 4;  // "pop" s8
    aRegs[riscv_s9]   = TargetInterface.peekWord(aRegs[riscv_sp]);
    aRegs[riscv_sp]  += 4;  // "pop" s9
    aRegs[riscv_s10]  = TargetInterface.peekWord(aRegs[riscv_sp]);
    aRegs[riscv_sp]  += 4;  // "pop" s10
    aRegs[riscv_s11]  = TargetInterface.peekWord(aRegs[riscv_sp]);
    aRegs[riscv_sp]  += 4;  // "pop" s11
    aRegs[riscv_pc]   = TargetInterface.peekWord(aRegs[riscv_sp]);
    aRegs[riscv_ra]   = TargetInterface.peekWord(aRegs[riscv_sp]);
    aRegs[riscv_sp]  += 4;  // "pop" new pc / previous ra
    aRegs[riscv_sp]  += 8;  // "pop" alignment

    VectoredMode = Debug.evaluate("vtrap_entry");
    if (VectoredMode != NULL) {
      //
      // We've found vtrap_entry, thus are in vectored mode.
      //
      var OS_ISR_Local15;

      PreviousPC     = TargetInterface.peekWord(aRegs[riscv_sp] + 28);
      OS_ISR_Local15 = Debug.evaluate("OS_ISR_Local15");
      if ((PreviousPC >= VectoredMode) && (PreviousPC <= OS_ISR_Local15)) {
        Interrupted = 1;
      }
      VectoredMode = 1;
    } else {
      //
      // We haven't found vtrap_entry, thus are in direct mode.
      //
      var trap_entry;

      PreviousPC = TargetInterface.peekWord(aRegs[riscv_sp] + 60);
      trap_entry = Debug.evaluate("trap_entry");
      if ((PreviousPC >= trap_entry) && (PreviousPC <= trap_entry + 50)) {
        Interrupted = 1;
      }
      VectoredMode = 0;
    }

    if ((Interrupted == 1) && (VectoredMode == 1)){
      //
      // Task has been interrupted in vectored mode. Unstack exception frame and restore task register
      //
      aRegs[riscv_sp] += 32;  // "pop" high-level exception frame
      aRegs[riscv_ra]  = TargetInterface.peekWord(aRegs[riscv_sp]);
      aRegs[riscv_sp] += 4;  // "pop" ra
      aRegs[riscv_t0]  = TargetInterface.peekWord(aRegs[riscv_sp]);
      aRegs[riscv_sp] += 4;  // "pop" t0
      aRegs[riscv_t1]  = TargetInterface.peekWord(aRegs[riscv_sp]);
      aRegs[riscv_sp] += 4;  // "pop" t1
      aRegs[riscv_t2]  = TargetInterface.peekWord(aRegs[riscv_sp]);
      aRegs[riscv_sp] += 4;  // "pop" t2
      aRegs[riscv_a0]  = TargetInterface.peekWord(aRegs[riscv_sp]);
      aRegs[riscv_sp] += 4;  // "pop" a0
      aRegs[riscv_a1]  = TargetInterface.peekWord(aRegs[riscv_sp]);
      aRegs[riscv_sp] += 4;  // "pop" a1
      aRegs[riscv_a2]  = TargetInterface.peekWord(aRegs[riscv_sp]);
      aRegs[riscv_sp] += 4;  // "pop" a2
      aRegs[riscv_a3]  = TargetInterface.peekWord(aRegs[riscv_sp]);
      aRegs[riscv_sp] += 4;  // "pop" a3
      aRegs[riscv_a4]  = TargetInterface.peekWord(aRegs[riscv_sp]);
      aRegs[riscv_sp] += 4;  // "pop" a4
      aRegs[riscv_a5]  = TargetInterface.peekWord(aRegs[riscv_sp]);
      aRegs[riscv_sp] += 4;  // "pop" a5
      aRegs[riscv_a6]  = TargetInterface.peekWord(aRegs[riscv_sp]);
      aRegs[riscv_sp] += 4;  // "pop" a6
      aRegs[riscv_a7]  = TargetInterface.peekWord(aRegs[riscv_sp]);
      aRegs[riscv_sp] += 4;  // "pop" a7
      aRegs[riscv_t3]  = TargetInterface.peekWord(aRegs[riscv_sp]);
      aRegs[riscv_sp] += 4;  // "pop" t3
      aRegs[riscv_t4]  = TargetInterface.peekWord(aRegs[riscv_sp]);
      aRegs[riscv_sp] += 4;  // "pop" t4
      aRegs[riscv_t5]  = TargetInterface.peekWord(aRegs[riscv_sp]);
      aRegs[riscv_sp] += 4;  // "pop" t5
      aRegs[riscv_t6]  = TargetInterface.peekWord(aRegs[riscv_sp]);
      aRegs[riscv_sp] += 4;  // "pop" t6
      //
      // Since we've been interrupted, using ra would result in an erroneous stack frame. Use mepc instead!
      //
      aRegs[riscv_pc]  = TargetInterface.peekWord(aRegs[riscv_sp]);
      aRegs[riscv_sp] += 16;  // Drop remaining stack frame, so the debugger can take care of any subsequent function parameters
    } else if ((Interrupted == 1) && (VectoredMode == 0)) {
      aRegs[riscv_sp] += 32;  // "pop" high-level exception frame
      aRegs[riscv_pc]  = TargetInterface.peekWord(aRegs[riscv_sp] + 8);
      aRegs[riscv_ra]  = TargetInterface.peekWord(aRegs[riscv_sp] + 32);
      aRegs[riscv_sp] += 36;  // Drop remaining stack frame, so the debugger can take care of any subsequent function parameters
    } else {
      aRegs[riscv_ra]  = "0x00000000";  // unknown after cooperative task switch
      aRegs[riscv_t0]  = "0x00000000";  // unknown after cooperative task switch
      aRegs[riscv_t1]  = "0x00000000";  // unknown after cooperative task switch
      aRegs[riscv_t2]  = "0x00000000";  // unknown after cooperative task switch
      aRegs[riscv_a0]  = "0x00000000";  // unknown after cooperative task switch
      aRegs[riscv_a1]  = "0x00000000";  // unknown after cooperative task switch
      aRegs[riscv_a2]  = "0x00000000";  // unknown after cooperative task switch
      aRegs[riscv_a3]  = "0x00000000";  // unknown after cooperative task switch
      aRegs[riscv_a4]  = "0x00000000";  // unknown after cooperative task switch
      aRegs[riscv_a5]  = "0x00000000";  // unknown after cooperative task switch
      aRegs[riscv_a6]  = "0x00000000";  // unknown after cooperative task switch
      aRegs[riscv_a7]  = "0x00000000";  // unknown after cooperative task switch
      aRegs[riscv_t3]  = "0x00000000";  // unknown after cooperative task switch
      aRegs[riscv_t4]  = "0x00000000";  // unknown after cooperative task switch
      aRegs[riscv_t5]  = "0x00000000";  // unknown after cooperative task switch
      aRegs[riscv_t6]  = "0x00000000";  // unknown after cooperative task switch
    }
    //
    // Return default values for global pointer and thread pointer
    //
    aRegs[riscv_gp] = "0x00000000";  // unknown, this is never modified and is therefore not preserved
    aRegs[riscv_tp] = "0x00000000";  // unknown, this is never modified and is therefore not preserved

    return aRegs;
  } else {
    var aRegs       = new Array(17);
    var Interrupted = 0;
    var FPU_Used    = 0;
    var OSSwitch;
    var MPU_Used;

    MPU_Used = Debug.evaluate("(unsigned int*)(*(OS_TASK*)" + x + ").MPU_Config");
    MPU_Used = (MPU_Used != NULL) ? 1 : 0;

    //
    // Retrieve current top-of-stack
    //
    aRegs[arm_sp] = Debug.evaluate("((OS_TASK*)" + x + ")->pStack");
    //
    // Handle well known registers
    //
    aRegs[arm_sp]   += 4;  // "pop" Counters
    aRegs[arm_r4]    = TargetInterface.peekWord(aRegs[arm_sp]);
    aRegs[arm_sp]   += 4;  // "pop" R4
    aRegs[arm_r5]    = TargetInterface.peekWord(aRegs[arm_sp]);
    aRegs[arm_sp]   += 4;  // "pop" R5
    aRegs[arm_r6]    = TargetInterface.peekWord(aRegs[arm_sp]);
    aRegs[arm_sp]   += 4;  // "pop" R6
    aRegs[arm_r7]    = TargetInterface.peekWord(aRegs[arm_sp]);
    aRegs[arm_sp]   += 4;  // "pop" R7
    aRegs[arm_r8]    = TargetInterface.peekWord(aRegs[arm_sp]);
    aRegs[arm_sp]   += 4;  // "pop" R8
    aRegs[arm_r9]    = TargetInterface.peekWord(aRegs[arm_sp]);
    aRegs[arm_sp]   += 4;  // "pop" R9
    aRegs[arm_r10]   = TargetInterface.peekWord(aRegs[arm_sp]);
    aRegs[arm_sp]   += 4;  // "pop" R10
    aRegs[arm_r11]   = TargetInterface.peekWord(aRegs[arm_sp]);
    aRegs[arm_sp]   += 4;  // "pop" R11
    if (MPU_Used != 0) {
      aRegs[arm_sp] += 8;  // "pop" CONTROL & Dummy
    }
    aRegs[arm_exec]  = TargetInterface.peekWord(aRegs[arm_sp]);
    aRegs[arm_sp]   += 4;  // "pop" exec location
    //
    // Handle remaining registers through special treatment
    //
    if (Debug.evaluate("OS_SwitchAfterISR_ARM")) {  // This is for embOS for ARM
      //
      // Check if this task has been interrupted (i.e., exec location is addr. of OS_Switch() + 12)
      //
      OSSwitch = Debug.evaluate("OS_SwitchAfterISR_ARM");
      if ((aRegs[arm_exec] & ~1) == (OSSwitch + 12)) {
        Interrupted = 1;
      }
      //
      // Restore appropriate register contents
      //
      if (Interrupted == 0) {  // Remaining register contents have NOT been preserved.
        aRegs[arm_r0]  = "0x00000000";                         // unknown after cooperative task switch
        aRegs[arm_r1]  = "0x00000000";                         // unknown after cooperative task switch
        aRegs[arm_r2]  = "0x00000000";                         // unknown after cooperative task switch
        aRegs[arm_r3]  = "0x00000000";                         // unknown after cooperative task switch
        aRegs[arm_r12] = "0x00000000";                         // unknown after cooperative task switch
        aRegs[arm_lr]  = aRegs[arm_exec];                      // Set LR to exec location
        aRegs[arm_psr] = (aRegs[arm_exec] & 1) ? 0x3F : 0x1F;  // Thumb vs. ARM mode?
      } else {                 // Remaining register contents have been preserved.
        aRegs[arm_r0]  = TargetInterface.peekWord(aRegs[arm_sp]);
        aRegs[arm_sp] += 4;  // "pop" R0
        aRegs[arm_r1]  = TargetInterface.peekWord(aRegs[arm_sp]);
        aRegs[arm_sp] += 4;  // "pop" R1
        aRegs[arm_r2]  = TargetInterface.peekWord(aRegs[arm_sp]);
        aRegs[arm_sp] += 4;  // "pop" R2
        aRegs[arm_r3]  = TargetInterface.peekWord(aRegs[arm_sp]);
        aRegs[arm_sp] += 4;  // "pop" R3
        aRegs[arm_r12] = TargetInterface.peekWord(aRegs[arm_sp]);
        aRegs[arm_sp] += 4;  // "pop" R12
        aRegs[arm_lr]  = TargetInterface.peekWord(aRegs[arm_sp]);
        aRegs[arm_sp] += 4;  // "pop" LR
        aRegs[arm_pc]  = TargetInterface.peekWord(aRegs[arm_sp]);
        aRegs[arm_sp] += 4;  // "pop" PC
        aRegs[arm_psr] = TargetInterface.peekWord(aRegs[arm_sp]);
        aRegs[arm_sp] += 4;  // "pop" PSR
      }
    } else {                                    // This is for embOS for Cortex-M
      var OSSwitchEnd;
      var v;
      //
      // Check if this task has used the FPU
      //
      v = TargetInterface.peekWord(0xE000ED88);
      if ((v & 0x00F00000) != 0) {                    // Is the FPU enabled (CPACR b23..b20)?
        v = TargetInterface.peekWord(0xE000EF34);
        if ((v & 0xC0000000) != 0) {                  // Is the (lazy) hardware state preservation enabled (FPCCR b31..b30)?
          if ((aRegs[arm_exec] & 0x00000010) == 0) {  // Has this task used the FPU (LR b4)?
            FPU_Used = 1;
          }
        }
      }
      //
      // Check if this task has been interrupted (i.e., PC is located in OS_Switch() function)
      //
      if (FPU_Used == 0) {  // FPU registers have not been preserved, PC is located on stack 6 bytes after current SP
        aRegs[arm_pc] = TargetInterface.peekWord(aRegs[arm_sp] + (4 *  6));
      } else {              // FPU registers have been preserved, PC is located on stack 22 bytes after current SP
        aRegs[arm_pc] = TargetInterface.peekWord(aRegs[arm_sp] + (4 * 22));
      }
      OSSwitch    = Debug.evaluate("OS_Switch");
      OSSwitchEnd = Debug.evaluate("OS_Switch_End");
      if ((aRegs[arm_pc] <= OSSwitch) || (aRegs[arm_pc] >= OSSwitchEnd)) {
        Interrupted = 1;
      }
      //
      // Restore appropriate register contents
      //
      if (FPU_Used == 0) {       // FPU registers have not been preserved, use regular stack layout
        if (Interrupted == 0) {  // Task called OS_Switch(), unwind stack to show previous state.
          aRegs[arm_r0]   = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp]  += 4;  // "pop" R0
          aRegs[arm_r1]   = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp]  += 4;  // "pop" R1
          aRegs[arm_r2]   = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp]  += 4;  // "pop" R2
          aRegs[arm_r3]   = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp]  += 4;  // "pop" R3
          aRegs[arm_r12]  = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp]  += 4;  // "pop" R12
          aRegs[arm_lr]   = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp]  += 4;  // "pop" LR
          // aRegs[arm_pc] has already been read above
          aRegs[arm_sp]  += 4;  // "pop" PC
          aRegs[arm_psr]  = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp]  += 4;  // "pop" PSR
          //
          // unwind OS_Switch()
          //
          aRegs[arm_r2]  = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp] += 4;  // "pop" R2
          aRegs[arm_r3]  = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp] += 4;  // "pop" R3
          aRegs[arm_pc]  = aRegs[arm_lr];
        } else {                 // Task was preempted, no additional unwinding required.
          aRegs[arm_r0]   = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp]  += 4;  // "pop" R0
          aRegs[arm_r1]   = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp]  += 4;  // "pop" R1
          aRegs[arm_r2]   = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp]  += 4;  // "pop" R2
          aRegs[arm_r3]   = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp]  += 4;  // "pop" R3
          aRegs[arm_r12]  = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp]  += 4;  // "pop" R12
          aRegs[arm_lr]   = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp]  += 4;  // "pop" LR
          // aRegs[arm_pc] has already been read above
          aRegs[arm_sp]  += 4;  // "pop" PC
          aRegs[arm_psr]  = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp]  += 4;  // "pop" APSR
        }
      } else {                   // FPU registers have been preserved, use extended stack layout
        if (Interrupted == 0) {  // Task called OS_Switch(), unwind stack to show previous state.
          aRegs[arm_sp]  += (4 * 16);  // "pop" S16..S31
          aRegs[arm_r0]   = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp]  += 4;         // "pop" R0
          aRegs[arm_r1]   = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp]  += 4;         // "pop" R1
          aRegs[arm_r2]   = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp]  += 4;         // "pop" R2
          aRegs[arm_r3]   = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp]  += 4;         // "pop" R3
          aRegs[arm_r12]  = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp]  += 4;         // "pop" R12
          aRegs[arm_lr]   = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp]  += 4;         // "pop" LR
          // aRegs[arm_pc] has already been read above
          aRegs[arm_sp]  += 4;         // "pop" PC
          aRegs[arm_psr]  = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp]  += 4;         // "pop" PSR
          aRegs[arm_sp]  += (4 * 18);  // "pop" S0..S15, FPSCR, and Res.
          //
          // unwind OS_Switch()
          //
          aRegs[arm_r2]  = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp] += 4;         // "pop" R2
          aRegs[arm_r3]  = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp] += 4;         // "pop" R3
          aRegs[arm_pc]  = aRegs[arm_lr];
        } else {                 // Task was preempted, no additional unwinding required.
          aRegs[arm_sp]  += (4 * 16);  // "pop" S16..S31
          aRegs[arm_r0]   = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp]  += 4;         // "pop" R0
          aRegs[arm_r1]   = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp]  += 4;         // "pop" R1
          aRegs[arm_r2]   = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp]  += 4;         // "pop" R2
          aRegs[arm_r3]   = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp]  += 4;         // "pop" R3
          aRegs[arm_r12]  = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp]  += 4;         // "pop" R12
          aRegs[arm_lr]   = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp]  += 4;         // "pop" LR
          // aRegs[arm_pc] has already been read above
          aRegs[arm_sp]  += 4;         // "pop" PC
          aRegs[arm_psr]  = TargetInterface.peekWord(aRegs[arm_sp]);
          aRegs[arm_sp]  += 4;         // "pop" APSR
          aRegs[arm_sp]  += (4 * 18);  // "pop" S0..S15, FPSCR, and Res.
        }
      }
    }
    return aRegs;
  }
}

/*********************************************************************
*
*       init()
*
*  Functions description:
*    This function describes the look of the threads window.
*/
function init() {
  Threads.clear();
  Threads.setColumns("Priority", "Id", "Name", "Status", "Timeout", "Stack Info", "Run Count", "Time Slice", "Task Events");
  Threads.setSortByNumber("Priority");
  if (Threads.setColor) {
    Threads.setColor("Status", "Ready", "Executing", "Waiting");
  }
  if (Threads.setColumns2) {
    Threads.setColumns2(Timers,             "Id(" + Timers + ")",       "Name", "Hook",         "Timeout",        "Period"                                           );
    Threads.setColumns2(Mailboxes,          "Id(" + Mailboxes + ")",    "Name", "Messages",     "Message Size",   "Buffer Address", "Waiting Tasks",  "In Use"       );
    Threads.setColumns2(Queues,             "Id(" + Queues + ")",       "Name", "Messages",     "Buffer Address", "Buffer Size",    "Waiting Tasks"                  );
    Threads.setColumns2(Mutexes,            "Id(" + Mutexes + ")",      "Name", "Owner",        "Use Counter",    "Waiting Tasks"                                    );
    Threads.setColumns2(Semaphores,         "Id(" + Semaphores + ")",   "Name", "Count",        "Waiting Tasks"                                                      );
    Threads.setColumns2(MemoryPools,        "Id(" + MemoryPools + ")",  "Name", "Total Blocks", "Block Size",     "Max. Usage",     "Buffer Address", "Waiting Tasks");
    Threads.setColumns2(EventObjects,       "Id(" + EventObjects + ")", "Name", "Signaled",     "Reset Mode",     "Mask Mode",      "Waiting Tasks"                  );
    Threads.setColumns2(WatchDogs,          "Id(" + WatchDogs + ")",    "Name", "Timeout",      "Period"                                                             );
    Threads.setColumns2(SystemInformation,  SystemInformation,          "Value"                                                                                      );
  }
}

/*********************************************************************
*
*       getstate()
*
*  Parameters
*    task: Task Id
*
*  Functions description:
*    This function returns the state of a task.
*/
function getState(task) {
  var WaitObj;
  var sName;
  var sText;

  //
  // Retrieve waiting object name if available
  //
  WaitObj = Debug.evaluate("(((OS_TASK*)" + task.pWaitList + ")->pWaitList)");
  sName   = GetObjectName(WaitObj);
  sText   = (WaitObj) ? (" 0x" + WaitObj.toString(16).toUpperCase()) : "n.a.";
  if (sName != "") {
    sText += " (" + sName + ")";
  }
  //
  // Return corresponding state + waiting object name if available
  //
  if ((task.Stat & TS_MASK_SUSPEND_CNT) != 0) {
    return "Suspended";
  } else {
    switch (task.Stat & (TS_MASK_TIMEOUT | TS_MASK_STATE)) {
    case TS_READY:
      return "Ready";
    case TS_READY | TS_MASK_TIMEOUT:
      return "Delayed";
    case TS_WAIT_ANY:
    case TS_WAIT_ANY | TS_MASK_TIMEOUT:
      return "Blocked";
    case TS_WAIT_EVENT:
    case TS_WAIT_EVENT | TS_MASK_TIMEOUT:
      return "Waiting for Task Event" + sText;
    case TS_WAIT_EVENTOBJ:
    case TS_WAIT_EVENTOBJ | TS_MASK_TIMEOUT:
      return "Waiting for Event Object" + sText;
    case TS_WAIT_MBNE:
    case TS_WAIT_MBNE | TS_MASK_TIMEOUT:
      return "Waiting for message in Mailbox" + sText;
    case TS_WAIT_MBNF:
    case TS_WAIT_MBNF | TS_MASK_TIMEOUT:
      return "Waiting for space in Mailbox" + sText;
    case TS_WAIT_MEMPOOL:
    case TS_WAIT_MEMPOOL | TS_MASK_TIMEOUT:
    case TS_WAIT_MEMPOOL_old:
    case TS_WAIT_MEMPOOL_old | TS_MASK_TIMEOUT:
      return "Waiting for Memory Pool" + sText;
    case TS_WAIT_QNE:
    case TS_WAIT_QNE | TS_MASK_TIMEOUT:
      return "Waiting for message in Queue" + sText;
    case TS_WAIT_QNF:
    case TS_WAIT_QNF | TS_MASK_TIMEOUT:
      return "Waiting for space in Queue" + sText;
    case TS_WAIT_MUTEX:
    case TS_WAIT_MUTEX | TS_MASK_TIMEOUT:
      return "Waiting for Mutex" + sText;
    case TS_WAIT_SEMA:
    case TS_WAIT_SEMA | TS_MASK_TIMEOUT:
      return "Waiting for Semaphore" + sText;
    default:
      return "Invalid";
    }
  }
}

/*********************************************************************
*
*       gettls()
*
*  Functions description:
*    This function is called to retrieve the TLS address of the
*    specified task.
*/
function gettls(pTask) {
  var pTLS;
  var OS_Global;

  //
  // If null is passed to gettls(pTask), get the current task.
  //
  if (pTask == null) {
    OS_Global = Debug.evaluate("&OS_Global");
    pTask = Debug.evaluate("(*(OS_GLOBAL*)" + OS_Global + ").pCurrentTask");
  }
  //
  // Load the pTLS address of the selected task.
  //
  pTLS = Debug.evaluate("(*(OS_TASK*)" + pTask + ").pTLS");
  //
  // If the selected task does not use TLS, load tbss section address instead.
  //
  if (pTLS == 0) {
    pTLS = Debug.evaluate("__tbss_start__");
  }
  return pTLS;
}

/*********************************************************************
*
*       update()
*
*  Functions description:
*    This function is called to update the threads window and its
*    entries upon debug stop.
*/
function update() {
  var pTask;
  var pCurrentTask;
  var OS_TASK;
  var Id;
  var TaskName;
  var NumActivations;
  var StackUsage;
  var TimeSliceReload;
  var TimeSliceRem;
  var sName;
  var OS_Global;

  //
  // Retrive OS_Global object
  //
  OS_Global = Debug.evaluate("OS_Global");
  //
  // Check if embOS is existent
  //
  if (OS_Global == undefined) {
    return;
  }
  //
  // Retrieve start of linked task list from target
  //
  pTask = OS_Global.pTask;
  //
  // Clear entire threads window
  //
  Threads.clear();
  //
  // Update plug in lists
  //
  if (Threads.newqueue2 != undefined) {
    if (Threads.shown(Timers))             UpdateTimers(Threads);
    if (Threads.shown(Mailboxes))          UpdateMailboxes(Threads);
    if (Threads.shown(Queues))             UpdateQueues(Threads);
    if (Threads.shown(Mutexes))            UpdateMutexes(Threads);
    if (Threads.shown(Semaphores))         UpdateSemaphores(Threads);
    if (Threads.shown(MemoryPools))        UpdateMemoryPools(Threads);
    if (Threads.shown(EventObjects))       UpdateEventObjects(Threads);
    if (Threads.shown(WatchDogs))          UpdateWatchDogs(Threads);
    if (Threads.shown(SystemInformation))  UpdateSystemInformation(Threads);
  }
  //
  // Create new queue if at least one task is present
  //
  if (pTask != 0) {
    if (Threads.newqueue2 == undefined) {
      Threads.newqueue("Task List");
    }
    //
    // Retrieve currently executing task, if any
    //
    pCurrentTask = OS_Global.pCurrentTask;
  }
  //
  // Iterate through linked list of tasks and create an entry to the queue for each
  //
  while (pTask != 0) {
    //
    // Get a pointer to this task control block
    //
    OS_TASK = Debug.evaluate("*(OS_TASK*)" + pTask);
    //
    // Retrieve Task ID
    //
    Id = pTask;
    //
    // Retrieve TaskName (unavailable in some libmodes)
    //
    TaskName = GetTaskName(pTask);
    //
    // Retrieve NumActivations (unavailable in some libmodes)
    //
    NumActivations = Debug.evaluate("(unsigned int*)(*(OS_TASK*)" + pTask + ").NumActivations");
    NumActivations = (NumActivations != NULL) ? OS_TASK.NumActivations : "n.a.";
    //
    // Retrieve Stackinfo (unavailable in some libmodes)
    //
    if (OS_TASK.StackSize != undefined) {
      if (OS_TASK.StackSize > StackCheckLimit) {
        //
        // Skip stack check if stack is too big
        //
        MaxStackUsed = "??";
      } else {
        var MaxStackUsed;
        var index;
        if (TargetInterface.findByte) {
          //
          // Check if TargetInterface.findByte is implemented
          //
          index = TargetInterface.findNotByte(OS_TASK.pStackBot, OS_TASK.StackSize, 0xCD);
        } else {
          var abStack = TargetInterface.peekBytes(OS_TASK.pStackBot, OS_TASK.StackSize);
          for (index = 0; index < OS_TASK.StackSize; index++) {
            if (abStack[index].toString(16) != "cd") {
              break;
            }
          }
        }
        MaxStackUsed = OS_TASK.StackSize - index;
      }
      StackUsage   = MaxStackUsed + " / " + OS_TASK.StackSize + " @ 0x" + OS_TASK.pStackBot.toString(16).toUpperCase();
    } else {
      StackUsage = "n.a.";
    }
    //
    // Retrieve TimeSliceReload and TimeSliceRem (unavailable in some libmodes)
    //
    TimeSliceReload = Debug.evaluate("(unsigned int*)(*(OS_TASK*)" + pTask + ").TimeSliceReload");
    TimeSliceReload = (TimeSliceReload != NULL) ? OS_TASK.TimeSliceReload : "n.a.";
    TimeSliceRem    = Debug.evaluate("(unsigned int*)(*(OS_TASK*)" + pTask + ").TimeSliceRem");
    TimeSliceRem    = (TimeSliceRem != NULL) ? OS_TASK.TimeSliceRem : "n.a.";
    //
    // Add task to queue
    //
    Threads.add(OS_TASK.Priority,                                                                                                  // Priority
                "0x" + Id.toString(16).toUpperCase(),                                                                              // ID
                TaskName,                                                                                                          // Name
                (pTask == pCurrentTask) ? "Executing" : getState(OS_TASK),                                                         // Status
                ((OS_TASK.Stat & TS_MASK_TIMEOUT) != 0) ? (OS_TASK.Timeout - OS_Global.Time) + " (" + OS_TASK.Timeout + ")" : "",  // Timeout
                StackUsage,                                                                                                        // Stack info
                NumActivations,                                                                                                    // Run count
                TimeSliceRem + " / " + TimeSliceReload,                                                                            // Time slice
                "0x" + OS_TASK.Events.toString(16).toUpperCase(),                                                                  // Events
                //
                // If task is executing, getregs() must not be called. We therefore pass a null pointer.
                // If task is inactive, getregs() must be called. We therefore pass a pointer to this task.
                //
                (pTask == pCurrentTask) ? [] : pTask
               );
    pTask = OS_TASK.pNext;
  }
}

/****** End Of File *************************************************/
