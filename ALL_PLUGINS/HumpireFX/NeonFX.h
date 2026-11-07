#pragma once
// HUMPIRE FX - neon light and live animation shared by the DJBILBOX BEATS synths (header only, drawn in code).
// The processor fills a LiveTap from the audio thread (lock-free, once per block); the editor reads it in a timer
// through a LiveView and draws with the helpers below: neon strokes, halos, a neon title, sparks, lightning,
// LED meters and a keyboard whose keys glow and leave a trail.
#include <juce_audio_utils/juce_audio_utils.h>
#include <array>
#include <atomic>
#include <cmath>

namespace hfx
{
//============================================================================== audio thread -> interface
struct LiveTap
{
    // published once per block; relaxed atomics: a value one frame late is harmless
    std::atomic<float> peakL { 0.f }, peakR { 0.f };          // output peak of the block
    std::atomic<float> amp { 0.f }, filt { 0.f };             // loudest voice: amp and filter envelope levels
    std::atomic<int> ampStage { -1 }, filtStage { -1 };       // 0 attack, 1 decay, 2 sustain, 3 release, -1 silent
    std::atomic<float> lfo { 0.f }, lfoPhase { 0.f };         // loudest voice's LFO: value -1..1 and phase 0..1
    std::atomic<float> cutoff { 0.f };                        // loudest voice's live cutoff, 0..1 on a log scale
    std::atomic<int> voices { 0 };
    std::atomic<unsigned> noteOns { 0 };                      // grows with every note-on: the interface flashes on change

    // audio thread only: the voices fold into this while rendering, publish() hands it to the interface
    struct Block { float amp = 0, filt = 0, lfo = 0, lfoPhase = 0, cutoff = 0; int ampStage = -1, filtStage = -1, voices = 0; } block;

    void beginBlock (const juce::MidiBuffer& midi)
    {
        block = {};
        unsigned n = 0;
        for (const auto m : midi)
            if (m.getMessage().isNoteOn()) ++n;
        if (n != 0) noteOns.fetch_add (n, std::memory_order_relaxed);
    }

    // every sounding voice, at the end of its render
    void voice (float ampLevel, int stage, float filtLevel, int filtStageNow, float lfoValue, float phase, float cutoffHz)
    {
        ++block.voices;
        if (ampLevel < block.amp) return;
        block.amp = ampLevel; block.ampStage = stage; block.filt = filtLevel; block.filtStage = filtStageNow;
        block.lfo = lfoValue; block.lfoPhase = phase;
        block.cutoff = juce::jlimit (0.f, 1.f, std::log2 (juce::jmax (20.f, cutoffHz) / 20.f) / std::log2 (1000.f));
    }

    void publish (const float* L, const float* R, int n)
    {
        float pl = 0.f, pr = 0.f;
        for (int i = 0; i < n; ++i) { pl = juce::jmax (pl, std::abs (L[i])); pr = juce::jmax (pr, std::abs (R[i])); }
        constexpr auto o = std::memory_order_relaxed;
        peakL.store (pl, o); peakR.store (pr, o);
        amp.store (block.amp, o); filt.store (block.filt, o); ampStage.store (block.ampStage, o); filtStage.store (block.filtStage, o);
        lfo.store (block.lfo, o); lfoPhase.store (block.lfoPhase, o); cutoff.store (block.cutoff, o); voices.store (block.voices, o);
    }
};

// Envelope stage from the time since note-on, for synths whose ADSR does not expose its state.
inline int envStage (bool released, float secondsSinceOn, float attack, float decay)
{
    if (released) return 3;
    return secondsSinceOn < attack ? 0 : secondsSinceOn < attack + decay ? 1 : 2;
}

//============================================================================== interface side
struct LiveView
{
    float time = 0.f;                         // seconds since the editor opened
    float levelL = 0.f, levelR = 0.f, level = 0.f, holdL = 0.f, holdR = 0.f;   // 0..1 over a 48 dB scale
    float amp = 0.f, filt = 0.f, lfo = 0.f, lfoPhase = 0.f, cutoff = 0.f;
    int ampStage = -1, filtStage = -1;
    bool sounding = false, newNote = false;
    float hit = 0.f, sinceHit = 99.f;         // hit: 1 on every new note, then fades

