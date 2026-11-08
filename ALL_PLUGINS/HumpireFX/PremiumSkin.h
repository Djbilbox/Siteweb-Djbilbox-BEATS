#pragma once
// HUMPIRE premium skin, shared by the DJBILBOX BEATS synths.
//  - FaceplateBackground: the whole front panel is one photo-real render (made with Google Flow from the synth's own
//    interface, same layout, without pointers, fader caps or screen texts); drawn from a cache at the screen's pixel size.
//  - plateRotary / plateLinear: what the sliders still draw on top of it (value ring, lit index, fader cap).
//  - PremiumOverlay: a transparent layer over the whole panel with everything that lights up and moves: knob halos,
//    the LFO dot, the live cutoff marker, fader LEDs following the envelopes, keys, the mascot's halo and shock ring,
//    the neon title, a stereo LED meter, and one animation of the synth's own theme.
#include "NeonFX.h"

namespace hfx
{
//============================================================================== background
class FaceplateBackground
{
public:
    void setImage (juce::Image img) { face = std::move (img); cache = {}; }
    // the synth's own mascot artwork put back in the faceplate's round medallion (centre, radius: panel pixels)
    void setMascot (juce::Image img, juce::Point<float> centre, float radius) { mascot = std::move (img); mascotC = centre; mascotR = radius; cache = {}; }
    void draw (juce::Graphics& g, juce::Rectangle<int> bounds)
    {
        const float sc = g.getInternalContext().getPhysicalPixelScaleFactor();
        if (cache.isNull() || std::abs (sc - scale) > 0.01f || bounds != area)
        {
            scale = sc; area = bounds;
            cache = juce::Image (juce::Image::RGB, juce::jmax (1, juce::roundToInt ((float) bounds.getWidth() * sc)),
                                 juce::jmax (1, juce::roundToInt ((float) bounds.getHeight() * sc)), false);
            juce::Graphics gc (cache);
            gc.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
            gc.drawImage (face, cache.getBounds().toFloat(), juce::RectanglePlacement::stretchToFit);
            if (mascot.isValid() && mascotR > 0.f)
            {
                gc.addTransform (juce::AffineTransform::scale ((float) cache.getWidth() / (float) bounds.getWidth()));
                juce::Path disc; disc.addEllipse (juce::Rectangle<float> (mascotR * 2.f, mascotR * 2.f).withCentre (mascotC));
                gc.reduceClipRegion (disc);
                gc.setOpacity (1.f);
                gc.drawImage (mascot, juce::Rectangle<float> (mascotR * 2.08f, mascotR * 2.08f).withCentre (mascotC), juce::RectanglePlacement::centred);
            }
        }
        juce::Graphics::ScopedSaveState ss (g);
        g.setImageResamplingQuality (juce::Graphics::lowResamplingQuality);   // already at pixel size: a plain copy
        g.drawImage (cache, bounds.toFloat());
    }
private:
    juce::Image face, cache, mascot;
    juce::Point<float> mascotC; float mascotR = 0.f;
    float scale = 0.f;
    juce::Rectangle<int> area;
};

//============================================================================== slider drawing on the faceplate
// Value ring just outside the knob cap of the faceplate, and a lit index on the cap turning with the value.
inline void plateRotary (juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider& s, juce::Colour col)
{
    const auto c = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (3.f);
    const float r = juce::jmin (c.getWidth(), c.getHeight()) * 0.5f;
    const auto ctr = c.getCentre();
    const float ang = a0 + pos * (a1 - a0);
    const float glow = (float) s.getProperties().getWithDefault ("glow", 0.f);
    const float ring = r > 15.f ? 3.f : 2.2f;
    juce::Path track; track.addCentredArc (ctr.x, ctr.y, r, r, 0, a0, a1, true);
    g.setColour (juce::Colours::black.withAlpha (0.45f)); g.strokePath (track, juce::PathStrokeType (ring + 1.f));
    juce::Path val; val.addCentredArc (ctr.x, ctr.y, r, r, 0, a0, ang, true);
    g.setColour (col); g.strokePath (val, juce::PathStrokeType (ring));
    if (glow > 0.45f)
    { g.setColour (col.interpolatedWith (juce::Colours::white, 0.7f).withAlpha (juce::jmin (1.f, (glow - 0.45f) * 1.2f))); g.strokePath (val, juce::PathStrokeType (ring * 0.4f)); }
    const float capR = r * 0.79f;
    const juce::Point<float> dir (std::sin (ang), -std::cos (ang));
    juce::Path idx; idx.startNewSubPath (ctr + dir * (capR * 0.3f)); idx.lineTo (ctr + dir * (capR * 0.86f));
    neonStroke (g, idx, col, r > 15.f ? 2.f : 1.5f, 0.6f + 0.6f * glow);
}

// Fader on the faceplate's slot: a thin lit strip below the cap, the cap photo, its groove lit while its stage plays.
inline void plateLinear (juce::Graphics& g, int x, int y, int w, int h, float pos, juce::Slider& s, const juce::Image& capImg, juce::Colour col)
{
    const float cx = (float) x + (float) w * 0.5f;
    g.setColour (col.withAlpha (0.8f)); g.fillRoundedRectangle (cx - 1.f, pos, 2.f, (float) y + (float) h - pos, 1.f);
    const float led = (float) s.getProperties().getWithDefault ("led", 0.f);
    const float cw = juce::jmin ((float) w, 30.f), ch = cw * (float) capImg.getHeight() / (float) juce::jmax (1, capImg.getWidth());
    const auto cap = juce::Rectangle<float> (cx - cw * 0.5f, pos - ch * 0.5f, cw, ch);
    glowSpot (g, cap.getCentre().translated (0.f, 4.f), cw * 0.55f, juce::Colours::black, 0.75f);
    g.setOpacity (1.f);
    g.drawImage (capImg, cap, juce::RectanglePlacement::stretchToFit);
    if (led > 0.f)
    {
        juce::Path groove; groove.startNewSubPath (cap.getX() + cw * 0.22f, cap.getCentreY()); groove.lineTo (cap.getRight() - cw * 0.22f, cap.getCentreY());
        neonStroke (g, groove, col, 1.2f, 0.5f + led);
    }
}

// Pitch bend / mod wheel cap on the faceplate's slot.
inline void plateWheel (juce::Graphics& g, int x, int w, float pos)
{
    const float cx = (float) x + (float) w * 0.5f, th = 52.f;
    auto cap = juce::Rectangle<float> (cx - 20.f, pos - th * 0.5f, 40.f, th);
    glowSpot (g, cap.getCentre().translated (0.f, 6.f), 30.f, juce::Colours::black, 0.7f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff6a6d72), cap.getX(), 0, juce::Colour (0xff1d1e21), cap.getRight(), 0, false));
    g.fillRoundedRectangle (cap, 8.f);
    g.setColour (juce::Colours::black.withAlpha (0.8f));
    for (int i = 1; i < 7; ++i) g.drawHorizontalLine ((int) (cap.getY() + (float) i * th / 7.f), cap.getX() + 4, cap.getRight() - 4);
    g.setColour (juce::Colours::white.withAlpha (0.18f)); g.drawRoundedRectangle (cap.reduced (0.5f), 8.f, 1.f);
}

