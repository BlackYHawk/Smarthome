# Build both ARMv5TE and ARMv7-A machine code.
#APP_ABI := armeabi armeabi-v7a arm64-v8a
#APP_ABI := armeabi
APP_ABI := armeabi-v7a
#APP_ABI := arm64-v8a
#APP_ABI  := x86

#APP_PLATFORM := android-8     #对应2.2
APP_PLATFORM := android-9     #对应2.3.1
#APP_STL := stlport_static 
APP_STL := gnustl_static
