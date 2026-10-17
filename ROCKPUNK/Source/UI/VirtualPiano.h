#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <set>
#include <vector>

#include "Palette.h"
#include "RockpunkLookAndFeel.h"

namespace rk
{

//==============================================================================
// Virtual piano. The keyboard artwork is a direct crop of the background image
// (RockpunkLookAndFeel::drawCropSprite); the neon violet/rose glow overlays the
// keys while a note is held. If no artwork is available it falls back to plain
// drawn keys. Clicking / dragging keys fires onNoteChange so the processor can
// retrigger the internal synth (wired through MidiKeyboardState in the editor).
//==============================================================================
class VirtualPiano final : public juce::Component
{
public:
    VirtualPiano (RockpunkLookAndFeel& lfRef, const juce::String& spriteKey)
        : artwork (lfRef),
          key (spriteKey)
    {
        setOpaque (true);
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
    }

    void setRange (int low, int high)
    {
        lowNote = low;
        highNote = high;
        rebuildLayout();
        repaint();
    }

    void setNotesDown (const std::set<int>& notes)
    {
        notesDown = notes;
        repaint();
    }

    void setGlow01 (float g) noexcept { glow = juce::jlimit (0.0f, 1.0f, g); }

    std::function<void (int note, bool down)> onNoteChange;

    void paint (juce::Graphics& g) override
    {
        g.fillAll (Palette::panel());

        const bool spriteMode = artwork.hasSprite (key);
        if (spriteMode)
            artwork.drawCropSprite (g, key, getLocalBounds());

        // white keys
        for (const auto& k : keys)
        {
            if (k.black)
                continue;

            const bool down  = notesDown.count (k.note) > 0;
            const bool hover = (k.note == hoverNote) && ! down;
            const auto rect  = k.rect.toFloat();

            if (spriteMode)
            {
                if (down)
                {
                    g.setColour (Palette::neonViolet().withAlpha (0.55f));
                    g.fillRoundedRectangle (rect.expanded (2.0f), 4.0f);
                    g.setColour (Palette::neonRose().withAlpha (0.75f));
                    g.fillRoundedRectangle (rect.reduced (1.0f, 2.0f), 2.0f);
                }
                else if (hover)
                {
                    g.setColour (Palette::neonViolet().withAlpha (0.18f));
                    g.fillRoundedRectangle (rect, 3.0f);
                }
            }
            else
            {
                if (down)
                {
                    g.setColour (Palette::neonViolet().withAlpha (0.55f));
                    g.fillRoundedRectangle (rect.expanded (3.0f), 4.0f);
                    g.setColour (Palette::neonViolet().brighter (0.25f));
                    g.fillRoundedRectangle (rect, 3.0f);
                    g.setColour (Palette::neonRose().withAlpha (0.6f));
                    g.fillRoundedRectangle (rect.reduced (2.0f, 2.0f), 2.0f);
                }
                else
                {
                    g.setColour (hover ? Palette::whiteKey().darker (0.12f) : Palette::whiteKey());
                    g.fillRoundedRectangle (rect, 3.0f);
                }

                g.setColour (Palette::chrome());
                g.drawRect (rect, 1.0f);
            }
        }

        // black keys (drawn last, on top)
        for (const auto& k : keys)
        {
            if (! k.black)
                continue;

            const bool down = notesDown.count (k.note) > 0;
            const auto rect = k.rect.toFloat();

            if (spriteMode)
            {
                if (down)
                {
                    g.setColour (Palette::neonViolet().withAlpha (0.6f));
                    g.fillRoundedRectangle (rect.expanded (2.0f), 3.0f);
                    g.setColour (Palette::neonRose());
                    g.fillRoundedRectangle (rect, 3.0f);
                }
            }
            else
            {
                if (down)
                {
                    g.setColour (Palette::neonViolet().withAlpha (0.6f));
                    g.fillRoundedRectangle (rect.expanded (2.0f), 3.0f);
                    g.setColour (Palette::neonRose());
                    g.fillRoundedRectangle (rect, 3.0f);
                }
                else
                {
                    g.setColour (juce::Colour (0xFF14110F));
                    g.fillRoundedRectangle (rect, 3.0f);
                }

                g.setColour (Palette::chrome());
                g.drawRect (rect, 1.0f);
            }
        }
    }

    void resized() override { rebuildLayout(); }

    void mouseDown (const juce::MouseEvent& e) override
    {
        const int note = noteAt (e.getPosition());
        if (note < 0)
            return;
        dragNote = note;
        notesDown.insert (note);
        hoverNote = note;
        repaint();
        if (onNoteChange)
            onNoteChange (note, true);
    }

    void mouseUp (const juce::MouseEvent&) override
    {
        if (dragNote >= 0)
        {
            notesDown.erase (dragNote);
            if (onNoteChange)
                onNoteChange (dragNote, false);
            dragNote = -1;
        }
        repaint();
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        const int note = noteAt (e.getPosition());
        if (note < 0 || note == dragNote)
            return;

        if (dragNote >= 0)
        {
            notesDown.erase (dragNote);
            if (onNoteChange)
                onNoteChange (dragNote, false);
        }

        dragNote = note;
        notesDown.insert (note);
        hoverNote = note;
        if (onNoteChange)
            onNoteChange (note, true);
        repaint();
    }

    void mouseExit (const juce::MouseEvent&) override
    {
        hoverNote = -1;
        repaint();
    }

private:
    struct KeyInfo
    {
        int note = 0;
        juce::Rectangle<int> rect;
        bool black = false;
    };

    static bool isBlack (int pc) noexcept
    {
        return pc == 1 || pc == 3 || pc == 6 || pc == 8 || pc == 10;
    }

    void rebuildLayout()
    {
        keys.clear();

        if (getWidth() <= 0 || getHeight() <= 0)
            return;

        const int h = getHeight();

        int whiteCount = 0;
        for (int n = lowNote; n <= highNote; ++n)
            if (! isBlack (n % 12))
                ++whiteCount;

        if (whiteCount == 0)
            return;

        const float whiteW = (float) getWidth() / (float) whiteCount;

        // white keys
        int wi = 0;
        for (int n = lowNote; n <= highNote; ++n)
        {
            if (isBlack (n % 12))
                continue;
            const int x0 = (int) (wi * whiteW);
            const int x1 = (int) ((wi + 1) * whiteW) + 1;
            keys.push_back ({ n, { x0, 0, x1 - x0, h }, false });
            ++wi;
        }

        // black keys, inserted after the white key they follow
        for (const auto& wk : keys)
        {
            const int pc = wk.note % 12;
            const bool hasBlack = (pc == 0 || pc == 2 || pc == 5 || pc == 7 || pc == 9) && (wk.note + 1 <= highNote);
            if (! hasBlack)
                continue;

            const float bw = whiteW * 0.62f;
            const int bx = (int) (wk.rect.getX() + wk.rect.getWidth() - bw * 0.5f);
            keys.push_back ({ wk.note + 1, { bx, 0, (int) bw + 1, (int) (h * 0.62f) }, true });
        }
    }

    int noteAt (juce::Point<int> p) const noexcept
    {
        for (auto it = keys.rbegin(); it != keys.rend(); ++it)
            if (it->rect.contains (p))
                return it->note;
        return -1;
    }

    int lowNote = 36;
    int highNote = 96;
    std::vector<KeyInfo> keys;
    std::set<int> notesDown;
    int hoverNote = -1;
    int dragNote = -1;
    float glow = 0.0f;
    RockpunkLookAndFeel& artwork;
    juce::String key;
};

} // namespace rk