# TD-JUCE

This repo builds [JUCE](http://juce.com/) into a dynamic linked library `TD-JUCE.dll`, currently about 3.2 MB. Users can make their own TouchDesigner JUCE DLLs by linking against this library, and these plugins are likely to be even lighter. For example, `TD-JUCE-Reverb.dll` is only 103 KB. Going forward, this repo will grow by containing more plugin examples.

Currently implemented:
* [Reverb](https://docs.juce.com/master/classdsp_1_1Reverb.html)
* [VST Effect](https://docs.juce.com/master/classAudioPluginInstance.html)

## Installation

Clone this repo with git. It will not work if you download it as a zip.

### All Platforms

### Windows

Install [CMake](https://cmake.org/download/) and make sure it's in your system path. Then in this repo, open a cmd window and do the following:

```bash
mkdir build
cd build
cmake ..
```

Open `build/TD-JUCE.sln` and build in Release (Debug is broken). Then press `F5` and TouchDesigner should open. This repo's `Plugins` folder should contain a newly compiled `TD-JUCE.dll` and other DLLs such as `TD-JUCE-Reverb.dll` and `TD-JUCE-VST-Effect`.

### OSX

Not fully tested yet, but the Windows instructions might work.

### Linux

TouchDesigner isn't on Linux ;)

## Roadmap

* Make it possible to build in debug mode
* Add new plugin for VST instrument
* Mac OSX support
* Add continuous integration testing
* Your suggestion here (open a Github issue)

## License

If you use this code you must obey the [JUCE 6.0 License](https://github.com/juce-framework/JUCE/blob/master/LICENSE.md).
