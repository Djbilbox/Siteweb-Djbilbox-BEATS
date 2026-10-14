#include "PluginEditor.h"

K2000AudioProcessorEditor::K2000AudioProcessorEditor(K2000AudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setLookAndFeel(&customLookAndFeel);
    setSize(1000, 750);

    // 1. Header & Patch Navigation
    addAndMakeVisible(browsePatchBtn);
    addAndMakeVisible(prevPatchBtn);
    addAndMakeVisible(nextPatchBtn);
    addAndMakeVisible(savePatchBtn);
    addAndMakeVisible(patchDisplay);

    patchDisplay.setFont(juce::Font(14.0f, juce::Font::bold));
    patchDisplay.setColour(juce::Label::textColourId, K2000UI::K2000LookAndFeel::neonCyan);
    patchDisplay.setColour(juce::Label::backgroundColourId, K2000UI::K2000LookAndFeel::recessedBg);
    patchDisplay.setColour(juce::Label::outlineColourId, K2000UI::K2000LookAndFeel::neonCyan);
    patchDisplay.setJustificationType(juce::Justification::centred);

    prevPatchBtn.onClick = [this]() {
        int cur = processor.getCurrentProgram();
        int num = processor.getNumPrograms();
        processor.setCurrentProgram((cur - 1 + num) % num);
        updatePatchDisplay();
    };

    nextPatchBtn.onClick = [this]() {
        int cur = processor.getCurrentProgram();
        int num = processor.getNumPrograms();
        processor.setCurrentProgram((cur + 1) % num);
        updatePatchDisplay();
    };

    browsePatchBtn.onClick = [this]() {
        juce::PopupMenu m;
        for (int i = 0; i < processor.getNumPrograms(); ++i)
        {
            m.addItem(i + 1, processor.getProgramName(i), true, i == processor.getCurrentProgram());
        }
        m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&browsePatchBtn), [this](int result) {
            if (result > 0)
            {
                processor.setCurrentProgram(result - 1);
                updatePatchDisplay();
            }
        });
    };

    // 2. Expansion Cassette
    addAndMakeVisible(cassetteSlot);
    cassetteSlot.onCassetteClicked = [this]() {
        browsePatchBtn.triggerClick();
    };

    // 3. LFO Section Controls
    setupKnob(lfoRateSlider, "lfo_rate", "RATE");
    setupKnob(lfoAmountSlider, "lfo_amount", "AMOUNT");
    addAndMakeVisible(lfoPitchToggle);
    buttonAttachments.push_back(std::make_unique<ButtonAttachment>(processor.getAPVTS(), "lfo_pitch", lfoPitchToggle));

    setupFader(lfoAttackSlider, "lfo_attack", "ATTACK");
    setupFader(lfoDecaySlider, "lfo_decay", "DECAY");
    setupFader(lfoSustainSlider, "lfo_sustain", "SUSTAIN");
    setupFader(lfoReleaseSlider, "lfo_release", "RELEASE");

    // 4. ADSR + Velocity Bank Section Controls
    setupFader(ampAttackSlider, "amp_attack", "ATTACK");
    setupFader(ampDecaySlider, "amp_decay", "DECAY");
    setupFader(ampSustainSlider, "amp_sustain", "SUSTAIN");
    setupFader(velBankSlider, "velocity_bank", "BANK");
    setupFader(velAdsrSlider, "velocity_adsr", "ADSR");

    // 5. FX & Master Knobs
    setupKnob(volSlider, "master_volume", "VOLUME");
    setupKnob(revSlider, "fx_reverb", "REVERB");
    setupKnob(choSlider, "fx_chorus", "CHORUS");
    setupKnob(distMasterSlider, "fx_distortion", "DISTORTION");

    // 6. Filter, Glide & Delay Knobs
    setupKnob(glideSlider, "glide_time", "GLIDE");
    setupKnob(filterSlider, "filter_cutoff", "FILTER");
    setupKnob(filterDriveSlider, "filter_drive", "DISTORTION");
    setupKnob(delayMixSlider, "fx_delay_mix", "DELAY");
    setupKnob(delayTimeSlider, "fx_delay_time", "TIME");
    setupKnob(delayFbSlider, "fx_delay_fb", "FEEDBACK");

    // 7. Dropdown Selectors
    setupCombo(waveCombo, "osc_waveform", { "SAWTOOTH", "SQUARE", "TRIANGLE", "SINE", "SUPERSAW", "80s BRASS" });
    setupCombo(lfoSourceCombo, "lfo_source", { "SINE", "TRIANGLE", "SQUARE", "SAW UP", "SAW DOWN", "RANDOM S&H" });
    setupCombo(lfoDestCombo, "lfo_dest", { "CUTOFF", "PITCH", "PULSE WIDTH", "VOLUME", "CHORUS", "PAN" });
    setupCombo(filterTypeCombo, "filter_type", { "LOWPASS 24dB", "LOWPASS 12dB", "BANDPASS", "HIGHPASS", "ACID LOWPASS" });
    setupCombo(voiceModeCombo, "voice_mode", { "POLY 8-VOICE", "POLY 16-VOICE", "MONO RETRIG", "MONO LEGATO" });

    // 8. Performance Wheels & Keyboard
    addAndMakeVisible(wheels);
    addAndMakeVisible(keyboard);

    wheels.onPitchBendChanged = [this](float val) {
        // Pitch wheel
    };
    wheels.onModWheelChanged = [this](float val) {
        // Mod wheel
    };

    updatePatchDisplay();
}

