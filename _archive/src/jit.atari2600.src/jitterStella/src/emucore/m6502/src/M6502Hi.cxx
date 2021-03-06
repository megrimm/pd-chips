//============================================================================
//
// MM     MM  6666  555555  0000   2222
// MMMM MMMM 66  66 55     00  00 22  22
// MM MMM MM 66     55     00  00     22
// MM  M  MM 66666  55555  00  00  22222  --  "A 6502 Microprocessor Emulator"
// MM     MM 66  66     55 00  00 22
// MM     MM 66  66 55  55 00  00 22
// MM     MM  6666   5555   0000  222222
//
// Copyright (c) 1995-1998 by Bradford W. Mott
//
// See the file "license" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//
// $Id: M6502Hi.cxx,v 1.2 2002/05/13 19:10:25 stephena Exp $
//============================================================================

#include "M6502Hi.hxx"
#include "jit.common.h"

#define debugStream cout

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
M6502High::M6502High(uInt32 systemCyclesPerProcessorCycle)
    : M6502(systemCyclesPerProcessorCycle)
{
  myNumberOfDistinctAccesses = 0;
  myLastAddress = 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
M6502High::~M6502High()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline uInt8 M6502High::peek(uInt16 address)
{
  if(address != myLastAddress)
  {
    myNumberOfDistinctAccesses++;
    myLastAddress = address;
  }
  mySystem->incrementCycles(mySystemCyclesPerProcessorCycle);
  return mySystem->peek(address);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline void M6502High::poke(uInt16 address, uInt8 value)
{
  if(address != myLastAddress)
  {
    myNumberOfDistinctAccesses++;
    myLastAddress = address;
  }
  mySystem->incrementCycles(mySystemCyclesPerProcessorCycle);
  mySystem->poke(address, value);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool M6502High::execute(uInt32 number)
{
  // Clear all of the execution status bits except for the fatal error bit
  myExecutionStatus &= FatalErrorBit;
  

  // Loop until execution is stopped or a fatal error occurs
  for(;;)
  {
    for(; !myExecutionStatus && (number != 0); --number)
    {
      uInt16 operandAddress = 0;
      uInt8 operand = 0;

#ifdef DEBUG
      debugStream << "PC=" << hex << setw(4) << PC << " ";
#endif

      // Fetch instruction at the program counter
      IR = peek(PC++);  //PC modified
	  //mySystem->fb.doSomething(SP);

#ifdef DEBUG
      debugStream << "IR=" << hex << setw(2) << (int)IR << " ";
      debugStream << "<" << ourAddressingModeTable[IR] << " ";
#endif

      // Call code to execute the instruction
      switch(IR)
      {
        // 6502 instruction emulation is generated by an M4 macro file
        #include "M6502Hi.ins"

        default:
          // Oops, illegal instruction executed so set fatal error flag
          myExecutionStatus |= FatalErrorBit;
		  
      }

#ifdef DEBUG
      debugStream << hex << setw(4) << operandAddress << " ";
      debugStream << setw(4) << ourInstructionMnemonicTable[IR];
      debugStream << "> ";
      debugStream << "A=" << ::hex << setw(2) << (int)A << " ";
      debugStream << "X=" << ::hex << setw(2) << (int)X << " ";
      debugStream << "Y=" << ::hex << setw(2) << (int)Y << " ";
      debugStream << "PS=" << ::hex << setw(2) << (int)PS() << " ";
      debugStream << "SP=" << ::hex << setw(2) << (int)SP << " ";
      debugStream << "Cyc=" << dec << mySystem->cycles();
      debugStream << endl;
#endif
    }

    // See if we need to handle an interrupt
    if((myExecutionStatus & MaskableInterruptBit) || 
        (myExecutionStatus & NonmaskableInterruptBit))
    {
      // Yes, so handle the interrupt
      interruptHandler();
    }

    // See if execution has been stopped
    if(myExecutionStatus & StopExecutionBit)
    {
      // Yes, so answer that everything finished fine
		//post("m6502 finishing fine.");
      return true;
    }

    // See if a fatal error has occured
    if(myExecutionStatus & FatalErrorBit)
    {
      // Yes, so answer that something when wrong
      return false;
    }

    // See if we've executed the specified number of instructions
    if(number == 0)
    {
      // Yes, so answer that everything finished fine
      return true;
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void M6502High::interruptHandler()
{
  // Handle the interrupt
  if((myExecutionStatus & MaskableInterruptBit) && !I)
  {
    mySystem->incrementCycles(7 * mySystemCyclesPerProcessorCycle);
    mySystem->poke(0x0100 + SP--, (PC - 1) >> 8);
    mySystem->poke(0x0100 + SP--, (PC - 1) & 0x00ff);
    mySystem->poke(0x0100 + SP--, PS() & (~0x10));
    D = false;
    I = true;
    PC = (uInt16)mySystem->peek(0xFFFE) | ((uInt16)mySystem->peek(0xFFFF) << 8); //PC modified
	//mySystem->fb.doSomething(PC);
  }
  else if(myExecutionStatus & NonmaskableInterruptBit)
  {
    mySystem->incrementCycles(7 * mySystemCyclesPerProcessorCycle);
    mySystem->poke(0x0100 + SP--, (PC - 1) >> 8);
    mySystem->poke(0x0100 + SP--, (PC - 1) & 0x00ff);
    mySystem->poke(0x0100 + SP--, PS() & (~0x10));
    D = false;
    PC = (uInt16)mySystem->peek(0xFFFA) | ((uInt16)mySystem->peek(0xFFFB) << 8); //PC modified
	//mySystem->fb.doSomething(PC);
  }

  // Clear the interrupt bits in myExecutionStatus
  myExecutionStatus &= ~(MaskableInterruptBit | NonmaskableInterruptBit);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const char* M6502High::name() const
{
  return "M6502High";
}
