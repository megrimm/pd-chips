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
// Copyright (c) 1995-2004 by Bradford W. Mott
//
// See the file "license" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//
// $Id: Console.cxx,v 1.38 2004/08/12 23:54:36 stephena Exp $
//============================================================================

#include <windows.h>		// Header File For Windows
#include <process.h>
#include <assert.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <winsock.h>
#include <math.h>


#include "Booster.hxx"
#include "Cart.hxx"
#include "Console.hxx"
#include "Control.hxx"
#include "Driving.hxx"
#include "Event.hxx"
#include "EventHandler.hxx"
#include "Joystick.hxx"
#include "Keyboard.hxx"
#include "M6502Low.hxx"
#include "M6502Hi.hxx"
#include "M6532.hxx"
#include "MD5.hxx"
#include "MediaSrc.hxx"
#include "Paddles.hxx"
#include "Props.hxx"
#include "PropsSet.hxx"
#include "Settings.hxx" 
#include "Sound.hxx"
#include "Switches.hxx"
#include "System.hxx"
#include "TIA.hxx"
#include "FrameBuffer.hxx"
#include "jit.common.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Console::Console(const uInt8* image, uInt32 size, const char* filename,
    Settings& settings, PropertiesSet& propertiesSet, 
    FrameBuffer& framebuffer, Sound& sound)
    : mySettings(settings),
      myPropSet(propertiesSet),
      myFrameBuffer(framebuffer),
      mySound(sound)
{
  const int frameSoundRate = 50; //PAL

  myControllers[0] = 0;
  myControllers[1] = 0;
  myMediaSource = 0;
  mySwitches = 0;
  mySystem = 0;
  myEvent = 0;
  
  myEventHandler = new EventHandler(this);
  myEvent = myEventHandler->event();

  // Get the MD5 message-digest for the ROM image
  string md5 = MD5(image, size);

  // Search for the properties based on MD5
  myPropSet.getMD5(md5, myProperties);

  // Make sure the MD5 value of the cartridge is set in the properties
  if(myProperties.get("Cartridge.MD5") == "")
  {
    myProperties.set("Cartridge.MD5", md5);
  }

  // Setup the controllers based on properties
  string left = myProperties.get("Controller.Left");
  string right = myProperties.get("Controller.Right");

  // Construct left controller
  if(left == "Booster-Grip")
  {
    myControllers[0] = new BoosterGrip(Controller::Left, *myEvent);
  }
  else if(left == "Driving")
  {
    myControllers[0] = new Driving(Controller::Left, *myEvent);
  }
  else if((left == "Keyboard") || (left == "Keypad"))
  {
    myControllers[0] = new Keyboard(Controller::Left, *myEvent);
  }
  else if(left == "Paddles")
  {
    myControllers[0] = new Paddles(Controller::Left, *myEvent);
  }
  else
  {
    myControllers[0] = new Joystick(Controller::Left, *myEvent);
  }
  
  // Construct right controller
  if(right == "Booster-Grip")
  {
    myControllers[1] = new BoosterGrip(Controller::Right, *myEvent);
  }
  else if(right == "Driving")
  {
    myControllers[1] = new Driving(Controller::Right, *myEvent);
  }
  else if((right == "Keyboard") || (right == "Keypad"))
  {
    myControllers[1] = new Keyboard(Controller::Right, *myEvent);
  }
  else if(right == "Paddles")
  {
    myControllers[1] = new Paddles(Controller::Right, *myEvent);
  }
  else
  {
    myControllers[1] = new Joystick(Controller::Right, *myEvent);
  }

  // Create switches for the console
  mySwitches = new Switches(*myEvent, myProperties);

  // Now, we can construct the system and components
  mySystem = new System(13, 6, myFrameBuffer);

  M6502* m6502;
  if(myProperties.get("Emulation.CPU") == "Low")
  {
    m6502 = new M6502Low(1);
  }
  else
  {
    m6502 = new M6502High(1);
  }

  M6532* m6532 = new M6532(*this);
  TIA* tia = new TIA(*this, mySound);
  
  Cartridge* cartridge = Cartridge::create(image, size, myProperties);
  //int result = myFrameBuffer.setImage(image);

  mySystem->attach(m6502);
  mySystem->attach(m6532);
  mySystem->attach(tia);
  mySystem->attach(cartridge);

  // Remember what my media source is
  myMediaSource = tia;

  // Reset, the system to its power-on state
  mySystem->reset();

  // Set the correct framerate based on the format of the ROM
  // This can be overridden by the '-framerate' option
  myFrameRate = frameSoundRate;
  if(mySettings.getInt("framerate") > 0)
    myFrameRate = mySettings.getInt("framerate");
//  else if(myProperties.get("Display.Format") == "NTSC")
//    myFrameRate = 60;
//  else if(myProperties.get("Display.Format") == "PAL")
//    myFrameRate = 50;
//  mySettings.setInt("framerate", myFrameRate, false);
  mySettings.setInt("framerate", myFrameRate);

  // Initialize the framebuffer interface.
  // This must be done *after* a reset, since it needs updated values.
  
  myFrameBuffer.initDisplay(this, myMediaSource);
  
  // Initialize the sound interface.
  //uInt32 soundFrameRate = (myProperties.get("Display.Format") == "PAL") ? 50 : 60;
  uInt32 soundFrameRate = frameSoundRate;
  mySound.init(this, myMediaSource, mySystem, soundFrameRate);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Console::Console(const Console& console)
    : mySettings(console.mySettings),
      myPropSet(console.myPropSet),
      myFrameBuffer(console.myFrameBuffer),
      mySound(console.mySound)
{
  // TODO: Write this method
  assert(false);
}
 
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Console::~Console()
{
  delete mySystem;
  delete mySwitches;
  delete myControllers[0];
  delete myControllers[1];
  delete myEventHandler;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Console::update(int rowStride, int dim0, int dim1, char* bp)
{
  myFrameBuffer.update(rowStride, dim0, dim1, bp);
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const Properties& Console::properties() const
{
  return myProperties;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Settings& Console::settings() const
{
  return mySettings;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
FrameBuffer& Console::frameBuffer() const
{
  return myFrameBuffer;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt32 Console::frameRate() const
{
  return myFrameRate;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Sound& Console::sound() const
{
  return mySound;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Console& Console::operator = (const Console&)
{
  // TODO: Write this method
  assert(false);

  return *this;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Console::toggleFormat()
{
  string format = myProperties.get("Display.Format");

  if(format == "NTSC")
  {
    myProperties.set("Display.Format", "PAL");
    mySystem->reset();
    //myFrameBuffer.showMessage("PAL Mode");
  }
  else if(format == "PAL")
  {
    myProperties.set("Display.Format", "NTSC");
    mySystem->reset();
    //myFrameBuffer.showMessage("NTSC Mode");
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Console::togglePalette()
{
  string type = mySettings.getString("palette");

  if(type == "standard")  // switch to original
  {
    //myFrameBuffer.showMessage("Original Stella colors");
    mySettings.setString("palette", "original");
  }
  else if(type == "original")  // switch to z26
  {
    //myFrameBuffer.showMessage("Z26 colors");
    mySettings.setString("palette", "z26");
  }
  else if(type == "z26")  // switch to standard
  {
    //myFrameBuffer.showMessage("Standard Stella colors");
    mySettings.setString("palette", "standard");
  }
  else  // switch to standard mode if we get this far
  {
    //myFrameBuffer.showMessage("Standard Stella colors");
    mySettings.setString("palette", "standard");
  }
}

extern "C" void call_updateConsole(Console* c, int rowStride, int dim0, int dim1, char* bp) { 
	return c->myUpdate(c, rowStride, dim0, dim1, bp);
};

extern "C" unsigned short* get_jit_pix(Console* c) { 
	return c->getJitPix();
};

extern "C" void call_manipulator(Console* c, long val) { 
	c->myMediaSource->manipulator = val;
};

extern "C" void call_manipulator2(Console* c, long val) { 
	c->myMediaSource->manipulator2 = val;
};

extern "C" void call_manipulator3(Console* c, long val) { 
	c->myMediaSource->manipulator3 = val;
};

extern "C" void call_manipulator4(Console* c, long val) { 
	c->myMediaSource->manipulator4 = val;
};

extern "C" void call_manipulator5(Console* c, long val) { 
	c->myMediaSource->manipulator5 = val;
};

extern "C" void call_manipulator6(Console* c, long val) { 
	c->myMediaSource->manipulator6 = val;
};

extern "C" void call_manipulator7(Console* c, long val) { 
	c->myMediaSource->manipulator7 = val;
};

extern "C" void call_manipulator8(Console* c, long val) { 
	c->myMediaSource->manipulator8 = val;
};

extern "C" void call_manipulator9(Console* c, long val) { 
	c->myMediaSource->manipulator9 = val;
};

extern "C" void call_manipulator10(Console* c, long val) { 
	c->myMediaSource->manipulator10 = val;
};
extern "C" void call_manipulator11(Console* c, long val) { 
	c->myMediaSource->manipulator11 = val;
};
extern "C" void call_manipulator12(Console* c, long val) { 
	c->myMediaSource->manipulator12 = val;
};
extern "C" void call_manipulator13(Console* c, long val) { 
	c->myMediaSource->manipulator13 = val;
};

extern "C" void fillAudioBuffer(Console* c, float* buf, unsigned int size) {
	c->mySound.processFragment(buf, size);
};

extern "C" void tiamanip(Console* c, int v1, int v2, int v3, int v4) {
	c->mySound.tiamanipulator(v1, v2, v3, v4);
}

extern "C" void display_reads(Console* c, long val) { 
	c->myFrameBuffer.displayReadsOnly = val;
};

extern "C" void set_visualizer_const(Console* c, long val) { 
	c->myFrameBuffer.visualizerConst = val;
};
extern "C" void mod_palette(Console* c, long index, long val) { 
	//Index has been checked in jitter source
	c->myFrameBuffer.myPalette[index] = val;
};

extern "C" void sendDirection(Console* c, char dir, char state) {
		switch(dir) {
			case(123):
			  c->eventHandler().myEvent->set(Event::JoystickZeroLeft, state);
			  break;
			case(124):
		 	  c->eventHandler().myEvent->set(Event::JoystickZeroRight, state);
			  break;
			case(126):
			  c->eventHandler().myEvent->set(Event::JoystickZeroUp, state);
			  break;
			case(125):
			  c->eventHandler().myEvent->set(Event::JoystickZeroDown, state);
			  break;
			case(49):
			  c->eventHandler().myEvent->set(Event::JoystickZeroFire, state);
			  break;
			case(12):
			  c->eventHandler().myEvent->set(Event::ConsoleReset, state);
			  break;
			case(117):
			  c->eventHandler().myEvent->set(Event::ConsoleSelect, state);
			  break;
		}
}

void Console::myUpdate(Console *c, int rowStride, int dim0, int dim1, char* bp) {
    c->update(rowStride, dim0, dim1, bp);
}

unsigned short* Console::getJitPix() {
	return myFrameBuffer.jitPix;
}
