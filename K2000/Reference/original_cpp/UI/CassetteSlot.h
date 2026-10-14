#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace K2000UI
{
    class CassetteSlot : public juce::Component
    {
    public:
        CassetteSlot();
        ~CassetteSlot() override = default;

        void paint(juce::Graphics& g) override;
        void mouseEnter(const juce::MouseEvent& event) override;
        void mouseExit(const juce::MouseEvent& event) override;
        void mouseDown(const juce::MouseEvent& event) override;

        std::function<void()> onCassetteClicked;

    private:
        bool isHovered = false;
    };
}