//============================================================================== the live layer
enum class Theme { electric, spotlights, glitch, police, goldRain, scanner, smoke, film, sun, bubbles, ghostFlame, sunset, rain, bassShake, orientalGold, snow, waves };

class PremiumOverlay : public juce::Component, private juce::Timer
{
public:
    // knob roles: what the knob's light follows
    enum Role { plain, volume, lfoRate, lfoAmount, cutoff, filterEnv, resonance, glide, effect, delayMix, delayTime, delayFeedback };

    PremiumOverlay (LiveTap& t, juce::MidiKeyboardState& ks, NeonKeyboard& kb, juce::ValueTree st)
        : tap (t), keyState (ks), keys (kb), state (std::move (st))
    {
        setInterceptsMouseClicks (false, false);
        setOpaque (false);
        lastTick = juce::Time::getMillisecondCounterHiRes();
        if (animationsOn()) startTimerHz (45);
    }
    ~PremiumOverlay() override { stopTimer(); }

    // ---- set-up (panel coordinates; the overlay covers the panel)
    juce::Colour a { 0xffff5a4a }, b { 0xff3fe06e };      // the synth's two accent colours
    Theme theme = Theme::electric;
    juce::Point<float> mascot; float mascotR = 0.f;       // round mascot medallion
    juce::Rectangle<float> meter;                         // stereo LED bar (empty = none)
    std::vector<juce::Point<float>> holes[2];             // optional speaker-grille LEDs (left, right channel)
    juce::Rectangle<float> effectArea;                    // where the theme animation lives (empty = whole panel)
    bool titleTube = true;                                // false: the title's letters are in the faceplate, only light here

    void setTitle (const juce::String& text, float fontSize, juce::Rectangle<float> area)
    {
        title.set (text, juce::FontOptions (fontSize, juce::Font::bold), area);
        hasTitle = true;
    }

    // The title is part of the faceplate picture: its letters are found in the picture (brighter than the plate
    // around them) and the light is made from their real shapes: a glow around them (never over them) and a band
    // of light sweeping across the letters only.
    void setTitleFromFace (const juce::Image& faceImg, juce::Rectangle<float> area)
    {
        const float s = (float) faceImg.getWidth() / 1000.f, pad = 26.f;
        titleArea = area.expanded (pad);
        const auto px = (titleArea * s).getSmallestIntegerContainer().getIntersection (faceImg.getBounds());
        if (px.isEmpty()) return;
        const juce::Image crop = faceImg.getClippedImage (px).createCopy();
        const int W = crop.getWidth(), H = crop.getHeight();
        std::vector<float> lum ((size_t) (W * H));
        double sum = 0, sum2 = 0; int n = 0;
        const int ip = (int) (pad * s);
        {
            const juce::Image::BitmapData bd (crop, juce::Image::BitmapData::readOnly);
            for (int y = 0; y < H; ++y)
                for (int x = 0; x < W; ++x)
                {
                    const auto c = bd.getPixelColour (x, y);
                    const float l = 0.3f * c.getFloatRed() + 0.59f * c.getFloatGreen() + 0.11f * c.getFloatBlue();
                    lum[(size_t) (y * W + x)] = l;
                    if (x >= ip && x < W - ip && y >= ip && y < H - ip) { sum += l; sum2 += l * l; ++n; }
                }
        }
        const float mean = (float) (sum / juce::jmax (1, n)), sd = (float) std::sqrt (juce::jmax (0.0, sum2 / juce::jmax (1, n) - mean * mean));
        const float t0 = mean + 0.35f * sd, t1 = mean + 0.9f * sd;
        titleMask = juce::Image (juce::Image::ARGB, W, H, true);
        {
            juce::Image::BitmapData bd (titleMask, juce::Image::BitmapData::writeOnly);
            for (int y = ip / 2; y < H - ip / 2; ++y)
                for (int x = ip / 2; x < W - ip / 2; ++x)
                {
                    const float a = juce::jlimit (0.f, 1.f, (lum[(size_t) (y * W + x)] - t0) / juce::jmax (0.001f, t1 - t0));
                    bd.setPixelColour (x, y, juce::Colours::white.withAlpha (a));
                }
        }
        // glow: the letters blurred at half size, the letters themselves cut out, tinted a -> b
        juce::Image small = titleMask.rescaled (juce::jmax (1, W / 2), juce::jmax (1, H / 2), juce::Graphics::mediumResamplingQuality);
        juce::ImageConvolutionKernel k (17); k.createGaussianBlur (6.f);
        k.applyToImage (small, small, small.getBounds()); k.applyToImage (small, small, small.getBounds());
        titleGlow = juce::Image (juce::Image::ARGB, W, H, true);
        {
            juce::Graphics gg (titleGlow);
            juce::ColourGradient brush (a, 0.f, 0.f, b, (float) W, 0.f, false);
            gg.setGradientFill (brush);
            gg.drawImage (small, juce::Rectangle<float> (0.f, 0.f, (float) W, (float) H), juce::RectanglePlacement::stretchToFit, true);
        }
        {
            juce::Image::BitmapData g (titleGlow, juce::Image::BitmapData::readWrite), m (titleMask, juce::Image::BitmapData::readOnly);
            for (int y = 0; y < H; ++y)
                for (int x = 0; x < W; ++x)
                {
                    const float keep = 1.f - m.getPixelColour (x, y).getFloatAlpha();
                    g.setPixelColour (x, y, g.getPixelColour (x, y).withMultipliedAlpha (keep));
                }
        }
        titleTube = false; hasTitle = true; titleFromFace = true;
    }
    void addKnob (juce::Slider& s, Role role) { knobs.push_back ({ &s, role }); }
    // fader of an envelope (amp or filter) stage 0..3; dots: its LED dots, column by column, top row first;
    // dimDots: also draw them unlit (when the faceplate has none)
    void addFader (juce::Slider& s, bool ampEnvelope, int stage, std::vector<juce::Point<float>> dots, bool dimDots = true)
    { faders.push_back ({ &s, ampEnvelope, stage, std::move (dots), dimDots }); }
    void addVelocityFader (juce::Slider& s, std::vector<juce::Point<float>> dots, bool dimDots = true) { faders.push_back ({ &s, false, -2, std::move (dots), dimDots }); }
    void addPlainFader (juce::Slider& s, std::vector<juce::Point<float>> dots, bool dimDots = true) { faders.push_back ({ &s, false, -3, std::move (dots), dimDots }); }
    // two columns of LED dots beside a fader slot (x = slot centre line), `rows` rows from top to bottom
    static std::vector<juce::Point<float>> ledColumns (float x, float top, float bottom, int rows = 8, float dx = 11.f)
    {
        std::vector<juce::Point<float>> d;
        for (int side = 0; side < 2; ++side)
            for (int r = 0; r < rows; ++r)
                d.push_back ({ x + (side == 0 ? -dx : dx), top + (bottom - top) * (float) r / (float) (rows - 1) });
        return d;
    }

    bool animationsOn() const { return (bool) state.getProperty ("animations", true); }
    void setAnimations (bool on)
    {
        state.setProperty ("animations", on, nullptr);
        if (on) { lastTick = juce::Time::getMillisecondCounterHiRes(); startTimerHz (45); } else stopTimer();
        repaint();
    }

