# 🌱 Plant Station

Station de surveillance de plante basée sur ESP32, avec un coeur en C++17
indépendant du matériel et entièrement testé.

![CI](https://github.com/<ton-pseudo>/plant-station/actions/workflows/ci.yml/badge.svg)

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
| Templates | _à compléter_ |
| Lambdas | _à compléter_ |
| C++ moderne | `std::optional`, `std::unique_ptr`, `override`, `final` |
| Tests unitaires | `tests/` |
