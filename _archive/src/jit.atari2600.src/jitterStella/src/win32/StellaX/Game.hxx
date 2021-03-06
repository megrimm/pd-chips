//============================================================================
//
//   SSSS    tt          lll  lll          XX     XX
//  SS  SS   tt           ll   ll           XX   XX
//  SS     tttttt  eeee   ll   ll   aaaa     XX XX
//   SSSS    tt   ee  ee  ll   ll      aa     XXX
//      SS   tt   eeeeee  ll   ll   aaaaa    XX XX
//  SS  SS   tt   ee      ll   ll  aa  aa   XX   XX
//   SSSS     ttt  eeeee llll llll  aaaaa  XX     XX
//
// Copyright (c) 1995-2000 by Jeff Miller
// Copyright (c) 2004 by Stephen Anthony
//
// See the file "license" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//
// $Id: Game.hxx,v 1.2 2004/07/15 03:03:27 stephena Exp $
//============================================================================

#ifndef GAME_HXX
#define GAME_HXX

#include "bspf.hxx"

/** Hold game info for an Atari2600 game. */
class Game
{
  public:
    Game( void );
    ~Game( void );

    /** sets the rom of a game */
    void setAvailable( bool available ) { _available = available; }
    /** returns the rom of a game */
    bool isAvailable( void ) { return _available; }

    /** sets the rom of a game */
    void setRom( const string& rom ) { _rom = rom; };
    /** returns the rom of a game */
    string rom( void ) const { return _rom; }

    /** sets the md5 sum of a game */
    void setMd5( const string& md5 )  { _md5 = md5; };
    /** returns the md5 sum of a game */
    string md5( void ) const { return _md5; }

    /** sets the name of a game */
    void setName( const string& name ) { _name = name; }
    /** returns the name of a game */
    string name( void ) const { return _name; }

    /** sets the rarity of a game */
    void setRarity( const string& rarity ) { _rarity = rarity; }
    /** returns the rarity of a game */
    string rarity( void ) const { return _rarity; }

    /** sets the manufacturer of a game */
    void setManufacturer( const string& manufacturer) { _manufacturer = manufacturer; }
    /** returns the manufacturer of a game */
    string manufacturer( void ) const { return _manufacturer; }

    /** sets the note of a game */
    void setNote( const string& note ) { _note = note; }
    /** returns the note of a game */
    string note( void ) const { return _note; }

  protected:
    bool _available;
    string _rom;
    string _md5;
    string _name;
    string _rarity;
    string _manufacturer;
    string _note;
};

#endif
