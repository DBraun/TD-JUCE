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

#include "TD-JUCE-VST-Effect.h"

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <assert.h>
#include <algorithm>

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
		info->customOPInfo.opType->setString("Vsteffect");

		// The opLabel is the text that will show up in the OP Create Dialog
		info->customOPInfo.opLabel->setString("VST Effect");
		info->customOPInfo.opIcon->setString("VST");

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
		return new TDVSTEffect(info);
	}

	DLLEXPORT
		void
		DestroyCHOPInstance(CHOP_CPlusPlusBase* instance)
	{
		// Delete the instance here, this will be called when
		// Touch is shutting down, when the CHOP using that instance is deleted, or
		// if the CHOP loads a different DLL
		delete (TDVSTEffect*)instance;
	}

};


TDVSTEffect::TDVSTEffect(const OP_NodeInfo* info) : myNodeInfo(info), mySampleRate(0.)
{
	myExecuteCount = 0;
	myOffset = 0.0;

	myBuffer = new juce::AudioBuffer<float>(2, 2048);
}

TDVSTEffect::~TDVSTEffect()
{
	if (myPlugin)
	{
		myPlugin->releaseResources();
		myPlugin.release();
	}
}

void
TDVSTEffect::getGeneralInfo(CHOP_GeneralInfo* ginfo, const OP_Inputs* inputs, void* reserved1)
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
TDVSTEffect::getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs, void* reserved1)
{

	double newSampleRate = inputs->getInputCHOP(0)->sampleRate;
	double newRate = inputs->getTimeInfo()->rate;

	if (newSampleRate != mySampleRate || myCookRate != newRate) {
		mySampleRate = newSampleRate;
		myCookRate = newRate;
		
		int bufferSize = (mySampleRate / newRate);

		if (myPlugin) {
			myPlugin->prepareToPlay(newSampleRate, bufferSize);
		}
		
	}
	else {
		mySampleRate = newSampleRate;
		myCookRate = newRate;
	}

	return false;
}

void
TDVSTEffect::getChannelName(int32_t index, OP_String* name, const OP_Inputs* inputs, void* reserved1)
{
	name->setString("chan1");
}

bool
TDVSTEffect::loadPreset(const std::string& path)
{
	using namespace juce;

	try {
		MemoryBlock mb;
		File file = File(path);
		file.loadFileAsData(mb);

#if JUCE_PLUGINHOST_VST
		// The VST2 way of loading preset. You need the entire VST2 SDK source, which is not public.
		VSTPluginFormat::loadFromFXBFile(myPlugin.get(), mb.getData(), mb.getSize());

#else

		setVST3PluginStateDirect(myPlugin.get(), mb);

#endif

		std::cout << "Loaded preset " << path << std::endl;

		return true;
	}
	catch (std::exception& e) {
		std::cout << "TDVSTEffect::loadPreset " << e.what() << std::endl;
		return false;
	}

}

void
TDVSTEffect::saveParameterInfo() {

	using namespace juce;

	myParameterMap.clear();

	//get the parameters as an AudioProcessorParameter array
	//const Array<AudioProcessorParameter*>& processorParams = myPlugin->getParameters();
	for (int i = 0; i < myPlugin->AudioProcessor::getNumParameters(); i++) {

		int maximumStringLength = 64;

		
		auto theName = myPlugin->getParameterName(i).toStdString();
		//std::string currentText = processorParams[i]->getText(processorParams[i]->getValue(), maximumStringLength).toStdString();
		//std::string label = processorParams[i]->getLabel().toStdString();

		myParameterMap[i] = std::make_pair(theName, myPlugin->getParameter(i));

		//py::dict myDictionary;
		//myDictionary["index"] = i;
		//myDictionary["name"] = theName;
		//myDictionary["numSteps"] = processorParams[i]->getNumSteps();
		//myDictionary["isDiscrete"] = processorParams[i]->isDiscrete();
		//myDictionary["label"] = label;
		//myDictionary["text"] = currentText;

		//myList.append(myDictionary);
	}
}

bool
TDVSTEffect::checkPlugin(const char* pluginFilepath) {

	using namespace juce;

	if (emptyString.compare(pluginFilepath) == 0) {
		return false;
	}

	if (myPluginPath.compare(pluginFilepath) != 0) {

		juce::OwnedArray<PluginDescription> pluginDescriptions;
		KnownPluginList pluginList;
		AudioPluginFormatManager pluginFormatManager;

		pluginFormatManager.addDefaultFormats();

		for (int i = pluginFormatManager.getNumFormats(); --i >= 0;)
		{
			pluginList.scanAndAddFile(String(pluginFilepath),
				true,
				pluginDescriptions,
				*pluginFormatManager.getFormat(i));
		}

		// If there is a problem here first check the preprocessor definitions
		// in the projucer are sensible - is it set up to scan for plugin's?
		jassert(pluginDescriptions.size() > 0);

		String errorMessage;

		if (myPlugin)
		{
			myPlugin->releaseResources();
			myPlugin.release();
		}

		myPlugin = pluginFormatManager.createPluginInstance(*pluginDescriptions[0],
			mySampleRate,
			mySamplesPerBlock,
			errorMessage);

		if (myPlugin != nullptr)
		{
			std::cout << "TDVSTEffect::loadPlugin success!" << std::endl;

			saveParameterInfo();
			myPlugin->prepareToPlay(mySampleRate, mySamplesPerBlock);
			myPlugin->setNonRealtime(false);  // todo: allow non-realtime render if TouchDesigner is set to non-realtime?

			myPluginPath = pluginFilepath;
			return true;
		}

		std::cout << "TDVSTEffect::loadPlugin error: " << errorMessage.toStdString() << std::endl;
		return false;
	}

	return true;
}

