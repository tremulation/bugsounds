/*
  ==============================================================================

    HelpCompendium.h
    Created: 27 Apr 2025 7:44:56pm
    Author:  Taro

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <BinaryData.h>

class HelpCompendium : public juce::Component {
public:
    HelpCompendium()
    {
        addAndMakeVisible(viewport);
        viewport.setViewedComponent(&contentComponent);
        viewport.setScrollBarsShown(true, false);
    }


    struct ContentElement {
        enum Type { Subheader, Text, Image };
        Type type;
        juce::String content;
        juce::Image image;
    };



    void paint(juce::Graphics& g) override {
        // Restored original background and border styling
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

        auto bounds = getLocalBounds();
        g.setColour(juce::Colours::white);
        g.drawLine(bounds.getX(), bounds.getY(), bounds.getX(), bounds.getBottom(), 2.0f);

        const int margin = 10;
        auto contentArea = bounds.reduced(margin);

        // Original border drawing
        g.setColour(juce::Colours::white);
        g.drawRect(contentArea, 1);

        // Restored original header styling
        auto headerArea = contentArea.removeFromTop(30);
        g.setColour(juce::Colours::white);
        g.drawRect(headerArea);

        // Original header text drawing
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(16.0f).boldened());
        g.drawText("Help: " + currentPageName, headerArea, juce::Justification::centred);

        // Original divider line
        g.setColour(juce::Colours::lightgrey);
        g.drawLine(headerArea.getX(),
            headerArea.getBottom(),
            headerArea.getRight(),
            headerArea.getBottom(),
            2.0f);
    }

    void resized() override {
        auto bounds = getLocalBounds().reduced(10);
        bounds.removeFromTop(30); // Account for header height
        viewport.setBounds(bounds);

        // Sync content width with viewport
        contentComponent.setSize(viewport.getMaximumVisibleWidth(), contentComponent.getHeight());
    }

    void setPage(const juce::String& pageID) {
        currentPageName = pageID;
        pageContent.clear();
        loadPageContent(pageID);
        contentComponent.setContent(pageContent);
        viewport.setViewPosition(0, 0);
    }

private:
    juce::String currentPageName = "closed";
    juce::Viewport viewport;

    struct ContentComponent : public juce::Component
    {
        void setContent(const std::vector<ContentElement>& newContent)
        {
            content = newContent;
            reflow();
            repaint();
        }

        void paint(juce::Graphics& g) override
        {
            const int hMargin = 5;
            const int vMargin = 10;
            const int spacing = 15;
            const int subH = 25;

            // Compute once:
            auto totalW = getWidth();
            auto contentW = totalW - 2 * hMargin;
            auto x0 = hMargin;
            int  y = vMargin;

            for (auto& e : content)
            {
                if (e.type == ContentElement::Subheader)
                {
                    // Full-width subheader background
                    g.setColour(juce::Colours::darkgrey.withAlpha(0.2f));
                    g.fillRect(x0, y, contentW, subH);

                    g.setColour(juce::Colours::white);
                    g.setFont(juce::Font(14.0f).boldened());
                    g.drawText(e.content,
                        x0, y,
                        contentW, subH,
                        juce::Justification::centredLeft);

                    y += subH + spacing;
                }
                else if (e.type == ContentElement::Text)
                {
                    g.setColour(juce::Colours::white);
                    g.setFont(juce::Font(14.0f));

                    // Preserve explicit newlines
                    auto paragraphs = juce::StringArray::fromTokens(e.content, "\n", "");

                    for (auto& p : paragraphs)
                    {
                        juce::AttributedString as;
                        as.append(p, juce::Font(14.0f), juce::Colours::white);
                        as.setWordWrap(juce::AttributedString::WordWrap::byWord);

                        juce::TextLayout tl;
                        // Use the single width:
                        tl.createLayout(as, (float)contentW);

                        // Draw at x0, y
                        tl.draw(g, { (float)x0, (float)y,
                                      (float)contentW, tl.getHeight() });

                        y += (int)tl.getHeight() + spacing;
                    }
                }
            }
        }

        void resized() override { reflow(); }
        void parentSizeChanged() override { reflow(); }

    private:
        std::vector<ContentElement> content;

        void reflow()
        {
            const int hMargin = 5;
            const int vMargin = 10;
            const int spacing = 15;
            const int subH = 25;

            auto totalW = getWidth();
            auto contentW = totalW - 2 * hMargin;
            int  totalH = vMargin;

            for (auto& e : content)
            {
                if (e.type == ContentElement::Subheader)
                {
                    totalH += subH + spacing;
                }
                else if (e.type == ContentElement::Text)
                {
                    auto paragraphs = juce::StringArray::fromTokens(e.content, "\n", "");
                    for (auto& p : paragraphs)
                    {
                        juce::AttributedString as;
                        as.append(p, juce::Font(14.0f), juce::Colours::white);
                        as.setWordWrap(juce::AttributedString::WordWrap::byWord);

                        juce::TextLayout tl;
                        // Use the same single width:
                        tl.createLayout(as, (float)contentW);

                        totalH += (int)tl.getHeight() + spacing;
                    }
                }
            }

            totalH += vMargin;  // bottom padding
            setSize(getWidth(), totalH);
        }
    } contentComponent;





    std::vector<ContentElement> pageContent;

    void loadPageContent(const juce::String& pageID) {
        const juce::String resourceName = pageID + "Help_json";
        int dataSize = 0;
        const char* data = BinData::getNamedResource(resourceName.toRawUTF8(), dataSize);

        if (data != nullptr && dataSize > 0) {
            auto json = juce::JSON::parse(juce::String(data, (size_t)dataSize));
            parseJsonContent(json);
        }
    }

    void parseJsonContent(const juce::var& json) {
        if (auto* obj = json.getDynamicObject()) {
            currentPageName = obj->getProperty("pageTitle").toString();

            if (auto* contentArray = obj->getProperty("content").getArray()) {
                for (const auto& element : *contentArray) {
                    if (auto* elemObj = element.getDynamicObject()) {
                        ContentElement ce;
                        juce::String type = elemObj->getProperty("type").toString();

                        if (type == "subheader") {
                            ce.type = ContentElement::Subheader;
                            ce.content = elemObj->getProperty("text").toString();
                        }
                        else if (type == "text") {
                            ce.type = ContentElement::Text;
                            ce.content = elemObj->getProperty("content").toString();
                        }

                        pageContent.push_back(ce);
                    }
                }
            }
        }
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HelpCompendium)
};