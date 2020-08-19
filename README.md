# TD-JUCE
JUCE audio in TouchDesigner CHOPs.

Currently implemented:

* Reverb

## Installation

### All Platforms

You need to modify the JUCE framework in a slight way. In this repo, go to `thirdparty/JUCE_6/modules/juce_dsp/frequency/juce_Convolution.cpp` and change [line 75](https://github.com/juce-framework/JUCE/blob/a30f7357863a7d480a771e069abf56909cdf0e13/modules/juce_dsp/frequency/juce_Convolution.cpp#L75) from `class BackgroundMessageQueue  : private Thread` to `class BackgroundMessageQueue  : public Thread`

### Windows

Install [CMake](https://cmake.org/download/). Then in this repo, open a cmd window and do the following:

```bash
mkdir build
cd build
cmake ..
```

Then open `build/TD-JUCE.sln` and build in Release (Debug is broken). It will fail the first time but succeed the second time (I need to fix the build dependencies...). Press `F5` and TouchDesigner should open.

### OSX

Not fully tested yet.

## License

If you use this code you must obey the [JUCE 6.0 License](https://github.com/juce-framework/JUCE/blob/master/LICENSE.md).

## Roadmap

* Fix the CMake build order issue.
* Make it possible to build in debug mode.
* Use this repo to build simultaneously multiple TouchDesigner DLLs: VST effect, VST instrument etc.
* Mac OSX support.
* Add continuous integration testing.
