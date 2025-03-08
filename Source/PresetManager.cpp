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
	currentPreset = "Default";
	//create a default director for presets if it doesn't exist yet
	if (!defaultDir.exists()) {
		const auto result = defaultDir.createDirectory();
		DBG("Preset directory created: " + defaultDir.getFullPathName());
	}
}


void PresetManager::savePreset(const juce::String& presetName) {
	if (presetName.isEmpty()) return;

	// 1. Create root XML element
	juce::XmlElement mainElement("Preset");
	mainElement.setAttribute("version", "1.0");

	// 2. Export APVTS parameters into XML
	std::unique_ptr<juce::XmlElement> parametersXml = apvts.copyState().createXml();
	mainElement.addChildElement(parametersXml.release());  // Attach APVTS data

	// 3. Export custom data
	exportXml(mainElement);

	// 4. Write the complete XML to file
	const auto presetFile = defaultDir.getChildFile(presetName + "." + extension);
	if (!mainElement.writeToFile(presetFile, {})) {
		// Handle error
	}

	currentPreset = presetName;
}




void PresetManager::exportXml(juce::XmlElement& parentElement) {
	auto* customData = new juce::XmlElement("CUSTOM_DATA");
	parentElement.addChildElement(customData);

	// Add string data
	customData->createNewChildElement("FREQ_SONG")->setAttribute("value", freqSong);
	customData->createNewChildElement("RES_SONG")->setAttribute("value", resSong);

	// Add Pips collection
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

	if (presetName == "Default") {
		// Use the same loading structure as regular presets
		auto xml = juce::XmlDocument::parse(defaultPresetXml);
		if (!xml || xml->getTagName() != "Preset") {
			jassertfalse;  // Invalid embedded XML
			return;
		}

		// 1. Load APVTS state
		if (auto* paramsXml = xml->getChildByName("Parameters")) {
			apvts.replaceState(juce::ValueTree::fromXml(*paramsXml));
		}

		// 2. Load custom data
		importXml(*xml);

		// 3. Update state and notify
		currentPreset = "Default";
		sendChangeMessage();
		return;
	}

	// Existing code for file-based presets
	const auto presetFile = defaultDir.getChildFile(presetName + "." + extension);
	if (!presetFile.existsAsFile()) return;

	juce::XmlDocument xmlDoc(presetFile);
	std::unique_ptr<juce::XmlElement> mainElement(xmlDoc.getDocumentElement());

	if (!mainElement || mainElement->getTagName() != "Preset") return;

	if (auto* paramsXml = mainElement->getChildByName("Parameters")) {
		apvts.replaceState(juce::ValueTree::fromXml(*paramsXml));
	}

	importXml(*mainElement);
	currentPreset = presetName;
	sendChangeMessage();
}


void PresetManager::importXml(const juce::XmlElement& parentElement) {
	juce::String fs = "";
	juce::String rs = "";
	std::vector<Pip> pipi;
	if (auto* customData = parentElement.getChildByName("CUSTOM_DATA")) {

		// Load string data
		if (auto* freqElement = customData->getChildByName("FREQ_SONG")) {
			fs = freqElement->getStringAttribute("value");
		}

		if (auto* resElement = customData->getChildByName("RES_SONG")) {
			rs = resElement->getStringAttribute("value");
		}

		// Load Pips collection
		if (auto* pipsElement = customData->getChildByName("PIPS")) {
			pipi.clear();
			for (auto* pipElement : pipsElement->getChildWithTagNameIterator("PIP")) {
				Pip pip;
				pip.frequency = pipElement->getDoubleAttribute("freq");
				pip.length = pipElement->getDoubleAttribute("len");
				pip.tail = pipElement->getDoubleAttribute("tail");
				pip.level = pipElement->getDoubleAttribute("level");

				pipi.push_back(pip);
			}
		}

		//send them to the processor to load
		audioProcessor.propogatePresetLoad(fs, rs, pipi);
	}
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