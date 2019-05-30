# RGF Library

A way for normal Win32 applications to have Load/Save features from either a local file or GoogleDrive/OneDrive/DropBox.


![WUP](https://raw.githubusercontent.com/WindowsNT/rgf/master/1.jpg)
![Plain](https://raw.githubusercontent.com/WindowsNT/rgf/master/2.jpg)


## Usage:

```C++
#include "rgf1.h" // Generic library, containing REST, GOD, ystring, RKEY, AX etc
#include "rgf2.h" // For UWP interface (Windows 10 Build 1903+)
#include "rgf3.h" // For Common dialog interface
```

If using the common dialog interface, add also rgf.rc/resource.h to your project.

If using the UWP interface, add 1.manifest to your project.


## Configuring the library with your client/secret for googledrive/onedrive/dropbox and access/refresh tokens if you have them:

```C++
RGB::RGBF s;
s.google.tokens.resize(3);
s.google.id = "...";
s.google.secret = "...";
s.google.tokens[0] = "... access token ...";
s.google.tokens[1] = "... refresh token ...";
// Same for s.one and s.db	
```

If you do not pass an access token, the user will be asked to login. By default, this is done in an external browser (your default), but 
you can change the function "RunUrl" to use my own AX interface which opens an IWebBrowser control.

Your redirect URL is listening at port 9932 by default.


## Opening

```C++
RGB::RGBF s;
std::vector<char> r;
s.read = &r;
HRESULT rv = RGF::Open(s); // Uses the UWP Interface
HRESULT rv = RGF::Open2(s); // Uses the old Interface
```

On success (rv == S_*) the passed vector is filled with the data of the opened file, whether it is a local file or a remote file.


## Saving
```C++
RGF::RGBF s;
s.d = d.data();
s.sz = (DWORD)d.size();
s.resultFile = L"r:\\1.dat"; // for default local saving
s.DefExt = L"dat"; // added automatically to remote files as well
HRESULT rv = RGF::Save(s); // Uses the UWP Interface
HRESULT rv = RGF::Save2(s); // Uses the old Interface
```


