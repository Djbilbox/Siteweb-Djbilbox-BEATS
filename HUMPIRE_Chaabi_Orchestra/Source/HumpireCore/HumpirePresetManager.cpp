#include "HumpirePresetManager.h"

namespace humpire
{
PresetManager::PresetManager (juce::AudioProcessorValueTreeState& s, juce::UndoManager& u,
                              juce::File dir, std::vector<FactoryPreset> factory)
    : state (s), undo (u), userDir (std::move (dir)), factoryPresets (std::move (factory))
{
    userDir.createDirectory();
    rescanUserPresets();
}

void PresetManager::rescanUserPresets()
{
    names.clear();
    categories.clear();
    userFiles.clear();

    for (auto& f : factoryPresets)
    {
        names.push_back (f.name);
        categories.push_back (f.category);
    }

    auto files = userDir.findChildFiles (juce::File::findFiles, false, "*.hpreset");
    files.sort();
    for (auto& f : files)
    {
        userFiles.push_back (f);
        names.push_back (f.getFileNameWithoutExtension());
        categories.push_back ("User");
    }
    current = juce::jlimit (0, juce::jmax (0, getNumPresets() - 1), current);
}

juce::String PresetManager::getCurrentName() const
{
    return names.empty() ? juce::String() : names[(size_t) current];
}

void PresetManager::loadIndex (int i)
{
    if (names.empty()) return;
    i = (i % getNumPresets() + getNumPresets()) % getNumPresets();
    current = i;

    undo.beginNewTransaction ("Preset");
    if (i < (int) factoryPresets.size())
        applyFactory (factoryPresets[(size_t) i]);
    else
        loadUserFile (userFiles[(size_t) i - factoryPresets.size()]);
}

void PresetManager::next() { loadIndex (current + 1); }
void PresetManager::prev() { loadIndex (current - 1); }

void PresetManager::applyFactory (const FactoryPreset& p)
{
    // 1) reset everything to the default values
    for (auto* param : state.processor.getParameters())
        if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (param))
            ranged->setValueNotifyingHost (ranged->getDefaultValue());

    // 2) preset values
    for (auto& [id, v] : p.values)
        if (auto* param = state.getParameter (id))
            param->setValueNotifyingHost (param->convertTo0to1 (v));

    // 3) state properties (SFZ paths, etc.)
    for (auto& [id, v] : p.props)
        state.state.setProperty (id, v, &undo);
}

bool PresetManager::loadUserFile (const juce::File& f)
{
    if (auto xml = juce::XmlDocument::parse (f))
    {
        auto tree = juce::ValueTree::fromXml (*xml);
        if (tree.isValid() && tree.hasType (state.state.getType()))
        {
            state.replaceState (tree);
            return true;
        }
    }
    return false;
}

bool PresetManager::saveUser (const juce::String& name)
{
    auto clean = juce::File::createLegalFileName (name.trim());
    if (clean.isEmpty()) return false;

    auto file = userDir.getChildFile (clean + ".hpreset");
    if (auto xml = state.copyState().createXml())
    {
        if (! xml->writeTo (file)) return false;
        rescanUserPresets();
        for (int i = 0; i < getNumPresets(); ++i)
            if (names[(size_t) i] == file.getFileNameWithoutExtension()) current = i;
        return true;
    }
    return false;
}

void PresetManager::toggleAB()
{
    const int cur = currentIsB ? 1 : 0, other = 1 - cur;
    ab[cur] = state.copyState();
    if (! ab[other].isValid()) ab[other] = ab[cur].createCopy();
    currentIsB = ! currentIsB;
    state.replaceState (ab[other].createCopy());
}
} // namespace humpire
