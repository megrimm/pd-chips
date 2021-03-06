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
// $Id: FrameBuffer.hxx,v 1.12 2004/07/28 23:54:39 stephena Exp $
//============================================================================

#ifndef FRAMEBUFFER_HXX
#define FRAMEBUFFER_HXX

#include "bspf.hxx"
#include "Event.hxx"
#include "MediaSrc.hxx"
#include "StellaEvent.hxx"

class Console;

/**
  This class encapsulates the MediaSource and is the basis for the video
  display in Stella.  All ports should derive from this class for
  platform-specific video stuff.

  This class also implements a MAME-like user interface where Stella settings
  can be changed.

  @author  Stephen Anthony
  @version $Id: FrameBuffer.hxx,v 1.12 2004/07/28 23:54:39 stephena Exp $
*/
class FrameBuffer
{
  public:
    /**
      Creates a new Frame Buffer
    */
    FrameBuffer();

    /**
      Destructor
    */
    virtual ~FrameBuffer();

    /**
      Initializes the framebuffer display.  This must be called before any
      calls are made to derived methods.

      @param console   The console
      @param mediasrc  The mediasource
    */
    void initDisplay(Console* console, MediaSource* mediasrc);

	virtual int setImage(const unsigned char* img);

	virtual void doSomething(uInt16 newPC);
	virtual void doRAM(bool read, uInt16 index ,uInt16 val);
	virtual void doTia(bool peek, uInt16 addr, uInt16 value);

    /**
      Updates the display.  Also draws any pending menus, etc.
    */
    void update(int rowStride, int dim0, int dim1, char* bp);

    /**
      Returns the current width of the framebuffer.
      Note that this will take into account the current scaling (if any).

      @return  The current width
    */
    uInt32 width()  { return myWidth; }

    /**
      Returns the current height of the framebuffer.
      Note that this will take into account the current scaling (if any).

      @return  The current height
    */
    uInt32 height() { return myHeight; }

    /**
      Send a keyboard event to the user interface.

      @param code   The StellaEvent code
      @param state  The StellaEvent state
    */
    void sendKeyEvent(StellaEvent::KeyCode code, Int32 state);

    /**
      Send a joystick button event to the user interface.

      @param stick The joystick activated
      @param code  The StellaEvent joystick code
      @param state The StellaEvent state
    */
    void sendJoyEvent(StellaEvent::JoyStick stick, StellaEvent::JoyCode code,
         Int32 state);

     /**
      Sets the pause status.  While pause is selected, the
      MediaSource will not be updated.

      @param status  Toggle pause based on status
    */
    void pause(bool status);

    /**
      Indicates that a redraw should be done, since the window contents
      are dirty.
    */
    void refresh() { theRedrawEntireFrameIndicator = true; }

	unsigned short* jitPix;
	int dataSize;

  public:
    //////////////////////////////////////////////////////////////////////
    // The following methods are system-specific and must be implemented
    // in derived classes.
    //////////////////////////////////////////////////////////////////////

    /**
      This routine should be called once the console is created to setup
      the video system for us to use.  Return false if any operation fails,
      otherwise return true.
    */
    virtual bool init() = 0;

    /**
      This routine should be called anytime the MediaSource needs to be redrawn
      to the screen.
    */
    virtual void drawMediaSource(int rowStride, int dim0, int dim1, char* bp) = 0;

    /**
      This routine is called when the emulation has received
      a pause event.

      @param status  The received pause status
    */
    virtual void pauseEvent(bool status) = 0;

    /**
      This routine is called to get the width of the onscreen image.
    */
    virtual uInt32 imageWidth() = 0;

    /**
      This routine is called to get the height of the onscreen image.
    */
    virtual uInt32 imageHeight() = 0;

	long displayReadsOnly;
	long visualizerConst;
	
    // SDL palette
    uInt32 myPalette[256];

	uInt16 ramAccess[32*1024];

  protected:
    // The Console for the system
    Console* myConsole;

    // The Mediasource for the system
    MediaSource* myMediaSource;

    // Bounds for the window frame
    uInt32 myWidth, myHeight;

    // Indicates if the entire frame should be redrawn
    bool theRedrawEntireFrameIndicator;

    // Table of bitmapped fonts.
    static const uInt8 ourFontData[2048];

    // Holds the foreground and background color table indices
    uInt8 myFGColor, myBGColor;

  private:
    // Enumeration representing the different types of user interface widgets
    enum Widget { W_NONE, MAIN_MENU, REMAP_MENU, INFO_MENU };

    Widget currentSelectedWidget();
    Event::Type currentSelectedEvent();

    // Add binding between a StellaEvent key and a core event
    void addKeyBinding(Event::Type event, StellaEvent::KeyCode key);

    // Add binding between a StellaEvent joystick and a core event
    void addJoyBinding(Event::Type event, StellaEvent::JoyStick stick,
                       StellaEvent::JoyCode code);

    // Remove all bindings for this core event
    void deleteBinding(Event::Type event);

    // Move the cursor up 'amt' lines, possibly scrolling the list of items
    void moveCursorUp(uInt32 amt);

    // Move the cursor down 'amt' lines, possibly scrolling the list of items
    void moveCursorDown(uInt32 amt);

    // scan the mapping arrays and update the remap menu
    void loadRemapMenu();

  private:
    // Indicates the current framerate of the system
    uInt32 myFrameRate;

    // Indicates the current pause status
    bool myPauseStatus;

    // Structure used for main menu items
    struct MainMenuItem
    {
      Widget widget;
      string action;
    };

    // Structure used for remap menu items
    struct RemapMenuItem
    {
      Event::Type event;
      string action;
      string key;
    };

    // Table of strings representing the various StellaEvent codes
    static const char* ourEventName[StellaEvent::LastKCODE];

    // Type of interface item currently slated for redraw
    Widget myCurrentWidget;

    // Indicates that an event is currently being remapped
    bool myRemapEventSelectedFlag;

    // Indicates the current selected event being remapped
    Event::Type mySelectedEvent;

    // Indicates if we are in menu mode
    bool myMenuMode;

    // Indicates if the menus should be redrawn
    bool theMenuChangedIndicator;

    // The maximum number of vertical lines of text that can be onscreen
    Int32 myMaxRows;

    // The maximum number of characters of text in a row
    Int32 myMaxColumns;

    // Keep track of current selected main menu item
    uInt32 myMainMenuIndex, myMainMenuItems;

    // Keep track of current selected remap menu item
    Int32 myRemapMenuIndex, myRemapMenuLowIndex, myRemapMenuHighIndex;
    Int32 myRemapMenuItems, myRemapMenuMaxLines;

    // Message timer
    Int32 myMessageTime;

    // Message text
    string myMessageText;

    // Number of times menu have been drawn
    uInt32 myMenuRedraws;

    // The width of the information menu, determined by the longest string
    Int32 myInfoMenuWidth;

    // Holds information about the current selected ROM image
    string ourPropertiesInfo[9];

    // Holds static strings for the main menu
    static MainMenuItem ourMainMenu[2];

    // Holds static strings for the remap menu
    static RemapMenuItem ourRemapMenu[57];

    // Holds the current key mappings
    Event::Type* myKeyTable;

    // Holds the number of items in the keytable array
    uInt32 myKeyTableSize;

    // Holds the current joystick mappings
    Event::Type* myJoyTable;

    // Holds the number of items in the joytable array
    uInt32 myJoyTableSize;
};

#endif
