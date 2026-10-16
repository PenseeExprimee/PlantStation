# 🌱 Plant Station

Station de surveillance de plante conçue pour ESP32, avec un coeur en C++17
indépendant du matériel et testé avec GoogleTest.

![CI](https://github.com/PenseeExprimee/PlantStation/actions/workflows/ci.yml/badge.svg)

## Objectif
Mesurer l'humidité du sol et le niveau du réservoir, décider d'arroser
selon une stratégie configurable, historiser les événements et alerter.

## Démarrage rapide
Prérequis : CMake 3.14 ou plus, un compilateur C++17, et un accès internet à la première configuration (GoogleTest est téléchargé).
```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/plant_simulation
```

La sortie du programme est en anglais, comme le code:
```text
Cycle   Read humidity     Tank level      Average humidity      Alarm counter    Alarm state     Event text
-------------------------------------------------------------------------------------------------
1       20.00             80.00           20.00                 0                idle            The plant needs watering.
2       25.00             75.00           22.50                 0                idle            The plant needs watering.
3       45.00             70.00           30.00                 0                idle            No watering needed. Let's wait.
4       50.00             65.00           40.00                 0                idle            No watering needed. Let's wait.
5       15.00             60.00           36.67                 0                idle            No watering needed. Let's wait.
6       10.00             55.00           25.00                 0                idle            The plant needs watering.
7       18.00             10.00           14.33                 1                idle            Not enough water left in the tank.
8       12.00             5.00            13.33                 2                idle            Not enough water left in the tank.
9       15.00             90.00           15.00                 0                idle            The plant needs watering.
10      60.00             85.00           29.00                 0                idle            The plant needs watering.
11      55.00             80.00           43.33                 0                idle            No watering needed. Let's wait.
12      20.00             n/a             45.00                 1                idle            The level of the tank cannot be read.
13      10.00             n/a             28.33                 2                idle            The level of the tank cannot be read.
14      10.00             n/a             13.33                 3                raised          The level of the tank cannot be read.
15      5.00              65.00           8.33                  0                raised          The plant needs watering.
16      8.00              60.00           7.67                  0                raised          The plant needs watering.
--------------JOURNAL------------------
The plant needs watering.
The plant needs watering.
No watering needed. Let's wait.
The level of the tank cannot be read.
The level of the tank cannot be read.
The level of the tank cannot be read.
The plant needs watering.
The plant needs watering.
```
Alarme avant acquittement:

```
Alarm status: raised
Anomalies counter: 0
```
Alarme après acquittement:

```
Alarm status: idle
Anomalies counter: 0
```
Note: Le programme renvoie 0 s'il se termine sans erreur.
Tout est simulé : les capteurs lisent des valeurs prédéfinies, et rien ne pilote de pompe.

### Comprendre la sortie
- La décision porte sur la moyenne des trois dernières mesures d'humidité. 
Au cycle 3, la moyenne vaut exactement le seuil, on attend.
Aux cycles 5 et 10, la décision ne suit pas la lecture brute : le sol lu est sec au cycle 5, mais la moyenne des trois dernières mesures (36,67) reste au-dessus du seuil de 30. C’est le rôle du filtre : une mesure isolée ne fait pas basculer la décision.
Entre les cycles 12 et 14, les anomalies sont comptées par le compteur d'anomalies. L'alarme ne se lève qu'au cycle 14, puisque l'alarme se lève au bout de 3 anomalies consécutives.
Pendant les cycles 15 et 16, l'arrivée d'événements qui ne sont pas des anomalies permet la remise à 0 du compteur d'anomalies. Cependant, l'alarme reste levée puisque seul un acquittement peut la faire descendre.

- Le réservoir ne passe pas par le filtre : sinon average() renverrait la moyenne des anciennes valeurs quand le capteur se tait, UnknownTank ne se produirait jamais.

- L'alarme se lève au cycle 14, et reste levée jusqu'à l'acquittement.

### Auto-vérification
En plus d’afficher la sortie, le programme compare sa propre exécution au scénario attendu : les seize événements, le contenu du journal, le cycle où l’alarme se lève, l’état de l’alarme avant et après l’acquittement. Si quelque chose diffère, il l’écrit sur la sortie d’erreur et retourne un code non nul. CTest lance ce programme et le CI lance CTest : une règle cassée fait donc échouer le build.


## Architecture
- `src/` : Programme principal
- `include/plant/` : coeur logique (aucune dépendance matérielle)
- `tests/` : tests unitaires GoogleTest
- `firmware/` : couche ESP32 (à venir)
- `plant_simulation` : cible CMake construite à partir de `src/main.cpp`


## Notions mises en oeuvre
| Notion | Où |
|---|---|
| Design patterns | Strategy `include/plant/IWateringStrategy.hpp` `include/plant/WateringThreshold.hpp` `include/plant/WateringController.hpp` Factory `include/plant/StrategyFactory.hpp` Observer `include/plant/SubscribeHandler.hpp` `include/plant/WateringController.hpp` `include/plant/EventJournal.hpp` `include/plant/Alarm.hpp`|
| Templates | `include/plant/RingBuffer.hpp`, `include/plant/MovingAverage.hpp` `include/plant/EventJournal.hpp` `include/plant/Alarm.hpp`|
| Lambdas | `src/main.cpp`,`tests/test_controller.cpp` `tests/test_handleSubscribers.cpp` `tests/test_journal.cpp`, `tests/test_alarm.cpp` |
| C++ moderne | `std::optional`, `std::unique_ptr`, `override`, `final`, `static_assert`, `std::move`, `std::function`, `enum class`, `constexpr`, capture avec initialisation|
| Tests unitaires | `tests/` |


## Comment les mesures sont-elles acquises?
`ISensor` sert à inverser la dépendance vis-à-vis du matériel, pour que le coeur soit testable sans carte. Le capteur réel sera un Adapter autour de l’API de l’ESP32.
`FakeSensor` est un test double (un fake) : il remplace le vrai capteur dans les tests et alimente la simulation.
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

Le template `MovingAverage` possède un tampon `RingBuffer` (composition plutôt qu’héritage : un filtre n’est pas un tampon, et on ne veut pas exposer son interface) qui stocke les N dernières mesures, N étant la taille maximale du tampon. 
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
Pour éviter de noyer la plante, lorsqu'aucune mesure n'est disponible, on attend. Le système présente à ce jour une limite dans le cas où le capteur reste muet pendant des jours (cf. la section sur l’alarme).


## Comment est contrôlé l'arrosage?

L'idée principale est le fail-safe: en cas de doute, on n'arrose pas.
Le `WateringController` possède une stratégie d'arrosage et fonctionne comme suit:
- Si le contrôleur est créé sans stratégie: la plante n'est pas arrosée. Le rôle de l'alarme est de détecter un système qui n'arrose jamais.
- Si le niveau d'eau du réservoir est strictement inférieur au seuil minimal d'eau requis (le seuil étant fixé à la construction du contrôleur), la plante n'est pas arrosée. En cas d'égalité, la stratégie décide. Confier la gestion du réservoir au contrôleur permet de ne pas l'oublier si l'on ajoute de nouvelles stratégies d'arrosage.
- Si le niveau est inconnu, par sécurité, la plante n'est pas arrosée.
- Sinon, la stratégie décide d'arroser ou non la plante.
- Il est possible de changer de stratégie d'arrosage en utilisant la méthode `changeStrategy` du contrôleur. Dans le cas où on tente de changer la stratégie d'arrosage par un pointeur nul, le système conserve la stratégie précédente (on considère que c'est une erreur de l'appelant, alors qu'à la construction, il n'y a pas d'ancienne stratégie à conserver).

Le contrôleur n'est pas copiable puisqu'il possède en attribut un `unique_ptr` vers la stratégie en cours. La présence de ce pointeur unique explique l'utilisation de `std::move` dans le constructeur et méthodes de classe, puisqu'un pointeur unique n'est pas copiable.

Pour tester le contrôleur, une `FakeStrategy` a été implémentée afin de tester le contrôleur indépendamment des autres stratégies. Le compteur partagé permet de vérifier que la stratégie n'est pas consultée lorsque des règles de sécurité doivent être appliquées.

Remarque: Le contrôleur se trouve dans une position d'attente pour des raisons diverses que l'appelant ne peut distinguer. C’est pour cela que le contrôleur diffuse la raison de chaque décision à ses abonnés (cf. le paragraphe Comment la station réagit aux différents événements ).


## Gestion des différentes configurations

La factory centralise la création des stratégies à partir d’une configuration. Elle retourne ensuite l’interface (pas la classe derrière). 
La factory comporte des contrôles de sécurité s'assurant que la configuration est valide (seuil compris entre 0 et 100 (bornes incluses) et différent de NaN ). Puisque cette validation se fait seulement dans la factory, elle doit être le point d'entrée. Construire directement `WateringThreshold` avec un NaN donnerait une stratégie qui arrose en permanence.
Lors de l'ajout d'une stratégie, on ajoute une classe et on met à jour la factory pour gérer cette nouvelle. (inutile de toucher au contrôleur.)
Dans le cas où une configuration invalide est passée à la factory, elle renvoie un pointeur nul (qui sera géré par le contrôleur). Une configuration invalide donne un contrôleur qui n’arrose jamais, silencieusement : c’est à l’alarme de le détecter.
Aujourd'hui la factory permet surtout d'isoler le seul endroit qui connaît les classes concrètes.

## Comment la station réagit aux différents événements qui surviennent?

La station doit effectuer diverses actions en fonction de l'événement qui survient:

| Événement                                        | Nature      | Réaction                                |
| ------------------------------------------------ | ----------- | --------------------------------------- |
| `StrategyMissing`, `UnknownTank`, `LowWaterLevel`                | anomalies | à signaler à l’alarme                   |
| `StrategyDecisionWait` / `StrategyDecisionWater` | normal      | exécuter la décision                    |

Le contrôleur diffuse un événement exactement une fois par décision, et les tests utilisent des lambdas qui enregistrent les événements. Il est le seul à connaître la raison de l'attente ( absence de stratégie, niveau du réservoir, manque d'information sur le réservoir, décision de la stratégie).
Le contrôleur possède donc une liste d'abonnés à qui il diffuse l'événement survenu. Chaque abonné est une `std::function<void(WateringEvent)>` stockée dans un `std::vector<std::function<void(WateringEvent)>>`. L'utilisation de std::function permet de ranger toutes les lambdas représentant les abonnés dans la même boîte. 
Lors d'une diffusion, les abonnés sont appelés dans l'ordre d'abonnement, ce qui signifie qu'un abonné lent retarde les suivants.

Chacune de ces actions sera représentée par un observateur: celui-ci reçoit un événement transmis par le contrôleur et agit en conséquence.
A venir: la notification de réservoir bas, la pompe.

Limites: Pour cette version, il est impossible de se désabonner après abonnement. Cela signifie qu'une lambda qui capture une variable par référence appelle une référence invalide si la variable meurt avant le contrôleur.

## Le journal d'événements

Le journal d'événements est le premier véritable abonné de production (testé en tests et branché dans le programme complet). Il contient l'historique des événements `WateringEvent` (stockés dans un `RingBuffer`). Puisqu'il ne contient que des `WateringEvent`, le template est déclaré avec un paramètre non-type uniquement. Le journal possède une capacité N fixe et seuls les N derniers événements sont conservés. Il possède les méthodes suivantes:
- `save(WateringEvent event)` enregistre l'événement dans le journal.
- `size()` renvoie le nombre d'événements stockés dans le journal au moment de l'appel.
- `at(std::size_t index)` renvoie les événements stockés dans le journal dans l'ordre chronologique (index 0 représentant l'événement le plus ancien). Pour un index hors limites, la méthode renvoie `std::nullopt`.
Un abonné est stocké dans le `SubscribeHandler` que possède le contrôleur. Pour réaliser l'abonnement, on appelle la méthode `subscribe()` du contrôleur sur cette lambda. Le lien entre le journal et le contrôleur se fait uniquement par l'abonnement.

