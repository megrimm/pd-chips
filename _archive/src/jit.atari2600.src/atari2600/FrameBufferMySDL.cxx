
#include <sstream>

#include "Console.hxx"
#include "FrameBuffer.hxx"
#include "FrameBufferMySDL.hxx"
#include "MediaSrc.hxx"
#include "Settings.hxx"

#include "stella.xpm"   // The Stella icon

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
FrameBufferMySDL::FrameBufferMySDL()
   :  theZoomLevel(1),
      theMaxZoomLevel(1),
      theAspectRatio(1.0),
	  newPC(0),
      myPauseStatus(false)
{
	myPalette[0] = myPalette[1]    = 0;
	myPalette[2] = myPalette[3]    = 19017;
	myPalette[4] = myPalette[5]    = 27501; 
	myPalette[6] = myPalette[7]    = 35953;
	myPalette[8] = myPalette[9]    = 44373;
	myPalette[10] = myPalette[11]  = 50712;
	myPalette[12] = myPalette[13]  = 54970;
	myPalette[14] = myPalette[15]  = 61309;
	myPalette[16] = myPalette[17]  = 19008;
	myPalette[18] = myPalette[19]  = 27457;
	myPalette[20] = myPalette[21]  = 33827;
	myPalette[22] = myPalette[23]  = 42245;
	myPalette[24] = myPalette[25]  = 48582;
	myPalette[26] = myPalette[27]  = 54920;
	myPalette[28] = myPalette[29]  = 61257;
	myPalette[30] = myPalette[31]  = 65514;
	myPalette[32] = myPalette[33]  = 31072;
	myPalette[34] = myPalette[35]  = 37442;
	myPalette[36] = myPalette[37]  = 41732;
	myPalette[38] = myPalette[39]  = 46022;
	myPalette[40] = myPalette[41]  = 50311;
	myPalette[42] = myPalette[43]  = 54569;
	myPalette[44] = myPalette[45]  = 56746;
	myPalette[46] = myPalette[47]  = 61004;
	myPalette[48] = myPalette[49]  = 37088;
	myPalette[50] = myPalette[51]  = 41410;
	myPalette[52] = myPalette[53]  = 45701;
	myPalette[54] = myPalette[55]  = 50023;
	myPalette[56] = myPalette[57]  = 54281;
	myPalette[58] = myPalette[59]  = 58539;
	myPalette[60] = myPalette[61]  = 62796;
	myPalette[62] = myPalette[63]  = 65006;
	myPalette[64] = myPalette[65]  = 36864;
	myPalette[66] = myPalette[67]  = 41155;
	myPalette[68] = myPalette[69]  = 47494;
	myPalette[70] = myPalette[71]  = 51785;
	myPalette[72] = myPalette[73]  = 53995;
	myPalette[74] = myPalette[75]  = 58221;
	myPalette[76] = myPalette[77]  = 62480;
	myPalette[78] = myPalette[79]  = 64658;
	myPalette[80] = myPalette[81]  = 32780;
	myPalette[82] = myPalette[83]  = 37071;
	myPalette[84] = myPalette[85]  = 43409;
	myPalette[86] = myPalette[87]  = 47668;
	myPalette[88] = myPalette[89]  = 49878;
	myPalette[90] = myPalette[91]  = 54136;
	myPalette[92] = myPalette[93]  = 58362;
	myPalette[94] = myPalette[95]  = 60540;
	myPalette[96] = myPalette[97]  = 20496;
	myPalette[98] = myPalette[99]  = 26835;
	myPalette[100] = myPalette[101]  = 31125;
	myPalette[102] = myPalette[103]  = 37432;
	myPalette[104] = myPalette[105]  = 41690;
	myPalette[106] = myPalette[107]  = 45948;
	myPalette[108] = myPalette[109]  = 50173;
	myPalette[110] = myPalette[111]  = 54399;
	myPalette[112] = myPalette[113]  = 4114;
	myPalette[114] = myPalette[115]  = 12500;
	myPalette[116] = myPalette[117]  = 18838;
	myPalette[118] = myPalette[119]  = 27224;
	myPalette[120] = myPalette[121]  = 31482;
	myPalette[122] = myPalette[123]  = 37756;
	myPalette[124] = myPalette[125]  = 44062;
	myPalette[126] = myPalette[127]  = 48287;
	myPalette[128] = myPalette[129]  = 18;

	myPalette[130] = myPalette[131]  = 6356;
	myPalette[132] = myPalette[133]  = 10647;
	myPalette[134] = myPalette[135]  = 16985;
	myPalette[136] = myPalette[137]  = 21242;
	myPalette[138] = myPalette[139]  = 25468;
	myPalette[140] = myPalette[141]  = 29726;
	myPalette[142] = myPalette[143]  = 33951;
	myPalette[144] = myPalette[145]  = 241;
	myPalette[146] = myPalette[147]  = 6611;
	myPalette[148] = myPalette[149]  = 10934;
	myPalette[150] = myPalette[151]  = 17304;
	myPalette[152] = myPalette[153]  = 21594;
	myPalette[154] = myPalette[155]  = 25884;
	myPalette[156] = myPalette[157]  = 30141;
	myPalette[158] = myPalette[159]  = 34399;
	myPalette[160] = myPalette[161]  = 396;
	myPalette[162] = myPalette[163]  = 6800;
	myPalette[164] = myPalette[165]  = 11123;
	myPalette[166] = myPalette[167]  = 17494;
	myPalette[168] = myPalette[169]  = 21784;
	myPalette[170] = myPalette[171]  = 26043;
	myPalette[172] = myPalette[173]  = 30333;
	myPalette[174] = myPalette[175]  = 34591;
	myPalette[176] = myPalette[177]  = 518;
	myPalette[178] = myPalette[179]  = 6921;
	myPalette[180] = myPalette[181]  = 11277;
	myPalette[182] = myPalette[183]  = 17648;
	myPalette[184] = myPalette[185]  = 21971;
	myPalette[186] = myPalette[187]  = 26261;
	myPalette[188] = myPalette[189]  = 30520;
	myPalette[190] = myPalette[191]  = 34810;
	myPalette[192] = myPalette[193]  = 544;
	myPalette[194] = myPalette[195]  = 6947;
	myPalette[196] = myPalette[197]  = 13350;
	myPalette[198] = myPalette[199]  = 19721;
	myPalette[200] = myPalette[201]  = 24011;
	myPalette[202] = myPalette[203]  = 28301;
	myPalette[204] = myPalette[205]  = 34640;
	myPalette[206] = myPalette[207]  = 38898;
	myPalette[208] = myPalette[209]  = 4576;
	myPalette[210] = myPalette[211]  = 13027;
	myPalette[212] = myPalette[213]  = 21477;
	myPalette[214] = myPalette[215]  = 27880;
	myPalette[216] = myPalette[217]  = 34218;
	myPalette[218] = myPalette[219]  = 40588;
	myPalette[220] = myPalette[221]  = 46894;
	myPalette[222] = myPalette[223]  = 53232;
	myPalette[224] = myPalette[225]  = 12736;
	myPalette[226] = myPalette[227]  = 21186;
	myPalette[228] = myPalette[229]  = 27557;
	myPalette[230] = myPalette[231]  = 35975;
	myPalette[232] = myPalette[233]  = 42313;
	myPalette[234] = myPalette[235]  = 46603;
	myPalette[236] = myPalette[237]  = 52941;
	myPalette[238] = myPalette[239]  = 59247;
	myPalette[240] = myPalette[241]  = 18784;
	myPalette[242] = myPalette[243]  = 27234;
	myPalette[244] = myPalette[245]  = 33604;
	myPalette[246] = myPalette[247]  = 42023;
	myPalette[248] = myPalette[249]  = 48360;
	myPalette[250] = myPalette[251]  = 54698;
	myPalette[252] = myPalette[253]  = 61036;
	myPalette[254] = myPalette[255]  = 65294;

	pcImage = 0;
	pcImageInfo = 0;
	displayReadsOnly = 0;
	visualizerConst = 0xFFFF;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
FrameBufferMySDL::~FrameBufferMySDL()
{	
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void FrameBufferMySDL::pauseEvent(bool status)
{
  myPauseStatus = status;
  setupPalette();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void FrameBufferMySDL::setupPalette()
{
  theRedrawEntireFrameIndicator = true;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void FrameBufferMySDL::toggleFullscreen()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int FrameBufferMySDL::setImage(const unsigned char* img) {
  return 1;
}

void FrameBufferMySDL::doSomething(uInt16 newPC) {
	pcImage[newPC] |= visualizerConst;
}

void FrameBufferMySDL::doRAM(bool read, uInt16 index ,uInt16 val) {
	//read ? ramAccess[index] = visualizerConst : ramAccess[index] = val;
}

void FrameBufferMySDL::doTia(bool peek, uInt16 addr, uInt16 value) {

}


void FrameBufferMySDL::resize(int mode)
{
  // reset size to that given in properties
  // this is a special case of allowing a resize while in fullscreen mode
  if(mode == 0)
  {
    myWidth  = myMediaSource->width() << 1;
    myHeight = myMediaSource->height();
  }
  else if(mode == 1)   // increase size
  {
    if(myConsole->settings().getBool("fullscreen"))
      return;

    if(theZoomLevel == theMaxZoomLevel)
      theZoomLevel = 1;
    else
      theZoomLevel++;
  }
  else if(mode == -1)   // decrease size
  {
    if(myConsole->settings().getBool("fullscreen"))
      return;

    if(theZoomLevel == 1)
      theZoomLevel = theMaxZoomLevel;
    else
      theZoomLevel--;
  }

  if(!createScreen())
    return;

  // Update the settings
  myConsole->settings().setInt("zoom", theZoomLevel);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void FrameBufferMySDL::showCursor(bool show)
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void FrameBufferMySDL::grabMouse(bool grab)
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool FrameBufferMySDL::fullScreen()
{
  return myConsole->settings().getBool("fullscreen");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt32 FrameBufferMySDL::maxWindowSizeForScreen()
{
  return 4;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void FrameBufferMySDL::setWindowAttributes()
{
}
