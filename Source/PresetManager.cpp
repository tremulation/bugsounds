/*
  ==============================================================================

	PresetManager.cpp
	Created: 6 Mar 2025 9:42:22pm
	Author:  Taro

  ==============================================================================
*/

#include "PresetManager.h"
#include "PluginProcessor.h"

//hardcoded default preset
const juce::String PresetManager::defaultPresetXml = R"(
<?xml version="1.0" encoding="UTF-8"?>
<Preset version="1.0">
  <Parameters>
    <PARAM id="Click Atack Decay Ratio" value="0.2100000083446503"/>
    <PARAM id="Click Low Frequency Attenuation" value="0.2199999988079071"/>
    <PARAM id="Click Max Volume Frequency" value="100.0"/>
    <PARAM id="Click Min Volume Frequency" value="0.0"/>
    <PARAM id="Click Pitch Random" value="0.0"/>
    <PARAM id="Click Timing Random" value="0.1400000005960464"/>
    <PARAM id="Resonator Gain" value="5.799999713897705"/>
    <PARAM id="Resonator On" value="0.0"/>
    <PARAM id="Resonator Original Mix" value="0.0"/>
    <PARAM id="Resonator Overtone Decay" value="0.5"/>
    <PARAM id="Resonator Overtone Number" value="10.0"/>
    <PARAM id="Resonator Q" value="37.0"/>
  </Parameters>
  <CUSTOM_DATA>
    <FREQ_SONG value="120 1000, 0 1000"/>
    <RES_SONG value="440 1000, 880 1000"/>
    <PIPS>
      <PIP freq="1153.18603515625" len="500" tail="43" level="0.1521739363670349"/>
      <PIP freq="1069.771362304688" len="670" tail="29" level="0.4239130616188049"/>
      <PIP freq="2839.251708984375" len="759" tail="0" level="0.02173912525177002"/>
      <PIP freq="920.605224609375" len="100" tail="0" level="0.5"/>
    </PIPS>
  </CUSTOM_DATA>
</Preset>
)";


const juce::File PresetManager::defaultDir{
	juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
	.getChildFile(ProjectInfo::companyName)
	.getChildFile(ProjectInfo::projectName)
};

const juce::String PresetManager::extension{ "preset" };



PresetManager::PresetManager(juce::AudioProcessorValueTreeState& valueTreeState, juce::String& freqSongRef,
	juce::String& resSongRef,std::vector<Pip>& pipsRef, BugsoundsAudioProcessor& p)
	: apvts(valueTreeState), freqSong(freqSongRef), resSong(resSongRef), pips(pipsRef), audioProcessor(p)
{
	//create a default director for presets if it doesn't exist yet
	if (!defaultDir.exists()) {
		const auto result = defaultDir.createDirectory();
		DBG("Preset directory created: " + defaultDir.getFullPathName());
	}
}


void PresetManager::savePreset(const juce::String& presetName) {
	if (presetName.isEmpty())return;

	//create root XML element and let exportXml populate it
	juce::XmlElement mainElement("Preset");
	exportXml(mainElement);

	//write to file
	const auto presetFile = defaultDir.getChildFile(presetName + "." + extension);
	if (!mainElement.writeToFile(presetFile, {})) {
		// Handle file write error
		jassertfalse;
	}

	currentPreset = presetName;
}



void PresetManager::exportXml(juce::XmlElement& parentElement){
	// Set XML version attribute
	parentElement.setAttribute("version", "1.0");

	//save APVTS parameters
	if (auto paramsXml = apvts.copyState().createXml())
		parentElement.addChildElement(paramsXml.release());

	//save custom data
	auto* customData = new juce::XmlElement("CUSTOM_DATA");
	parentElement.addChildElement(customData);

	//save string parameters
	customData->createNewChildElement("FREQ_SONG")
		->setAttribute("value", freqSong);
	customData->createNewChildElement("RES_SONG")
		->setAttribute("value", resSong);

	//save Pips collection
	auto* pipsElement = customData->createNewChildElement("PIPS");
	for (const auto& pip : pips) {
		auto* pipElement = pipsElement->createNewChildElement("PIP");
		pipElement->setAttribute("freq", pip.frequency);
		pipElement->setAttribute("len", pip.length);
		pipElement->setAttribute("tail", pip.tail);
		pipElement->setAttribute("level", pip.level);
	}
}




