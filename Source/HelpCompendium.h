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
#include <functional>
#include "UIDrawer.h"



class BugsoundsAudioProcessorEditor;

class HelpCompendium : public juce::Component, private UIDrawer {
public:
    HelpCompendium(BugsoundsAudioProcessorEditor& editor);


    struct ContentElement {
        enum Type { Subheader, Text, Image };
        Type type;
        juce::String content;
        juce::Image image;
    };


    void closeCompendium();
    void paint(juce::Graphics& g) override;

    void resized() override;

    void setPage(const juce::String& pageID);

    juce::String getPageID() {
        return currentPageID;
    }

    std::function<void()> onClose;

private:
    juce::String currentPageName = "closed";
    juce::String currentPageID = "closed";
    juce::Viewport viewport;
    juce::Label headerLabel;

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
            float scalar = getWidth() / 292.f;
            const float hMargin = 5.f * scalar;
            const float vMargin = 10.f * scalar;
            const float spacing = 15.f * scalar;
            const float subH = 25.f * scalar;
            const float fontSize = 14.f * scalar;

            auto totalW = getWidth();
            auto contentW = totalW - 2 * hMargin;
            auto x0 = hMargin;
            int  y = vMargin;

            for (auto& e : content)
            {
                if (e.type == ContentElement::Subheader)
                {
                    x0 = 0;
                    juce::Colour startColor = Colour(0xff72d9ff).withAlpha(.5f);
                    juce::Colour endColor  = Colour(0xff3e28bd).withAlpha(.1f);
                    ColourGradient gradient(
                        startColor,
                        x0, (float)y,
                        endColor,
                        x0 + contentW, (float)y,
                        false);

                    g.setGradientFill(gradient);
                    g.fillRect(x0, (float)y, contentW, subH);

                    x0 += hMargin * 2;
                    g.setColour(juce::Colours::black);
                    g.setFont(headerFont);
                    g.drawText(e.content,
                        x0, y,
                        contentW, subH,
                        juce::Justification::centredLeft);

                    y += subH + spacing;
                    x0 = hMargin;
                }
                else if (e.type == ContentElement::Text)
                {
                    g.setColour(juce::Colours::white);
                    g.setFont(font);

                    auto paragraphs = juce::StringArray::fromTokens(e.content, "\n", "");

                    for (auto& p : paragraphs)
                    {
                        juce::AttributedString as;
                        as.append(p, font, juce::Colours::black);
                        as.setWordWrap(juce::AttributedString::WordWrap::byWord);

                        juce::TextLayout tl;
                        tl.createLayout(as, (float)contentW);
                        tl.draw(g, { (float)x0, (float)y, (float)contentW, tl.getHeight() });

                        y += (int)tl.getHeight() + spacing;
                    }
                }
            }
        }

        void resized() override { reflow(); }
        void parentSizeChanged() override { reflow(); }
        juce::Font font;
        juce::Font headerFont;
    private:
        std::vector<ContentElement> content;

        void reflow()
        {
            float scalar = getWidth() / 292.f;
            const float hMargin = 5.f * scalar;
            const float vMargin = 10.f * scalar;
            const float spacing = 15.f * scalar;
            const float subH = 25.f * scalar;
            const float fontSize = 14.f * scalar;

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
                        as.append(p, juce::Font(fontSize), juce::Colours::white);
                        as.setWordWrap(juce::AttributedString::WordWrap::byWord);

                        juce::TextLayout tl;
                        tl.createLayout(as, (float)contentW);
                        totalH += (int)tl.getHeight() + spacing;
                    }
                }
            }

            totalH += vMargin;
            setSize(getWidth(), totalH * 2.f);
        }


    } contentComponent;
    std::vector<ContentElement> pageContent;
    std::unique_ptr<juce::TextButton> closeButton;
    BugsoundsAudioProcessorEditor& audioEditor;
    

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