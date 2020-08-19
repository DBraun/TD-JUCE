# TD-JUCE

This repo builds [JUCE](http://juce.com/) into a dynamic linked library `TD-JUCE.dll`, currently about 3.2 MB. Users can make their own TouchDesigner JUCE DLLs by linking against this library, and these plugins are likely to be even lighter. For example, `TD-JUCE-Reverb.dll` is only 103 KB. Going forward, this repo will grow by containing more plugin examples.

Currently implemented:
* [Reverb](https://docs.juce.com/master/classdsp_1_1Reverb.html)

## Installation

### All Platforms

You need to modify the JUCE framework in a slight way. In this repo, go to `thirdparty/JUCE_6/modules/juce_dsp/frequency/juce_Convolution.cpp` and change [line 75](https://github.com/juce-framework/JUCE/blob/a30f7357863a7d480a771e069abf56909cdf0e13/modules/juce_dsp/frequency/juce_Convolution.cpp#L75) from

`class BackgroundMessageQueue  : private Thread`

to

`class BackgroundMessageQueue  : public Thread`

### Windows

Install [CMake](https://cmake.org/download/) and make sure it's in your system path. Then in this repo, open a cmd window and do the following:

```bash
mkdir build
cd build
cmake ..
```

Open `build/TD-JUCE.sln` and build in Release (Debug is broken). Press `F5` and TouchDesigner should open. This repo's `Plugins` folder should contain a newly compiled `TD-JUCE.dll` and other DLLs such as `TD-JUCE-Reverb.dll`.

### OSX

Not fully tested yet, but the Windows instructions might work.

### Linux

TouchDesigner isn't on Linux ;)

## Roadmap

* Make it possible to build in debug mode
* Add new plugin for VST effect
* Add new plugin for VST instrument
* Mac OSX support
* Add continuous integration testing
* Your suggestion here (open a Github issue)

## License

If you use this code you must obey the [JUCE 6.0 License](https://github.com/juce-framework/JUCE/blob/master/LICENSE.md).
