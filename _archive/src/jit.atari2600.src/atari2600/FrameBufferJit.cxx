
#include <sstream>


#include "Console.hxx"
#include "FrameBuffer.hxx"
#include "FrameBufferMySDL.hxx"
#include "FrameBufferJit.hxx"
#include "MediaSrc.hxx"
#include "Settings.hxx"
#include "jit.common.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
FrameBufferJit::FrameBufferJit()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
FrameBufferJit::~FrameBufferJit()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool FrameBufferJit::createScreen()
{
  theRedrawEntireFrameIndicator = true;
  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool FrameBufferJit::init()
{

  // Get the desired width and height of the display
  myWidth  = myMediaSource->width() << 1;
  myHeight = myMediaSource->height();

  if(!createScreen()) return false;

  //kyle -  this looks ok
  setupPalette();

  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -



void FrameBufferJit::drawMediaSource(int rowStride, int dim0, int dim1, char* bp)
{
  // Copy the mediasource framebuffer to the RGB texture	
  uInt8* currentFrame  = myMediaSource->currentFrameBuffer();
  //uInt8* previousFrame = myMediaSource->previousFrameBuffer();
  float *p = (float *)bp;
  
  uInt16 pxl;
  uInt32 width         = 160;
  uInt32 height        = 210;
  
  unsigned char r, g, b;
  long tSize = dim1 * dim0;
  
  if(!displayReadsOnly) { 

	//ROM read visualizer OFF.  Just display game pixels.
	
	register Int32 y;
	for(y = 0; y < dim1; ++y )
	{
		const uInt32 bufofsY    = y * width;
		p = (float *)(bp + y*rowStride);

		register Int32 x;

		for(x = 0; x < dim0; ++x )
		{
			const uInt32 bufofs = bufofsY + x;
			uInt8 v = currentFrame[(tSize - bufofs) % (210*160)];
			p += 8;

			pxl = (uInt16) myPalette[v];

			r = (pxl & 0xF800) >> 11;
			g = (pxl & 0x07E0) >> 5;
			b =  pxl & 0x001F;
			*p++ = ((float)(r))/31.0f;
			*p++ = ((float)(g))/63.0f;
			*p++ = ((float)(b))/31.0f;
			*p++ = 1.0f;

			jitPix[(tSize - bufofs) % (210*160)] = pxl;
		}
	}

  } else {  //This is the ROM read visualizer

	 for(uInt32 addr = 0; addr < this->dataSize; addr++) {
		uInt8 va = pcImage[addr];
		jitPix[addr] = myPalette[va];
  		if(va == 0xFFFF && pcImageInfo[addr]) {
			pcImageInfo[addr] += 1;
			
			if(pcImageInfo[addr] < 255) {
				pcImageInfo[addr] += 1;
			}
			pcImage[addr] = ~va;

		} else if (va == 255) {
			pcImageInfo[addr] = 1;
		} else if(va < 255) {
			pcImageInfo[addr] = 0;
		}	
	}	

	for(uInt32 addr = 0; addr < this->dataSize; addr++) {
		uInt16 r = pcImage[addr];
		if(r > 0) {
			r-=1;
			pcImage[addr] = r;
		}
	}

	register Int32 y;
	for(y = 0; y < dim1; ++y )
	{
		const uInt32 bufofsY    = y * width;
		p = (float *)(bp + y*rowStride);

		register Int32 x;

		for(x = 0; x < dim0; ++x )
		{
			const uInt32 bufofs = bufofsY + x;
			uInt8 v = pcImage[(tSize - bufofs) % this->dataSize];
			p += 8;

			pxl = (uInt16) myPalette[v];

			r = (pxl & 0xF800) >> 11;
			g = (pxl & 0x07E0) >> 5;
			b =  pxl & 0x001F;
			*p++ = ((float)(r))/31.0f;
			*p++ = ((float)(g))/63.0f;
			*p++ = ((float)(b))/31.0f;
			*p++ = 1.0f;
		}
	}
  }

  theRedrawEntireFrameIndicator = false;
}
