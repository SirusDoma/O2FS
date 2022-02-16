# O2FS #

- **Author**: CXO2
- **Version**: 0.7.0

This program provides a customizable FileSystem that enables O2Jam Client (`OTwo.exe`) to load various file formats from various source.  
For an instance: Loading `BMS` file from Network directly.  

Please note that extension is **NOT** limited to music files.
As such, it is possible to implement a custom FileSystem for image files.

### Experimental Project ###
This project is highly experimental and only serve as Proof of Concept at this stage,
so bugs and errors are to be expected.  

Currently only capable to handle `BMS` format from a Disk with some features are still missing; including but not limited to: 
Custom file extension, `OJNList.dat` and `OJM` Sync support, Note Pan and Note Volume, Multi `BMS` Difficulty.

## Installation ##
1. [Download](https://github.com/SirusDoma/O2FS/releases/latest) or Build the project.
2. Place `O2FS.Launcher.exe` and `O2FS.dll` under the same directory of O2Jam game client.
3. Run `O2FS.Launcher.exe` with O2Jam client standard arguments (the launcher will launch `OTwo.exe` and forward the arguments).

## Improvements ##
Quite amount of effort is needed to complete this hook so it can provide flexible filesystem customization:
1. **`Charting` data classes / structs**  
Current design is not final and should follow O2Jam data structure as close as possible while maintaining customizability.
2. **Image FileSystem Implementation**  
Hooks that customize FileSystem for Image files such as `OPI` and `OPA` files.
3. **Synchronization Files**  
Headers inside `OJNList.dat` and Samples inside `OJM` need to be match or else the game may fail to load the conversion output.
4. **Serializers and Deserializers**  
Improvement for current Serializers and Deserializers, It also need a factory pattern or possibly other proper design that enable integration of new extension for other file formats.
5. **`FindFirstFile` and `FindNextFile` Hook**  
This will allows custom extension to be recognized as-is by the game.
6. **DLL Extension**  
Allows a third-party `DLL` file to be injected to the game with O2FS interface for dynamic custom FileSystem.

## License ##

This is an open-sourced library licensed under the [MIT License](http://github.com/SirusDoma/O2FS/blob/master/LICENSE).
