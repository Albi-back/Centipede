#pragma once

#include <d3d11.h>
#include <stdio.h>
#include <sstream>
#include <cassert>

#include "SimpleMath.h"

/*
Convenience macro for releasing COM objects (also known as resources).
If it isn't null, release it and nullify it.
These are functions/data that have been created by some other system/process
and they take up memory and will not go away until properly "released"
This macro just saves typing and therefore reduces silly bugs
*/
#define ReleaseCOM(x) \
{	if(x){	x->Release();	x = 0;	} }							

/*
Gives us a way to pass messages to the output window in Visual Studio
while the game is running. This means you don't have to halt the app
if you don't want to (with an assert).
*/
#define DBOUT( s )            \
{                             \
   std::ostringstream os_;    \
   os_ << s << "\n";                   \
   OutputDebugString( os_.str().c_str() );  \
}

#define WDBOUT(s)				\
{								\
   std::wostringstream os_;		\
   os_ << s << L"\n";				       \
   OutputDebugStringW( os_.str().c_str() );  \
}

/*
Makes checking if functions worked neater
	e.x.
		HR(SomeD3DFunction());
If it fails the error checking code will get the error message, the
line number and file name and display it all. Saves us typing the
same thing over and over. The app will be stopped.
This checking will disappear in a release build, which is more professional.
Just saves typing and so stops silly bugs.
*/
#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR													
inline void DXError(HRESULT hr, const char* pFileStr, int lineNum)
{
	LPSTR output;
	FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER| FORMAT_MESSAGE_IGNORE_INSERTS,	NULL, hr, MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),	(LPTSTR) &output,	0,	NULL);																							
	static char sErrBuffer[256];																		
	sprintf_s(sErrBuffer,256,"FILE:%s \rLINE:%i \rERROR:%s", pFileStr, lineNum, output);						
	MessageBox(0, sErrBuffer, "Error", 0);																
	assert(false);																						
}
#define HR(x)                                              \
{                                                          \
	HRESULT hr = (x);										\
	if(FAILED(hr))											\
		DXError( hr, __FILE__, __LINE__);					\
}
#endif

#else
#ifndef HR
#define HR(x) (x)
#endif
#endif 


/*
Some predefined common colours in RGBA format
*/
namespace Colours
{
	const DirectX::SimpleMath::Vector4 White = { 1.0f, 1.0f, 1.0f, 1.0f };
	const DirectX::SimpleMath::Vector4 Black = { 0.0f, 0.0f, 0.0f, 1.0f };
	const DirectX::SimpleMath::Vector4 Red = { 1.0f, 0.0f, 0.0f, 1.0f };
	const DirectX::SimpleMath::Vector4 Green = { 0.0f, 1.0f, 0.0f, 1.0f };
	const DirectX::SimpleMath::Vector4 Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
	const DirectX::SimpleMath::Vector4 Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
	const DirectX::SimpleMath::Vector4 Cyan = { 0.0f, 1.0f, 1.0f, 1.0f };
	const DirectX::SimpleMath::Vector4 Magenta = { 1.0f, 0.0f, 1.0f, 1.0f };
}

//pie is always welcome!
const float PI = 3.1415926535f;

/*
comparing floating point numbers is tricky
A number is rarely zero or one, it's more common for it
to be 0.001 or 0.998. So we need a way of comparing numbers
that takes this into account, this simple approach won't
work for everything, but it's useful most of the time.
*/
const float VERY_SMALL = 0.0001f;

inline bool Equals(float a, float b)
{
	return (fabs(a-b) < VERY_SMALL) ? true : false;
}

//knowing the time always comes in handy
float GetClock();
void AddSecToClock(float sec);


