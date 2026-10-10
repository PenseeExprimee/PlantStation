# 🌱 Plant Station

Station de surveillance de plante basée sur ESP32, avec un coeur en C++17
indépendant du matériel et testé avec GoogleTest.

![CI](https://github.com/PenseeExprimee/PlantStation/actions/workflows/ci.yml/badge.svg)

## Objectif
Mesurer l'humidité du sol et le niveau du réservoir, décider d'arroser
selon une stratégie configurable, historiser les mesures et alerter.

## Architecture
- `include/plant/` : coeur logique (aucune dépendance matérielle)
- `tests/` : tests unitaires GoogleTest
- `firmware/` : couche ESP32 (à venir)

## Build et tests
```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Notions mises en oeuvre
| Notion | Où |
|---|---|
| Design patterns | Strategy `include/plant/IWateringStrategy.hpp` `include/plant/WateringThreshold.hpp`  |
| Templates | `include/plant/RingBuffer.hpp`, `include/plant/MovingAverage.hpp` |
| Lambdas | (à venir dans le code de la station) |
| C++ moderne | `std::optional`, `std::unique_ptr`, `override`, `final` |
| Tests unitaires | `tests/` |


## Comment les mesures sont-elles acquises?
`ISensor` sert à inverser la dépendance vis-à-vis du matériel, pour que le cœur soit testable sans carte. Le capteur réel sera un Adapter autour de l’API de l’ESP32.
`FakeSensor` est un test double (un fake) : il remplace le vrai capteur dans les tests.
Ainsi le coeur dépend d'une interface et non pas du matériel.

## Où sont conservées les mesures?

Le template `RingBuffer` conserve les N dernières mesures de type T récemment réalisées. L'utilisation d'un template de classe permet de réutiliser `RingBuffer` pour des données de différents types (entier, double...).
Lorsque le buffer est plein, on écrase la valeur la plus ancienne pour la remplacer par une valeur plus récente.

La taille du buffer est déterminée à la compilation (utilisation de std::array). La mémoire
nécessaire au stockage est prévisible, aucun agrandissement du buffer ne se fait après la compilation.

La classe fournit:
- `.size()` renvoyant le nombre de mesures présentes dans l'historique, 
- `.full()` et `.empty()` renvoient l'état de l'historique (plein, vide).
- `.at(std::size_t index)`, permettant de consulter chaque mesure présente dans le tampon. La mesure la plus ancienne étant toujours (index 0). 
Dans le cas où un index hors limite est demandé, la méthode `.at(std::size_t index)` renverra `std::nullopt`. On évite ainsi de faire appel aux exceptions (idéal dans un contexte embarqué, et on reste cohérent avec `ISensor::read()` qui renvoie également un std::optional).
- `.add()` pour ajouter une mesure au tampon.

Le type T doit avoir un constructeur par défaut, sans quoi le code ne compilera pas.

## Comment sont traitées les mesures?

Le template `MovingAverage` possède un tampon `RingBuffer` (ce n'est pas un tampon, inutile d'exposer l'interface de `RingBuffer`) qui stocke les N dernières mesures, N étant la taille maximale du tampon. 
La classe fournit:

- `add(const T &measure)` qui ajoute une nouvelle mesure au tampon.
- `average()` qui calcule la moyenne des mesures disponibles dans le tampon. Cette méthode renvoie
`std::nullopt` dans le cas où le tampon est vide. La somme de tous les éléments est calculée comme un double (puisqu'une moyenne d'entiers n'est pas toujours un entier).

## Stratégie d'arrosage

Une politique d'arrosage a été implémentée pour le moment. Sans pattern, chaque nouvelle politique obligerait à modifier un `switch` déjà testé; avec Strategy, on ajoute une classe sans toucher à l’existant.
Chacune des stratégies d'arrosage prend en argument une structure `WateringContext`, permettant d'ajouter des informations au besoin, sans changer l'interface.
- `WateringThreshold`: on définit un seuil en deça duquel on arrose et au-delà duquel on ne fait rien. Au seuil, on attend également.
L'utilisation d'un `enum class` plutôt qu'un booléen permettra dans le futur l'ajout d'autres décisions.
Le seuil est fixé à la construction puisqu'un double n'est pas accepté en paramètre de template en C++17. De plus, un paramètre de template doit être connu à la compilation.
Pour éviter de noyer la plante, lorsqu'aucune mesure n'est disponible, on attend. Si le capteur reste muet pendant des jours, ce sera à l'alarme de le signaler et agir en conséquence. (alarme prévue)