K2000AudioProcessorEditor::~K2000AudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void K2000AudioProcessorEditor::setupKnob(juce::Slider& slider, const juce::String& paramId, const juce::String& /*labelText*/)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(slider);
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(processor.getAPVTS(), paramId, slider));
}

void K2000AudioProcessorEditor::setupFader(juce::Slider& slider, const juce::String& paramId, const juce::String& /*labelText*/)
{
    slider.setSliderStyle(juce::Slider::LinearVertical);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(slider);
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(processor.getAPVTS(), paramId, slider));
}

void K2000AudioProcessorEditor::setupCombo(juce::ComboBox& combo, const juce::String& paramId, const juce::StringArray& items)
{
    combo.addItemList(items, 1);
    addAndMakeVisible(combo);
    comboAttachments.push_back(std::make_unique<ComboAttachment>(processor.getAPVTS(), paramId, combo));
}

void K2000AudioProcessorEditor::updatePatchDisplay()
{
    patchDisplay.setText(processor.getProgramName(processor.getCurrentProgram()), juce::dontSendNotification);
}

void K2000AudioProcessorEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // 1. Brushed Anthracite Chassis Background
    juce::ColourGradient bgGrad(juce::Colour(0x1C, 0x20, 0x26), 0.0f, 0.0f,
                               juce::Colour(0x10, 0x13, 0x17), 0.0f, bounds.getHeight(), false);
    g.setGradientFill(bgGrad);
    g.fillRect(bounds);

    // Subtle brushed horizontal stripes
    g.setColour(juce::Colour(0x28, 0x30, 0x38).withAlpha(0.2f));
    for (int y = 0; y < getHeight(); y += 4)
        g.drawHorizontalLine(y, 0.0f, bounds.getWidth());

    // 2. Outer Chrome Bezel & Corner Torx Screws
    g.setColour(juce::Colour(0x3E, 0x48, 0x54));
    g.drawRect(bounds.reduced(1.0f), 2.0f);

    auto drawScrew = [&g](float cx, float cy) {
        g.setColour(juce::Colour(0x8A, 0x96, 0xA2));
        g.fillEllipse(cx - 7.0f, cy - 7.0f, 14.0f, 14.0f);
        g.setColour(juce::Colour(0x20, 0x24, 0x2A));
        g.drawEllipse(cx - 7.0f, cy - 7.0f, 14.0f, 14.0f, 1.0f);
        // Torx socket
        g.setColour(juce::Colour(0x14, 0x18, 0x1C));
        g.fillEllipse(cx - 3.0f, cy - 3.0f, 6.0f, 6.0f);
    };

    drawScrew(20.0f, 20.0f);
    drawScrew(bounds.getRight() - 20.0f, 20.0f);
    drawScrew(20.0f, bounds.getBottom() - 20.0f);
    drawScrew(bounds.getRight() - 20.0f, bounds.getBottom() - 20.0f);

    // 3. Chrome Vertical Carrying Handles on Left and Right
    auto drawHandle = [&g](float hx, float hy, float hw, float hh) {
        juce::ColourGradient handleGrad(juce::Colour(0xEB, 0xF2, 0xF8), hx, hy,
                                       juce::Colour(0x40, 0x4B, 0x55), hx + hw, hy, false);
        g.setGradientFill(handleGrad);
        g.fillRoundedRectangle(hx, hy, hw, hh, 5.0f);
        g.setColour(juce::Colour(0x98, 0xA6, 0xB5));
        g.drawRoundedRectangle(hx, hy, hw, hh, 5.0f, 1.2f);
    };
    drawHandle(8.0f, 60.0f, 14.0f, bounds.getHeight() - 120.0f);
    drawHandle(bounds.getRight() - 22.0f, 60.0f, 14.0f, bounds.getHeight() - 120.0f);

    // 4. Header Branding: DJBILBOXBEATS & CB Badge
    g.setColour(juce::Colour(0xD0, 0xDE, 0xEC));
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.drawText("DJBILBOXBEATS", bounds.getRight() - 190.0f, 22.0f, 110.0f, 20.0f, juce::Justification::centredRight);

    auto cbRect = juce::Rectangle<float>(bounds.getRight() - 70.0f, 18.0f, 28.0f, 28.0f);
    g.setColour(K2000UI::K2000LookAndFeel::recessedBg);
    g.fillRoundedRectangle(cbRect, 4.0f);
    g.setColour(K2000UI::K2000LookAndFeel::neonCyan);
    g.drawRoundedRectangle(cbRect, 4.0f, 1.5f);
    g.setFont(juce::Font(13.0f, juce::Font::bold));
    g.drawText("CB", cbRect, juce::Justification::centred);

    // 5. Center Mascot Medallion & Speaker Grilles
    float centerX = bounds.getCentreX();
    auto mascotRect = juce::Rectangle<float>(centerX - 42.0f, 85.0f, 84.0f, 84.0f);
    g.setColour(K2000UI::K2000LookAndFeel::recessedBg);
    g.fillEllipse(mascotRect);
    g.setColour(K2000UI::K2000LookAndFeel::neonCyan);
    g.drawEllipse(mascotRect, 2.0f);

    // Mascot Camel Icon / Head Profile
    g.setColour(juce::Colour(0xDC, 0xA8, 0x76));
    g.fillEllipse(centerX - 18.0f, 110.0f, 36.0f, 40.0f);
    // Cyan Visor
    g.setColour(K2000UI::K2000LookAndFeel::neonCyan);
    g.fillRoundedRectangle(centerX - 22.0f, 116.0f, 44.0f, 10.0f, 3.0f);

    // Grilles on sides of mascot
    auto drawGrille = [&g](float gx, float gy, float gw, float gh) {
        g.setColour(K2000UI::K2000LookAndFeel::recessedBg);
        g.fillRoundedRectangle(gx, gy, gw, gh, 4.0f);
        g.setColour(K2000UI::K2000LookAndFeel::neonCyan.withAlpha(0.6f));
        g.drawRoundedRectangle(gx, gy, gw, gh, 1.0f);
        g.setColour(juce::Colour(0x05, 0x08, 0x0A));
        for (float py = gy + 6.0f; py < gy + gh - 4.0f; py += 6.0f)
            for (float px = gx + 6.0f; px < gx + gw - 4.0f; px += 7.0f)
                g.fillEllipse(px, py, 3.0f, 3.0f);
    };
    drawGrille(centerX - 120.0f, 90.0f, 42.0f, 80.0f);
    drawGrille(centerX + 78.0f, 90.0f, 42.0f, 80.0f);

    // Title: K2000 By Humpire
    g.setColour(K2000UI::K2000LookAndFeel::neonCyan);
    g.setFont(juce::Font(28.0f, juce::Font::bold));
    g.drawText("K2000", centerX - 80.0f, 185.0f, 160.0f, 30.0f, juce::Justification::centred);

    g.setColour(juce::Colour(0xCC, 0xDC, 0xEB));
    g.setFont(juce::Font(12.0f, juce::Font::plain));
    g.drawText("By Humpire", centerX - 60.0f, 215.0f, 120.0f, 16.0f, juce::Justification::centred);

    // 6. Section Labels
    g.setColour(juce::Colour(0x8A, 0x9B, 0xAC));
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.drawText("LFO", 185.0f, 165.0f, 60.0f, 16.0f, juce::Justification::centred);
    g.drawText("ADSR+VELOCITY BANK", bounds.getRight() - 320.0f, 85.0f, 250.0f, 16.0f, juce::Justification::centred);

    // Knobs Labels
    g.setFont(juce::Font(9.0f, juce::Font::bold));
    g.drawText("RATE", 125.0f, 222.0f, 50.0f, 14.0f, juce::Justification::centred);
    g.drawText("PITCH", 175.0f, 252.0f, 50.0f, 14.0f, juce::Justification::centred);
    g.drawText("AMOUNT", 125.0f, 307.0f, 50.0f, 14.0f, juce::Justification::centred);

    // LFO Fader Labels
    g.drawText("ATTACK", 215.0f, 307.0f, 40.0f, 14.0f, juce::Justification::centred);
    g.drawText("DECAY", 255.0f, 307.0f, 40.0f, 14.0f, juce::Justification::centred);
    g.drawText("SUSTAIN", 295.0f, 307.0f, 44.0f, 14.0f, juce::Justification::centred);
    g.drawText("RELEASE", 335.0f, 307.0f, 44.0f, 14.0f, juce::Justification::centred);

    // ADSR Fader Labels
    float adX = bounds.getRight() - 320.0f;
    g.drawText("ATTACK", adX, 190.0f, 45.0f, 14.0f, juce::Justification::centred);
    g.drawText("DECAY", adX + 50.0f, 190.0f, 45.0f, 14.0f, juce::Justification::centred);
    g.drawText("SUSTAIN", adX + 100.0f, 190.0f, 45.0f, 14.0f, juce::Justification::centred);
    g.drawText("BANK", adX + 150.0f, 190.0f, 45.0f, 14.0f, juce::Justification::centred);
    g.drawText("ADSR", adX + 200.0f, 190.0f, 45.0f, 14.0f, juce::Justification::centred);

    // FX Top Row Labels
    float fxX = bounds.getRight() - 310.0f;
    g.drawText("VOLUME", fxX, 267.0f, 55.0f, 14.0f, juce::Justification::centred);
    g.drawText("REVERB", fxX + 65.0f, 267.0f, 55.0f, 14.0f, juce::Justification::centred);
    g.drawText("CHORUS", fxX + 130.0f, 267.0f, 55.0f, 14.0f, juce::Justification::centred);
    g.drawText("DISTORTION", fxX + 195.0f, 267.0f, 65.0f, 14.0f, juce::Justification::centred);

    // Middle Row Labels
    g.drawText("GLIDE", centerX - 65.0f, 317.0f, 50.0f, 14.0f, juce::Justification::centred);
    g.drawText("FILTER", centerX + 15.0f, 317.0f, 50.0f, 14.0f, juce::Justification::centred);

    g.drawText("DISTORTION", fxX, 342.0f, 65.0f, 14.0f, juce::Justification::centred);
    g.drawText("DELAY", fxX + 65.0f, 342.0f, 55.0f, 14.0f, juce::Justification::centred);
    g.drawText("TIME", fxX + 130.0f, 342.0f, 55.0f, 14.0f, juce::Justification::centred);
    g.drawText("FEEDBACK", fxX + 195.0f, 342.0f, 60.0f, 14.0f, juce::Justification::centred);
}

