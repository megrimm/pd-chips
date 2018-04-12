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
// $Id: Sound.hxx,v 1.12 2004/07/22 01:54:08 stephena Exp $
//============================================================================

#ifndef SOUND_HXX
#define SOUND_HXX

class Console;
class MediaSource;
class System;

#include "bspf.hxx"

/**
  This class is a base class for the various sound objects.
  It has almost no functionality, but is useful if one wishes
  to compile Stella with no sound support whatsoever.

  @author Stephen Anthony and Bradford W. Mott
  @version $Id: Sound.hxx,v 1.12 2004/07/22 01:54:08 stephena Exp $
*/
class Sound
{
  public:
    /**
      Create a new sound object.  The init method must be invoked before
      using the object.
    */
    Sound(uInt32 fragsize = 512);
 
    /**
      Destructor
    */
    virtual ~Sound();

  public: 
    /**
      The system cycle counter is being adjusting by the specified amount.  Any
      members using the system cycle counter should be adjusted as needed.

      @param amount The amount the cycle counter is being adjusted by
    */
    virtual void adjustCycleCounter(Int32 amount);

    /**
      Initializes the sound device.  This must be called before any
      calls are made to derived methods.

      @param console   The console
      @param mediasrc  The mediasource
      @param system    The system
      @param framerate The base framerate depending on NTSC or PAL ROM
    */
    virtual void init(Console* console, MediaSource* mediasrc, System* system,
                      double displayframerate);

    /**
      Return true iff the sound device was successfully initialized.

      @return true iff the sound device was successfully initialized.
    */
    virtual bool isSuccessfullyInitialized() const;

    /**
      Set the mute state of the sound object.  While muted no sound is played.

      @param state Mutes sound if true, unmute if false
    */
    virtual void mute(bool state);

    /**
      Reset the sound device.
    */
    virtual void reset();

    /**
      Sets the sound register to a given value.

      @param addr  The register address
      @param value The value to save into the register
      @param cycle The system cycle at which the register is being updated
    */
    virtual void set(uInt16 addr, uInt8 value, Int32 cycle);

    /**
      Sets the volume of the sound device to the specified level.  The
      volume is given as a percentage from 0 to 100.  Values outside
      this range indicate that the volume shouldn't be changed at all.

      @param percent The new volume percentage level for the sound device
    */
    virtual void setVolume(Int32 percent);

	/**
      Invoked by the sound callback to process the next sound fragment.

      @param stream Pointer to the start of the fragment
      @param length Length of the fragment
    */
    void processFragment(float* stream, unsigned int length);

	void tiamanipulator(int v1, int v2, int v3, int v4);

	protected:

    // Struct to hold information regarding a TIA sound register write
    struct RegWrite
    {
      uInt16 addr;
      uInt8 value;
      double delta;
    };

    /**
      A queue class used to hold TIA sound register writes before being
      processed while creating a sound fragment.
    */
    class RegWriteQueue
    {
      public:
        /**
          Create a new queue instance with the specified initial
          capacity.  If the queue ever reaches its capacity then it will
          automatically increase its size.
        */
        RegWriteQueue(uInt32 capacity = 512);

        /**
          Destroy this queue instance.
        */
        virtual ~RegWriteQueue();

      public:
        /**
          Clear any items stored in the queue.
        */
        void clear();

        /**
          Dequeue the first object in the queue.
        */
        void dequeue();

        /**
          Return the duration of all the items in the queue.
        */
        double duration();

        /**
          Enqueue the specified object.
        */
        void enqueue(const RegWrite& info);

        /**
          Return the item at the front on the queue.

          @return The item at the front of the queue.
        */
        RegWrite& front();

        /**
          Answers the number of items currently in the queue.

          @return The number of items in the queue.
        */
        uInt32 size() const;

      private:
        // Increase the size of the queue
        void grow();

      private:
        uInt32 myCapacity;
        RegWrite* myBuffer;
        uInt32 mySize;
        uInt32 myHead;
        uInt32 myTail;
    };

  protected:
    // The Console for the system
    Console* myConsole;

    // The Mediasource for the system
    MediaSource* myMediaSource;

    // The System for the system
    System* mySystem;

    // Indicates the cycle when a sound register was last set
    Int32 myLastRegisterSetCycle;

    // Indicates the base framerate depending on whether the ROM is NTSC or PAL
    double myDisplayFrameRate;

	// Queue of TIA register writes
    RegWriteQueue myRegWriteQueue;

	bool myIsInitializedFlag;

	double myFragmentSizeLogBase2;

};

#endif
