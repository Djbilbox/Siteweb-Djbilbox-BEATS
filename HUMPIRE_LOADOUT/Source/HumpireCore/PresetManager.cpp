#include "PresetManager.h"

namespace humpire
{
PresetManager::PresetManager (juce::AudioProcessorValueTreeState& s, const juce::String& n,
                              std::vector<FactoryPreset> factory, juce::StringArray keep)
    : apvts (s), pluginName (n), preserved (std::move (keep))
{
    for (auto& f : factory)
        entries.push_back ({ f.name, f.category, std::move (f.values), {} });
    numFactory = entries.size();
    rescanUser();
}

juce::File PresetManager::userDir() const
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
               .getChildFile ("HUMPIRE").getChildFile (pluginName).getChildFile ("Presets");
}

void PresetManager::rescanUser()
{
    entries.resize (numFactory);
    auto files = userDir().findChildFiles (juce::File::findFiles, false, "*.xml");
    files.sort();
    for (auto& f : files)
        entries.push_back ({ f.getFileNameWithoutExtension(), "User", {}, f });
    sendChangeMessage();
}

juce::String PresetManager::getCurrentName() const
{
    return juce::isPositiveAndBelow (current, (int) entries.size()) ? entries[(size_t) current].name : juce::String ("Init");
}

void PresetManager::load (int index)
{
    if (! juce::isPositiveAndBelow (index, (int) entries.size())) return;
    current = index;
    const auto& e = entries[(size_t) index];
    if (e.file != juce::File()) applyUser (e.file);  else applyFactory (e);
    sendChangeMessage();
}

void PresetManager::applyFactory (const Entry& e)
{
    if (apvts.undoManager != nullptr) apvts.undoManager->beginNewTransaction ("Preset");
    for (auto* p : apvts.processor.getParameters())
    {
        auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p);
        if (rp == nullptr || preserved.contains (rp->paramID)) continue;

        float target = rp->convertFrom0to1 (rp->getDefaultValue());
        for (auto& kv : e.values)
            if (kv.first == rp->paramID) { target = kv.second; break; }

        rp->beginChangeGesture();
        rp->setValueNotifyingHost (rp->convertTo0to1 (target));
        rp->endChangeGesture();
    }
}

void PresetManager::applyUser (const juce::File& f)
{
    if (auto xml = juce::parseXML (f))
        if (xml->hasTagName (apvts.state.getType()))
        {
            if (apvts.undoManager != nullptr) apvts.undoManager->beginNewTransaction ("Preset");
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
        }
}

bool PresetManager::saveUserPreset (const juce::String& name)
{
    const auto clean = juce::File::createLegalFileName (name.trim());
    if (clean.isEmpty()) return false;
    auto dir = userDir();
    if (! dir.createDirectory()) return false;
    auto file = dir.getChildFile (clean + ".xml");
    if (auto xml = apvts.copyState().createXml())
        if (xml->writeTo (file))
        {
            rescanUser();
            for (size_t i = 0; i < entries.size(); ++i)
                if (entries[i].file == file) current = (int) i;
            sendChangeMessage();
            return true;
        }
    return false;
}

juce::PopupMenu PresetManager::buildMenu (int baseId) const
{
    juce::PopupMenu root;
    juce::StringArray cats;
    for (auto& e : entries) cats.addIfNotAlreadyThere (e.category);
    for (auto& c : cats)
    {
        juce::PopupMenu sub;
        for (size_t i = 0; i < entries.size(); ++i)
            if (entries[i].category == c)
                sub.addItem (baseId + (int) i, entries[i].name, true, (int) i == current);
        root.addSubMenu (c, sub);
    }
    return root;
}

void PresetManager::toggleAB()
{
    (onB ? slotB : slotA) = apvts.copyState();          // memorise l'etat courant dans son slot
    onB = ! onB;
    auto& target = onB ? slotB : slotA;
    if (target.isValid()) apvts.replaceState (target.createCopy());
    sendChangeMessage();
}
} // namespace humpire
