#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace K2000UI
{
    class PitchModWheels : public juce::Component
    {
    public:
        PitchModWheels();
        ~PitchModWheels() override = default;

        void paint(juce::Graphics& g) override;
        void resized() override;

        void mouseDown(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;
        void mouseUp(const juce::MouseEvent& event) override;

        std::function<void(float)> onPitchBendChanged;
        std::function<void(float)> onModWheelChanged;

        void setModWheelValue(float val);

    private:
        float pitchValue = 0.0f; // -1.0 to 1.0 (0.0 center)
        float modValue = 0.0f;   // 0.0 to 1.0

        bool isDraggingPitch = false;
        bool isDraggingMod = false;
        float dragStartY = 0.0f;
        float initialVal = 0.0f;

        void drawWheel(juce::Graphics& g, const juce::Rectangle<float>& bounds, float value, bool isPitch, const juce::String& label);
    };
}
