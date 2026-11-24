# Compresseur Legends — v0.1.2 TITANIUM RED
Liberty Music Software. Prototype expérimental : huit profils originaux, huit slots, série/parallèle, interface graphite/titane/carmin, FR par défaut et EN. Les profils ne sont pas des émulations mesurées. Objectif commercial : 60 modèles et 400 presets, pas encore implémentés.
## DSP
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
## Plugin
cmake -S . -B build-plugin -DBUILD_PLUGIN=ON
cmake --build build-plugin --config Release
Les builds CI produisent les formats AU/VST3/Standalone sur macOS ARM64 et VST3/Standalone sur Windows x64. Non signés, non notarisés. Validation hôte, lissage des paramètres, optimisation CPU, MIDI Learn, presets et licence commerciale JUCE à traiter avant vente.

Interface v0.1.2 : primitives graphiques reprises de Liberty-Pedal-Legends, rack 4 x 2, édition détaillée, bibliothèque de 8 profils, routage, historique de famille, A/B, permutation réelle des slots. Pas de presets ni MIDI Learn à ce stade.