    void update (const LiveTap& t, float dt)
    {
        constexpr auto o = std::memory_order_relaxed;
        time += dt;
        auto toLevel = [] (float pk) { return juce::jlimit (0.f, 1.f, (juce::Decibels::gainToDecibels (pk, -60.f) + 48.f) / 48.f); };
        auto follow = [dt] (float& v, float target, float up, float down) { v += (target - v) * (1.f - std::exp (-dt / (target > v ? up : down))); };
        follow (levelL, toLevel (t.peakL.load (o)), 0.012f, 0.22f);
        follow (levelR, toLevel (t.peakR.load (o)), 0.012f, 0.22f);
        level = juce::jmax (levelL, levelR);
        holdL = levelL > holdL ? levelL : juce::jmax (0.f, holdL - dt * 0.6f);
        holdR = levelR > holdR ? levelR : juce::jmax (0.f, holdR - dt * 0.6f);
        follow (amp, t.amp.load (o), 0.01f, 0.08f);
        follow (filt, t.filt.load (o), 0.01f, 0.08f);
        follow (cutoff, t.cutoff.load (o), 0.02f, 0.06f);
        lfo = t.lfo.load (o); lfoPhase = t.lfoPhase.load (o);
        ampStage = t.ampStage.load (o); filtStage = t.filtStage.load (o);
        sounding = t.voices.load (o) > 0 || level > 0.05f;
        const unsigned n = t.noteOns.load (o);
        newNote = n != lastNoteOns; lastNoteOns = n;
        if (newNote) { hit = 1.f; sinceHit = 0.f; }
        else { hit *= std::exp (-dt / 0.28f); sinceHit += dt; }
    }