    //==========================================================================
    void advance (float dt)
    {
        view.update (tap, dt);
        const float lv = view.level, hit = view.hit;
        for (auto& k : knobs)
        {
            const double v = k.s->getValue();
            const float p = (float) k.s->valueToProportionOfLength (v);
            k.glow = 0.28f + 0.55f * lv; k.mod = -1.f; k.orbit = -1.f;
            switch (k.role)
            {
                case volume:     k.glow = 0.3f + 1.0f * lv; break;
                case lfoRate:    k.idlePhase = std::fmod (k.idlePhase + dt * (float) v, 1.f); k.orbit = view.sounding ? view.lfoPhase : k.idlePhase; break;
                case lfoAmount:  k.glow = 0.28f + 0.7f * p * (0.5f + 0.5f * (view.sounding ? view.lfo : 0.f)); break;
                case cutoff:     k.glow = 0.3f + 0.6f * lv + 0.6f * view.filt;
                                 if (view.sounding) k.mod = (float) k.s->valueToProportionOfLength (20.0 * std::pow (1000.0, (double) view.cutoff)); break;
                case filterEnv:  k.glow = 0.25f + 1.1f * view.filt * juce::jmax (0.2f, p); break;
                case resonance:  k.glow = 0.25f + 0.9f * lv * (0.3f + p); break;
                case glide:      k.glow = p > 0.001f ? 0.28f + 0.9f * hit : 0.28f; break;
                case effect:     k.glow = p > 0.01f ? 0.3f + 0.9f * lv : 0.2f; break;
                default: break;
            }
        }
        // delay: TIME and FEEDBACK flash on each echo of the last note
        float mix = 0.f, T = 0.3f, fb = 0.f;
        for (auto& k : knobs)
        {
            if (k.role == delayMix) mix = (float) k.s->valueToProportionOfLength (k.s->getValue());
            if (k.role == delayTime) T = juce::jmax (0.03f, (float) k.s->getValue());
            if (k.role == delayFeedback) fb = (float) k.s->valueToProportionOfLength (k.s->getValue());
        }
        float echo = 0.f, amp = 1.f;
        if (mix > 0.01f)
            for (int i = 1; i <= 6; ++i) { amp *= fb; const float d = (view.sinceHit - (float) i * T) / 0.05f; echo += amp * std::exp (-d * d); }
        echo = juce::jmin (1.f, echo * mix * 2.f);
        for (auto& k : knobs)
        {
            if (k.role == delayTime || k.role == delayFeedback) k.glow = 0.28f + 1.1f * echo;
            if (k.role == delayMix) k.glow = 0.28f + 0.6f * lv + 0.6f * echo;
            k.s->getProperties().set ("glow", k.glow);
        }
        for (auto& f : faders)
        {
            float led = 0.f;
            if (f.stage == -2) led = hit * 0.9f;
            else if (f.stage == -3) led = (0.25f + 0.75f * lv) * (float) f.s->valueToProportionOfLength (f.s->getValue());   // a timbre fader: lit to its value, breathing with the sound
            else if (f.amp ? view.ampStage == f.stage : view.filtStage == f.stage) led = juce::jmax (0.08f, f.amp ? view.amp : view.filt);
            f.s->getProperties().set ("led", led);
            f.led = led;
        }

        // keys: sparks from every key that goes down
        for (int n = 0; n < 128; ++n)
        {
            const bool down = keyState.isNoteOnForChannels (0xffff, n);
            if (down && ! keyDown[(size_t) n] && keys.isShowing() && n >= keys.getRangeStart() && n <= keys.getRangeEnd())
            {
                const auto kr = keys.getRectangleForKey (n).translated ((float) keys.getX(), (float) keys.getY());
                sparks.emit ({ kr.getX(), kr.getY() - 2.f, kr.getWidth(), 4.f }, 6, { 0.f, -110.f }, 70.f, 0.9f, 2.f, n % 2 ? a : b);
            }
            keyDown[(size_t) n] = down;
        }
        keys.advance (dt);
        themeAdvance (dt);
        sparks.update (dt);
        bolts.update (dt);
        invalidate();
    }