void K2000AudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    float centerX = getWidth() * 0.5f;

    // Header Patch Selector
    browsePatchBtn.setBounds(centerX - 170, 18, 28, 28);
    prevPatchBtn.setBounds(centerX - 138, 18, 28, 28);
    patchDisplay.setBounds(centerX - 105, 18, 210, 28);
    nextPatchBtn.setBounds(centerX + 110, 18, 28, 28);
    savePatchBtn.setBounds(centerX + 142, 18, 28, 28);

    // Cassette EXPANSION Slot
    cassetteSlot.setBounds(65, 30, 120, 130);

    // LFO Section
    lfoRateSlider.setBounds(130, 180, 42, 42);
    lfoPitchToggle.setBounds(190, 215, 24, 38);
    lfoAmountSlider.setBounds(130, 265, 42, 42);

    lfoAttackSlider.setBounds(225, 195, 22, 110);
    lfoDecaySlider.setBounds(265, 195, 22, 110);
    lfoSustainSlider.setBounds(305, 195, 22, 110);
    lfoReleaseSlider.setBounds(345, 195, 22, 110);

    // ADSR + Velocity Faders
    int adX = getWidth() - 310;
    ampAttackSlider.setBounds(adX + 10, 110, 22, 80);
    ampDecaySlider.setBounds(adX + 60, 110, 22, 80);
    ampSustainSlider.setBounds(adX + 110, 110, 22, 80);
    velBankSlider.setBounds(adX + 160, 110, 22, 80);
    velAdsrSlider.setBounds(adX + 210, 110, 22, 80);

    // FX Knobs
    int fxX = getWidth() - 305;
    volSlider.setBounds(fxX + 5, 225, 44, 44);
    revSlider.setBounds(fxX + 70, 225, 44, 44);
    choSlider.setBounds(fxX + 135, 225, 44, 44);
    distMasterSlider.setBounds(fxX + 205, 225, 44, 44);

    // Middle Knobs
    glideSlider.setBounds(centerX - 60, 275, 44, 44);
    filterSlider.setBounds(centerX + 20, 275, 44, 44);

    filterDriveSlider.setBounds(fxX + 5, 300, 44, 44);
    delayMixSlider.setBounds(fxX + 70, 300, 44, 44);
    delayTimeSlider.setBounds(fxX + 135, 300, 44, 44);
    delayFbSlider.setBounds(fxX + 205, 300, 44, 44);

    // Dropdown Selectors
    int ddY = 370;
    int ddW = 155;
    int ddH = 26;
    waveCombo.setBounds(65, ddY, ddW, ddH);
    lfoSourceCombo.setBounds(235, ddY, ddW, ddH);
    lfoDestCombo.setBounds(405, ddY, ddW, ddH);
    filterTypeCombo.setBounds(575, ddY, ddW, ddH);
    voiceModeCombo.setBounds(745, ddY, ddW, ddH);

    // Performance Bay & Keyboard
    wheels.setBounds(65, 420, 160, 180);
    keyboard.setBounds(240, 420, getWidth() - 305, 180);
}
