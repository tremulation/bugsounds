/*
  ==============================================================================

    HelpCompendium.cpp
    Created: 1 Jun 2025 12:45:12am
    Author:  Taro

  ==============================================================================
*/

#include "HelpCompendium.h"
#include "PluginEditor.h"  


HelpCompendium::HelpCompendium(BugsoundsAudioProcessorEditor& editor) : audioEditor(editor)
{
    //setup viewport
    addAndMakeVisible(viewport);
    viewport.setViewedComponent(&contentComponent, false);
    viewport.setScrollBarsShown(true, false);
    viewport.getVerticalScrollBar().setAutoHide(false);

    //setup fonts
    contentComponent.font = UIDrawer::getFontCode();
    contentComponent.headerFont = UIDrawer::getFontCode();

    closeButton = std::make_unique<juce::TextButton>();
    closeButton->onClick = [this] {
        this->audioEditor.toggleHelpCompendium("close");
        };
    closeButton->setName("WikiClose");
    addAndMakeVisible(closeButton.get());

    headerLabel.setText("Help: " + currentPageName, juce::NotificationType::dontSendNotification);
    headerLabel.setFont(UIDrawer::getFontInterBold().withHeight(21.0f).withExtraKerningFactor(.05f));
    headerLabel.setColour(Label::textColourId, Colour::fromString("#000000").withAlpha(1.0f));
    headerLabel.setJustificationType(juce::Justification::left);
    addAndMakeVisible(headerLabel);
}


void HelpCompendium::closeCompendium() {
    setVisible(false);
    if (onClose != nullptr) onClose();
    currentPageName = "closed";
    currentPageID = "closed";
    pageContent.clear();
}


void HelpCompendium::paint(juce::Graphics& g)  {
    auto  bounds = getLocalBounds();
    float scalar = getHeight() / 640.f;
    auto  headerBounds = bounds.removeFromTop(35.f * scalar);
    auto  bodyBounds = bounds;

    //header
    Colour c1 = Colour::fromString("#81CAA5").withAlpha(1.0f);
    Colour c2 = Colour::fromString("#81CAA5").withAlpha(1.0f);
    drawUIBlock(g, headerBounds, c1, c2, false, true, scalar);

    //body
    c1 = Colour::fromString("#D9D9D9").withAlpha(1.0f);
    c2 = Colour::fromString("#D9D9D9").withAlpha(1.0f);
    drawUIBlock(g, bodyBounds, c1, c2, false, true, scalar);
}


void HelpCompendium::resized() {
    float scalar = getHeight() / 640.f;
    auto bounds = getLocalBounds();
    float headerHeight = 35.f * scalar;
    auto headerBounds = bounds.removeFromTop(headerHeight);
    closeButton->setBounds(headerBounds.removeFromRight(headerHeight));

    //add margin around viewport
    const float margin = 10.f * scalar;
    auto viewportArea = bounds.reduced(margin);

    //set viewport bounds with margin
    viewport.setBounds(viewportArea);
    auto* content = viewport.getViewedComponent();
    float scrollbarSpacing = 30.f * scalar;
    //only width matters here, since we reflow to set height inside the content component when this is called
    content->setSize(getWidth() - scrollbarSpacing, 0); 

    //resize fonts
    contentComponent.font = UIDrawer::getFontCode().withHeight(15.f * scalar);
    contentComponent.headerFont = UIDrawer::getFontCode().withHeight(21.f * scalar);

    //resize titlte
    headerLabel.setBounds(headerBounds.withTrimmedLeft(5.f * scalar));
    headerLabel.setFont(UIDrawer::getFontInterBold().withHeight(21.0f * scalar).withExtraKerningFactor(.05f * scalar));
}



void HelpCompendium::setPage(const juce::String& pageID) {
    currentPageID = pageID;
    currentPageName = pageID;
    pageContent.clear();
    loadPageContent(pageID);
    contentComponent.setContent(pageContent);

    //get the prettified page name to display in the header
    juce::String pageName = String(pageID);
    Logger::writeToLog("pageName: " + pageName);
    if (pageName == "bugSounds") pageName = "Getting Started";
    else if (pageName == "resonatorSettings") pageName = "Resonator";
    else if (pageName == "subclickSequencer") pageName = "Subclick Sequencer";
    else if (pageName == "clickSettings") pageName = "Click Settings";
    else if (pageName == "resonatorEditor") pageName = "Resonator Editor";
    else if (pageName == "frequencyEditor") pageName = "Frequency Editor";
    else if (pageName == "chorus") pageName = "Chorus";

    headerLabel.setText("Help: " + pageName, juce::NotificationType::dontSendNotification);

    //reset to top position
    viewport.setViewPosition(0, 0);

    //trigger resize to update layout
    resized();
    repaint();
}