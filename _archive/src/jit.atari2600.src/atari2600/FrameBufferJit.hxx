#ifndef FRAMEBUFFER_JIT_HXX
#define FRAMEBUFFER_JIT_HXX

//#include <SDL.h>
//#include <SDL_opengl.h>
//#include <SDL_syswm.h>

#include "FrameBuffer.hxx"
#include "FrameBufferMySDL.hxx"
#include "bspf.hxx"
#include <gl/gl.h>

class Console;
class MediaSource;

class FrameBufferJit : public FrameBufferMySDL
{
  public:
    /**
      Creates a new SDL OpenGL framebuffer
    */
    FrameBufferJit();

	//virtual int setImage(const unsigned char* img);

    /**
      Destructor
    */
    virtual ~FrameBufferJit();

    //////////////////////////////////////////////////////////////////////
    // The following methods are derived from FrameBufferMySDL.hxx
    //////////////////////////////////////////////////////////////////////
    /**
      This routine is called whenever the screen needs to be recreated.
      It updates the global screen variable.
    */
    virtual bool createScreen();

    /**
      This routine is called to map a given r,g,b triple to the screen palette.

      @param r  The red component of the color.
      @param g  The green component of the color.
      @param b  The blue component of the color.
    */
    virtual uInt32 mapRGB(uInt8 r, uInt8 g, uInt8 b)
      { return 0; }

    //////////////////////////////////////////////////////////////////////
    // The following methods are derived from FrameBuffer.hxx
    //////////////////////////////////////////////////////////////////////
    /**
      This routine should be called once the console is created to setup
      the video system for us to use.  Return false if any operation fails,
      otherwise return true.
    */
    virtual bool init();

    /**
      This routine should be called anytime the MediaSource needs to be redrawn
      to the screen.
    */
    virtual void drawMediaSource(int rowStride, int dim0, int dim1, char* bp);

};

#endif
