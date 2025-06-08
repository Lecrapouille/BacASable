# Commande Optimale : Guide Théorique et Pratique

## Table des matières
1. [Fondements théoriques](#1-fondements-théoriques)
   - 1.1 [La méthode des multiplicateurs de Lagrange](#11-la-méthode-des-multiplicateurs-de-lagrange)
   - 1.2 [Cas de contraintes multiples](#12-cas-de-contraintes-multiples)
2. [De l'optimisation classique à la commande optimale](#2-de-loptimisation-classique-à-la-commande-optimale)
3. [Le principe du maximum de Pontryagin](#3-le-principe-du-maximum-de-pontryagin)
   - 3.1 [Construction du Hamiltonien](#31-construction-du-hamiltonien)
   - 3.2 [Conditions nécessaires d'optimalité](#32-conditions-nécessaires-doptimalité)
   - 3.3 [Interprétation du vecteur adjoint](#33-interprétation-du-vecteur-adjoint)
   - 3.4 [Dérivation par intégration par parties](#34-dérivation-par-intégration-par-parties)
4. [Exemples d'application](#4-exemples-dapplication)
5. [Implémentation numérique](#5-implémentation-numérique)

---

## 1. Fondements théoriques

### 1.1 La méthode des multiplicateurs de Lagrange

#### Problème général

Soit un problème d'optimisation avec contraintes :
- On cherche à **minimiser** la fonction objectif $f(x, u)$
- **sous la contrainte** de $g(x, u) = 0$.

**Terminologie** :
- **Problème primal** : le problème d'optimisation original à résoudre. *Ex: minimiser le coût de carburant d'un trajet.*
- **Problème dual** : transformation du problème via multiplicateurs $\lambda$. *Ex: $\lambda$ = impact marginal du temps, de l'énergie.*
- **Variables de contrôle** $u$ : ce qu'on peut choisir/ajuster (nos degrés de liberté). *Ex: accélération, débit, investissement.*
- **Variables d'état** $x$ : ce qui évolue selon les lois du système. *Ex: position, vitesse, température, capital.*
- **Contraintes** : conditions à respecter (égalités, inégalités, dynamiques). *Ex: arriver à l'heure, ne pas dépasser la vitesse limite.*

#### Méthode de résolution

1. **Construction du Lagrangien** :
   $$\mathcal{L}(x, u, \lambda) = f(x, u) + \lambda \cdot g(x, u)$$

   où $\lambda$ est le multiplicateur de Lagrange.

   **Note** : Le symbole $\cdot$ représente la **multiplication**, ici entre le scalaire $\lambda$ et l'expression $g(x, u)$.

2. **Conditions d'optimalité** :
   $$\nabla \mathcal{L} = 0$$

   Ce qui donne le système :
   $$\frac{\partial \mathcal{L}}{\partial x} = 0, \quad \frac{\partial \mathcal{L}}{\partial u} = 0, \quad \frac{\partial \mathcal{L}}{\partial \lambda} = 0$$

#### Interprétation physique

Le multiplicateur $\lambda$ représente le **coût marginal** d'imposer la contrainte. Il quantifie l'effet de la contrainte sur la fonction objectif.

**Problème dual** : En introduisant $\lambda$, on transforme le problème avec contraintes en un problème sans contraintes. $\lambda$ mesure l'**impact marginal** de la contrainte.

### 1.2 Cas de contraintes multiples

#### Problème général avec contraintes multiples
Lorsqu'il y a plusieurs contraintes, on étend naturellement la méthode :

- **Minimiser** : $f(x, u)$ (fonction objectif)
- **Sous contraintes** : $g_1(x, u) = 0, g_2(x, u) = 0, \ldots, g_p(x, u) = 0$

#### Lagrangien généralisé

On introduit un multiplicateur par contrainte :

$$\mathcal{L}(x, u, \lambda_1, \lambda_2, \ldots, \lambda_p) = f(x, u) + \sum_{i=1}^p \lambda_i \cdot g_i(x, u)$$

#### Notation vectorielle

En posant $\mathbf{g}(x,u) = [g_1(x,u), g_2(x,u), \ldots, g_p(x,u)]^T$ et $\boldsymbol{\lambda} = [\lambda_1, \lambda_2, \ldots, \lambda_p]^T$ :

$$\mathcal{L}(x, u, \boldsymbol{\lambda}) = f(x, u) + \boldsymbol{\lambda}^T \cdot \mathbf{g}(x, u)$$

**Note:** Le symbole $\cdot$ représente le produit scalaire. Le symbole $^T$ représente la **transposée**, qui transforme un vecteur ligne en vecteur colonne.

#### Conditions d'optimalité

Le système à résoudre devient :

$$\frac{\partial \mathcal{L}}{\partial x} = 0, \quad \frac{\partial \mathcal{L}}{\partial u} = 0, \quad \frac{\partial \mathcal{L}}{\partial \lambda_i} = 0 \quad \forall i = 1, \ldots, p$$

---

## 2. De l'optimisation classique à la commande optimale

### 2.1 Différences fondamentales

| Aspect | Optimisation classique | Commande optimale |
|--------|----------------------|-------------------|
| **Variables** | Valeurs fixes (sans temps) | Fonctions qui évoluent dans le temps |
| **Contraintes** | Équations algébriques | Équations différentielles |
| **Horizon** | Instantané | Sur une période de temps |
| **Complexité** | Système d'équations | Équations différentielles couplées |

### 2.2 Formulation du problème de commande optimale

#### Éléments du problème

- **État** : $x(t) \in \mathbb{R}^n$ (évolution du système)
- **Commande** : $u(t) \in \mathbb{R}^m$ (variables de décision)
- **Dynamique** : $\dot{x}(t) = f(x(t), u(t))$ (contrainte dynamique)
- **Horizon temporel** : $t \in [0, T]$

#### Fonction coût générale

$$J = \int_0^T L(x(t), u(t)) \, dt + \phi(x(T))$$

où :
- $L(x, u)$ : coût instantané (Lagrangien), par exemple $L(x, u) = q x^2 + r u^2$.
- $\phi(x(T))$ : coût terminal (quand $t = T$).

#### Contraintes

- **Dynamique** : $\dot{x}(t) = f(x(t), u(t))$
- **Conditions initiales** : $x(0) = x_0$
- **Conditions finales** : $\psi(x(T)) = 0$ (optionnel)
- **Contraintes sur les commandes** : $u(t) \in U$ (optionnel)

---

## 3. Le principe du maximum de Pontryagin

### 3.1 Construction du Hamiltonien

#### Du Lagrangien au Hamiltonien : pourquoi changer ?

**Problème avec le Lagrangien classique** :
- Dans les sections précédentes : $\mathcal{L}(x, u, \lambda) = f(x, u) + \lambda \cdot g(x, u)$
- Fonctionne bien pour l'optimisation **sans temps**
- Mais en commande optimale, la contrainte est **dynamique** : $\dot{x} = f(x, u)$

**Solution : le Hamiltonien** :
- Adapte le Lagrangien aux problèmes **avec évolution temporelle**
- Traite spécifiquement les contraintes de type $\dot{x} = f(x, u)$

#### Construction du Hamiltonien

Le Hamiltonien généralise le Lagrangien pour les systèmes dynamiques :

$$H(x, u, \boldsymbol{\lambda}, t) = L(x, u) + \boldsymbol{\lambda}^T f(x, u)$$

où $\boldsymbol{\lambda}(t) \in \mathbb{R}^n$ est le **vecteur adjoint** (multiplicateur de Lagrange temporel).

#### Comparaison Lagrangien vs Hamiltonien

| Aspect | Lagrangien classique | Hamiltonien |
|--------|---------------------|-------------|
| **Contrainte** | $g(x, u) = 0$ (algébrique) | $\dot{x} = f(x, u)$ (différentielle) |
| **Formule** | $\mathcal{L} = \text{coût} + \lambda \cdot \text{contrainte}$ | $H = \text{coût} + \lambda \cdot \text{dynamique}$ |
| **Multiplicateur** | $\lambda$ (constant) | $\lambda(t)$ (fonction du temps) |
| **Application** | Optimisation statique | Commande optimale |

**L'idée clé** : Le Hamiltonien $H$ remplace simplement la contrainte algébrique $g(x,u)$ par la dynamique $f(x,u)$ !

#### Cas avec états multiples

Si le système a plusieurs états $x_i(t)$ et donc plusieurs équations dynamiques $\dot{x}_i = f_i(x, u)$ pour $i = 1, \ldots, n$, alors :

- **Vecteur d'état** : $\mathbf{x}(t) = [x_1(t), x_2(t), \ldots, x_n(t)]^T$
- **Vecteur adjoint** : $\boldsymbol{\lambda}(t) = [\lambda_1(t), \lambda_2(t), \ldots, \lambda_n(t)]^T$
- **Dynamique vectorielle** : $\mathbf{f}(x,u) = [f_1(x,u), f_2(x,u), \ldots, f_n(x,u)]^T$

Le Hamiltonien devient :
$$H(\mathbf{x}, u, \boldsymbol{\lambda}, t) = L(\mathbf{x}, u) + \boldsymbol{\lambda}^T \mathbf{f}(\mathbf{x}, u) = L(\mathbf{x}, u) + \sum_{i=1}^n \lambda_i f_i(\mathbf{x}, u)$$

### 3.2 Conditions nécessaires d'optimalité

#### Équations d'état: Vers l'avant dans le temps (forward)
$$\dot{x}(t) = \frac{\partial H}{\partial \lambda} = f(x, u)$$

- On connaît $x(0)$ (condition initiale).
- On intègre de $t = 0$ vers $t = T$.
- $x$ évolue naturellement du passé vers le futur.

#### Équations adjointes: Vers l'arrière dans le temps (backward)
$$\dot{\lambda}(t) = -\frac{\partial H}{\partial x}$$

- On connaît $\lambda(T)$ (condition finale).
- On intègre de $t = T$ vers $t = 0$.
- $\lambda$ "remonte" du futur vers le passé.

#### Illustration du sens d'intégration

```
États x(t) :     x(0) ────────────────► x(T)
                 donné                 calculé
                 (Forward: t = 0 → T)

Adjoints λ(t) :  λ(0) ◄──────────────── λ(T)
                 calculé               donné
                 (Backward: t = T → 0)
```

**Pourquoi cette différence ?**
- **États** : évolution naturelle d'un système physique (du présent vers le futur)
- **Adjoints** : "prix" calculé en remontant depuis l'objectif final (du futur vers le présent)

#### Condition d'optimalité sur la commande
$$\frac{\partial H}{\partial u} = 0 \quad \text{ou} \quad u^*(t) = \arg\min_u H(x, u, \lambda, t)$$

#### Conditions aux limites
- **Initiales** : $x(0) = x_0$
- **Finales** : $\lambda(T) = \frac{\partial \phi}{\partial x}\bigg|_{x=x(T)}$

### 3.3 Interprétation du vecteur adjoint
- $\lambda_i(t)$ représente la **valeur marginale** de l'état $x_i$ à l'instant $t$
- Il indique l'impact d'une variation infinitésimale de $x_i(t)$ sur le coût total
- C'est le "prix" associé à chaque composante de l'état

### 3.4 Méthode de résolution: intégration par parties

L'intégration par parties est une méthode fondamentale pour dériver les conditions d'optimalité en commande optimale. Elle permet de passer des variations d'état aux variations de commande.

#### Rappel : Qu'est-ce que l'intégration par parties ?

**Principe** : Transformer une intégrale difficile en une intégrale plus simple en "transférant" la dérivation d'une fonction à l'autre.

**Formule générale** :
$$\int u \, dv = uv - \int v \, du$$

ou encore :
$$\int_a^b u(x) v'(x) \, dx = u(x)v(x)\Big|_a^b - \int_a^b u'(x) v(x) \, dx$$

#### Pourquoi utiliser l'IPP en commande optimale ?

**Le problème** :
- On veut optimiser par rapport à la commande $u(t)$
- Mais on a des termes avec $\delta \dot{x}(t)$ (dérivée de l'état) dans nos variations
- Or $\delta \dot{x}(t)$ dépend de $\delta u(t)$, ce qui complique l'analyse

**La solution par IPP** :
- **Éliminer** les termes $\delta \dot{x}(t)$ gênants
- **Transférer** la dérivation de $\delta x$ vers $\lambda$
- **Obtenir** une expression où $\delta u(t)$ apparaît directement

#### Analogie concrète

**Pensez à** : $\int_0^T \lambda(t) \cdot \delta \dot{x}(t) \, dt$

- **Sans IPP** : $\delta \dot{x}(t)$ est la dérivée de quelque chose qu'on ne contrôle pas directement
- **Avec IPP** : On transforme en $\lambda(T)\delta x(T) - \int_0^T \dot{\lambda}(t) \delta x(t) \, dt$
- **Résultat** : Plus de dérivée de $\delta x$, mais $\lambda$ qui dérive à la place !

**C'est comme** passer de "vitesse de changement de $x$" à "changement de vitesse de $\lambda$".

#### Principe de la variation
Considérons une petite perturbation $\delta u(t)$ de la commande optimale. Cette perturbation induit une variation $\delta x(t)$ de l'état via :

$$\delta \dot{x}(t) = \frac{\partial f}{\partial x}\delta x(t) + \frac{\partial f}{\partial u}\delta u(t)$$

#### Application de l'intégration par parties
La variation du coût s'écrit :
$$\delta J = \int_0^T \left( \frac{\partial L}{\partial x}\delta x + \frac{\partial L}{\partial u}\delta u \right) dt + \frac{\partial \phi}{\partial x}\bigg|_{x=x(T)} \delta x(T)$$

Pour éliminer les variations d'état $\delta x(t)$, on utilise l'intégration par parties :

1. **Étape 1** : Introduire le multiplicateur $\lambda(t)$ :
   $$\int_0^T \lambda(t) \left[ \delta \dot{x}(t) - \frac{\partial f}{\partial x}\delta x(t) - \frac{\partial f}{\partial u}\delta u(t) \right] dt = 0$$

2. **Étape 2** : Intégrer par parties le terme $\lambda(t) \delta \dot{x}(t)$ :
   $$\int_0^T \lambda(t) \delta \dot{x}(t) dt = \lambda(T)\delta x(T) - \lambda(0)\delta x(0) - \int_0^T \dot{\lambda}(t) \delta x(t) dt$$

3. **Étape 3** : Regrouper les termes en $\delta x(t)$ :
   $$\int_0^T \left[ \frac{\partial L}{\partial x} - \lambda \frac{\partial f}{\partial x} + \dot{\lambda} \right] \delta x(t) dt$$

#### Conditions d'optimalité dérivées
Pour que cette expression soit nulle pour toute variation admissible :

1. **Équation adjointe** :
   $$\dot{\lambda}(t) = -\frac{\partial L}{\partial x} + \lambda \frac{\partial f}{\partial x} = -\frac{\partial H}{\partial x}$$

2. **Condition de transversalité** :
   $$\lambda(T) = \frac{\partial \phi}{\partial x}\bigg|_{x=x(T)}$$

3. **Condition d'optimalité sur la commande** :
   $$\frac{\partial L}{\partial u} + \lambda \frac{\partial f}{\partial u} = \frac{\partial H}{\partial u} = 0$$

#### Illustration : Cas du régulateur quadratique
Pour le problème $J = \int_0^T (x^2 + u^2) dt$ avec $\dot{x} = u$ :

1. **Hamiltonien** : $H = x^2 + u^2 + \lambda u$

2. **Application de l'intégration par parties** :
   - $\delta J = \int_0^T (2x \delta x + 2u \delta u) dt$
   - Contrainte : $\delta \dot{x} = \delta u$
   - Terme de couplage : $\int_0^T \lambda (\delta \dot{x} - \delta u) dt = 0$

3. **Intégration par parties** :
   $$\int_0^T \lambda \delta \dot{x} dt = \lambda(T)\delta x(T) - \int_0^T \dot{\lambda} \delta x dt$$

4. **Regroupement** :
   $$\delta J = \int_0^T [(2x + \dot{\lambda})\delta x + (2u - \lambda)\delta u] dt + \lambda(T)\delta x(T)$$

5. **Conditions d'optimalité** :
   - $\dot{\lambda} = -2x$ (coefficient de $\delta x = 0$)
   - $2u - \lambda = 0 \Rightarrow u = \lambda/2$ (coefficient de $\delta u = 0$)
   - $\lambda(T) = 0$ (condition de transversalité)

#### Avantages de cette méthode
- **Compréhension conceptuelle** : Montre comment les variations se propagent
- **Généralisation** : S'étend facilement aux problèmes à contraintes multiples
- **Conditions de transversalité** : Dérive naturellement les conditions aux limites
- **Lien avec le calcul des variations** : Connecte à la théorie classique d'Euler-Lagrange

---

## 4. Exemples d'application

### 4.1 Exemple simple : Régulateur quadratique

#### Énoncé du problème
Minimiser le coût :
$$J = \int_0^T (x^2(t) + u^2(t)) \, dt$$

sous la contrainte dynamique :
$$\dot{x}(t) = u(t), \quad x(0) = x_0$$

#### Application de la méthode

**Étape 1 : Construction du Hamiltonien**
$$H(x, u, \lambda) = x^2 + u^2 + \lambda u$$

**Étape 2 : Équations d'état et adjointes**
- Équation d'état : $\dot{x} = \frac{\partial H}{\partial \lambda} = u$
- Équation adjointe : $\dot{\lambda} = -\frac{\partial H}{\partial x} = -2x$

**Étape 3 : Condition d'optimalité**
$$\frac{\partial H}{\partial u} = 2u + \lambda = 0 \Rightarrow u^* = -\frac{\lambda}{2}$$

**Étape 4 : Système final**
$$\begin{cases}
\dot{x} = -\frac{\lambda}{2} \\
\dot{\lambda} = -2x \\
x(0) = x_0 \\
\lambda(T) = 0
\end{cases}$$

### 4.2 Exemple avancé : Contrôle d'un oscillateur

#### Énoncé du problème
Contrôler un système masse-ressort-amortisseur :
$$\ddot{x} + 2\zeta\omega_n\dot{x} + \omega_n^2 x = u$$

en minimisant :
$$J = \int_0^T (q x^2 + r \dot{x}^2 + u^2) \, dt$$

#### Formulation en espace d'état
Poser $x_1 = x$ et $x_2 = \dot{x}$ :
$$\begin{cases}
\dot{x_1} = x_2 \\
\dot{x_2} = -2\zeta\omega_n x_2 - \omega_n^2 x_1 + u
\end{cases}$$

#### Hamiltonien
$$H = q x_1^2 + r x_2^2 + u^2 + \lambda_1 x_2 + \lambda_2(-2\zeta\omega_n x_2 - \omega_n^2 x_1 + u)$$

#### Conditions d'optimalité
- $\frac{\partial H}{\partial u} = 2u + \lambda_2 = 0 \Rightarrow u^* = -\frac{\lambda_2}{2}$
- $\dot{\lambda_1} = -2q x_1 + \omega_n^2 \lambda_2$
- $\dot{\lambda_2} = -2r x_2 - \lambda_1 + 2\zeta\omega_n \lambda_2$

---

## 5. Implémentation numérique

### 5.1 Défis numériques

#### Problème à deux points
- Les conditions initiales sont données en $t = 0$ pour $x(t)$
- Les conditions finales sont données en $t = T$ pour $\lambda(t)$
- Nécessite des méthodes de **shooting** ou de **collocation**

#### Méthodes de résolution

**Méthode du shooting simple**
1. Deviner $\lambda(0)$
2. Intégrer le système forward
3. Vérifier si $\lambda(T) = 0$
4. Ajuster $\lambda(0)$ jusqu'à convergence

**Méthode d'Euler backward-forward**
```cpp
// Pseudo-algorithme
for (int iter = 0; iter < max_iter; ++iter) {
    // Forward integration for x
    for (int i = 0; i < N; ++i) {
        u[i] = -lambda[i] / 2;
        x[i+1] = x[i] + dt * u[i];
    }

    // Backward integration for lambda
    lambda[N] = 0.0;  // Condition finale
    for (int i = N-1; i >= 0; --i) {
        lambda[i] = lambda[i+1] + dt * (-2 * x[i+1]);
    }

    // Check convergence
    if (convergence_test()) break;
}
```

### 5.2 Exemple d'implémentation : Régulateur quadratique

```cpp
#include <iostream>
#include <vector>
#include <cmath>

class OptimalControlSolver {
private:
    const double T;           // Horizon temporel
    const int N;              // Nombre de points
    const double dt;          // Pas de temps
    const double x0;          // Condition initiale

    std::vector<double> x, lambda, u;

public:
    OptimalControlSolver(double T_val, int N_val, double x0_val)
        : T(T_val), N(N_val), dt(T/N), x0(x0_val),
          x(N+1), lambda(N+1), u(N+1) {}

    void solve() {
        // Initialisation
        x[0] = x0;
        lambda[N] = 0.0;  // Condition terminale

        // Itérations pour résoudre le problème à deux points
        for (int iter = 0; iter < 100; ++iter) {
            // Intégration forward pour x
            for (int i = 0; i < N; ++i) {
                u[i] = -lambda[i] / 2.0;
                x[i+1] = x[i] + dt * u[i];
            }

            // Intégration backward pour lambda
            for (int i = N-1; i >= 0; --i) {
                lambda[i] = lambda[i+1] + dt * 2.0 * x[i+1];
            }
        }
    }

    void printResults() const {
        std::cout << "Résultats de la commande optimale:\n";
        std::cout << "t\t\tx(t)\t\tu(t)\t\tlambda(t)\n";
        for (int i = 0; i <= N; i += N/10) {
            printf("%.2f\t\t%.4f\t\t%.4f\t\t%.4f\n",
                   i*dt, x[i], u[i], lambda[i]);
        }
    }

    double computeCost() const {
        double cost = 0.0;
        for (int i = 0; i < N; ++i) {
            cost += dt * (x[i]*x[i] + u[i]*u[i]);
        }
        return cost;
    }
};

int main() {
    OptimalControlSolver solver(5.0, 1000, 1.0);
    solver.solve();
    solver.printResults();

    std::cout << "\nCoût total: " << solver.computeCost() << std::endl;

    return 0;
}
```

---

## 6. Résumé et perspectives

### 6.1 Points clés à retenir

| Concept | Rôle | Interprétation |
|---------|------|----------------|
| **Hamiltonien** | Fonction centrale combinant coût et dynamique | Généralise le Lagrangien |
| **Vecteur adjoint** | Prix marginal des états | Importance économique de chaque état |
| **Principe du maximum** | Conditions d'optimalité | Généralise les conditions de Lagrange |
| **Problème à deux points** | Défi numérique principal | Conditions aux deux extrémités temporelles |

### 6.2 Extensions possibles
- **Commande optimale stochastique** (équations de Hamilton-Jacobi-Bellman)
- **Commande robuste** (incertitudes paramétriques)
- **Commande optimale à horizon infini** (régulateurs LQR)
- **Contraintes d'inégalité** (commande bang-bang)

### 6.3 Applications industrielles
- **Aérospatiale** : trajectoires optimales de satellites
- **Robotique** : planification de mouvements
- **Économie** : théorie de la croissance optimale
- **Ingénierie** : régulation de procédés industriels