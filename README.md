# ext4-browser

ext4-browser is a file-archiver like utility which can create, read and modify ext2/3/4 disks and disk images on Windows and Mac OS systems.

## Main features
* Add and extract files to/from ext2/3/4 disks and disk images.
* Write existing disk images to disks.
* No installation of any system services required (but it may need admin privilegdes to access physical disks).

## Build guide
### Mac OSX
* In the "makefile" set "TARGET_SYSTEM" to "APPLE".
* Adjust "APPLE_LIB_PATH" and "APPLE_INC_PATH" to the location of your dependencies.
* Build "lwext4" and "wxWidgets" (see below).
* Run "make"

#### wxWidgets
Download the latest source code from https://wxwidgets.org. Then run
```
./configure --enable-unicode --enable-threads CXXFLAGS="-std=c++11 -stdlib=libc++" LDFLAGS="-lc++ -stdlib=libc++" CC=clang CXX=clang++ --with-macosx-version-min=10.9
make -k
```
Please note that something called "webview" is expected to fail building. This can be safely ignored.
To configure the generated .dylib's for relative paths run:
```
install_name_tool -id "@executable_path/libwx_baseu-3.0.0.2.0.dylib" libwx_baseu-3.0.0.2.0.dylib
install_name_tool -id "@executable_path/libwx_osx_cocoau_core-3.0.0.2.0.dylib" libwx_osx_cocoau_core-3.0.0.2.0.dylib
install_name_tool -change /path/to/libwx_baseu-3.0.0.2.0.dylib @executable_path/libwx_baseu-3.0.0.2.0.dylib libwx_osx_cocoau_core-3.0.0.2.0.dylib
install_name_tool -id "@executable_path/libwx_osx_cocoau_adv-3.0.0.2.0.dylib" libwx_osx_cocoau_adv-3.0.0.2.0.dylib
install_name_tool -change /path/to/libwx_baseu-3.0.0.2.0.dylib @executable_path/libwx_baseu-3.0.0.2.0.dylib libwx_osx_cocoau_adv-3.0.0.2.0.dylib
install_name_tool -change /path/to/libwx_osx_cocoau_core-3.0.0.2.0.dylib @executable_path/libwx_osx_cocoau_core-3.0.0.2.0.dylib libwx_osx_cocoau_adv-3.0.0.2.0.dylib
install_name_tool -id "@executable_path/libwx_osx_cocoau_ribbon-3.0.0.2.0.dylib" libwx_osx_cocoau_ribbon-3.0.0.2.0.dylib
install_name_tool -change /path/to/libwx_baseu-3.0.0.2.0.dylib @executable_path/libwx_baseu-3.0.0.2.0.dylib libwx_osx_cocoau_ribbon-3.0.0.2.0.dylib
install_name_tool -change /path/to/libwx_osx_cocoau_core-3.0.0.2.0.dylib @executable_path/libwx_osx_cocoau_core-3.0.0.2.0.dylib libwx_osx_cocoau_ribbon-3.0.0.2.0.dylib
install_name_tool -change /path/to/libwx_osx_cocoau_adv-3.0.0.2.0.dylib @executable_path/libwx_osx_cocoau_adv-3.0.0.2.0.dylib libwx_osx_cocoau_ribbon-3.0.0.2.0.dylib
```

#### lwext4
Get the latest lwext4 sources from https://github.com/gkostka/lwext4. Then run:
```
cmake -DLWEXT4_BUILD_SHARED_LIB=ON /path/to/lwext4/source
make
```
After building execute:
```
install_name_tool -id "@executable_path/liblwext4.dylib" liblwext4.dylib
```

#### Deployment
* Pack all .dylib's from above, Info.plist, the icons folder and the .app file into a folder called "Contents"
* Add the .icns file to "Contents/Resources"
* Put "Contents" into a folder called "ext4-browser.app"

### Windows (MinGW cross compilation)
* In the "makefile" set "TARGET_SYSTEM" to "MINGW".
* Adjust "MINGW_LIB_PATH" and "MINGW_INC_PATH" to the location of your dependencies.
* Build "lwext4" and "wxWidgets" (see below).
* Run "make"

#### wxWidgets
Download the latest source code from https://wxwidgets.org. Then run
```
./configure --prefix=/usr/i686-mingw32/include --host=i686-w64-mingw32 --build=i686-linux --with-mingw --enable-threads --enable-unicode
make
```
#### lwext4
* Get the latest lwext4 sources from https://github.com/gkostka/lwext4.
* Run CMake with the following settings:
```
SYSTEM_NAME=Windows
Version=
Processor=
CC=/usr/bin/i686-w64-mingw32-gcc
CXX=/usr/bin/i686-w64-mingw32-g++
Fortran=
Target Root=/usr/i686-w64-mingw32
Program Mode= Search in Target Root, then native system
Library Mode= Search only in Target Root
Include Mode= Search only in Target Root
LWEXT4_BUILD_SHARED_LIB=ON
```
Now run "make"
#### Deployment
Put all .dll's below, the icons folder and the .exe file into a folder. Done!
```
libgcc_s_sjlj-1.dll
liblwext4.dll
libstdc++-6.dll
libwinpthread-1.dll
wxbase30u_gcc_custom.dll
wxmsw30u_adv_gcc_custom.dll
wxmsw30u_core_gcc_custom.dll
wxmsw30u_ribbon_gcc_custom.dll
```
