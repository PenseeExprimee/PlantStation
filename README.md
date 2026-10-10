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
| Design patterns | Strategy `include/plant/IWateringStrategy.hpp` `include/plant/WateringThreshold.hpp` `ìnclude/plant/WateringController.hpp`  |
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


## Comment est contrôlé l'arrosage?

L'idée principale est le fail-safe: en cas de doute, on n'arrose pas.
Le `WateringController` possède une stratégie d'arrosage et fonctionne comme suit:
- Si le contrôleur est crée sans stratégie: la plante n'est pas arrosée. Le rôle de l'alarme (pas encore implémentée) sera de détecter un système qui n'arrose jamais.
- Si le niveau d'eau du réservoir est strictement inférieur au seuil minimal d'eau requis (le seuil étant fixé à la construction du contrôleur), la plante n'est pas arrosée. En cas d'égalité, la stratégie décide. Confier la gestion du réservoir au contrôleur permet de ne pas l'oublier si l'on ajoute de nouvelles stratégies d'arrosage.
- Si le niveau est inconnu, par sécurité, la plante n'est pas arrosée.
- Sinon, la stratégie décide d'arroser ou non la plante.
- Il est possible de changer de stratégie d'arrosage en utilisant la méthode `changeStrategy` du contrôleur. Dans le cas où on tente de changer la stratégie d'arrosage par un pointeur nul, le système conserve la stratégie précédente (on considère que c'est une erreur de l'appelant, alors qu'à la construction, il n'y a pas d'ancienne stratégie à conserver).

Le contrôleur n'est pas copiable puisqu'il possède en attribut un unique_ptr vers la stratégie en cours. La présence de ce pointeur unique explique l'utilisation de std::move dans le constructeur et méthodes de classe, puisqu'un pointeur unique n'est pas copiable.

Pour tester le contrôleur, une `FakeStrategy` a été implémentée afin de tester le contrôleur indépendamment des autres stratégies. Le compteur partagé permet de vérifier que la stratégie n'est pas consultée lorsque des règles de sécurité doivent être appliquées.

Remarque: Le contrôleur se trouve dans une position d'attente pour des raisons diverses que l'appelant ne peut distinguer, d'où la nécessité d'un observateur (pas encore implémenté).


## Gestion des différentes configurations

La factory centralise la création des stratégies à partir d’une configuration. Elle retourne ensuite l’interface (pas la classe derrière). 
La factory comporte des contrôles de sécurité s'assurant que la configuration est valide (seuil compris entre 0 et 100 (bornes incluses) et différent de NaN ). Puisque cette validation se fait seulement dans la factory, elle doit être le point d'entrée. Construire directement WateringThreshold avec un NaN donnerait une stratégie qui arrose en permanence.
Lors de l'ajout d'une stratégie, on ajoute une classe et on met à jour la factory pour gérer cette nouvelle. (inutile de toucher au contrôleur.)
Dans le cas où une configuration invalide est passée à la factory, elle renvoie un pointeur nul (qui sera géré par le contrôleur). Une configuration invalide donne un contrôleur qui n’arrose jamais, silencieusement : c’est à l’alarme de le détecter (pas encore implémentée).
Aujourd'hui la factory permet surtout d'isoler le seul endroit qui connaît les classes concrètes.