void PresetManager::deletePreset(const juce::String& presetName) {
	if (presetName.isEmpty()) {
		return;
	}

	const auto presetFile = defaultDir.getChildFile(presetName + "." + extension);
	if (!presetFile.existsAsFile()) {
		DBG("Failed to delete preset file: " + presetFile.getFullPathName());
		jassertfalse;
	}

	if (!presetFile.deleteFile()) {
		DBG("Failed to delete preset file: " + presetFile.getFullPathName());
		jassertfalse;
	}
	currentPreset = "";
}




void PresetManager::loadPreset(const juce::String& presetName) {
	if (presetName.isEmpty()) return;
	std::unique_ptr<juce::XmlElement> xml;
	if (presetName == "Default") {
		//load default preset from embedded XML
		xml = juce::XmlDocument::parse(defaultPresetXml);
	}else {
		//load from file
		const auto presetFile = defaultDir.getChildFile(presetName + "." + extension);
		if (!presetFile.existsAsFile()) return;
		xml = juce::XmlDocument(presetFile).getDocumentElement();
	}

	//validate XML root element
	if (!xml || xml->getTagName() != "Preset") {
		jassertfalse; // Invalid XML structure
		return;
	}

	//process xml
	importXml(*xml);

	//update state and notify
	currentPreset = presetName;
	sendChangeMessage();
}


void PresetManager::importXml(const juce::XmlElement& parentElement) {
	//load APVTS parameters from "Parameters" child
	if (auto* paramsXml = parentElement.getChildByName("Parameters")) {
		apvts.replaceState(juce::ValueTree::fromXml(*paramsXml));
	}

	//load custom data from "CUSTOM_DATA" child
	juce::String freqSong, resSong;
	std::vector<Pip> pips;

	if (auto* customData = parentElement.getChildByName("CUSTOM_DATA")) {
		//load FREQ_SONG and RES_SONG
		if (auto* freqElement = customData->getChildByName("FREQ_SONG"))
			freqSong = freqElement->getStringAttribute("value");

		if (auto* resElement = customData->getChildByName("RES_SONG"))
			resSong = resElement->getStringAttribute("value");

		//load pip list
		if (auto* pipsElement = customData->getChildByName("PIPS")) {
			for (auto* pipElement : pipsElement->getChildWithTagNameIterator("PIP")) {
				Pip pip;
				pip.frequency = pipElement->getDoubleAttribute("freq");
				pip.length = pipElement->getDoubleAttribute("len");
				pip.tail = pipElement->getDoubleAttribute("tail");
				pip.level = pipElement->getDoubleAttribute("level");
				pips.push_back(pip);
			}
		}
	}

	// Propagate loaded data to the processor
	audioProcessor.propogatePresetLoad(freqSong, resSong, pips);
}


juce::StringArray PresetManager::getAllPresets() const {
	juce::StringArray presets;
	presets.add("Default");
	const auto fileArray = defaultDir.findChildFiles(
		juce::File::TypesOfFileToFind::findFiles, false, "*." + extension);
	for (const auto& file : fileArray){
		const auto name = file.getFileNameWithoutExtension();
		if(name != "Default") presets.add(name);	//can't load a file called default.preset
	}
	return presets;
}


juce::String PresetManager::getCurrentPreset() const {
	return currentPreset;
}

void PresetManager::setCurrentPresetName(const juce::String& pname) {
	 currentPreset = pname;
}


int PresetManager::loadNextPreset() {
	const auto allPresets = getAllPresets();
	if (allPresets.isEmpty()) {
		return - 1;
	}
	const auto currentIndex = allPresets.indexOf(currentPreset);
	const auto nextIndex = currentIndex + 1 > (allPresets.size() - 1) ? 0 : currentIndex + 1;;

	loadPreset(allPresets.getReference(nextIndex));
	return nextIndex;
}

int PresetManager::loadPreviousPreset() {
	const auto allPresets = getAllPresets();
	if (allPresets.isEmpty()) {
		return -1;
	}
	const auto currentIndex = allPresets.indexOf(currentPreset);

	const auto previousIndex = (currentIndex - 1 < 0) ? allPresets.size() - 1 : currentIndex - 1;

	loadPreset(allPresets.getReference(previousIndex));
	return previousIndex;
}