    // a value breathing slowly between lo and hi (seconds per cycle)
    float breathe (float period, float lo, float hi) const
    {
        return lo + (hi - lo) * (0.5f + 0.5f * std::sin (juce::MathConstants<float>::twoPi * time / period));
    }
    // neon ignition: flickers during the first second, then stays on
    float ignition() const
    {
        if (time > 1.3f) return 1.f;
        static constexpr float steps[] = { 0, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0.3f, 1, 1 };
        return steps[juce::jlimit (0, 12, (int) (time * 10.f))];
    }

private:
    unsigned lastNoteOns = 0;
};

//============================================================================== drawing
// Glowing tube along a path: three soft halos, the coloured tube, a white-hot core.
inline void neonStroke (juce::Graphics& g, const juce::Path& p, juce::Colour c, float width, float intensity, bool core = true)
{
    if (intensity <= 0.002f) return;
    const float k = juce::jlimit (0.f, 1.6f, intensity);
    const juce::PathStrokeType::JointStyle j = juce::PathStrokeType::curved;
    const juce::PathStrokeType::EndCapStyle e = juce::PathStrokeType::rounded;
    const float halo[3][2] = { { 7.f, 0.035f }, { 4.f, 0.07f }, { 2.2f, 0.14f } };
    for (auto& h : halo)
    {
        g.setColour (c.withAlpha (juce::jmin (1.f, h[1] * k)));
        g.strokePath (p, juce::PathStrokeType (width * h[0], j, e));
    }
    if (! core) return;
    g.setColour (c.withAlpha (juce::jmin (1.f, 0.35f + 0.6f * k)));
    g.strokePath (p, juce::PathStrokeType (width, j, e));
    g.setColour (c.interpolatedWith (juce::Colours::white, 0.65f).withAlpha (juce::jmin (1.f, 0.85f * k)));
    g.strokePath (p, juce::PathStrokeType (width * 0.4f, j, e));
}

// Soft light ring just outside a circle of radius r (centre c): bright at r, fading out over `spread`.
inline void halo (juce::Graphics& g, juce::Point<float> c, float r, float spread, juce::Colour col, float intensity)
{
    if (intensity <= 0.002f) return;
    const float outer = r + spread, inner = r / outer;
    juce::ColourGradient gr (col.withAlpha (0.f), c.x, c.y, col.withAlpha (0.f), c.x + outer, c.y, true);
    gr.addColour (juce::jmax (0.0, (double) inner - 0.06), col.withAlpha (0.f));
    gr.addColour (inner, col.withAlpha (juce::jmin (1.f, 0.6f * intensity)));
    gr.addColour (inner + (1.0 - inner) * 0.35, col.withAlpha (juce::jmin (1.f, 0.18f * intensity)));
    g.setGradientFill (gr);
    g.fillEllipse (c.x - outer, c.y - outer, outer * 2.f, outer * 2.f);
}

// The same light ring composed once into an image (at the screen's pixel size) and only faded each frame.
class HaloImage
{
public:
    void draw (juce::Graphics& g, juce::Point<float> c, float r, float spread, juce::Colour col, float intensity)
    {
        if (intensity <= 0.002f) return;
        const float sc = g.getInternalContext().getPhysicalPixelScaleFactor();
        const float outer = r + spread;
        if (img.isNull() || std::abs (sc - scale) > 0.01f || std::abs (outer - size) > 0.5f || col != colour)
        {
            scale = sc; size = outer; colour = col;
            img = juce::Image (juce::Image::ARGB, (int) std::ceil (outer * 2.f * sc), (int) std::ceil (outer * 2.f * sc), true);
            juce::Graphics gi (img);
            gi.addTransform (juce::AffineTransform::scale (sc));
            halo (gi, { outer, outer }, r, spread, col, 1.f);
        }
        juce::Graphics::ScopedSaveState ss (g);
        g.setImageResamplingQuality (juce::Graphics::lowResamplingQuality);
        g.setOpacity (juce::jmin (1.f, intensity));
        g.drawImage (img, { c.x - outer, c.y - outer, outer * 2.f, outer * 2.f });
        if (intensity > 1.f) { g.setOpacity (juce::jmin (1.f, intensity - 1.f)); g.drawImage (img, { c.x - outer, c.y - outer, outer * 2.f, outer * 2.f }); }
    }
private:
    juce::Image img;
    float scale = 0.f, size = 0.f;
    juce::Colour colour;
};

// Round blob of light.
inline void glowSpot (juce::Graphics& g, juce::Point<float> c, float r, juce::Colour col, float intensity)
{
    if (intensity <= 0.002f) return;
    juce::ColourGradient gr (col.withAlpha (juce::jmin (1.f, intensity)), c.x, c.y, col.withAlpha (0.f), c.x + r, c.y, true);
    gr.addColour (0.3, col.withAlpha (juce::jmin (1.f, 0.45f * intensity)));
    g.setGradientFill (gr);
    g.fillEllipse (c.x - r, c.y - r, r * 2.f, r * 2.f);
}

// Segmented LED bar (horizontal), green -> amber -> red, with a peak-hold segment.
inline void ledBar (juce::Graphics& g, juce::Rectangle<float> r, float level, float hold, int segs = 26)
{
    const float gap = 1.5f, w = (r.getWidth() - gap * (float) (segs - 1)) / (float) segs;
    const int holdSeg = juce::jlimit (0, segs - 1, (int) (hold * (float) segs));
    for (int i = 0; i < segs; ++i)
    {
        const float t = (float) i / (float) (segs - 1);
        const juce::Colour c = t < 0.65f ? juce::Colour (0xff35ff6a) : t < 0.85f ? juce::Colour (0xffffc23a) : juce::Colour (0xffff3b3b);
        const bool on = (float) i < level * (float) segs || (i == holdSeg && hold > 0.04f);
        auto s = juce::Rectangle<float> (r.getX() + (float) i * (w + gap), r.getY(), w, r.getHeight());
        g.setColour (on ? c : c.withAlpha (0.10f));
        g.fillRoundedRectangle (s, 1.f);
        if (on) { g.setColour (c.withAlpha (0.25f)); g.fillRoundedRectangle (s.expanded (1.5f), 2.f); }
    }
}

// Jagged electric arc from a to b (midpoint displacement), bulging sideways by `bulge`.
inline juce::Path lightning (juce::Point<float> a, juce::Point<float> b, juce::Random& rnd, float jag, float bulge = 0.f)
{
    std::vector<juce::Point<float>> pts { a, b };
    const juce::Point<float> nrm ((a.y - b.y), (b.x - a.x));
    const float len = a.getDistanceFrom (b) + 1e-3f;
    for (int depth = 0; depth < 5; ++depth)
    {
        std::vector<juce::Point<float>> next;
        for (size_t i = 0; i + 1 < pts.size(); ++i)
        {
            const float d = jag * len * std::pow (0.55f, (float) depth) / len;
            auto m = (pts[i] + pts[i + 1]) * 0.5f + nrm * ((rnd.nextFloat() - 0.5f) * d);
            next.push_back (pts[i]); next.push_back (m);
        }
        next.push_back (pts.back());
        pts.swap (next);
    }
    juce::Path p;
    p.startNewSubPath (pts.front());
    for (size_t i = 1; i < pts.size(); ++i)
    {
        const float t = (float) i / (float) (pts.size() - 1);
        p.lineTo (pts[i] + nrm * (bulge * std::sin (juce::MathConstants<float>::pi * t) / len));
    }
    return p;
}

// Short-lived electric arcs.
struct Bolts
{
    struct Bolt { juce::Path path; float life, max; juce::Colour colour; float width; };
    std::vector<Bolt> bolts;
    void add (juce::Path p, juce::Colour c, float life, float width = 1.4f) { if (bolts.size() < 24) bolts.push_back ({ std::move (p), life, life, c, width }); }
    void update (float dt)
    {
        for (auto& b : bolts) b.life -= dt;
        bolts.erase (std::remove_if (bolts.begin(), bolts.end(), [] (const Bolt& b) { return b.life <= 0.f; }), bolts.end());
    }
    void draw (juce::Graphics& g) const
    {
        for (auto& b : bolts) neonStroke (g, b.path, b.colour, b.width, 1.2f * b.life / b.max);
    }
};

// Floating sparks: emitted at a point or inside an area, they drift, slow down and fade.
struct Sparks
{
    struct P { juce::Point<float> pos, vel; float life, max, size; juce::Colour colour; };
    std::vector<P> ps;
    juce::Random rnd { 7 };

