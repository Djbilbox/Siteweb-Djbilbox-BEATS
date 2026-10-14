#include "NeonKeyboard.h"
#include "K2000LookAndFeel.h"

namespace K2000UI
{
    NeonKeyboard::NeonKeyboard()
    {
        keyStates.fill(false);
        keyVelocities.fill(1.0f);
    }

    void NeonKeyboard::resized()
    {
    }

    bool NeonKeyboard::isBlackKey(int noteIndex) const
    {
        int noteInOctave = (startNote + noteIndex) % 12;
        return (noteInOctave == 1 || noteInOctave == 3 || noteInOctave == 6 || noteInOctave == 8 || noteInOctave == 10);
    }

    void NeonKeyboard::setNoteState(int midiNote, bool isDown, float velocity)
    {
        int idx = midiNote - startNote;
        if (idx >= 0 && idx < numKeys)
        {
            keyStates[static_cast<size_t>(idx)] = isDown;
            keyVelocities[static_cast<size_t>(idx)] = velocity;
            repaint();
        }
    }

    int NeonKeyboard::getNoteAtPosition(int x, int y)
    {
        float whiteKeyWidth = static_cast<float>(getWidth()) / numWhiteKeys;
        float blackKeyWidth = whiteKeyWidth * 0.6f;
        float blackKeyHeight = getHeight() * 0.6f;

        // Check black keys first (higher z-order)
        if (y < blackKeyHeight)
        {
            int currentWhite = 0;
            for (int i = 0; i < numKeys; ++i)
            {
                if (isBlackKey(i))
                {
                    float bx = currentWhite * whiteKeyWidth - (blackKeyWidth * 0.5f);
                    if (x >= bx && x <= bx + blackKeyWidth)
                        return startNote + i;
                }
                else
                {
                    currentWhite++;
                }
            }
        }

        // Check white keys
        int whiteIndex = static_cast<int>(x / whiteKeyWidth);
        int whiteCount = 0;
        for (int i = 0; i < numKeys; ++i)
        {
            if (!isBlackKey(i))
            {
                if (whiteCount == whiteIndex)
                    return startNote + i;
                whiteCount++;
            }
        }

        return -1;
    }

    void NeonKeyboard::paint(juce::Graphics& g)
    {
        auto bounds = getLocalBounds().toFloat();
        float whiteKeyWidth = bounds.getWidth() / numWhiteKeys;
        float blackKeyWidth = whiteKeyWidth * 0.6f;
        float blackKeyHeight = bounds.getHeight() * 0.6f;

        // 1. Keyboard Chassis Frame & Cyan Neon Under-Rail
        g.setColour(K2000LookAndFeel::recessedBg);
        g.fillRect(bounds);
        g.setColour(K2000LookAndFeel::neonCyan);
        g.drawLine(bounds.getX(), bounds.getY(), bounds.getRight(), bounds.getY(), 2.0f);

        // 2. Draw White Keys
        int whiteIdx = 0;
        for (int i = 0; i < numKeys; ++i)
        {
            if (!isBlackKey(i))
            {
                float kx = whiteIdx * whiteKeyWidth;
                auto keyRect = juce::Rectangle<float>(kx + 0.5f, 2.0f, whiteKeyWidth - 1.0f, bounds.getHeight() - 3.0f);
                bool isPressed = keyStates[static_cast<size_t>(i)];

                // Key body gradient
                juce::Colour topCol = isPressed ? juce::Colour(0xCC, 0xE5, 0xEE) : juce::Colour(0xFA, 0xFA, 0xFC);
                juce::Colour botCol = isPressed ? juce::Colour(0x99, 0xCC, 0xDD) : juce::Colour(0xDE, 0xE4, 0xEA);
                juce::ColourGradient grad(topCol, kx, 2.0f, botCol, kx, bounds.getHeight(), false);
                g.setGradientFill(grad);
                g.fillRoundedRectangle(keyRect, 2.0f);

                // Cyan underglow when pressed
                if (isPressed)
                {
                    float vel = keyVelocities[static_cast<size_t>(i)];
                    g.setColour(K2000LookAndFeel::neonCyan.withAlpha(0.6f * vel));
                    g.fillRoundedRectangle(keyRect.withTrimmedTop(bounds.getHeight() * 0.7f), 2.0f);
                }

                // White key border
                g.setColour(juce::Colour(0x50, 0x5C, 0x68));
                g.drawRoundedRectangle(keyRect, 2.0f, 1.0f);

                whiteIdx++;
            }
        }

        // 3. Draw Black Keys
        whiteIdx = 0;
        for (int i = 0; i < numKeys; ++i)
        {
            if (isBlackKey(i))
            {
                float bx = whiteIdx * whiteKeyWidth - (blackKeyWidth * 0.5f);
                auto keyRect = juce::Rectangle<float>(bx, 2.0f, blackKeyWidth, blackKeyHeight);
                bool isPressed = keyStates[static_cast<size_t>(i)];

                juce::Colour topCol = isPressed ? juce::Colour(0x2A, 0x40, 0x50) : juce::Colour(0x28, 0x2E, 0x35);
                juce::Colour botCol = isPressed ? juce::Colour(0x10, 0x22, 0x2E) : juce::Colour(0x12, 0x15, 0x18);
                juce::ColourGradient grad(topCol, bx, 2.0f, botCol, bx, blackKeyHeight, false);
                g.setGradientFill(grad);
                g.fillRoundedRectangle(keyRect, 2.0f);

                // Cyan glow on black key pressed
                if (isPressed)
                {
                    float vel = keyVelocities[static_cast<size_t>(i)];
                    g.setColour(K2000LookAndFeel::neonCyan.withAlpha(0.7f * vel));
                    g.drawRoundedRectangle(keyRect, 2.0f, 1.5f);
                }
                else
                {
                    g.setColour(juce::Colour(0x40, 0x4C, 0x58));
                    g.drawRoundedRectangle(keyRect, 2.0f, 1.0f);
                }
            }
            else
            {
                whiteIdx++;
            }
        }
    }

    void NeonKeyboard::mouseDown(const juce::MouseEvent& event)
    {
        int note = getNoteAtPosition(event.x, event.y);
        if (note != -1)
        {
            currentPlayingNote = note;
            setNoteState(note, true, 0.9f);
            if (onNoteOn) onNoteOn(note, 0.9f);
        }
    }

    void NeonKeyboard::mouseDrag(const juce::MouseEvent& event)
    {
        int note = getNoteAtPosition(event.x, event.y);
        if (note != currentPlayingNote)
        {
            if (currentPlayingNote != -1)
            {
                setNoteState(currentPlayingNote, false);
                if (onNoteOff) onNoteOff(currentPlayingNote);
            }
            if (note != -1)
            {
                currentPlayingNote = note;
                setNoteState(note, true, 0.9f);
                if (onNoteOn) onNoteOn(note, 0.9f);
            }
        }
    }

    void NeonKeyboard::mouseUp(const juce::MouseEvent& /*event*/)
    {
        if (currentPlayingNote != -1)
        {
            setNoteState(currentPlayingNote, false);
            if (onNoteOff) onNoteOff(currentPlayingNote);
            currentPlayingNote = -1;
        }
    }
}