    void paint (juce::Graphics& g) override
    {
        const float lv = view.level, hit = view.hit, ign = view.ignition(), t = view.time;
        themeBehind (g);
        // knob halos, LFO dot, live cutoff
        for (auto& k : knobs)
        {
            const auto bb = k.s->getBounds();
            if (! g.clipRegionIntersects (bb.expanded (16))) continue;
            const auto box = bb.toFloat().reduced (3.f);
            const float r = juce::jmin (box.getWidth(), box.getHeight()) * 0.5f;
            const auto c = box.getCentre();
            const auto rp = k.s->getRotaryParameters();
            const float pos = (float) k.s->valueToProportionOfLength (k.s->getValue());
            const float ang = rp.startAngleRadians + pos * (rp.endAngleRadians - rp.startAngleRadians);
            juce::Path val; val.addCentredArc (c.x, c.y, r, r, 0.f, rp.startAngleRadians, ang, true);
            neonStroke (g, val, a.interpolatedWith (b, pos), r > 15.f ? 2.6f : 1.8f, 0.8f * ign * k.glow, false);
            if (k.orbit >= 0.f)
            {
                const float oa = juce::MathConstants<float>::twoPi * k.orbit - juce::MathConstants<float>::halfPi;
                const auto p = c + juce::Point<float> (std::cos (oa), std::sin (oa)) * (r + 5.f);
                glowSpot (g, p, 9.f, b, 0.9f * ign);
                g.setColour (juce::Colours::white); g.fillEllipse (p.x - 1.8f, p.y - 1.8f, 3.6f, 3.6f);
            }
            if (k.mod >= 0.f)
            {
                const float ma = rp.startAngleRadians + juce::jlimit (0.f, 1.f, k.mod) * (rp.endAngleRadians - rp.startAngleRadians);
                glowSpot (g, c + juce::Point<float> (std::sin (ma), -std::cos (ma)) * r, 10.f, juce::Colours::white, 0.8f);
            }
            knobCore (g, c, r, pos, k.glow * ign, (int) (&k - knobs.data()));
        }
        // fader LEDs on the faceplate's dots
        for (auto& f : faders)
        {
            if (f.dots.empty() || ! g.clipRegionIntersects (f.s->getBounds().expanded (22, 10))) continue;
            const int rows = (int) f.dots.size() / 2;
            for (size_t i = 0; i < f.dots.size(); ++i)
            {
                const int row = (int) i % juce::jmax (1, rows);
                const float h = rows > 1 ? (float) (rows - 1 - row) / (float) (rows - 1) : 0.f;
                if (f.led <= 0.f || h > f.led + 0.02f)
                {
                    if (f.dimDots)   // an unlit LED: a small dark lens with a faint colour
                    {
                        const auto p = f.dots[i];
                        g.setColour (juce::Colours::black.withAlpha (0.55f)); g.fillEllipse (p.x - 2.6f, p.y - 2.6f, 5.2f, 5.2f);
                        g.setColour ((h < 0.55f ? juce::Colour (0xff35ff6a) : h < 0.8f ? juce::Colour (0xffffc23a) : juce::Colour (0xffff3b3b)).withAlpha (0.16f));
                        g.fillEllipse (p.x - 1.8f, p.y - 1.8f, 3.6f, 3.6f);
                    }
                    continue;
                }
                g.setOpacity (1.f);
                g.drawImageAt (sprite (h < 0.55f ? 0 : h < 0.8f ? 1 : 2), juce::roundToInt (f.dots[i].x) - 7, juce::roundToInt (f.dots[i].y) - 7);
            }
        }
        // speaker-grille LEDs (if the faceplate has grilles)
        for (int ch = 0; ch < 2; ++ch)
        {
            const auto& hs = holes[ch];
            if (hs.empty()) continue;
            juce::Rectangle<float> box (hs[0], hs[0]);
            for (auto p : hs) box = box.getUnion ({ p, p });
            if (! g.clipRegionIntersects (box.expanded (8.f).getSmallestIntegerContainer())) continue;
            const float level = ch == 0 ? view.levelL : view.levelR, hold = ch == 0 ? view.holdL : view.holdR;
            const float span = juce::jmax (1.f, box.getHeight()), halfW = juce::jmax (1.f, box.getWidth() * 0.5f);
            for (auto p : hs)
            {
                const float h = (box.getBottom() - p.y) / span;
                const float centre = 1.f - std::abs (p.x - box.getCentreX()) / halfW;
                const float colLevel = juce::jlimit (0.f, 1.f, level * (0.7f + 0.3f * centre) + 0.06f * std::sin (t * 7.f + p.x * 0.21f) * level);
                const bool lit = h <= colLevel + 0.02f, peak = std::abs (h - hold) < 5.f / span && hold > 0.05f;
                if (! (lit || peak)) continue;
                g.setOpacity (peak && ! lit ? 0.8f : 1.f);
                g.drawImageAt (sprite (h < 0.6f ? 0 : h < 0.82f ? 1 : 2), juce::roundToInt (p.x) - 7, juce::roundToInt (p.y) - 7);
            }
        }
        g.setOpacity (1.f);
        // mascot: a halo breathing with the sound, a shock ring on every note
        if (mascotR > 0.f)
        {
            if (g.clipRegionIntersects (juce::Rectangle<float> (2.f * (mascotR + 48.f), 2.f * (mascotR + 48.f)).withCentre (mascot).getSmallestIntegerContainer()))
            {
                const float mix = 0.5f + 0.5f * std::sin (t * 0.7f), k = ign * (0.3f + 1.1f * lv + 0.7f * hit);
                haloA.draw (g, mascot, mascotR, 34.f, a, k * (1.f - mix));
                haloB.draw (g, mascot, mascotR, 34.f, b, k * mix);
            }
            if (view.sinceHit < 0.6f)
            {
                juce::Path ring; const float rr = mascotR + view.sinceHit * 170.f;
                ring.addEllipse (mascot.x - rr, mascot.y - rr, rr * 2.f, rr * 2.f);
                neonStroke (g, ring, a.interpolatedWith (b, view.sinceHit / 0.6f), 1.6f, 0.9f * (1.f - view.sinceHit / 0.6f));
            }
        }
        // title made of the faceplate's own letters: light around them, a band sweeping over them
        if (titleFromFace && g.clipRegionIntersects (titleArea.getSmallestIntegerContainer()))
        {
            const float k = ign * flicker * (0.35f + 0.15f * view.breathe (3.5f, 0.f, 1.f) + 0.55f * lv + 0.45f * hit);
            juce::Graphics::ScopedSaveState ss (g);
            g.setImageResamplingQuality (juce::Graphics::mediumResamplingQuality);
            g.setOpacity (juce::jmin (1.f, k)); g.drawImage (titleGlow, titleArea);
            if (k > 1.f) { g.setOpacity (k - 1.f); g.drawImage (titleGlow, titleArea); }
            const float sweep = std::fmod (t, 7.f) / 1.3f;
            if (sweep < 1.2f)
            {
                g.setOpacity (1.f);
                g.reduceClipRegion (titleMask, juce::AffineTransform::scale (titleArea.getWidth() / (float) titleMask.getWidth(), titleArea.getHeight() / (float) titleMask.getHeight())
                                                    .translated (titleArea.getX(), titleArea.getY()));
                const float x = titleArea.getX() + titleArea.getWidth() * sweep;
                juce::ColourGradient band (juce::Colours::white.withAlpha (0.f), x - 60.f, 0.f, juce::Colours::white.withAlpha (0.f), x + 60.f, 0.f, false);
                band.addColour (0.5, juce::Colours::white.withAlpha (0.5f));
                g.setGradientFill (band); g.fillRect (titleArea);
            }
        }
        // neon title
        else if (hasTitle && g.clipRegionIntersects (title.getBounds().expanded (44.f).getSmallestIntegerContainer()))
        {
            const auto tb = title.getBounds();
            juce::ColourGradient brush (a, tb.getX(), 0.f, b, tb.getRight(), 0.f, false);
            const float sweep = std::fmod (t, 7.f) / 1.3f;
            title.draw (g, brush, juce::Colour (0xfffbf6ea), ign * flicker * (0.55f + 0.15f * view.breathe (3.5f, 0.f, 1.f) + 0.55f * lv + 0.45f * hit),
                        sweep < 1.2f ? sweep : -1.f, titleTube);
        }
        // stereo LED meter
        if (! meter.isEmpty() && g.clipRegionIntersects (meter.expanded (4.f).getSmallestIntegerContainer()))
        {
            ledBar (g, meter.withHeight (meter.getHeight() * 0.42f), view.levelL, view.holdL);
            ledBar (g, meter.withTrimmedTop (meter.getHeight() * 0.58f), view.levelR, view.holdR);
        }
        // keyboard: a neon line along its top edge
        {
            const auto kb = keys.getBounds();
            if (g.clipRegionIntersects (kb.withHeight (1).expanded (2, 22)))
            {
                juce::Path top; top.startNewSubPath ((float) kb.getX(), (float) kb.getY() - 1.f); top.lineTo ((float) kb.getRight(), (float) kb.getY() - 1.f);
                neonStroke (g, top, b.interpolatedWith (a, hit), 1.4f, ign * (0.25f + 0.9f * lv));
            }
        }
        themeFront (g);
        sparks.draw (g);
        bolts.draw (g);
    }

private:
    struct KnobRef { juce::Slider* s; Role role; float glow = 0.f, mod = -1.f, orbit = -1.f, idlePhase = 0.f, shownGlow = -1.f; bool shownMod = false; };
    struct FaderRef { juce::Slider* s; bool amp; int stage; std::vector<juce::Point<float>> dots; bool dimDots = true; float led = 0.f, shownLed = -1.f; };