    void emit (juce::Rectangle<float> area, int n, juce::Point<float> vel, float spread, float life, float size, juce::Colour c)
    {
        for (int i = 0; i < n && ps.size() < 260; ++i)
        {
            const juce::Point<float> p (area.getX() + rnd.nextFloat() * area.getWidth(), area.getY() + rnd.nextFloat() * area.getHeight());
            const juce::Point<float> v (vel.x + (rnd.nextFloat() - 0.5f) * spread, vel.y + (rnd.nextFloat() - 0.5f) * spread);
            const float l = life * (0.6f + 0.8f * rnd.nextFloat());
            ps.push_back ({ p, v, l, l, size * (0.6f + 0.8f * rnd.nextFloat()), c });
        }
    }
    void update (float dt)
    {
        for (auto& p : ps)
        {
            p.pos += p.vel * dt;
            p.vel *= std::exp (-dt * 1.6f);
            p.vel.y -= 14.f * dt;                     // light rises
            p.pos.x += std::sin ((p.life + p.pos.y) * 3.f) * 6.f * dt;
            p.life -= dt;
        }
        ps.erase (std::remove_if (ps.begin(), ps.end(), [] (const P& p) { return p.life <= 0.f; }), ps.end());
    }
    void draw (juce::Graphics& g) const
    {
        for (auto& p : ps)
        {
            const float a = juce::jlimit (0.f, 1.f, p.life / p.max);
            glowSpot (g, p.pos, p.size * 3.f, p.colour, 0.35f * a);
            g.setColour (p.colour.interpolatedWith (juce::Colours::white, 0.6f).withAlpha (a));
            g.fillEllipse (p.pos.x - p.size * 0.5f, p.pos.y - p.size * 0.5f, p.size, p.size);
        }
    }
};

// A title written in neon: blurred glow (built once at low resolution, tinted by the current brush when drawn),
// the lit tube, a white-hot core and a band of light that sweeps across the letters.
class NeonTitle
{
public:
    void set (const juce::String& text, const juce::Font& font, juce::Rectangle<float> area)
    {
        juce::GlyphArrangement ga;
        ga.addFittedText (font, text, area.getX(), area.getY(), area.getWidth(), area.getHeight(), juce::Justification::centred, 1);
        path.clear(); ga.createPath (path);
        bounds = path.getBounds();
        glowTight = glowWide = lit = tubeImg = {};
    }

