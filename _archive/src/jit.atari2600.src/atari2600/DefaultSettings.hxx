#ifndef SETTINGS_DEFAULT_HXX
#define SETTINGS_DEFAULT_HXX

#include "bspf.hxx"

/**
  This class defines an empty set of settings.

  @author  Kyle Buza
  @author  Stephen Anthony
*/
class DefaultSettings : public Settings
{
  public:
    
    DefaultSettings();

    /**
      Destructor
    */
    virtual ~DefaultSettings();

  public:
    /**
      This method should be called to get the filename of a state file
      given the state number.

      @param md5   The md5sum to use as part of the filename.
      @param state The state to use as part of the filename.

      @return String representing the full path of the state filename.
    */
    virtual string stateFilename(const string& md5, uInt32 state);

    /**
      This method should be called to test whether the given file exists.

      @param filename The filename to test for existence.

      @return boolean representing whether or not the file exists
    */
    virtual bool fileExists(const string& filename);
};

#endif
