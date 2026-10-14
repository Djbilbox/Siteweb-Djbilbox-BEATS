# Roadmap des 7 plugins restants (du plus simple au plus dur)
1. **LAG (delay)** - DelayLine interpolee, sync BPM via AudioPlayHead, feedback + SVF filtre, ducking par envelope du dry, morphing de 4 snapshots (interpolation des parametres lissee).
2. **COMBO (performance)** - ring buffer 4 s : Stutter (boucle de longueur 1/16..1/4 BPM), Tape Stop (rampe de vitesse de lecture), Sweep (SVF), Reverse (lecture inverse par blocs). MIDI learn via map CC -> parametre.
3. **CLUTCH (compresseur)** - detecteur RMS/peak, courbe soft-knee, attack/release 1-pole, sidechain HPF, mix parallele ; Punch = attack lente, Crush = ratio haut + saturation.
4. **OVERCLOCK (enhancer)** - split LR4 3 bandes, saturation asymetrique par bande (oversampling x4), compensation loudness, Curve = forme de la transfer function.
5. **RESPAWN (reverb)** - FDN 8 lignes + matrice de Householder, damping HF par ligne, pre-delay, early reflections, ducking sidechain.
6. **HARDCAP (limiteur)** - lookahead (latence declaree), true-peak via oversampling x4, enveloppe double release (auto), mesure LUFS K-weighting (BS.1770) integrated/short-term.
7. **SKILLTREE (EQ)** - 6 bandes biquad RBJ + HP/LP, Mid/Side, linear-phase par FIR FFT (latence declaree), analyseur FFT 4096 + courbe cliquable.
