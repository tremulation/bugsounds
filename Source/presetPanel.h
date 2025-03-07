/*
  ==============================================================================

    presetPanel.h
    Created: 6 Mar 2025 9:18:30pm
    Author:  Taro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class PresetPanel : public juce::Component, juce::Button::Listener, juce::ComboBox::Listener
{
public:
    PresetPanel(PresetManager& pm) : presetManager(pm) {
		configureButton(saveButton, "Save");
		configureButton(deleteButton, "Delete");
		configureButton(previousPresetButton, "<");
		configureButton(nextPresetbutton, ">");

		presetList.setTextWhenNothingSelected("Select a preset");
		presetList.setMouseCursor(juce::MouseCursor::PointingHandCursor);
		addAndMakeVisible(presetList);
		presetList.addListener(this);

		loadPresetList();
    }


	void loadPresetList() {
		presetList.clear(juce::dontSendNotification);
		const auto allPresets = presetManager.getAllPresets();
		const auto currentPreset = presetManager.getCurrentPreset();
		presetList.addItemList(allPresets, 1);
		presetList.setSelectedItemIndex(allPresets.indexOf(currentPreset), juce::dontSendNotification);
	}

	~PresetPanel() {
		saveButton.removeListener(this);
		deleteButton.removeListener(this);
		previousPresetButton.removeListener(this);
		nextPresetbutton.removeListener(this);
		presetList.removeListener(this);
	}

	void resized() override {
		const auto container = getLocalBounds().reduced(4);
		auto bounds = container;
		saveButton.setBounds(bounds.removeFromLeft(container.proportionOfWidth(0.2f)).reduced(4));
		previousPresetButton.setBounds(bounds.removeFromLeft(container.proportionOfWidth(0.1f)).reduced(4));
		presetList.setBounds(bounds.removeFromLeft(container.proportionOfWidth(0.4f)).reduced(4));
		nextPresetbutton.setBounds(bounds.removeFromLeft(container.proportionOfWidth(0.1f)).reduced(4));
		deleteButton.setBounds(bounds.reduced(4));
	}
private:

	void buttonClicked(juce::Button* button) override {
		if (button == &saveButton) {
			 fileChooser = std::make_unique<juce::FileChooser>(
				"please enter the name of the preset to save",
				PresetManager::defaultDir,
				"*." + PresetManager::extension
			);
			fileChooser->launchAsync(juce::FileBrowserComponent::saveMode,
				[&](const juce::FileChooser& fc) {
					const auto result = fc.getResult();
					presetManager.savePreset(result.getFileNameWithoutExtension());
					loadPresetList();
				});
		}
		if (button == &previousPresetButton) {
			int index = presetManager.loadPreviousPreset();
			presetList.setSelectedItemIndex(index, juce::dontSendNotification);
		}
		if (button == &nextPresetbutton) {
			int index = presetManager.loadNextPreset();
			presetList.setSelectedItemIndex(index, juce::dontSendNotification);
		}
		if (button == &deleteButton) {
			juce::AlertWindow::showAsync(
				juce::MessageBoxOptions()
				.withIconType(juce::MessageBoxIconType::QuestionIcon)
				.withTitle("Delete Preset")
				.withMessage("Are you sure you want to delete '" + presetManager.getCurrentPreset() + "'?")
				.withButton("Yes")
				.withButton("No")
				.withAssociatedComponent(this),
				[this](int result) {
					if (result == 1) { // "Yes" button clicked
						presetManager.deletePreset(presetManager.getCurrentPreset());
						loadPresetList();
					}
				}
			);
		}
	}


	void comboBoxChanged(juce::ComboBox* comboBox) override {
		if (comboBox = &presetList) {
			presetManager.loadPreset(presetList.getItemText(presetList.getSelectedItemIndex()));
		}
	}	

	void configureButton(juce::TextButton& button, const juce::String& buttonText) {
		button.setButtonText(buttonText);
		button.setMouseCursor(juce::MouseCursor::PointingHandCursor);
		addAndMakeVisible(button);
		button.addListener(this);
	}

	PresetManager& presetManager;
    juce::TextButton saveButton, deleteButton, previousPresetButton, nextPresetbutton;
    juce::ComboBox presetList;
	std::unique_ptr<juce::FileChooser> fileChooser;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetPanel)
};