    juce::Rectangle<float> getBounds() const { return bounds; }

    // glow: brush for the light around the letters (a colour or a gradient); tube: letter colour; intensity 0..1.5;
    // sweep: position of the light band across the title 0..1 (outside = none); withTube = false: only the light around
    // the letters (the letters themselves are already part of a faceplate picture)
    void draw (juce::Graphics& g, const juce::FillType& glow, juce::Colour tube, float intensity, float sweep = -1.f, bool withTube = true)
    {
        if (path.isEmpty()) return;
        if (glowTight.isNull()) build();
        const float k = juce::jlimit (0.f, 1.5f, intensity);
        juce::Graphics::ScopedSaveState ss (g);
        // the tinted glow is composed once at the screen's pixel size; each frame only fades it in or out
        const float sc = g.getInternalContext().getPhysicalPixelScaleFactor();
        if (lit.isNull() || std::abs (sc - litScale) > 0.01f)
        {
            litScale = sc;
            lit = juce::Image (juce::Image::ARGB, juce::jmax (1, (int) std::ceil (wideArea.getWidth() * sc)), juce::jmax (1, (int) std::ceil (wideArea.getHeight() * sc)), true);
            juce::Graphics gl (lit);
            gl.addTransform (juce::AffineTransform::translation (-wideArea.getX(), -wideArea.getY()).scaled (sc));
            gl.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
            gl.setFillType (glow); gl.setOpacity (0.55f);
            gl.drawImage (glowWide, wideArea, juce::RectanglePlacement::stretchToFit, true);
            gl.setFillType (glow); gl.setOpacity (0.9f);
            gl.drawImage (glowTight, tightArea, juce::RectanglePlacement::stretchToFit, true);
            // the tube, also composed once: dark outline (the glass), letters lit from inside, a hot core
            tubeImg = juce::Image (juce::Image::ARGB, lit.getWidth(), lit.getHeight(), true);
            juce::Graphics gt (tubeImg);
            gt.addTransform (juce::AffineTransform::translation (-wideArea.getX(), -wideArea.getY()).scaled (sc));
            gt.setColour (juce::Colours::black.withAlpha (0.55f)); gt.strokePath (path, juce::PathStrokeType (3.f));
            gt.setColour (tube); gt.fillPath (path);
            gt.setColour (juce::Colours::white.withAlpha (0.55f)); gt.strokePath (path, juce::PathStrokeType (0.9f));
        }
        g.setImageResamplingQuality (juce::Graphics::lowResamplingQuality);
        g.setOpacity (juce::jmin (1.f, k));
        g.drawImage (lit, wideArea);
        if (k > 1.f) { g.setOpacity (k - 1.f); g.drawImage (lit, wideArea); }
        if (withTube) { g.setOpacity (juce::jlimit (0.25f, 1.f, 0.35f + 0.65f * k)); g.drawImage (tubeImg, wideArea); }
        g.setOpacity (1.f);
        if (sweep > -0.2f && sweep < 1.2f)
        {
            g.reduceClipRegion (path);
            const float x = bounds.getX() + bounds.getWidth() * sweep;
            juce::ColourGradient band (juce::Colours::white.withAlpha (0.f), x - 70.f, 0.f, juce::Colours::white.withAlpha (0.f), x + 70.f, 0.f, false);
            band.addColour (0.5, juce::Colours::white.withAlpha (0.55f * juce::jmin (1.f, k)));
            g.setGradientFill (band);
            g.fillRect (bounds.expanded (4.f));
        }
    }

private:
    juce::Path path;
    juce::Rectangle<float> bounds, tightArea, wideArea;
    juce::Image glowTight, glowWide, lit, tubeImg;   // lit: both glows tinted by the brush; tubeImg: the letters (screen pixel size)
    float litScale = 0.f;

