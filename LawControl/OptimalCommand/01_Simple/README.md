# Commande Optimale

## Introduction à la méthode de Lagrange

La méthode des multiplicateurs de Lagrange sert à résoudre un problème d'optimisation avec des contraintes.

On veut **minimiser** (ou maximiser) une fonction $f(x, u)$ sous **contraintes** $g(x, u) = 0$.

Exemple :

> Trouver le point $(x, y)$ qui minimise $f(x, y) = x^2 + y^2$
> sous la contrainte : $g(x, y) = x + y - 1 = 0$

### Qu'est-ce qu'une contrainte ?

Imaginons qu'on veuille minimiser le coût de carburant sur un trajet. Mais aussi **arriver à l'heure**. Cette condition est une **contrainte** : on ne peut pas faire ce que l'on veut librement.

### Idée principale

Ne pouvant pas minimiser $f$ librement, on doit rester sur la ligne $x + y = 1$. L’idée est d’intégrer la contrainte dans le problème, en la combinant avec une fonction auxiliaire $\lambda$.

On écrit un **Lagrangien** :

$$\mathcal{L}(x, y, \lambda) = f(x, y) + \lambda \cdot g(x, y)$$

Ici :

$$\mathcal{L}(x, y, \lambda) = x^2 + y^2 + \lambda (x + y - 1)$$

Ensuite on cherche les points où tous les **dérivés (gradients)** sont nuls :

