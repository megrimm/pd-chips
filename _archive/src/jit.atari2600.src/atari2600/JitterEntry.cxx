#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <cstdlib>

#ifdef HAVE_GETTIMEOFDAY
  #include <time.h>
  #include <sys/time.h>
#endif

#include "bspf.hxx"
#include "Console.hxx"
#include "Event.hxx"
#include "StellaEvent.hxx"
#include "EventHandler.hxx"
#include "FrameBuffer.hxx"
#include "FrameBufferMySDL.hxx"
#include "FrameBufferJit.hxx"
#include "PropsSet.hxx"
#include "Sound.hxx"
#include "Settings.hxx"
#include "DefaultSettings.hxx"
#include "jit.common.h"

Console* theConsole = (Console*) NULL;

// Pointer to the display object or the null pointer
FrameBufferMySDL* theDisplay = (FrameBufferMySDL*) NULL;

// Pointer to the sound object or the null pointer
Sound* theSound = (Sound*) NULL;

// Pointer to the settings object or the null pointer
Settings* theSettings = (Settings*) NULL;

void Cleanup()
{
  post("Cleaning up the Atari 2600...");
  if(theSettings) delete theSettings;
  if(theConsole) delete theConsole;
  if(theSound) delete theSound;
  if(theDisplay) delete theDisplay;
  if(theDisplay->pcImage) delete[] theDisplay->pcImage;
  if(theDisplay->pcImageInfo) delete[] theDisplay->pcImageInfo;
}

void SetupProperties(PropertiesSet& set)
{
  bool useMemList = false;
  string theAlternateProFile = theSettings->getString("altpro");
  string theUserProFile      = theSettings->userPropertiesFilename();
  string theSystemProFile    = theSettings->systemPropertiesFilename();

  // When 'listrominfo' or 'mergeprops' is specified, we need to have the
  // full list in memory
  if(theSettings->getBool("listrominfo") || theSettings->getBool("mergeprops"))
    useMemList = true;

  if(theAlternateProFile != "")
    set.load(theAlternateProFile, useMemList);
  else if(theUserProFile != "")
    set.load(theUserProFile, useMemList);
  else if(theSystemProFile != "")
    set.load(theSystemProFile, useMemList);
  else set.load("", false);
}

//New entry point
int jitterEntry(const char* file) {

  if(theSettings) Cleanup();

  const char* filename = (!strrchr(file, '/')) ? file : strrchr(file, '/') + 1;
  uInt8* image;
  uInt32 size;
  
  theSound = new Sound();

  ifstream in(file, ios_base::binary);
  if(!in) {
    post("ERROR. Could not open ROM file \"%s\".  Check the location of the ROM.", file);
	return 0;
  }

  image = new uInt8[512 * 1024];
  in.read((char*)image, 512 * 1024);
  size = in.gcount();
  in.close();

  theDisplay = new FrameBufferJit();

  theDisplay->dataSize = size;
  theDisplay->pcImage = new uInt16[size];
  theDisplay->pcImageInfo = new uInt8[size];

  for(uInt32 addr = 0; addr < size; addr++)
  {
    theDisplay->pcImage[addr] = theDisplay->pcImageInfo[addr] = 0;
  }

  //theSettings = new SettingsWin32();
  theSettings = new DefaultSettings();
  //theSettings = new Settings();
  PropertiesSet propertiesSet;
  SetupProperties(propertiesSet);
 
  theConsole = new Console(image, size, filename, *theSettings, propertiesSet,
                           *theDisplay, *theSound);


  delete[] image;
  return (int)theConsole;
}

