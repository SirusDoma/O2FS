# O2MusicHook #

- **Author**: CXO2
- **Email**: com@cxo2.me
- **Version**: 0.0.1

Experiment artifacts of O2Jam music parser hook. ([Demo](https://www.facebook.com/CXO2JAM/videos/3249496448417106/))  
The main idea is to add functionality to the OTwo.exe client so it able to load BMS music file.  

This project is only serve as Proof of Concept and cannot be used as-is, lot of function need to be implemented in order to eliminate hardcoded variables (e.g music header, ojnlist patch, etc).

## How it works ##
1. This project has 2 projects that generate 2 binary files: `O2MusicHook.Launcher.exe` and `O2MusicHook.dll`
2. DLL and Launcher need to be placed under the same directory of o2jam game client. When the Launcher is executed, it will inject `O2MusicHook.dll` into the game client
3. Once `O2MusicHook.dll` injected, it will began to hook few functions, only 2 functions that actually needed for current implementation: `ReadFile` and `GetFileSize`
4. When hooked function attempt to open ojn that actually contain bms file, it will intercept the function
5. The interception works by loading bms file and convert it into ojn data and then return them instead without reading the requested file
6. File size hook is needed so the game use the output (ojn) file size instead of input (bms) file size, otherwise reading sequence can be messed up and the game may crashed.

## What need to be done ##
Quite amount of effort is needed to complete this hook so it can be used for any music:
1. Proper BMS parser, OJN Writer and Chart data classes. Again, current implementation only serve as proof of concept, large amount of features are missing, even essential ones are replaced by hardcoded variables (e.g BPM)
2. Synchronization between output OJN and OJNList, these two headers must match or else the game won't load the conversion output. There's few ways to do this, read the source for further detail
3. Decision to load music samples, whether it sourced from original bms music samples instead of OJM files
4. Additional asset handling aside from music samples, for example music cover image
5. `FindFirstFile` and `FindNextFile` need to be hooked so `.bms` files are recognized as-is

With this proven concept, it is possible to support other format than BMS files.

## License ##

This is an open-sourced library licensed under the [MIT License](http://github.com/SirusDoma/O2MusicHook/blob/master/LICENSE)