Que se passe-t-il quand un nouvel événement est diffusé?
La lambda abonnée capture le journal par référence, et le met à jour chaque fois qu'un nouvel événement est reçu. Le journal doit donc survivre au contrôleur, sinon la prochaine diffusion appelle une référence invalide.


## L'alarme
L'alarme est le second abonné de production. Elle permet au système de déceler des anomalies et de lever un drapeau.
Pour cette version, l'alarme lève un drapeau mais à ce jour, elle est seulement affichée par la simulation. (prévu: implémentation d'une couche matérielle avec un buzzer ou une LED).

L'alarme étant un template, elle doit être créée avec un seuil N `Alarm<N>`, N représentant le nombre d'anomalies consécutives au-delà duquel (valeur seuil incluse) l'alarme sera déclenchée. Le seuil de l’alarme est un paramètre de template parce que c’est un entier fixé à la compilation, ce qui permet de refuser 0 avec `static_assert` ; s’il devait venir de la configuration, il faudrait le passer au constructeur et valider à l’exécution, comme pour la stratégie.

Les anomalies correspondent aux événements:
- `LowWaterLevel`
- `UnknownTank`
- `StrategyMissing`
Un compteur appartenant à l'alarme représente le nombre d'anomalies consécutives survenues. Voici son fonctionnement:
- Dans le cas où un événement qui n'est pas une anomalie survient, l'alarme si elle était levée, reste levée. Il faudra utiliser la méthode `acknowledge()` de l'alarme pour explicitement éteindre l'alarme. 
- Le compteur propre à l'alarme continue de compter tant que l’alarme est levée.
- Un acquittement sur une alarme non levée ne change rien.
- Un événement normal remet le compteur à zéro, mais ne baisse pas l’alarme.
- Le terme "consécutives" désigne des décisions consécutives (pas le temps écoulé).

Limite: 
- L'alarme ne couvre pas le cas d'un capteur muet. Imaginons que le capteur d’humidité ne donne plus rien. `WateringThreshold` répond `Wait` quand l’humidité est absente. Le contrôleur diffuse alors `StrategyDecisionWait`, qui est un événement normal, donc l’alarme remet son compteur à zéro. L’alarme ne se lèvera jamais, et la plante ne sera jamais arrosée.
Solution envisagée: Ajouter un événement `UnknownHumidity`. Cela impliquerait qu'en plus de vérifier le niveau du réservoir avant de déléguer la prise de décision à la stratégie, le contrôleur doit également vérifier que l'humidité est connue. Cela ne pose pas de problème si les stratégies à implémenter dans le futur utilisent toutes le niveau d'humidité pour prendre une décision.

Comme le journal, l'alarme doit survivre au contrôleur.

## Reste à faire
- Désabonnement
- Acquittement physique (bouton)
- Couche matérielle (ESP32 et capteurs)