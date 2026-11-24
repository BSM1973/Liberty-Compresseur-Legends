# Compresseur Legends — v0.1.3 TITANIUM RED
Liberty Music Software. Prototype expérimental : huit profils originaux, huit slots, série/parallèle, interface graphite/titane/carmin, FR par défaut et EN. Les profils ne sont pas des émulations mesurées. Objectif commercial : 60 modèles et 400 presets, pas encore implémentés.
## DSP
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
## Plugin
cmake -S . -B build-plugin -DBUILD_PLUGIN=ON
cmake --build build-plugin --config Release
Les builds CI produisent les formats AU/VST3/Standalone sur macOS ARM64 et VST3/Standalone sur Windows x64. Non signés, non notarisés. Validation hôte, lissage des paramètres, optimisation CPU, MIDI Learn, presets et licence commerciale JUCE à traiter avant vente.

Interface v0.1.3 : primitives graphiques reprises de Liberty-Pedal-Legends, rack 4 x 2, édition détaillée, bibliothèque de 8 profils, routage, historique de famille, A/B, permutation réelle des slots. Pas de presets ni MIDI Learn à ce stade.

Version 0.1.3 : fenêtre redimensionnable au ratio 1600:930, cible 1440x837 limitée par le bureau utilisable, encyclopédie FR/EN en trois onglets (48 paragraphes par langue), 8 profils. DSP inchangé. Voir HISTORY_SOURCES.md.

## v0.2.0
20 profils de compression originaux (8 précédents inchangés), 100 presets d'usine calculés à partir de réglages par usage, 50 série / 50 parallèle, navigateur paginé avec recherche et favoris de session. Encyclopédie FR/EN 20 fiches, trois onglets. Les IDs de modèles 0-7 restent identiques ; la plage normalisée du paramètre model s'élargit, vérifier les anciennes automatisations de choix de modèle. Ni émulations mesurées ni nouveaux circuits PWM/multibandes à ce stade. MIDI Learn et favoris persistants à venir.

## v0.3.0
40 profils et 200 configurations (100 série / 100 parallèle). Nouveaux détecteurs filtrés, facteur de crête, relâchement adaptatif et compression deux bandes à 220 Hz. Sidechain externe par slot (retour interne si bus désactivé), MIDI CC par canal sur 57 potentiomètres avec mémorisation projet, favoris sur disque, courbe nominale et historique réel de GR. Les 20 premiers profils et 100 presets sont conservés. Automation normalisée de choix du modèle à vérifier après extension. Tests hôte, performance et compatibilité MIDI à valider ; pas d'émulation mesurée, pas de PWM physique ni FFT spectrale.

## v0.4.0 (experimental)
60 original profiles, 400 preset configurations, 200 series / 200 parallel. Three/four-band fixed crossovers with linked per-band gain; 256-point FFT spectral compressor with independent linked bin envelopes; PWM-inspired duty curve (not physical PWM); Mid/Side selection, RMS-based Auto Gain, Delta; rack oversampling via JUCE up to 16x, lookahead 0-5 ms per slot, matched serial/parallel latency. Change quality, routing/latency modes while stopped. Spectral latency uses uniform slot padding, plus OS filter latency; not a low-latency live mode. CPU caching for ordinary envelopes, performance not yet benchmarked. Presets are programmed configurations, not auditioned factory sound design. Host validation and commercial QC pending.

## v0.4.1 Studio Hardware
Native JUCE rack inspired by approved hardware concept, 60 model-specific faceplates across family finishes, all 56 slot knobs directly accessible, MIDI Learn retained, 2/3/4-band and 16 grouped FFT gain-reduction taps, new automatable master output and latency-aligned global bypass with 20 ms smoothing. Factory presets reset new controls to neutral; previous model DSP and 400 preset configurations unchanged. No generated artwork used as interface. Visual and host latency verification pending.
