# Validation professionnelle — critères de sortie v0.9
## CI
- DSP historique et nouveaux tests : 60 modèles et 400 presets.
- quality_control : models.csv (120 mesures, modes Legacy/Analog), presets.csv (400 x 3 fréquences), cpu.csv (3 scénarios de 60 s AUDIO simulées, rack 8 slots, 48 kHz/128), meter.csv (ancrages 997 Hz mono et crête interéchantillons fs/4).
- CPU : durée moyenne, 95e percentile, maximum et nombre de dépassements. Les timings sont informatifs sur machines CI partagées et ne constituent pas un test de temps réel hôte. Ne pas confondre durée audio et durée murale.
- THD H2-H9 sur sinus 1 kHz à ratio 1:1 ; alias probe à 15 kHz pour une entrée 11 kHz (repli de H3). Pas une mesure d'aliasing exhaustive ni test de l'oversampling JUCE.
- processor_qc : sauvegarde/restauration des paramètres, rejet d'état illisible, exécution du processeur complet aux cinq facteurs de suréchantillonnage.
- UTF-8 strict et motifs usuels de corruption. Contrôle graphique réel reste requis.
- Artefacts AU/VST3/Standalone et rapports QC séparés. Un build rouge ne valide pas une release.
## Références de mesure
ITU-R BS.1770-5 (11/2023) : https://www.itu.int/rec/R-REC-BS.1770/en
Filtrage K et fenêtres de 400 ms / 3 s ; estimation TP par interpolation FIR 4x. Les ancrages synthétiques n'établissent pas la conformité exhaustive. Tester le jeu officiel EBU Tech 3341 (tolérances, taux, phase, signaux multiples) et comparer à un instrument de référence avant toute revendication. Pas de LUFS intégré avec gate dans cette version.
## Validation manuelle bloquante avant 1.0
- Écouter 60 modèles et 400 presets sur voix, batterie, basse, guitare, claviers et mix ; consigner artefacts, niveau, utilité et modifications.
- Enregistrer les résultats AU dans Logic et Ableton, VST3 dans Ableton/Cubase/Reaper, Standalone ; tester réouverture, automation, MIDI, sidechain, A/B, Preset Lab, import/export.
- Captures réelles FR et EN des pages à 1280x800 et 1440x900 logiques, sur Mac 13 pouces. Vérifier absence de glyphes corrompus, texte tronqué, hitboxes superposées et ascenseurs.
- Test réel 60 s en DAW, buffers 64/128/256, 44.1/48/96 kHz, 8 slots, Analog et PRO FFT, facteurs 1/2/4/8/16, avec et sans Analyzer ; noter dropouts et CPU. Comparer les résultats CI entre plateformes ; ne pas annoncer une optimisation SIMD sans mesure.
- Mesurer PDC avec impulsion, changements de qualité à l'arrêt, routages série/A/B/Designer, mix sec/traité, bypass et sidechain ; tester sauvegarde de projets anciens.
- Valider le manuel, la signature/notarisation et les installateurs via Liberty Release Center seulement après ces contrôles.
