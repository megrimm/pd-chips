//============================================================================
//
//   SSSS    tt          lll  lll       
//  SS  SS   tt           ll   ll        
//  SS     tttttt  eeee   ll   ll   aaaa 
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-1998 by Bradford W. Mott
//
// See the file "license" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//
// $Id: Cart4K.cxx,v 1.3 2002/05/14 15:22:28 stephena Exp $
//============================================================================

#include <assert.h>
#include "Cart4K.hxx"
#include "System.hxx"
#include "jit.common.h"
#include <iostream>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Cartridge4K::Cartridge4K(const uInt8* image)
{
  cartSize = 4096;

  post("Creating Cart4K");

  // Copy the ROM image into my buffer
  for(uInt32 addr = 0; addr < 4096; ++addr)
  {
    myImage[addr] = image[addr];
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Cartridge4K::~Cartridge4K()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const char* Cartridge4K::name() const
{
  return "Cartridge4K";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Cartridge4K::reset()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Cartridge4K::install(System& system)
{
  mySystem = &system;
  uInt16 shift = mySystem->pageShift();
  uInt16 mask = mySystem->pageMask();

  // Make sure the system we're being installed in has a page size that'll work
  assert((0x1000 & mask) == 0);

  System::PageAccess access;
  access.directPokeBase = 0;
  access.device = this;

  // Map ROM image into the system
  for(uInt32 address = 0x1000; address < 0x2000; address += (1 << shift))
  {
	//This associated PageAccess mapped from addresses 64 -> ??
    //access.directPeekBase = &myImage[address & 0x0FFF];
	access.directPeekBase = 0;
	//uInt32 dd = address >> mySystem->pageShift();
    mySystem->setPageAccess(address >> mySystem->pageShift(), access);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 Cartridge4K::peek(uInt16 address)
{
  mySystem->fb.doSomething(address & 0x0FFF);
	  //address += 10;
  return myImage[address & 0x0FFF];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Cartridge4K::poke(uInt16, uInt8)
{
  // This is ROM so poking has no effect :-)
} 