$$\nabla \mathcal{L} = 0 \quad \text{(système d'équations)}$$

On dérive par rapport à $x$, $y$, et $\lambda$, et on résout le système d'équations :

$$\frac{\partial \mathcal{L}}{\partial x} = 0, \quad \frac{\partial \mathcal{L}}{\partial y} = 0, \quad \frac{\partial \mathcal{L}}{\partial \lambda} = 0$$

C’est ça, la **méthode de Lagrange** : on transforme un problème à contraintes en un problème sans contrainte avec un multiplicateur $\lambda$.

* * *

## Commande optimale : la même idée, mais en dynamique

En **commande optimale**, on cherche une **commande $u(t)$** qui **minimise un coût** tout en respectant une **équation différentielle** (l’évolution de l’état) $\dot{x}(t) = f(x(t), u(t))$.

Exemple:

> On veut donc trouver une fonction $u(t)$ qui rend la somme des carrés de $u$ et de $x$ aussi petite que possible, tout en respectant la contrainte $\dot{x}(t) = u(t)$ avec une condition initiale $x(0)$ connue.

On veut minimiser $J$:

$$J = \int_0^T (x^2(t) + u^2(t)) \, dt$$

Ici :
- $x(t)$ est l’**état**
- $u(t)$ est le **contrôle** (la commande)
- La contrainte est **dynamique** : respecter l'équation $x' = u$

Note: On ajoute généralement une pénalité $\phi$ sur l'état final $x_T$:

$$J = \int_0^T (x^2(t) + u^2(t)) \, dt + \phi(x_T)$$

* * *

### Méthode de Pontryagin (ou Lagrange continue)

On fait comme avant : on construit un **Lagrangien**, mais en version continue. On introduit une fonction $\lambda(t)$ appelée **multiplicateur de Lagrange**, ou **coût adjoint** :

$$\mathcal{L}(x, u, \lambda) = x^2 + u^2 + \lambda ( \dot{x} - u )$$

On va construire un objet plus utile : le **Hamiltonien**.

$$H(x, u, \lambda) = x^2 + u^2 + \lambda \cdot f(x, u)$$

Ici, $f(x, u) = u$, donc :

$$H(x, u, \lambda) = x^2 + u^2 + \lambda u$$

Ensuite, on applique les équations suivantes :

1.  **Équation d’état :**

    $$\dot{x} = \frac{\partial H}{\partial \lambda} = u$$

2.  **Équation du coût adjoint** (on remonte dans le temps) :

    $$\dot{\lambda} = -\frac{\partial H}{\partial x} = -2x$$

3.  **Condition d’optimalité** (optimisation) :

    $$\frac{\partial H}{\partial u} = 2u + \lambda = 0 \Rightarrow u = -\frac{\lambda}{2}$$

#### Remarque: Pourquoi on remonte le temps ?

Parce que la condition sur $\lambda$ est donnée **à la fin** (ex : $\lambda(T) = 0$) !   On connaît $x(0) = 1$, mais pas $\lambda(0)$. C’est ce qu’on appelle un **problème à deux points** :

*   $x$ part de $t=0$

*   $\lambda$ part de $t=T$

On doit "tirer" dans le bon sens pour que ça colle à la fin.

#### Remarque: $\lambda$ **produit scalaire** ou **multiplication** ?

La notation:

$$\lambda(t) \cdot f(x(t), u(t))$$

- En dimension 1: est une **multiplication classique de scalaires**. Par exemple: $\lambda(t) \cdot (\dot{x}(t) - u(t))$ ce qu’on fait ici, c’est ajouter au coût une "pénalité" si $\dot{x} \ne u$, via ce produit.

- En dimension plus grande (ex : plusieurs états et contrôles): Si on avait un vecteur $x \in \mathbb{R}^n$, alors $\lambda$ serait aussi un vecteur et :

$$\lambda(t) \cdot f(x(t), u(t)) = \text{produit scalaire} = \sum_{i=1}^n \lambda_i(t) f_i(x(t), u(t))$$

* * *

### Résolution complète

On a donc ce système :

$$\dot{x} = u = -\frac{\lambda}{2}$$

$$\dot{\lambda} = -2x$$

Et on sait que $x(0) = 1$, on peut aussi imposer $\lambda(T) = 0$ comme condition terminale (typique en commande optimale).

On peut résoudre ce système numériquement (méthode de Runge-Kutta, Euler, etc.).

```cpp
#include <iostream>
#include <vector>
#include <cmath>

// Résolution avec Euler, pas super précis, mais ça illustre l'idée.
int main()
{
    const double T = 5.0;
    const int N = 1000;
    const double dt = T / N;

    std::vector<double> x(N + 1, 0.0);
    std::vector<double> lambda(N + 1, 0.0);
    std::vector<double> u(N + 1, 0.0);

    // Condition initiale x(0) = 1
    x[0] = 1.0;

    // Condition finale lambda(T) = 0
    lambda[N] = 0.0;

    // Backward Euler for lambda (remonter le temps)
    for (int i = N - 1; i >= 0; --i)
    {
        lambda[i] = lambda[i + 1] + dt * 2 * x[i + 1];  // \dot{\lambda} = -2x
    }

    // Forward Euler for x (avancer dans le temps)
    for (int i = 0; i < N; ++i)
    {
        u[i] = -lambda[i] / 2;              // optimalité : u = -lambda/2
        x[i + 1] = x[i] + dt * u[i];        // \dot{x} = u
    }

    // Affichage de quelques valeurs
    for (int i = 0; i <= N; i += N / 10)
    {
        std::cout << "t=" << i * dt
                  << ", x=" << x[i]
                  << ", u=" << u[i]
                  << ", lambda=" << lambda[i]
                  << std::endl;
    }

    return 0;
}
```

### 🧠 Commentaires :

*   `lambda[i] = lambda[i+1] + dt * 2 * x[i+1]` : c’est l’équation $\dot{\lambda} = -2x$

*   `u = -lambda/2` : c’est la condition d’optimalité

*   `x[i+1] = x[i] + dt * u[i]` : c’est l’évolution de l’état


* * *


🧩 8. Résumé
------------

| Concept | Rôle |
| --- | --- |
| Contrainte | Ce qu'on doit respecter (ici : $x'=u$) |
| Méthode de Lagrange | Ajoute un multiplicateur pour gérer les contraintes |
| Hamiltonien | Fonction pour combiner état, contrôle et adjoint |
| Gradient / dérivées | Trouver les minima (ou maxima) en résolvant des équations |
| Conditions initiales | Ce qu’on connaît au début (ex : x(0) = 1) |
| Conditions finales | Ce qu’on connaît à la fin (ex : $\lambda(T) = 0$) |
| Remonter le temps | Nécessaire car la contrainte sur $\lambda$ est fixée à la fin |