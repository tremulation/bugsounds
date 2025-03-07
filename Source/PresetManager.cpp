/*
  ==============================================================================

	PresetManager.cpp
	Created: 6 Mar 2025 9:42:22pm
	Author:  Taro

  ==============================================================================
*/

#include "PresetManager.h"
#include "PluginProcessor.h"

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

	const auto presetFile = defaultDir.getChildFile(presetName + "." + extension);
	if (!presetFile.existsAsFile()) return;

	// 1. Parse XML file
	juce::XmlDocument xmlDoc(presetFile);
	std::unique_ptr<juce::XmlElement> mainElement(xmlDoc.getDocumentElement());

	if (!mainElement || mainElement->getTagName() != "Preset") {
		// Handle invalid preset file
		return;
	}

	// 2. Load APVTS state
	if (auto* paramsXml = mainElement->getChildByName("Parameters")) {
		apvts.replaceState(juce::ValueTree::fromXml(*paramsXml));
	}

	// 3. Load custom data
	importXml(*mainElement);
	currentPreset = presetName;

	//4. notify ui elements to reload
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
	const auto fileArray = defaultDir.findChildFiles(
		juce::File::TypesOfFileToFind::findFiles, false, "*." + extension);
	for (const auto& file : fileArray)
	{
		presets.add(file.getFileNameWithoutExtension());
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