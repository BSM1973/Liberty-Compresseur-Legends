# Compresseur Legends — Manuel utilisateur FR, v1.0
Liberty Music Software — édition commerciale candidate.

## 1. Installation et formats
macOS Apple Silicon (macOS 12 ou supérieur) : AU et VST3 via l’installateur .pkg. Windows x64 : VST3 et Standalone via l’installateur .exe. Fermer le DAW avant installation, lancer l’installateur, puis relancer le scan. AU : /Library/Audio/Plug-Ins/Components ; VST3 macOS : /Library/Audio/Plug-Ins/VST3 ; VST3 Windows : dossier VST3 commun. Dans Logic Pro, utiliser AU. L’installateur macOS doit être signé et notarisé avant distribution ; vérifier le ticket. L’installateur Windows reste non signé Authenticode. Les Mac Intel ne sont pas pris en charge par ce package ARM64.

## 2. Premier traitement
Insérer sur une piste audio. Dans RACK, activer un slot, choisir un modèle dans BIBLIOTHÈQUE. Seuil : niveau de déclenchement ; Ratio : pente ; Attaque et Relâchement en ms ; Gain : compensation de sortie ; Mélange : dry/wet ; Couleur : saturation selon le modèle. Commencer à 2:1, attaque 20 ms, relâchement 150 ms, régler le seuil pour 2–4 dB de réduction ; comparer à niveau égal. Les modèles sont des interprétations logicielles originales, pas des mesures de matériel.

## 3. Architecture
Huit slots, routage série ou parallèle A/B. Balance A/B mélange les branches. ROUTAGE > DESIGNER : quatre branches, déplacer un slot à l'arrêt, régler gain, pan, S/M/POL. Chaque slot n'appartient qu'à une branche. La somme est normalisée par branches peuplées. La latence dépend du plus long chemin en Designer, du padding FFT et du lookahead. Modifier routage et qualité à l'arrêt ; vérifier la compensation de l'hôte.

## 4. Bibliothèque, histoire et presets
60 profils ; 400 configurations d'usine, 200 série et 200 parallèle. Choisir les familles, pages et recherche ; marquer les favoris. HISTOIRE ouvre les références et explications de chaque modèle. Les paramètres d'usine chargent les modes historiques par défaut. Auditionner chaque preset sur le signal réel avant de juger son utilité.

## 5. Traitements avancés
ÉDITER > TRAITEMENTS AVANCÉS : Stereo, Mid, Side, Auto Gain RMS, Delta, oversampling 1/2/4/8/16, lookahead global par slot 0–5 ms. Delta restitue la différence entre sec et traité. Le bypass master reste aligné sur le délai déclaré. Le gain master s'applique au signal traité. L'oversampling augmente la charge CPU et ne garantit pas l'absence d'aliasing.

## 6. Analog Perfection
Dans les slots compatibles, ANALOG ouvre le choix Compatibilité / Analog ; Caractère règle l'influence du modèle, Couleur/Harmoniques règle la coloration. All Buttons sur FET change le comportement dynamique. Les autres familles numériques ne reçoivent pas de saturation supplémentaire.

## 7. Spectral & Multiband PRO
Pour les modèles multibandes et spectraux, ouvrir PRO. Choisir 2–6 bandes, cliquer dans le graphique, régler seuil, ratio, attaque, relâchement, Solo/Mute/Bypass et crossovers. FFT 512/1024/2048/4096 pour les modèles spectraux. À 48 kHz et x1, une FFT 4096 peut induire une latence importante. Les filtres de crossover sont complémentaires, pas des Linkwitz-Riley sélectionnables. SIDECHAIN EQ : HP, LP, cloche, Stereo Link et écoute. L'écoute est locale au slot ; les modules en aval restent actifs.

## 8. Intelligent Dynamics
INTELLIGENCE > DNA : mesurer les 60 modèles, choisir deux profils et comparer les valeurs mesurées avec le stimulus de référence. MATCH : lire quatre secondes de musique, sélectionner un objectif, choisir une proposition et appliquer au slot cible. L'application d'une chaîne passe en série et exige un slot suivant libre. ANALYZER PRO : formes d'onde, spectre FFT 2048, H2/H3 relatives au pic dominant, RMS, LUFS M/S, True Peak estimé et corrélation/vectorscope. Les harmoniques sur musique ne constituent pas une mesure THD normalisée. Les mesures LUFS/TP sont des estimations en cours de validation, pas un instrument certifié.

## 9. Preset Lab
PERSONNELS : nom, catégorie, tags, enregistrer, rechercher, charger, importer/exporter .clpreset. Bibliothèque locale dans les données d'application Liberty Music Software/Compresseur Legends/User Presets. Sauvegarder ce dossier. HISTORIQUE : 32 états par session d'éditeur, Undo/Redo, variations douce/intense ; les automations hôte et MIDI ne sont pas historisées. A/B : mesurer quatre secondes identiques dans les deux états, activer niveau égal ; compensation B temporaire ±12 dB, non enregistrée.

## 10. MIDI et automatisation
Clic droit sur une commande compatible : MIDI Learn, envoyer un CC, effacer au besoin. L'hôte doit acheminer les messages MIDI à l'effet ; selon le format, cette fonction peut varier. Les nouveaux paramètres PRO et de routage sont automatisables mais pas tous assignables par MIDI Learn. Le projet conserve les paramètres et affectations MIDI. Les presets personnels conservent les paramètres, pas les affectations MIDI.

## 11. Dépannage et limites de validation
En cas de charge élevée : réduire oversampling, FFT et slots actifs. Éviter de modifier la topologie en lecture. Vérifier la latence avec un signal impulsionnel et le bypass. Interface FR/EN, taille adaptable au bureau. Tester les textes, menus et commandes sur écran 13 pouces avant publication. Signaler le format, OS, DAW, fréquence, buffer, version du plugin, preset et étapes de reproduction. Ne distribuer que les installateurs vérifiés issus du Liberty Release Center. Les mesures de conformité et les essais d’écoute en DAW restent à documenter avant la mise en vente.
