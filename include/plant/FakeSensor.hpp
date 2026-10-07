#pragma once
#include "plant/ISensor.hpp"

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace plant {

// Simulating a sensor : replay a predefined sequence of values
// To test without material
class FakeSensor final : public ISensor {
public:
    FakeSensor(std::string name, std::vector<std::optional<double>> script)
        : name_(std::move(name)), script_(std::move(script)) {}

    std::string name() const override { return name_; }

    std::optional<double> read() override {
        if (index_ >= script_.size()) {
            return std::nullopt;  // séquence épuisée
        }
        return script_[index_++];
    }

private:
    std::string name_;
    std::vector<std::optional<double>> script_;
    std::size_t index_ = 0;
};

} // namespace plant