    // alpha masks of the letters, blurred at 1/4 resolution (cheap) and stretched back when drawn
    juce::Image mask (float pad, float blur, juce::Rectangle<float>& area) const
    {
        constexpr float s = 0.25f;
        area = bounds.expanded (pad);
        juce::Image img (juce::Image::ARGB, juce::jmax (1, (int) std::ceil (area.getWidth() * s)), juce::jmax (1, (int) std::ceil (area.getHeight() * s)), true);
        {
            juce::Graphics gi (img);
            gi.setColour (juce::Colours::white);
            gi.fillPath (path, juce::AffineTransform::translation (-area.getX(), -area.getY()).scaled (s));
            gi.strokePath (path, juce::PathStrokeType (3.f), juce::AffineTransform::translation (-area.getX(), -area.getY()).scaled (s));
        }
        juce::ImageConvolutionKernel k ((int) (blur * 2.f) | 1);
        k.createGaussianBlur (blur);
        k.applyToImage (img, img, img.getBounds());
        k.applyToImage (img, img, img.getBounds());   // twice: a softer, wider falloff
        return img;
    }
    void build()
    {
        glowTight = mask (12.f, 2.2f, tightArea);
        glowWide = mask (36.f, 5.f, wideArea);
    }
};

// Keyboard whose pressed keys light up in neon and fade out slowly (a trail when playing fast).
class NeonKeyboard : public juce::MidiKeyboardComponent
{
public:
    NeonKeyboard (juce::MidiKeyboardState& s, Orientation o) : juce::MidiKeyboardComponent (s, o), keyState (s) {}

    juce::Colour neon { 0xff3fe06e }, neonHot { 0xffff5a4a };
    std::array<float, 128> heat {};
    bool overlayOnly = false;   // the keys are drawn by a photo behind: only their light is drawn here

    void setOverlayOnly()
    {
        overlayOnly = true;
        for (int id : { (int) whiteNoteColourId, (int) blackNoteColourId, (int) keySeparatorLineColourId, (int) mouseOverKeyOverlayColourId,
                        (int) keyDownOverlayColourId, (int) textLabelColourId, (int) shadowColourId, (int) upDownButtonBackgroundColourId,
                        (int) upDownButtonArrowColourId })
            setColour (id, juce::Colours::transparentBlack);
        setOpaque (false);
    }

    // returns true while something is still glowing (the keyboard then needs a repaint)
    bool advance (float dt)
    {
        bool any = false;
        const float decay = std::exp (-dt / 0.45f);
        for (int n = 0; n < 128; ++n)
        {
            heat[(size_t) n] = keyState.isNoteOnForChannels (0xffff, n) ? 1.f : heat[(size_t) n] * decay;
            if (heat[(size_t) n] < 0.01f) heat[(size_t) n] = 0.f;
            any = any || heat[(size_t) n] > 0.f;
        }
        return any;
    }

    void drawWhiteNote (int note, juce::Graphics& g, juce::Rectangle<float> area, bool down, bool over, juce::Colour line, juce::Colour textCol) override
    {
        if (! overlayOnly) juce::MidiKeyboardComponent::drawWhiteNote (note, g, area, false, over, line, textCol);
        glowKey (g, area.reduced (1.f, 0.f), heat[(size_t) note], down);
    }
    void drawBlackNote (int note, juce::Graphics& g, juce::Rectangle<float> area, bool down, bool over, juce::Colour fill) override
    {
        if (! overlayOnly) juce::MidiKeyboardComponent::drawBlackNote (note, g, area, false, over, fill);
        glowKey (g, area.reduced (1.f, 0.f), heat[(size_t) note], down);
    }

private:
    juce::MidiKeyboardState& keyState;

    void glowKey (juce::Graphics& g, juce::Rectangle<float> r, float h, bool down)
    {
        if (h <= 0.f && ! down) return;
        h = juce::jmax (h, down ? 1.f : 0.f);
        // the key fills with light from the bottom: hot colour at the front edge, the neon colour above
        juce::ColourGradient gr (neonHot.withAlpha (0.95f * h), r.getX(), r.getBottom(), neon.withAlpha (0.25f * h), r.getX(), r.getY(), false);
        gr.addColour (0.35, neonHot.interpolatedWith (neon, 0.5f).withAlpha (0.8f * h));
        gr.addColour (0.7, neon.withAlpha (0.55f * h));
        g.setGradientFill (gr);
        g.fillRect (r);
        g.setColour (neon.withAlpha (0.9f * h));                       // lit edges
        g.drawRect (r, 1.5f);
        g.setColour (juce::Colours::white.withAlpha (0.85f * h));
        g.fillRect (r.removeFromBottom (4.f));
    }
};
}
