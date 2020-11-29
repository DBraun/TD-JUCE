#include "TD-JUCE-Reverb.h"


/* Shared Use License: This file is owned by Derivative Inc. (Derivative)
* and can only be used, and/or modified for use, in conjunction with
* Derivative's TouchDesigner software, and only if you are a licensee who has
* accepted Derivative's TouchDesigner license or assignment agreement
* (which also govern the use of this file). You may share or redistribute
* a modified version of this file provided the following conditions are met:
*
* 1. The shared file or redistribution must retain the information set out
* above and this list of conditions.
* 2. Derivative's name (Derivative Inc.) or its trademarks may not be used
* to endorse or promote products derived from this file without specific
* prior written permission from Derivative.
*/

#include "TD-JUCE-Reverb.h"

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <assert.h>

// These functions are basic C function, which the DLL loader can find
// much easier than finding a C++ Class.
// The DLLEXPORT prefix is needed so the compile exports these functions from the .dll
// you are creating
extern "C"
{

	DLLEXPORT
		void
		FillCHOPPluginInfo(CHOP_PluginInfo* info)
	{
		// Always set this to CHOPCPlusPlusAPIVersion.
		info->apiVersion = CHOPCPlusPlusAPIVersion;

		// The opType is the unique name for this CHOP. It must start with a 
		// capital A-Z character, and all the following characters must lower case
		// or numbers (a-z, 0-9)
		info->customOPInfo.opType->setString("Jucereverb");

		// The opLabel is the text that will show up in the OP Create Dialog
		info->customOPInfo.opLabel->setString("JUCE Reverb");
		info->customOPInfo.opIcon->setString("JRC"); // JUCE Reverb CHOP

		// Information about the author of this OP
		info->customOPInfo.authorName->setString("David Braun");
		info->customOPInfo.authorEmail->setString("github.com/dbraun");

		// This CHOP can work with 0 inputs
		info->customOPInfo.minInputs = 1;

		// It can accept up to 1 input though, which changes it's behavior
		info->customOPInfo.maxInputs = 1;
	}

	DLLEXPORT
		CHOP_CPlusPlusBase*
		CreateCHOPInstance(const OP_NodeInfo* info)
	{
		// Return a new instance of your class every time this is called.
		// It will be called once per CHOP that is using the .dll
		return new TDJuceReverb(info);
	}

	DLLEXPORT
		void
		DestroyCHOPInstance(CHOP_CPlusPlusBase* instance)
	{
		// Delete the instance here, this will be called when
		// Touch is shutting down, when the CHOP using that instance is deleted, or
		// if the CHOP loads a different DLL
		delete (TDJuceReverb*)instance;
	}

};


TDJuceReverb::TDJuceReverb(const OP_NodeInfo* info) : myNodeInfo(info), mySampleRate(0.)
{
	myExecuteCount = 0;
	myOffset = 0.0;
}

void
TDJuceReverb::prepareToPlay(double sampleRate, int samplesPerBlock) {
	juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32> (samplesPerBlock) };
	myReverb.prepare(spec);
}

TDJuceReverb::~TDJuceReverb()
{

}

void
TDJuceReverb::getGeneralInfo(CHOP_GeneralInfo* ginfo, const OP_Inputs* inputs, void* reserved1)
{
	// This will cause the node to cook every frame
	ginfo->cookEveryFrameIfAsked = true;

	// Note: To disable timeslicing you'll need to turn this off, as well as ensure that
	// getOutputInfo() returns true, and likely also set the info->numSamples to how many
	// samples you want to generate for this CHOP. Otherwise it'll take on length of the
	// input CHOP, which may be timesliced.
	ginfo->timeslice = true;

	ginfo->inputMatchIndex = 0;
}

bool
TDJuceReverb::getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs, void* reserved1)
{

	double newSampleRate = inputs->getInputCHOP(0)->sampleRate;
	double newRate = inputs->getTimeInfo()->rate;

	if (newSampleRate != mySampleRate || myRate != newRate) {
		mySampleRate = newSampleRate;
		myRate = newRate;
		
		int bufferSize = (mySampleRate / newRate);

		prepareToPlay(newSampleRate, bufferSize);
	}
	else {
		mySampleRate = newSampleRate;
		myRate = newRate;
	}

	return false;
}

void
TDJuceReverb::getChannelName(int32_t index, OP_String* name, const OP_Inputs* inputs, void* reserved1)
{
	name->setString("chan1");
}

void
TDJuceReverb::execute(CHOP_Output* output,
	const OP_Inputs* inputs,
	void* reserved)
{
	myExecuteCount++;

	if (inputs->getNumInputs() == 0)
	{
		return;
	}

	auto inputCHOP = inputs->getInputCHOP(0);

	juce::dsp::Reverb::Parameters params;

	//float roomSize = 0.5f;     /**< Room size, 0 to 1.0, where 1.0 is big, 0 is small. */
	//float damping = 0.5f;     /**< Damping, 0 to 1.0, where 0 is not damped, 1.0 is fully damped. */
	//float wetLevel = 0.33f;    /**< Wet level, 0 to 1.0 */
	//float dryLevel = 0.4f;     /**< Dry level, 0 to 1.0 */
	//float width = 1.0f;     /**< Reverb width, 0 to 1.0, where 1.0 is very wide. */
	//float freezeMode = 0.0f;     /**< Freeze mode - values < 0.5 are "normal" mode, values > 0.5
	//								  put the reverb into a continuous feedback loop. */

	params.damping = inputs->getParDouble("Damping");
	params.dryLevel = inputs->getParDouble("Drylevel");
	params.roomSize = inputs->getParDouble("Roomsize");
	params.wetLevel = inputs->getParDouble("Wetlevel");
	params.width = inputs->getParDouble("Width");
	params.freezeMode = inputs->getParDouble("Freeze");
	myReverb.setParameters(params);

	myBlock = juce::dsp::AudioBlock<float>((float* const*)inputCHOP->channelData, inputCHOP->numChannels, 0, inputCHOP->numSamples);
	juce::dsp::ProcessContextReplacing<float> context(myBlock);
	myReverb.process(context);

	for (int chan = 0; chan < output->numChannels; chan++) {
		auto chanPtr = myBlock.getChannelPointer(chan);
		for (int i = 0; i < output->numSamples; i++)
		{
			output->channels[chan][i] = *chanPtr++;
		}
	}
}

