# 🌱 Plant Station

Station de surveillance de plante basée sur ESP32, avec un coeur en C++17
indépendant du matériel et entièrement testé.

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
| Design patterns | _à compléter_ |
| Templates | `include/plant/RingBuffer.hpp`, `include/plant/MovingAverage.hpp` |
| Lambdas | _à compléter_ |
| C++ moderne | `std::optional`, `std::unique_ptr`, `override`, `final` |
| Tests unitaires | `tests/` |


## Où sont conservées les mesures?

Le template RingBuffer conserve les N dernières mesures de type T récemment réalisées. L'utilisation d'un template de classe permet de réutiliser RingBuffer pour des données de différents types (entier, double...).
Lorsque le buffer est plein, on écrase la valeur la plus ancienne pour la remplacer par une valeur plus récente.

La taille du buffer est déterminée à la compilation (utilisation de std::array). La mémoire
nécessaire au stockage est prévisible, aucun agrandissement du buffer ne se fait après la compilation.

La classe fournit:
- `.size()` renvoyant le nombre de mesures présentes dans l'historique, 
_ `.full()` et `.empty()` renvoient l'état de l'historique (plein, vide).
_ `.at(std::size_t index)`, permettant de consulter chaque mesure présente dans le tampon. La mesure la plus ancienne étant toujours (index 0).Dans le cas où un index hors limite est demandé, la méthode `.at(std::size_t index)` renverra `std::nullopt`. On évite ainsi de faire appel aux exceptions (idéal dans un contexte embarqué, et on reste cohérent avec `ISensor::read()` qui renvoie également un std::optional).
_ `.add()` pour ajouter une mesure au tampon.

Le type T doit avoir un constructeur par défaut, sans quoi le code ne compilera pas.

## Comment sont traitées les mesures?

Le template MovingAverage contient un tampon (RingBuffer) qui stocke les N dernières mesures, N étant la taille maximale du tampon. 
La classe fournit:
- `add(const T &measure)` qui ajoute une nouvelle mesure au tampon.
_ `average()` qui calcule la moyenne des measures présentent dans le tampon. Cette méthode renvoie
`std::nullopt` dans le cas où le tampon est vide. La somme de tous les élements est calculée comme un double.