void
TDVSTEffect::execute(CHOP_Output* output,
	const OP_Inputs* inputs,
	void* reserved)
{

	using namespace juce;

	myExecuteCount++;

	if (inputs->getNumInputs() == 0)
	{
		return;
	}

	auto inputCHOP = inputs->getInputCHOP(0);

	auto vstParameterCHOP = inputs->getInputCHOP(1);

	if (!checkPlugin(inputs->getParFilePath("Vstfile"))) return;

	if (vstParameterCHOP) {
		for (size_t i = 0; i < std::min(vstParameterCHOP->numChannels, myPlugin->getNumParameters()); i++)
		{
			myPlugin->setParameter(i, vstParameterCHOP->getChannelData(i)[0]);
			myParameterMap[i] = std::make_pair(myPlugin->getParameterName(i).toStdString(), myPlugin->getParameter(i));
		}
	}

	if (myDoLoadPreset) {
		myDoLoadPreset = !loadPreset(inputs->getParFilePath("Fxpfile"));
	}

	myBuffer->setDataToReferTo((float**) inputCHOP->channelData, inputCHOP->numChannels, inputCHOP->numSamples);
	myPlugin->processBlock(*myBuffer, myRenderMidiBuffer);

	for (int chan = 0; chan < output->numChannels; chan++) {
		auto chanPtr = myBuffer->getReadPointer(chan);
		for (int i = 0; i < output->numSamples; i++)
		{
			output->channels[chan][i] = *chanPtr++;
		}
	}
}

int32_t
TDVSTEffect::getNumInfoCHOPChans(void* reserved1)
{
	// We return the number of channel we want to output to any Info CHOP
	// connected to the CHOP. In this example we are just going to send one channel.
	return 2;
}

void
TDVSTEffect::getInfoCHOPChan(int32_t index,
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
TDVSTEffect::getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved1)
{
	infoSize->rows = myParameterMap.size();
	infoSize->cols = 2;
	// Setting this to false means we'll be assigning values to the table
	// one row at a time. True means we'll do it one column at a time.
	infoSize->byColumn = false;
	return true;
}

void
TDVSTEffect::getInfoDATEntries(int32_t index,
	int32_t nEntries,
	OP_InfoDATEntries* entries,
	void* reserved1)
{
	char buffer[64];

	int ret = snprintf(buffer, sizeof buffer, "%f", myParameterMap[index].second);

	entries->values[0]->setString(myParameterMap[index].first.c_str());
	entries->values[1]->setString(buffer);
}

void
TDVSTEffect::setupParameters(OP_ParameterManager* manager, void* reserved1)
{

	// VST File Path
	{
		OP_StringParameter sp;

		sp.name = "Vstfile";
		sp.label = "VST File";
		sp.defaultValue = "";

		OP_ParAppendResult res = manager->appendFile(sp);
		assert(res == OP_ParAppendResult::Success);
	}

	// VST File Path
	{
		OP_StringParameter sp;

		sp.name = "Fxpfile";
		sp.label = "FXP File";
		sp.defaultValue = "";

		OP_ParAppendResult res = manager->appendFile(sp);
		assert(res == OP_ParAppendResult::Success);
	}

	//// Width
	//{
	//	OP_NumericParameter	np;

	//	np.name = "Width";
	//	np.label = "Width";
	//	np.defaultValues[0] = 1.0;
	//	np.minSliders[0] = 0.;
	//	np.maxSliders[0] = 1.;
	//	np.clampMins[0] = true;
	//	np.clampMaxes[0] = true;

	//	OP_ParAppendResult res = manager->appendFloat(np);
	//	assert(res == OP_ParAppendResult::Success);
	//}

	//// Freeze Mode
	//{
	//	OP_NumericParameter	np;

	//	np.name = "Freeze";
	//	np.label = "Freeze";
	//	np.defaultValues[0] = 0;

	//	OP_ParAppendResult res = manager->appendToggle(np);
	//	assert(res == OP_ParAppendResult::Success);
	//}

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

		np.name = "Loadfxp";
		np.label = "Load FXP";

		OP_ParAppendResult res = manager->appendPulse(np);
		assert(res == OP_ParAppendResult::Success);
	}

	//// pulse
	//{
	//	OP_NumericParameter	np;

	//	np.name = "Reset";
	//	np.label = "Reset";

	//	OP_ParAppendResult res = manager->appendPulse(np);
	//	assert(res == OP_ParAppendResult::Success);
	//}

}

void
TDVSTEffect::pulsePressed(const char* name, void* reserved1)
{
	if (!strcmp(name, "Reset"))
	{
		myOffset = 0.0;
	}

	if (!strcmp(name, "Loadfxp") && myPlugin)
	{
		std::cout << "loadfxp pulse" << std::endl;
		myDoLoadPreset = true;
	}

}