int32_t
TDJuceReverb::getNumInfoCHOPChans(void* reserved1)
{
	// We return the number of channel we want to output to any Info CHOP
	// connected to the CHOP. In this example we are just going to send one channel.
	return 2;
}

void
TDJuceReverb::getInfoCHOPChan(int32_t index,
	OP_InfoCHOPChan* chan,
	void* reserved1)
{
	// This function will be called once for each channel we said we'd want to return
	// In this example it'll only be called once.

	if (index == 0)
	{
		chan->name->setString("executeCount");
		chan->value = (float)myExecuteCount;
	}

	if (index == 1)
	{
		chan->name->setString("offset");
		chan->value = (float)myOffset;
	}
}

bool
TDJuceReverb::getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved1)
{
	infoSize->rows = 2;
	infoSize->cols = 2;
	// Setting this to false means we'll be assigning values to the table
	// one row at a time. True means we'll do it one column at a time.
	infoSize->byColumn = false;
	return true;
}

void
TDJuceReverb::getInfoDATEntries(int32_t index,
	int32_t nEntries,
	OP_InfoDATEntries* entries,
	void* reserved1)
{
	char tempBuffer[4096];

	if (index == 0)
	{
		// Set the value for the first column
		entries->values[0]->setString("executeCount");

		// Set the value for the second column
#ifdef _WIN32
		sprintf_s(tempBuffer, "%d", myExecuteCount);
#else // macOS
		snprintf(tempBuffer, sizeof(tempBuffer), "%d", myExecuteCount);
#endif
		entries->values[1]->setString(tempBuffer);
	}

	if (index == 1)
	{
		// Set the value for the first column
		entries->values[0]->setString("offset");

		// Set the value for the second column
#ifdef _WIN32
		sprintf_s(tempBuffer, "%g", myOffset);
#else // macOS
		snprintf(tempBuffer, sizeof(tempBuffer), "%g", myOffset);
#endif
		entries->values[1]->setString(tempBuffer);
	}
}

void
TDJuceReverb::setupParameters(OP_ParameterManager* manager, void* reserved1)
{
	// Roomsize
	{
		OP_NumericParameter	np;

		np.name = "Roomsize";
		np.label = "Room Size";
		np.defaultValues[0] = .5;
		np.minSliders[0] = 0.;
		np.maxSliders[0] = 1.;
		np.clampMins[0] = true;
		np.clampMaxes[0] = true;

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Damping
	{
		OP_NumericParameter	np;

		np.name = "Damping";
		np.label = "Damping";
		np.defaultValues[0] = .5;
		np.minSliders[0] = 0.;
		np.maxSliders[0] = 1.;
		np.clampMins[0] = true;
		np.clampMaxes[0] = true;

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Wetlevel
	{
		OP_NumericParameter	np;

		np.name = "Wetlevel";
		np.label = "Wet Level";
		np.defaultValues[0] = .33;
		np.minSliders[0] = 0.;
		np.maxSliders[0] = 1.;
		np.clampMins[0] = true;
		np.clampMaxes[0] = true;

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Drylevel
	{
		OP_NumericParameter	np;

		np.name = "Drylevel";
		np.label = "Dry Level";
		np.defaultValues[0] = .4;
		np.minSliders[0] = 0.;
		np.maxSliders[0] = 1.;
		np.clampMins[0] = true;
		np.clampMaxes[0] = true;

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Width
	{
		OP_NumericParameter	np;

		np.name = "Width";
		np.label = "Width";
		np.defaultValues[0] = 1.0;
		np.minSliders[0] = 0.;
		np.maxSliders[0] = 1.;
		np.clampMins[0] = true;
		np.clampMaxes[0] = true;

		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Freeze Mode
	{
		OP_NumericParameter	np;

		np.name = "Freeze";
		np.label = "Freeze";
		np.defaultValues[0] = 0;

		OP_ParAppendResult res = manager->appendToggle(np);
		assert(res == OP_ParAppendResult::Success);
	}

	//// shape
	//{
	//	OP_StringParameter	sp;

	//	sp.name = "Shape";
	//	sp.label = "Shape";

	//	sp.defaultValue = "Sine";

	//	const char* names[] = { "Sine", "Square", "Ramp" };
	//	const char* labels[] = { "Sine", "Square", "Ramp" };

	//	OP_ParAppendResult res = manager->appendMenu(sp, 3, names, labels);
	//	assert(res == OP_ParAppendResult::Success);
	//}

	// pulse
	{
		OP_NumericParameter	np;

		np.name = "Reset";
		np.label = "Reset";

		OP_ParAppendResult res = manager->appendPulse(np);
		assert(res == OP_ParAppendResult::Success);
	}

}

void
TDJuceReverb::pulsePressed(const char* name, void* reserved1)
{
	if (!strcmp(name, "Reset"))
	{
		myOffset = 0.0;
		myReverb.reset();
	}
}