    LiveTap& tap;
    juce::MidiKeyboardState& keyState;
    NeonKeyboard& keys;
    juce::ValueTree state;
    LiveView view;
    NeonTitle title; bool hasTitle = false, titleFromFace = false;
    juce::Image titleMask, titleGlow; juce::Rectangle<float> titleArea;   // title found in the faceplate
    HaloImage haloA, haloB;
    Sparks sparks;
    Bolts bolts;
    juce::Random rnd { 2027 };
    std::vector<KnobRef> knobs;
    std::vector<FaderRef> faders;
    std::array<bool, 128> keyDown {};
    juce::Image sprites[3];
    double lastTick = 0.0;
    float idleDt = 0.f, flicker = 1.f, shownMeter = -1.f, shownLine = -1.f, themeT = 0.f, themeTimer = 0.f;
    int skip = 0;
    juce::RectangleList<int> prevMoving;
    // theme state
    struct Blob { juce::Point<float> p, v; float life, max, size; juce::Colour c; };
    std::vector<Blob> blobs;
    float beam = 0.f, shake = 0.f;

    const juce::Image& sprite (int i)
    {
        if (sprites[i].isNull())
        {
            const juce::Colour cols[3] { juce::Colour (0xff35ff6a), juce::Colour (0xffffc23a), juce::Colour (0xffff3b3b) };
            sprites[i] = juce::Image (juce::Image::ARGB, 15, 15, true);
            juce::Graphics gs (sprites[i]);
            glowSpot (gs, { 7.5f, 7.5f }, 7.f, cols[i], 0.35f);
            gs.setColour (cols[i].withAlpha (0.95f)); gs.fillEllipse (5.f, 5.f, 5.f, 5.f);
        }
        return sprites[i];
    }

    void timerCallback() override
    {
        const double now = juce::Time::getMillisecondCounterHiRes();
        float dt = (float) juce::jlimit (0.0, 0.1, (now - lastTick) * 0.001);
        lastTick = now;
        // 45 frames per second while playing; quiet: 15 are plenty for slow breathing light
        if (! view.sounding && view.sinceHit > 2.f && (++skip % 3) != 0) { idleDt += dt; return; }
        dt += idleDt; idleDt = 0.f;
        flicker = rnd.nextFloat() < 0.004f ? 0.55f : 1.f;
        advance (dt);
    }

    juce::Rectangle<float> area() const { return effectArea.isEmpty() ? getLocalBounds().toFloat() : effectArea; }

    //------------------------------------------------------------------ the theme animations
    void themeAdvance (float dt)
    {
        themeT += dt;
        const float lv = view.level;
        const auto ar = area();
        auto spawn = [this] (juce::Point<float> p, juce::Point<float> v, float life, float size, juce::Colour c)
        { if (blobs.size() < 90) blobs.push_back ({ p, v, life, life, size, c }); };
        auto rx = [&] { return ar.getX() + rnd.nextFloat() * ar.getWidth(); };
        auto ry = [&] { return ar.getY() + rnd.nextFloat() * ar.getHeight(); };
        const float rate = view.sounding ? 1.f : 0.25f;   // fewer things move when nothing plays
        themeTimer -= dt * rate;
        const bool emit = themeTimer <= 0.f;
        if (emit) themeTimer = 0.12f;
        switch (theme)
        {
            case Theme::electric:
                if (view.newNote && mascotR > 0.f) for (int i = 0; i < 2; ++i) arc (1.f - 0.4f * (float) i);
                if (rnd.nextFloat() < dt * 0.3f && mascotR > 0.f) arc (0.3f);
                break;
            case Theme::goldRain:      // gold coins / flakes falling and turning
                if (emit) spawn ({ rx(), ar.getY() - 6.f }, { (rnd.nextFloat() - 0.5f) * 20.f, 60.f + 60.f * rnd.nextFloat() }, 5.f, 3.f + 3.f * rnd.nextFloat(), a);
                if (view.newNote) for (int i = 0; i < 8; ++i) spawn ({ mascot.x + (rnd.nextFloat() - 0.5f) * mascotR, mascot.y + mascotR * 0.6f }, { (rnd.nextFloat() - 0.5f) * 140.f, -80.f - 80.f * rnd.nextFloat() }, 1.6f, 4.f, a);
                break;
            case Theme::waves:         // sound waves rings leaving the mascot's mouth, louder notes send more
                if (mascotR > 0.f && (view.newNote || (view.sounding && emit && rnd.nextFloat() < 0.35f + 0.5f * lv)))
                    spawn ({ mascot.x + mascotR * 0.35f, mascot.y + mascotR * 0.05f }, { 0.f, 0.f }, 1.6f, mascotR * 0.25f, view.newNote ? a : b);
                break;
            case Theme::snow:          // snowflakes drifting down, a flurry from the mascot on every note
                if (emit || rnd.nextFloat() < 0.5f) spawn ({ rx(), ar.getY() - 6.f }, { (rnd.nextFloat() - 0.5f) * 16.f, 22.f + 34.f * rnd.nextFloat() }, 14.f, 1.2f + 2.6f * rnd.nextFloat(), juce::Colour (0xfff2f8ff));
                if (view.newNote && mascotR > 0.f) for (int i = 0; i < 10; ++i) spawn ({ mascot.x + (rnd.nextFloat() - 0.5f) * mascotR * 1.6f, mascot.y - mascotR * 0.8f }, { (rnd.nextFloat() - 0.5f) * 120.f, -40.f + 100.f * rnd.nextFloat() }, 3.f, 1.5f + 2.5f * rnd.nextFloat(), juce::Colours::white);
                break;
            case Theme::bubbles:       // bubbles rising
                if (emit) spawn ({ rx(), ar.getBottom() + 6.f }, { 0.f, -30.f - 40.f * rnd.nextFloat() }, 9.f, 2.f + 5.f * rnd.nextFloat(), b);
                if (view.newNote) for (int i = 0; i < 6; ++i) spawn ({ mascot.x + (rnd.nextFloat() - 0.5f) * mascotR, mascot.y + mascotR }, { 0.f, -60.f - 60.f * rnd.nextFloat() }, 3.f, 3.f + 4.f * rnd.nextFloat(), a);
                break;
            case Theme::smoke:         // smoke puffs rising from the mascot, embers
                if (emit && mascotR > 0.f) spawn ({ mascot.x + (rnd.nextFloat() - 0.5f) * mascotR * 1.4f, mascot.y + mascotR * 0.9f }, { (rnd.nextFloat() - 0.5f) * 14.f, -22.f - 22.f * rnd.nextFloat() }, 4.5f, 18.f + 16.f * rnd.nextFloat(), b);
                if (view.newNote) sparks.emit ({ mascot.x - mascotR, mascot.y + mascotR * 0.7f, mascotR * 2.f, 10.f }, 10, { 0.f, -80.f }, 90.f, 1.4f, 2.2f, a);
                break;
            case Theme::rain:          // rain streaks falling in the light of the street lamp
                for (int i = 0; i < (view.sounding ? 3 : 1); ++i)
                    if (rnd.nextFloat() < 0.6f) spawn ({ rx(), ar.getY() }, { -40.f, 520.f + 200.f * rnd.nextFloat() }, 1.3f, 1.f, b);
                break;
            case Theme::ghostFlame:    // pale blue flames licking up from the bottom
                if (emit) spawn ({ rx(), ar.getBottom() }, { (rnd.nextFloat() - 0.5f) * 10.f, -50.f - 50.f * lv - 30.f * rnd.nextFloat() }, 1.4f, 10.f + 10.f * rnd.nextFloat(), a);
                break;
            case Theme::orientalGold:  // golden glints appearing and fading here and there
                if (emit && rnd.nextFloat() < 0.5f + 0.5f * lv) spawn ({ rx(), ry() }, { 0.f, 0.f }, 1.2f, 3.f + 3.f * rnd.nextFloat(), a);
                break;
            case Theme::police:
            case Theme::spotlights:
            case Theme::scanner:
            case Theme::sun:
            case Theme::sunset:
            case Theme::film:
            case Theme::glitch:
            case Theme::bassShake:
                beam += dt;
                if (theme == Theme::bassShake) shake = juce::jmax (shake * std::exp (-dt / 0.15f), view.newNote ? 1.f : 0.f);
                if (theme == Theme::glitch && view.newNote) shake = 1.f;
                if (theme == Theme::glitch) shake *= std::exp (-dt / 0.12f);
                break;
        }
        for (auto& o : blobs) { o.p += o.v * dt; o.life -= dt; if (theme == Theme::goldRain || theme == Theme::bubbles || theme == Theme::snow) o.p.x += std::sin (themeT * 2.f + o.size) * 10.f * dt; if (theme == Theme::waves) o.size += 70.f * dt; }
        blobs.erase (std::remove_if (blobs.begin(), blobs.end(), [&ar] (const Blob& o) { return o.life <= 0.f || ! ar.expanded (60.f).contains (o.p); }), blobs.end());
    }

