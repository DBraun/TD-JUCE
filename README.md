
# TD-JUCE

This repo builds [JUCE](http://juce.com/) into a dynamic linked library `TD-JUCE.dll`, currently about 5.5 MB. Users can make their own TouchDesigner JUCE DLLs by linking against this library, and these plugins are likely to be even lighter. For example, `TD-JUCE-Reverb.dll` is only 31 KB. Going forward, this repo will grow by containing more plugin examples.

## Currently implemented:

#### [Reverb](https://docs.juce.com/master/classdsp_1_1Reverb.html)
The input should be stereo waveform.

#### [VST](https://docs.juce.com/master/classAudioPluginInstance.html)

This plugin works as both a VST instrument (**DLL** files) and VST effect (**DLL** and **.vst3** files). For both instruments and effects, the second CHOP input, which is optional, should contain the VST parameter choices. These channels can be either low sample rate (60 Hz) or audio rate (44100 Hz). The "Block size" custom parameter determines how many samples are processed for each time the parameters get updated. Use the Info DAT on the plugin to figure out which channels correspond to which parameters.

When the VST is an effect, the first CHOP input should be a stereo waveform. When the VST is an instrument, the third CHOP input should be 128 channels, which correspond to [MIDI](https://en.wikipedia.org/wiki/MIDI#General_MIDI) notes. Middle-C is 60. The values in this CHOP are the velocities of the notes, from 0 to 1. The CHOP's sample rate can be 60 fps or audio rate.

## Installation

### All Platforms

Clone this repo with git. It will not work if you download it as a zip.

### Windows

Install [CMake](https://cmake.org/download/) and make sure it's in your system path. Then in this repo, open a cmd window and do the following:

```bash
mkdir build
cd build
cmake ..
```

Open `build/TD-JUCE.sln` and build in Release (Debug is broken). Then press `F5` and TouchDesigner should open. This repo's `Plugins` folder should contain a newly compiled `TD-JUCE.dll` and other DLLs such as `TD-JUCE-Reverb.dll` and `TD-JUCE-VST`.

### OSX

Not fully tested yet, but the Windows instructions might work.

### Linux

TouchDesigner isn't on Linux ;)

## Roadmap

* Make it possible to build in debug mode
* Mac OSX support
* Add continuous integration testing
* Your suggestion here (open a Github issue)

## License

If you use this code you must obey the [JUCE 6.0 License](https://github.com/juce-framework/JUCE/blob/master/LICENSE.md).
