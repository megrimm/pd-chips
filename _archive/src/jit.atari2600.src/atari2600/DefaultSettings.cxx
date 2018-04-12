#include <sstream>
#include <fstream>
#include <direct.h>

#include "bspf.hxx"
#include "Settings.hxx"
#include "DefaultSettings.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DefaultSettings::DefaultSettings()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DefaultSettings::~DefaultSettings()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
string DefaultSettings::stateFilename(const string& md5, uInt32 state)
{
	return NULL;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool DefaultSettings::fileExists(const string& filename)
{
	return false;
}