    void arc (float strength)
    {
        const float an = rnd.nextFloat() * juce::MathConstants<float>::twoPi, spread = 0.25f + rnd.nextFloat() * 0.5f;
        const auto p0 = mascot + juce::Point<float> (std::cos (an), std::sin (an)) * (mascotR + 2.f);
        const auto p1 = mascot + juce::Point<float> (std::cos (an + spread), std::sin (an + spread)) * (mascotR + 24.f + 30.f * strength);
        const juce::Colour cols[3] { a, b, juce::Colour (0xffffd34a) };
        bolts.add (lightning (p0, p1, rnd, 0.35f, 14.f), cols[rnd.nextInt (3)], 0.12f + 0.12f * strength, 1.2f + strength);
    }

    // light that sits behind the knobs and title (big, soft)
    void themeBehind (juce::Graphics& g)
    {
        const float lv = view.level, hit = view.hit;
        const auto ar = area();
        switch (theme)
        {
            case Theme::spotlights:   // two stadium spotlights sweeping from the top corners
            case Theme::police:       // red and blue lights turning at the top corners
                for (int i = 0; i < 2; ++i)
                {
                    const juce::Point<float> src (i == 0 ? ar.getX() + 20.f : ar.getRight() - 20.f, ar.getY() + 10.f);
                    const float swing = theme == Theme::police ? beam * 4.5f + (float) i * juce::MathConstants<float>::pi : std::sin (beam * 0.6f + (float) i * 2.1f) * 0.6f;
                    const float ang = (i == 0 ? 0.6f : juce::MathConstants<float>::pi - 0.6f) + swing;
                    const juce::Colour c = theme == Theme::police ? (i == 0 ? juce::Colour (0xffff2a2a) : juce::Colour (0xff2a6bff)) : (i == 0 ? a : b);
                    const float len = theme == Theme::police ? 160.f : 520.f;
                    const float k = theme == Theme::police ? (0.25f + 0.5f * lv) * (0.5f + 0.5f * std::sin (beam * 9.f + (float) i * 3.1f)) : 0.05f + 0.12f * lv + 0.10f * hit;
                    juce::Path cone; cone.startNewSubPath (src);
                    cone.lineTo (src + juce::Point<float> (std::cos (ang - 0.16f), std::sin (ang - 0.16f)) * len);
                    cone.lineTo (src + juce::Point<float> (std::cos (ang + 0.16f), std::sin (ang + 0.16f)) * len);
                    cone.closeSubPath();
                    if (! g.clipRegionIntersects (cone.getBounds().getSmallestIntegerContainer())) continue;
                    juce::ColourGradient gr (c.withAlpha (juce::jmin (1.f, k)), src.x, src.y, c.withAlpha (0.f), src.x + std::cos (ang) * len, src.y + std::sin (ang) * len, false);
                    g.setGradientFill (gr); g.fillPath (cone);
                    glowSpot (g, src, 26.f, c, 0.9f * k + 0.2f);
                }
                break;
            case Theme::sun:          // slow sun rays turning behind the mascot
            case Theme::sunset:
                if (mascotR > 0.f && g.clipRegionIntersects (juce::Rectangle<float> (mascotR * 4.4f, mascotR * 4.4f).withCentre (mascot).getSmallestIntegerContainer()))
                {
                    const int n = theme == Theme::sun ? 16 : 10;
                    for (int i = 0; i < n; ++i)
                    {
                        const float an = beam * 0.15f + (float) i * juce::MathConstants<float>::twoPi / (float) n;
                        juce::Path ray; ray.startNewSubPath (mascot + juce::Point<float> (std::cos (an - 0.05f), std::sin (an - 0.05f)) * mascotR);
                        ray.lineTo (mascot + juce::Point<float> (std::cos (an), std::sin (an)) * mascotR * (2.0f + 0.2f * std::sin (beam + (float) i)));
                        ray.lineTo (mascot + juce::Point<float> (std::cos (an + 0.05f), std::sin (an + 0.05f)) * mascotR);
                        ray.closeSubPath();
                        g.setColour ((i % 2 ? a : b).withAlpha (0.10f + 0.25f * lv + 0.15f * hit)); g.fillPath (ray);
                    }
                }
                break;
            default: break;
        }
    }

