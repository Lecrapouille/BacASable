# Les mathématiques de la commande prédictive - Exemple d'une voiture

Je vais vous expliquer les mathématiques de la commande prédictive en utilisant l'exemple concret d'une voiture, qui est à la fois intuitif et pratique.

## Modélisation d'une voiture

Imaginons une voiture se déplaçant sur une route. Pour simplifier, nous considérons uniquement le mouvement longitudinal (avant/arrière) :

### Variables d'état (ce que nous surveillons)

*   Position $p$ (m)
*   Vitesse $v$ (m/s)
*   Accélération $a$ (m/s²)

### Variable de commande (ce que nous contrôlons)

*   Force appliquée $u$ (N) - via l'accélérateur ou le frein

### Équations du modèle (lois physiques)

En temps discret avec un intervalle $\Delta t$ :

*   $p_{k+1} = p_k + v_k \cdot \Delta t + \frac{1}{2} a_k \cdot \Delta t^2$
*   $v_{k+1} = v_k + a_k \cdot \Delta t$
*   $a_{k+1} = \frac{1}{m}(u_k - b \cdot v_k)$

Où :

*   $m$ est la masse de la voiture
*   $b$ est un coefficient de frottement
*   $k$ représente l'instant présent, $k+1$ l'instant suivant

Formulation mathématique du MPC
-------------------------------

### 1\. Formalisation sous forme matricielle

On peut réécrire notre modèle sous la forme standard : $X_{k+1} = A \cdot X_k + B \cdot u_k$

Où $X = [p, v, a]^T$ est le vecteur d'état et :

$$A = \begin{bmatrix}
1 & \Delta t & \frac{\Delta t^2}{2} \\
0 & 1 & \Delta t \\
0 & -\frac{b}{m} & 0
\end{bmatrix},$$

$$B = \begin{bmatrix}
0 \\
0 \\
\frac{1}{m}
\end{bmatrix}$$

### 2\. Prédiction des états futurs

Si nous avons l'état actuel $X_0$ et une séquence de commandes futures $U = [u_0, u_1, ..., u_{N-1}]$, nous pouvons prédire les états futurs :

*   $X_1 = A \cdot X_0 + B \cdot u_0$
*   $X_2 = A \cdot X_1 + B \cdot u_1 = A^2 \cdot X_0 + A \cdot B \cdot u_0 + B \cdot u_1$
*   ...et ainsi de suite jusqu'à l'horizon de prédiction $N$

### 3\. Fonction objectif (à minimiser)

La fonction de coût J typique combine :

*   L'écart entre la trajectoire prédite et la trajectoire désirée
*   L'effort de commande nécessaire

$$J = \sum_{k=1}^{N} (X_k - X_{ref})^T Q (X_k - X_{ref}) + \sum_{k=0}^{N-1} u_k^T R u_k$$

Où :

*   $X_{ref}$ est l'état de référence (où nous voulons être)
*   $Q$ est une matrice de pondération pour l'écart d'état (importance de suivre la référence)
*   $R$ est une matrice de pondération pour l'effort de commande (importance d'économiser l'énergie)

### 4\. Contraintes

Nous avons des contraintes physiques :

*   Limites sur la position : $p_{min} \leq p_k \leq p_{max}$ (rester sur la route)
*   Limites sur la vitesse : $0 \leq v_k \leq v_{max}$ (limitations de vitesse)
*   Limites sur l'accélération : $a_{min} \leq a_k \leq a_{max}$ (confort et physique)
*   Limites sur la commande : $u_{min} \leq u_k \leq u_{max}$ (capacités du moteur/frein)

## Exemple concret de conduite autonome

Imaginons que notre voiture doit suivre une trajectoire prédéfinie tout en respectant les limites de vitesse :

1.  **État initial** : La voiture est à la position $p_0 = 0$ m, avec une vitesse $v_0 = 15$ m/s et une accélération $a_0 = 0$ m/s².
2.  **Objectif** : Atteindre la position $p_{ref} = 100$ m avec une vitesse $v_{ref} = 20$ m/s.
3.  **Contraintes** :
    *   Vitesse maximale : 25 m/s (90 km/h)
    *   Accélération entre -3 m/s² et 2 m/s²
    *   Force de commande entre -5000 N et 3000 N
