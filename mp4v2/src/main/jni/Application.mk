# Build both ARMv5TE and ARMv7-A machine code.
APP_ABI := armeabi armeabi-v7a arm64-v8a

APP_PLATFORM := android-9     #对应2.3.1
#APP_STL := stlport_static 
APP_STL := gnustl_static
