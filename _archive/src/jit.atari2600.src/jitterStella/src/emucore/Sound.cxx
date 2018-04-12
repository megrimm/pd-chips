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
// $Id: Sound.cxx,v 1.13 2004/07/22 01:54:08 stephena Exp $
//============================================================================

#include "Sound.hxx"
#include "Console.hxx"
#include "TIASound.h"
#include "jit.common.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Sound::Sound(uInt32 fragsize)
    : myLastRegisterSetCycle(0)
{
	Tia_sound_init(31400, 31400);
	myIsInitializedFlag = true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Sound::~Sound()
{
	myIsInitializedFlag = false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Sound::adjustCycleCounter(Int32 amount)
{
  myLastRegisterSetCycle += amount;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Sound::mute(bool state)
{
}

void Sound::tiamanipulator(int v1, int v2, int v3, int v4) {
	Tia_manipulate(v1, v2, v3, v4);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Sound::init(Console* console, MediaSource* mediasrc, System* system,
                 double displayframerate)
{
  myConsole = console;
  myMediaSource = mediasrc;
  mySystem = system;
  myLastRegisterSetCycle = 0;
  myDisplayFrameRate = displayframerate;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Sound::isSuccessfullyInitialized() const
{
  return myIsInitializedFlag;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Sound::reset()
{
	myRegWriteQueue.clear();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Sound::set(uInt16 addr, uInt8 value, Int32 cycle)
{
  // First, calulate how many seconds would have past since the last
  // register write on a real 2600
  double delta = (((double)(cycle - myLastRegisterSetCycle)) / 
      (1193191.66666667));

  // Now, adjust the time based on the frame rate the user has selected
  delta = delta * (myDisplayFrameRate / (double)myConsole->frameRate());

  RegWrite info;
  info.addr = addr;
  info.value = value;
  info.delta = delta;
  myRegWriteQueue.enqueue(info);

  // Update last cycle counter to the current cycle
  myLastRegisterSetCycle = cycle;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Sound::setVolume(Int32 volume)
{
}

void Sound::processFragment(float* stream, unsigned int length)
{
  if(!myIsInitializedFlag)
  {
    return;
  }

  // If there are excessive items on the queue then we'll remove some
  if(myRegWriteQueue.duration() > (myFragmentSizeLogBase2 / myDisplayFrameRate))
  {
    double removed = 0.0;
    while(removed < ((myFragmentSizeLogBase2 - 1) / myDisplayFrameRate))
    {
      RegWrite& info = myRegWriteQueue.front();
      removed += info.delta;
      Update_tia_sound(info.addr, info.value);
      myRegWriteQueue.dequeue();
    }
//    cout << "Removed Items from RegWriteQueue!" << endl;
  }

  double position = 0.0;
  double remaining = length;

  while(remaining > 0.0)
  {
    if(myRegWriteQueue.size() == 0)
    {
      // There are no more pending TIA sound register updates so we'll
      // use the current settings to finish filling the sound fragment
      Tia_processX(stream + (uInt32)position, length - (uInt32)position);

      // Since we had to fill the fragment we'll reset the cycle counter
      // to zero.  NOTE: This isn't 100% correct, however, it'll do for
      // now.  We should really remember the overrun and remove it from
      // the delta of the next write.
      myLastRegisterSetCycle = 0;
      break;
    }
    else
    {
      // There are pending TIA sound register updates so we need to
      // update the sound buffer to the point of the next register update
      RegWrite& info = myRegWriteQueue.front();

      // How long will the remaing samples in the fragment take to play
      double duration = remaining / (double)31400;

      // Does the register update occur before the end of the fragment?
      if(info.delta <= duration)
      {
        // If the register update time hasn't already passed then
        // process samples upto the point where it should occur
        if(info.delta > 0.0)
        {
          // Process the fragment upto the next TIA register write.  We
          // round the count passed to Tia_processX up if needed.
          double samples = (31400 * info.delta);
          Tia_processX(stream + (uInt32)position, (uInt32)samples +
              (uInt32)(position + samples) - 
              ((uInt32)position + (uInt32)samples));
          position += samples;
          remaining -= samples;
        }
        Update_tia_sound(info.addr, info.value);
        myRegWriteQueue.dequeue();
      }
      else
      {
        // The next register update occurs in the next fragment so finish
        // this fragment with the current TIA settings and reduce the register
        // update delay by the corresponding amount of time
        Tia_processX(stream + (uInt32)position, length - (uInt32)position);
        info.delta -= duration;
        break;
      }
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Sound::RegWriteQueue::RegWriteQueue(uInt32 capacity)
    : myCapacity(capacity),
      myBuffer(0),
      mySize(0),
      myHead(0),
      myTail(0)
{
  myBuffer = new RegWrite[myCapacity];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Sound::RegWriteQueue::~RegWriteQueue()
{
  delete[] myBuffer;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Sound::RegWriteQueue::clear()
{
  myHead = myTail = mySize = 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Sound::RegWriteQueue::dequeue()
{
  if(mySize > 0)
  {
    myHead = (myHead + 1) % myCapacity;
    --mySize;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double Sound::RegWriteQueue::duration()
{
  double duration = 0.0;
  for(uInt32 i = 0; i < mySize; ++i)
  {
    duration += myBuffer[(myHead + i) % myCapacity].delta;
  }
  return duration;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Sound::RegWriteQueue::enqueue(const RegWrite& info)
{
  // If an attempt is made to enqueue more than the queue can hold then
  // we'll enlarge the queue's capacity.
  if(mySize == myCapacity)
  {
    grow();
  }

  myBuffer[myTail] = info;
  myTail = (myTail + 1) % myCapacity;
  ++mySize;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Sound::RegWrite& Sound::RegWriteQueue::front()
{
  //assert(mySize != 0);
  return myBuffer[myHead];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt32 Sound::RegWriteQueue::size() const
{
  return mySize;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Sound::RegWriteQueue::grow()
{
  RegWrite* buffer = new RegWrite[myCapacity * 2];
  for(uInt32 i = 0; i < mySize; ++i)
  {
    buffer[i] = myBuffer[(myHead + i) % myCapacity];
  }
  myHead = 0;
  myTail = mySize;
  myCapacity = myCapacity * 2;
  delete[] myBuffer;
  myBuffer = buffer;
}