4.  **Processus MPC** :
    *   À chaque pas de temps (par exemple, 0.1s) :
        *   Observer l'état actuel $X_0 = [p_0, v_0, a_0]^T$
        *   Prédire les trajectoires possibles sur les prochaines 3 secondes (horizon N = 30)
        *   Trouver la séquence de commandes $U$ qui minimise J tout en respectant les contraintes
        *   Appliquer uniquement $u_0$ (la première commande)
        *   Répéter au prochain pas de temps
5.  **Résolution mathématique** :
    *   Le problème devient un problème d'optimisation quadratique sous contraintes
    *   Les méthodes de résolution incluent l'algorithme du point intérieur, la méthode des ensembles actifs, etc.

## Adaptation au contexte réel

Dans un véhicule autonome réel, le MPC devient plus complexe :

*   Modèle à 2 dimensions (x, y) pour la navigation
*   Ajout de l'angle de direction comme variable de commande
*   Modèles non-linéaires pour mieux représenter la dynamique du véhicule
*   Couches supplémentaires pour l'évitement d'obstacles et la gestion du trafic

Ce qui distingue le MPC des autres méthodes est sa capacité à anticiper les événements futurs. Par exemple, si le MPC "voit" un virage qui approche, il commencera à ralentir avant même d'y arriver, tout comme le ferait un conducteur humain expérimenté.

## Explication du code de MPC pour une voiture

J'ai créé un code C++ qui implémente la commande prédictive pour le modèle de voiture que nous avons discuté. Voici les points clés de cette implémentation :

### Compilation

```bash
g++ --std=c++11 -Wall -Wextra vehicle_mpc.cpp -o vehicle_mpc
./vehicle_mpc > results.csv
python3 plot.py results.csv
```


### Structure du code

1.  **Modèle du véhicule** :
    *   État représenté par position, vitesse et accélération
    *   Dynamique basée sur les équations de mouvement avec force et frottement
    *   Contraintes physiques sur toutes les variables d'état et de commande
2.  **Classe `VehicleMPC`** :
    *   Gère la prédiction des états futurs selon le modèle dynamique
    *   Calcule le coût d'une séquence de commandes
    *   Optimise la commande pour minimiser la fonction de coût
3.  **Fonction de prédiction** :
    *   Implémente les équations :
        *   position ← position + vitesse × dt + ½ × accélération × dt²
        *   vitesse ← vitesse + accélération × dt
        *   accélération ← (force - frottement × vitesse) / masse
4.  **Fonction de coût** :
    *   Pénalise les écarts par rapport à la position cible, la vitesse cible et l'accélération cible
    *   Pénalise également l'effort de commande (force appliquée)
5.  **Optimisation** :
    *   Utilise une méthode de recherche avec raffinement progressif
    *   Commence par une recherche large puis affine autour des meilleures valeurs

### Paramètres de simulation

*   **Véhicule** : Masse de 1500 kg, coefficient de frottement de 50
*   **État initial** : Position 0 m, vitesse 15 m/s, accélération 0 m/s²
*   **Objectif** : Atteindre la position 100 m avec une vitesse de 20 m/s
*   **Contraintes** :
    *   Vitesse entre 0 et 25 m/s (0-90 km/h)
    *   Accélération entre -3 et 2 m/s²
    *   Force de commande entre -5000 et 3000 N

### Résultats attendus

Quand vous exécuterez ce code, vous verrez :

1.  La voiture accélérer progressivement pour atteindre la vitesse cible
2.  La position augmenter vers la cible de 100 m
3.  À mi-parcours (7,5 s), un changement de cible vers position 50 m et vitesse 10 m/s
4.  Le contrôleur s'adapter immédiatement à ce changement

### Points techniques importants

1.  **Horizon glissant** : À chaque pas de temps, nous recalculons la commande optimale
2.  **Principe du MPC** : On n'applique que la première commande calculée
3.  **Simplification** : L'optimisation réelle utiliserait des méthodes plus sophistiquées (comme la programmation quadratique)
4.  **Adaptabilité** : Le contrôleur réagit aux changements de référence en temps réel

### Application pratique

Dans un contexte réel, ce code pourrait être étendu pour :

*   Inclure le mouvement latéral (direction)
*   Prendre en compte les obstacles et autres véhicules
*   Utiliser des capteurs pour estimer l'état réel du véhicule
*   Implémenter des algorithmes d'optimisation plus performants