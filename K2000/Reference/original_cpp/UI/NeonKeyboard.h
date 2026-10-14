#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <array>

namespace K2000UI
{
    class NeonKeyboard : public juce::Component
    {
    public:
        NeonKeyboard();
        ~NeonKeyboard() override = default;

        void paint(juce::Graphics& g) override;
        void resized() override;

        void mouseDown(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;
        void mouseUp(const juce::MouseEvent& event) override;

        void setNoteState(int midiNote, bool isDown, float velocity = 1.0f);

        std::function<void(int midiNote, float velocity)> onNoteOn;
        std::function<void(int midiNote)> onNoteOff;

    private:
        static constexpr int startNote = 48; // C3 (25 keys = C3 to C5)
        static constexpr int numKeys = 25;
        static constexpr int numWhiteKeys = 15;

        std::array<bool, numKeys> keyStates {};
        std::array<float, numKeys> keyVelocities {};

        int currentPlayingNote = -1;

        int getNoteAtPosition(int x, int y);
        bool isBlackKey(int noteIndex) const;
    };
}