    // things in front: particles of the theme, scanner bar, film flicker, glitch slices, bass rings
    void themeFront (juce::Graphics& g)
    {
        const float lv = view.level, hit = view.hit;
        const auto ar = area();
        for (auto& o : blobs)
        {
            const float k = juce::jlimit (0.f, 1.f, o.life / o.max);
            const juce::Rectangle<float> bb (o.p.x - o.size * 1.6f, o.p.y - o.size * 1.6f, o.size * 3.2f, o.size * 3.2f);
            if (! g.clipRegionIntersects (bb.expanded (2.f).getSmallestIntegerContainer())) continue;
            switch (theme)
            {
                case Theme::goldRain:   // a coin seen turning: an ellipse whose width breathes
                {
                    const float w = o.size * (0.25f + 0.75f * std::abs (std::sin (themeT * 4.f + o.size * 3.f)));
                    glowSpot (g, o.p, o.size * 2.4f, o.c, 0.3f * k);
                    g.setColour (o.c.withAlpha (0.9f * k)); g.fillEllipse (o.p.x - w, o.p.y - o.size, w * 2.f, o.size * 2.f);
                    g.setColour (juce::Colours::white.withAlpha (0.6f * k)); g.fillEllipse (o.p.x - w * 0.4f, o.p.y - o.size * 0.7f, w * 0.5f, o.size * 0.5f);
                    break;
                }
                case Theme::waves:      // an expanding ring
                    g.setColour (o.c.withAlpha (0.55f * k)); g.drawEllipse (o.p.x - o.size, o.p.y - o.size, o.size * 2.f, o.size * 2.f, 2.f);
                    g.setColour (o.c.withAlpha (0.18f * k)); g.drawEllipse (o.p.x - o.size, o.p.y - o.size, o.size * 2.f, o.size * 2.f, 6.f);
                    break;
                case Theme::snow:       // a soft flake: halo and a bright core
                    glowSpot (g, o.p, o.size * 2.6f, o.c, 0.35f * k);
                    g.setColour (o.c.withAlpha (0.9f * k)); g.fillEllipse (o.p.x - o.size * 0.6f, o.p.y - o.size * 0.6f, o.size * 1.2f, o.size * 1.2f);
                    break;
                case Theme::bubbles:
                    g.setColour (o.c.withAlpha (0.55f * k)); g.drawEllipse (o.p.x - o.size, o.p.y - o.size, o.size * 2.f, o.size * 2.f, 1.f);
                    g.setColour (juce::Colours::white.withAlpha (0.6f * k)); g.fillEllipse (o.p.x - o.size * 0.5f, o.p.y - o.size * 0.6f, o.size * 0.5f, o.size * 0.4f);
                    break;
                case Theme::smoke:
                case Theme::ghostFlame:
                {
                    const float grow = theme == Theme::smoke ? 1.f + (1.f - k) * 1.2f : k;
                    glowSpot (g, o.p, o.size * grow, o.c, (theme == Theme::smoke ? 0.16f : 0.35f) * k * (0.6f + 0.6f * lv));
                    break;
                }
                case Theme::rain:
                    g.setColour (o.c.withAlpha (0.35f * k));
                    g.drawLine (o.p.x, o.p.y, o.p.x + o.v.x * 0.02f, o.p.y + o.v.y * 0.02f, 1.f);
                    break;
                case Theme::orientalGold:
                {
                    const float s = o.size * std::sin (juce::MathConstants<float>::pi * k);
                    glowSpot (g, o.p, s * 3.f, o.c, 0.5f);
                    juce::Path star;
                    star.addStar (o.p, 4, s * 0.25f, s * 1.4f, themeT);
                    g.setColour (juce::Colours::white.withAlpha (0.8f * std::sin (juce::MathConstants<float>::pi * k))); g.fillPath (star);
                    break;
                }
                default: break;
            }
        }
        switch (theme)
        {
            case Theme::scanner:      // a red light bar sweeping left and right across the top of the panel
            {
                const float w = ar.getWidth() * 0.6f, x = ar.getX() + ar.getWidth() * 0.2f + (0.5f + 0.5f * std::sin (beam * 2.2f)) * w;
                const float y = ar.getY() + 12.f;
                if (g.clipRegionIntersects (juce::Rectangle<int> ((int) ar.getX(), (int) y - 20, (int) ar.getWidth(), 40)))
                    for (int i = 0; i < 6; ++i)
                        glowSpot (g, { x - std::cos (beam * 2.2f) * (float) i * 14.f, y }, 22.f - (float) i * 2.f, a, (0.9f - (float) i * 0.14f) * (0.6f + 0.6f * lv));
                break;
            }
            case Theme::film:         // the light of a projector flickering over the panel and a light leak on every note
                if (g.clipRegionIntersects (ar.getSmallestIntegerContainer()))
                {
                    g.setColour (juce::Colours::white.withAlpha (0.015f + 0.02f * std::abs (std::sin (beam * 23.f)) * (0.3f + lv)));
                    g.fillRect (ar.withHeight (ar.getHeight() * 0.08f).withY (ar.getY() + std::fmod (beam * 140.f, ar.getHeight())));
                    if (hit > 0.05f) glowSpot (g, { ar.getRight() - 60.f, ar.getY() + 60.f }, 220.f, a, 0.25f * hit);
                }
                break;
            case Theme::glitch:       // glitch: thin bright slices shifted sideways on every note
                if (shake > 0.05f)
                    for (int i = 0; i < 5; ++i)
                    {
                        juce::Random r2 ((juce::int64) (beam * 30.f) + i);
                        const float y = ar.getY() + r2.nextFloat() * ar.getHeight(), h = 2.f + r2.nextFloat() * 6.f;
                        g.setColour ((i % 2 ? a : b).withAlpha (0.35f * shake));
                        g.fillRect (ar.getX() + (r2.nextFloat() - 0.5f) * 30.f * shake, y, ar.getWidth(), h);
                    }
                break;
            case Theme::bassShake:    // the bass hits push rings of light out of the mascot
                if (mascotR > 0.f && shake > 0.02f)
                    for (int i = 0; i < 3; ++i)
                    {
                        const float rr = mascotR + (1.f - shake) * 220.f + (float) i * 18.f;
                        juce::Path ring; ring.addEllipse (mascot.x - rr, mascot.y - rr, rr * 2.f, rr * 2.f);
                        neonStroke (g, ring, i % 2 ? a : b, 2.f, 0.8f * shake);
                    }
                break;
            default: break;
        }
        juce::ignoreUnused (lv);
    }

