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
// $Id: Console.hxx,v 1.22 2004/07/10 13:20:35 stephena Exp $
//============================================================================

#ifndef CONSOLE_HXX
#define CONSOLE_HXX


class Console;
class Controller;
class Event;
class EventHandler;
class MediaSource;
class PropertiesSet;
class Settings;
class Sound;
class Switches;
class System;
class FrameBuffer;

#include <windows.h>
#include "bspf.hxx"
#include "Control.hxx"
#include "Props.hxx"


/**
  This class represents the entire game console.

  @author  Bradford W. Mott
  @version $Id: Console.hxx,v 1.22 2004/07/10 13:20:35 stephena Exp $
*/
class Console
{
  public:

	  

    enum KeyCode
    {
      KCODE_a, KCODE_b, KCODE_c, KCODE_d, KCODE_e, KCODE_f, KCODE_g, KCODE_h,
      KCODE_i, KCODE_j, KCODE_k, KCODE_l, KCODE_m, KCODE_n, KCODE_o, KCODE_p,
      KCODE_q, KCODE_r, KCODE_s, KCODE_t, KCODE_u, KCODE_v, KCODE_w, KCODE_x,
      KCODE_y, KCODE_z,

      KCODE_0, KCODE_1, KCODE_2, KCODE_3, KCODE_4, KCODE_5, KCODE_6, KCODE_7,
      KCODE_8, KCODE_9,

      KCODE_KP0, KCODE_KP1, KCODE_KP2, KCODE_KP3, KCODE_KP4, KCODE_KP5, KCODE_KP6,
      KCODE_KP7, KCODE_KP8, KCODE_KP9, KCODE_KP_PERIOD, KCODE_KP_DIVIDE,
      KCODE_KP_MULTIPLY, KCODE_KP_MINUS, KCODE_KP_PLUS, KCODE_KP_ENTER,
      KCODE_KP_EQUALS,

      KCODE_BACKSPACE, KCODE_TAB, KCODE_CLEAR, KCODE_RETURN, 
      KCODE_ESCAPE, KCODE_SPACE, KCODE_COMMA, KCODE_MINUS, KCODE_PERIOD,
      KCODE_SLASH, KCODE_BACKSLASH, KCODE_SEMICOLON, KCODE_EQUALS,
      KCODE_QUOTE, KCODE_BACKQUOTE, KCODE_LEFTBRACKET, KCODE_RIGHTBRACKET,

      KCODE_PRTSCREEN, KCODE_SCRLOCK, KCODE_PAUSE,
      KCODE_INSERT,    KCODE_HOME,    KCODE_PAGEUP,
      KCODE_DELETE,    KCODE_END,     KCODE_PAGEDOWN,

      KCODE_LCTRL, KCODE_RCTRL, KCODE_LALT, KCODE_RALT, KCODE_LWIN,
      KCODE_RWIN, KCODE_MENU, KCODE_UP, KCODE_DOWN, KCODE_LEFT, KCODE_RIGHT,

      KCODE_F1, KCODE_F2, KCODE_F3, KCODE_F4, KCODE_F5, KCODE_F6, KCODE_F7,
      KCODE_F8, KCODE_F9, KCODE_F10, KCODE_F11, KCODE_F12, KCODE_F13,
      KCODE_F14, KCODE_F15,

      LastKCODE
    };
    /**
      Create a new console for emulating the specified game using the
      given event object and game profiles.

      @param image       The ROM image of the game to emulate
      @param size        The size of the ROM image  
      @param filename    The name of the file that contained the ROM image
      @param settings    The settings object to use
      @param profiles    The game profiles object to use
      @param framebuffer The framebuffer object to use
      @param sound       The sound object to use
    */
    Console(const uInt8* image, uInt32 size, const char* filename,
        Settings& settings, PropertiesSet& propertiesSet,
        FrameBuffer& framebuffer, Sound& sound);

    /**
      Create a new console object by copying another one

      @param console The object to copy
    */
    Console(const Console& console);
 
    /**
      Destructor
    */
    virtual ~Console();

  public:
    /**
      Updates the console by one frame.  Each frontend should
      call this method 'framerate' times per second.
    */
    void update(int rowStride, int dim0, int dim1, char* bp);
	void myUpdate(Console *c, int rowStride, int dim0, int dim1, char* bp);
	unsigned short* getJitPix();
	void sendDirection(Console* c, char dir, char state);

	Event* myEvent;

	//void DatagramServer(short nPort);

	//void call_Console_foo(Console* c, int i);

	//HANDLE myOSCThread();

    /**
      Get the controller plugged into the specified jack

      @return The specified controller
    */
    Controller& controller(Controller::Jack jack) const
    {
      return (jack == Controller::Left) ? *myControllers[0] : *myControllers[1];
    }

    /**
      Get the properties being used by the game

      @return The properties being used by the game
    */
    const Properties& properties() const;

    /**
      Get the settings of the console

      @return The settings for this console
    */
    Settings& settings() const;

    /**
      Get the frame buffer of the console

      @return The frame buffer
    */
    FrameBuffer& frameBuffer() const;

    /**
      Get the frame rate for the emulation
    */
    uInt32 frameRate() const;

    /**
      Get the sound object of the console

      @return The sound object for this console
    */
    Sound& sound() const;

    /**
      Get the console switches

      @return The console switches
    */
    Switches& switches() const
    {
      return *mySwitches;
    }

    /**
      Get the 6502 based system used by the console to emulate the game

      @return The 6502 based system
    */
    System& system() const
    {
      return *mySystem;
    }

    /**
      Get the event handler of the console

      @return The event handler
    */
    EventHandler& eventHandler() const
    {
      return *myEventHandler;
    }

  public:
    /**
      Overloaded assignment operator

      @param console The console object to set myself equal to
      @return Myself after assignment has taken place
    */
    Console& operator = (const Console& console);

	// Reference to the Sound object
    Sound& mySound;

  public:

    /**
      Toggle between NTSC and PAL mode.  The frontends will need to
      reload their palette.
    */
    void toggleFormat();

    /**
      Toggle between the available palettes.  The frontends will need to
      reload their palette.
    */
    void togglePalette();

	// Pointer to the media source object 
    MediaSource* myMediaSource;

	// Reference to the FrameBuffer object
    FrameBuffer& myFrameBuffer;

  private:
    // Pointers to the left and right controllers
    Controller* myControllers[2];

    // Properties for the game
    Properties myProperties; 

    // Pointer to the switches on the front of the console
    Switches* mySwitches;
 
    // Pointer to the 6502 based system being emulated 
    System* mySystem;

    // Reference to the Settings object
    Settings& mySettings;

    // Reference to the PropertiesSet object
    PropertiesSet& myPropSet;



    // Frame rate being used by the emulator
    uInt32 myFrameRate;

    // Pointer to the EventHandler object
    EventHandler* myEventHandler;
};
#endif
