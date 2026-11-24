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