    // A living knob: a small animation of the theme inside the cap, as strong as the knob's value, moving while the
    // sound plays (the knob is repainted when its light changes). Kept faint so the lit index stays readable.
    void knobCore (juce::Graphics& g, juce::Point<float> c, float r, float pos, float glow, int seed)
    {
        const float R = r * 0.56f;
        const float e = juce::jlimit (0.f, 1.f, pos * (0.55f + 0.6f * glow));
        if (R < 5.f || e < 0.03f) return;
        juce::Graphics::ScopedSaveState ss (g);
        juce::Path disk; disk.addEllipse (c.x - R, c.y - R, R * 2.f, R * 2.f);
        g.reduceClipRegion (disk);
        const float t = themeT + (float) seed * 0.37f;
        auto hash = [seed] (int i, int salt) { juce::Random h ((juce::int64) (seed * 977 + i * 131 + salt)); return h.nextFloat(); };
        const int n = 2 + (int) (e * 5.f);
        switch (theme)
        {
            case Theme::goldRain: case Theme::orientalGold: case Theme::sun:      // gold sparkles twinkling
                for (int i = 0; i < n; ++i)
                {
                    const float ang = hash (i, 1) * juce::MathConstants<float>::twoPi + t * 0.4f, d = std::sqrt (hash (i, 2)) * R * 0.85f;
                    const auto p = c + juce::Point<float> (std::cos (ang), std::sin (ang)) * d;
                    const float tw = 0.5f + 0.5f * std::sin (t * (2.5f + 2.f * hash (i, 3)) + hash (i, 4) * 6.f);
                    glowSpot (g, p, 3.f + 4.f * tw, a, 0.8f * e * tw);
                    g.setColour (juce::Colours::white.withAlpha (0.9f * e * tw)); g.fillEllipse (p.x - 1.f, p.y - 1.f, 2.f, 2.f);
                }
                break;
            case Theme::bubbles:                                                   // bubbles rising through the cap
                for (int i = 0; i < n; ++i)
                {
                    const float ph = std::fmod (t * (0.25f + 0.3f * hash (i, 1)) + hash (i, 2), 1.f);
                    const auto p = c + juce::Point<float> ((hash (i, 3) - 0.5f) * R * 1.2f + std::sin (t * 2.f + (float) i) * 2.f, R - ph * R * 2.f);
                    const float s = 1.2f + 2.2f * hash (i, 4);
                    g.setColour (b.withAlpha (0.85f * e * (1.f - ph))); g.drawEllipse (p.x - s, p.y - s, s * 2.f, s * 2.f, 1.f);
                }
                break;
            case Theme::smoke: case Theme::ghostFlame: case Theme::sunset:       // a small flame licking up from the bottom of the cap
                for (int i = 0; i < 5; ++i)
                {
                    const float f = (float) i / 4.f, fl = 0.75f + 0.25f * std::sin (t * (7.f + (float) i) + (float) i * 1.7f);
                    const auto p = c + juce::Point<float> (std::sin (t * 3.f + (float) i) * R * 0.12f * f, R * 0.7f - f * R * 1.2f * e);
                    glowSpot (g, p, R * (0.55f - 0.35f * f) * fl, (theme == Theme::ghostFlame ? b : a).interpolatedWith (juce::Colours::white, f * 0.5f), 0.75f * e * (1.f - 0.4f * f));
                }
                break;
            case Theme::snow:                                                      // flakes falling slowly across the cap
                for (int i = 0; i < n; ++i)
                {
                    const float ph = std::fmod (t * (0.18f + 0.15f * hash (i, 1)) + hash (i, 2), 1.f);
                    const auto p = c + juce::Point<float> ((hash (i, 3) - 0.5f) * R * 1.4f + std::sin (t * 1.5f + (float) i) * 2.f, -R + ph * R * 2.f);
                    const float sz = 0.8f + 1.4f * hash (i, 4);
                    glowSpot (g, p, sz * 3.f, juce::Colours::white, 0.5f * e);
                    g.setColour (juce::Colours::white.withAlpha (0.9f * e)); g.fillEllipse (p.x - sz * 0.5f, p.y - sz * 0.5f, sz, sz);
                }
                break;
            case Theme::rain:                                                      // drops falling across the cap
                for (int i = 0; i < n; ++i)
                {
                    const float ph = std::fmod (t * (0.9f + 0.6f * hash (i, 1)) + hash (i, 2), 1.f);
                    const float x = c.x + (hash (i, 3) - 0.5f) * R * 1.6f, y = c.y - R + ph * R * 2.f;
                    g.setColour (b.withAlpha (0.8f * e)); g.drawLine (x, y, x - 1.f, y + 5.f, 1.1f);
                }
                break;
            case Theme::electric: case Theme::glitch: case Theme::scanner: case Theme::police: case Theme::bassShake: case Theme::spotlights:
            {                                                                      // a plasma arc spinning inside the cap
                juce::Path arcP; const float rr = R * (0.45f + 0.35f * e), st = t * (1.5f + 3.f * e);
                arcP.addCentredArc (c.x, c.y, rr, rr, 0.f, st, st + 1.2f + 2.f * e, true);
                neonStroke (g, arcP, a.interpolatedWith (b, 0.5f + 0.5f * std::sin (t)), 1.5f, 0.9f * e);
                break;
            }
            default:                                                               // film and the rest: a soft light breathing
                glowSpot (g, c, R * (0.6f + 0.3f * std::sin (t * 2.f)), a, 0.6f * e);
                break;
        }
    }

    //------------------------------------------------------------------ repaint only what changes
    void invalidate()
    {
        if (view.time < 1.5f) { repaint(); prevMoving.clear(); return; }   // neon ignition: everything flickers
        juce::RectangleList<int> dirty, moving;
        auto add = [] (juce::RectangleList<int>& l, juce::Rectangle<float> r) { l.add (r.getSmallestIntegerContainer()); };
        auto square = [] (juce::Point<float> c, float half) { return juce::Rectangle<float> (half * 2.f, half * 2.f).withCentre (c); };
        if (titleFromFace) add (dirty, titleArea);
        else if (hasTitle) add (dirty, title.getBounds().expanded (44.f));
        if (mascotR > 0.f)
        {
            add (dirty, square (mascot, mascotR + 48.f));
            if (view.sinceHit < 0.7f) add (moving, square (mascot, mascotR + view.sinceHit * 170.f + 14.f));
        }
        const float m = view.levelL + view.levelR + view.holdL + view.holdR;
        if (std::abs (m - shownMeter) > 0.002f)
        {
            if (! meter.isEmpty()) add (dirty, meter.expanded (4.f));
            for (auto& hs : holes)
                if (! hs.empty()) { juce::Rectangle<float> box (hs[0], hs[0]); for (auto p : hs) box = box.getUnion ({ p, p }); add (dirty, box.expanded (9.f)); }
            shownMeter = m;
        }
        for (auto& k : knobs)
            if (std::abs (k.glow - k.shownGlow) > 0.004f || k.orbit >= 0.f || k.mod >= 0.f || k.shownMod)
            { add (dirty, k.s->getBounds().toFloat().expanded (18.f)); k.shownGlow = k.glow; k.shownMod = k.mod >= 0.f; }
        for (auto& f : faders)
            if (std::abs (f.led - f.shownLed) > 0.004f) { add (dirty, f.s->getBounds().toFloat().expanded (24.f, 10.f)); f.shownLed = f.led; }
        if (std::any_of (keys.heat.begin(), keys.heat.end(), [] (float h) { return h > 0.f; }) || keysLit) keys.repaint();
        keysLit = std::any_of (keys.heat.begin(), keys.heat.end(), [] (float h) { return h > 0.f; });
        const float line = view.level + 0.3f * view.hit;
        if (std::abs (line - shownLine) > 0.004f) { add (dirty, keys.getBounds().toFloat().withHeight (1.f).expanded (2.f, 22.f)); shownLine = line; }
        for (auto& p : sparks.ps) add (moving, square (p.pos, p.size * 3.f + 2.f));
        for (auto& bo : bolts.bolts) add (moving, bo.path.getBounds().expanded (16.f));
        for (auto& o : blobs)
            add (moving, theme == Theme::rain ? juce::Rectangle<float> (o.p, o.p + o.v * 0.02f).expanded (3.f) : square (o.p, o.size * 3.3f + 2.f));
        // whole-area effects
        const auto ar = area();
        switch (theme)
        {
            case Theme::spotlights: case Theme::police:
                add (moving, juce::Rectangle<float> (ar.getX(), ar.getY(), 560.f, 560.f)); add (moving, juce::Rectangle<float> (ar.getRight() - 560.f, ar.getY(), 560.f, 560.f)); break;
            case Theme::sun: case Theme::sunset: if (mascotR > 0.f) add (moving, square (mascot, mascotR * 2.2f)); break;
            case Theme::scanner: add (moving, juce::Rectangle<float> (ar.getX(), ar.getY() - 10.f, ar.getWidth(), 50.f)); break;
            case Theme::film: add (moving, ar); break;
            case Theme::glitch: if (shake > 0.05f) add (moving, ar); break;
            case Theme::bassShake: if (shake > 0.02f) add (moving, square (mascot, mascotR + 280.f)); break;
            default: break;
        }
        dirty.add (moving); dirty.add (prevMoving);
        prevMoving = moving;
        dirty.consolidate();
        for (auto& r : dirty) repaint (r);
    }
    bool keysLit = false;
};
}
