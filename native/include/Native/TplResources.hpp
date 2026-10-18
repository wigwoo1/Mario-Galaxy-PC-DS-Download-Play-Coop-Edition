#pragma once

#include "Native/GpuResources.hpp"

#include <cstddef>
#include <mutex>
#include <stdexcept>
#include <unordered_map>

namespace Native {
namespace detail {
struct TplResourceRegistration {
    std::size_t size;
    std::size_t references;
    bool ownsGpuMapping;
};

struct TplResourceRegistry {
    std::mutex mutex;
    std::unordered_map<const void*, TplResourceRegistration> resources;
};

inline TplResourceRegistry& tplResourceRegistry() {
    static TplResourceRegistry registry;
    return registry;
}
} // namespace detail

inline void registerTplResource(const void* resource, std::size_t size) {
    if (!resource || size < 12) {
        throw std::invalid_argument("invalid TPL resource range");
    }

    auto& registry = detail::tplResourceRegistry();
    std::lock_guard<std::mutex> lock(registry.mutex);
    auto existing = registry.resources.find(resource);
    if (existing != registry.resources.end()) {
        if (existing->second.size != size) {
            throw std::invalid_argument("TPL resource registered with inconsistent size");
        }
        ++existing->second.references;
        return;
    }

    bool ownsGpuMapping = false;
    try {
        const auto address = gpuResources().byteAddress(resource);
        gpuResources().resolve(address, size);
    } catch (const std::exception&) {
        gpuResources().add(resource, size);
        ownsGpuMapping = true;
    }

    registry.resources.emplace(resource, detail::TplResourceRegistration{size, 1, ownsGpuMapping});
}

inline std::size_t tplResourceSize(const void* resource) {
    auto& registry = detail::tplResourceRegistry();
    std::lock_guard<std::mutex> lock(registry.mutex);
    const auto found = registry.resources.find(resource);
    return found == registry.resources.end() ? 0 : found->second.size;
}

inline void unregisterTplResource(const void* resource) {
    auto& registry = detail::tplResourceRegistry();
    std::lock_guard<std::mutex> lock(registry.mutex);
    const auto found = registry.resources.find(resource);
    if (found == registry.resources.end()) {
        return;
    }
    if (--found->second.references != 0) {
        return;
    }

    const bool ownsGpuMapping = found->second.ownsGpuMapping;
    registry.resources.erase(found);
    if (ownsGpuMapping) {
        gpuResources().remove(resource);
    }
}
} // namespace Native
