#include "PresetManager.h"

PresetManager::PresetManager (juce::AudioProcessorValueTreeState& a, const juce::String& name)
    : apvts (a), pluginName (name)
{
    preserved = { "bypass", "simple", "inGain", "outGain" };
    rescanUser();
}

void PresetManager::setFactoryPresets (std::vector<HumpirePreset> p)
{
    factory = std::move (p);
    rescanUser();
}

juce::File PresetManager::userDir() const
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
               .getChildFile ("HUMPIRE").getChildFile (pluginName).getChildFile ("Presets");
}

void PresetManager::rescanUser()
{
    userFiles.clear();
    auto dir = userDir();
    if (dir.isDirectory())
        userFiles = dir.findChildFiles (juce::File::findFiles, false, "*.hpreset");
}

juce::String PresetManager::getCurrentName() const
{
    if (current < (int) factory.size()) return factory[(size_t) current].name;
    const int u = current - (int) factory.size();
    if (u >= 0 && u < userFiles.size()) return userFiles[u].getFileNameWithoutExtension();
    return "Init";
}

void PresetManager::applyFactory (const HumpirePreset& p)
{
    if (apvts.undoManager != nullptr) apvts.undoManager->beginNewTransaction ("Preset " + p.name);
    for (auto* base : apvts.processor.getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (base))
            if (! preserved.contains (rp->paramID))
                rp->setValueNotifyingHost (rp->getDefaultValue());
    for (auto& [id, v] : p.values)
        if (auto* rp = apvts.getParameter (id))
            rp->setValueNotifyingHost (rp->convertTo0to1 (v));
}

void PresetManager::loadPreset (int index)
{
    const int total = getNumPresets();
    if (total == 0) return;
    index = ((index % total) + total) % total;
    current = index;

    if (index < (int) factory.size())
        applyFactory (factory[(size_t) index]);
    else if (auto xml = juce::XmlDocument::parse (userFiles[index - (int) factory.size()]))
    {
        auto vt = juce::ValueTree::fromXml (*xml);
        if (vt.isValid() && vt.hasType (apvts.state.getType()))
        {
            if (apvts.undoManager != nullptr) apvts.undoManager->beginNewTransaction ("Preset");
            // on conserve les parametres "preserved" de l'etat courant
            for (auto& id : preserved)
                if (auto* rp = apvts.getParameter (id))
                    for (auto child : vt)
                        if (child.getProperty ("id").toString() == id)
                            child.setProperty ("value", rp->convertFrom0to1 (rp->getValue()), nullptr);
            apvts.replaceState (vt);
            if (onStateRestored) onStateRestored();
        }
    }
    if (onPresetChanged) onPresetChanged();
}

void PresetManager::next() { loadPreset (current + 1); }
void PresetManager::prev() { loadPreset (current - 1); }

bool PresetManager::saveUserPreset (const juce::String& name)
{
    auto clean = juce::File::createLegalFileName (name.trim());
    if (clean.isEmpty()) return false;
    auto dir = userDir();
    if (! dir.createDirectory()) return false;
    auto file = dir.getChildFile (clean + ".hpreset");
    if (auto xml = apvts.copyState().createXml())
    {
        xml->setAttribute ("presetName", clean);
        if (! xml->writeTo (file)) return false;
    }
    else return false;
    rescanUser();
    current = (int) factory.size() + userFiles.indexOf (file);
    if (onPresetChanged) onPresetChanged();
    if (onOrbitComplete) onOrbitComplete();
    return true;
}

void PresetManager::showMenu (juce::Component* target)
{
    juce::PopupMenu menu;
    juce::String lastCat;
    juce::PopupMenu sub;
    auto flush = [&] { if (lastCat.isNotEmpty()) menu.addSubMenu (lastCat, sub); sub = juce::PopupMenu(); };
    for (size_t i = 0; i < factory.size(); ++i)
    {
        if (factory[i].category != lastCat) { flush(); lastCat = factory[i].category; }
        sub.addItem ((int) i + 1, factory[i].name, true, (int) i == current);
    }
    flush();
    if (! userFiles.isEmpty())
    {
        juce::PopupMenu user;
        for (int i = 0; i < userFiles.size(); ++i)
            user.addItem ((int) factory.size() + i + 1, userFiles[i].getFileNameWithoutExtension(), true, (int) factory.size() + i == current);
        menu.addSubMenu ("Utilisateur", user);
    }
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (target).withMinimumWidth (target != nullptr ? target->getWidth() : 160),
                        [this] (int r) { if (r > 0) loadPreset (r - 1); });
}

void PresetManager::toggleAB()
{
    const int other = abSlot ^ 1;
    abState[abSlot] = apvts.copyState();
    if (! abState[other].isValid()) abState[other] = apvts.copyState();   // 1er passage : B = copie de A
    abSlot = other;
    if (apvts.undoManager != nullptr) apvts.undoManager->beginNewTransaction ("A/B");
    apvts.replaceState (abState[abSlot].createCopy());
    if (onStateRestored) onStateRestored();
}
