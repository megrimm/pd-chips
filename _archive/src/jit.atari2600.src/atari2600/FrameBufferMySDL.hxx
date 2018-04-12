#ifndef FRAMEBUFFER_MYSDL_HXX
#define FRAMEBUFFER_MYSDL_HXX

//#include <SDL.h>
//#include <SDL_syswm.h>

#include "FrameBuffer.hxx"
#include "Settings.hxx"
#include "bspf.hxx"

class FrameBufferMySDL : public FrameBuffer
{
  public:
    /**
      Creates a new SDL framebuffer
    */
    FrameBufferMySDL();

    /**
      Destructor
    */
    virtual ~FrameBufferMySDL();

    /**
      Toggles between fullscreen and window mode.  Grabmouse and hidecursor
      activated when in fullscreen mode.
    */
    void toggleFullscreen();

    /**
      This routine is called when the user wants to resize the window.
      A '1' argument indicates that the window should increase in size, while '-1'
      indicates that the windows should decrease in size.  A '0' indicates that
      the window should be sized according to the current properties.
      Can't resize in fullscreen mode.  Will only resize up to the maximum size
      of the screen.
    */
    void resize(int mode);

    /**
      Shows or hides the cursor based on the given boolean value.
    */
    void showCursor(bool show);

    /**
      Grabs or ungrabs the mouse based on the given boolean value.
    */
    void grabMouse(bool grab);

    /**
      Answers if the display is currently in fullscreen mode.
    */
    bool fullScreen();

    /**
      Answers the current zoom level of the SDL 
    */
    uInt32 zoomLevel() { return theZoomLevel; }

    /**
      Calculate the maximum window size that the current screen can hold.
      Only works in X11 for now.  If not running under X11, always return 4.
    */
    uInt32 maxWindowSizeForScreen();

    /**
      This routine is called to get the width of the onscreen image.
    */
	//kyle - only used for PNG generation.  Return 0
    uInt32 imageWidth() { return 0; }

    /**
      This routine is called to get the height of the onscreen image.
    */
	//kyle - only used for PNG generation.  Return 0
    uInt32 imageHeight() { return 0; }

    /**
      Set the title and icon for the main SDL window.
    */
    void setWindowAttributes();

    /**
      Set up the palette for a screen of any depth > 8.
    */
    void setupPalette();

    //////////////////////////////////////////////////////////////////////
    // The following methods are derived from FrameBuffer.hxx
    //////////////////////////////////////////////////////////////////////
    /**
      This routine is called when the emulation has been paused.

      @param status  Toggle pause based on status
    */
    void pauseEvent(bool status);

    //////////////////////////////////////////////////////////////////////
    // The following methods must be defined in child classes
    //////////////////////////////////////////////////////////////////////
    /**
      This routine is called whenever the screen needs to be recreated.
      It updates the global screen variable.
    */
    virtual bool createScreen() = 0;

    /**
      This routine is called to map a given r,g,b triple to the screen palette.

      @param r  The red component of the color.
      @param g  The green component of the color.
      @param b  The blue component of the color.
    */
    virtual uInt32 mapRGB(uInt8 r, uInt8 g, uInt8 b) = 0;

	int setImage(const unsigned char* img);

	void doSomething(uInt16 newPC);

	void doRAM(bool read, uInt16 index, uInt16 val);

	void doTia(bool peek, uInt16 addr, uInt16 value);

	uInt16 *pcImage;
	
	uInt8 *pcImageInfo;


  protected:
   
    // SDL initialization flags
    uInt32 mySDLFlags;

    // Indicates the current zoom level of the SDL screen
    uInt32 theZoomLevel;

    // Indicates the maximum zoom of the SDL screen
    uInt32 theMaxZoomLevel;

    // The aspect ratio of the window
    float theAspectRatio;

    // Indicates whether the emulation has paused
	bool myPauseStatus;

	uInt16 newPC;
};

#endif
