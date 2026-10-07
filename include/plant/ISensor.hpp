#pragma once
#include <optional>
#include <string>

namespace plant {

// Commune interface to all sensors
//The heart on the application depends only on this interface
class ISensor {
public:
    virtual ~ISensor() = default;

    virtual std::string name() const = 0;

    // std::nullopt = mesure invalide / capteur déconnecté
    virtual std::optional<double> read() = 0;
};

} // namespace plant
