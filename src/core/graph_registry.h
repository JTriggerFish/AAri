//

#ifndef AARI_GRAPH_REGISTRY_H
#define AARI_GRAPH_REGISTRY_H

#include <entt/entt.hpp>
#include <optional>
#include <tuple>
#include <memory>
#include "graph.h"
#include <mutex>
#include "../miniaudio.h"

namespace AAri {
    class SpinLockGuard {
    public:
        SpinLockGuard(ma_spinlock &spinlock) : spinlock(spinlock) {
            ma_spinlock_lock(&spinlock);
        }

        ~SpinLockGuard() {
            ma_spinlock_unlock(&spinlock);
        }

    private:
        ma_spinlock &spinlock;
    };

    class IGraphRegistry {
    public:
        virtual std::tuple<entt::registry &, SpinLockGuard> get_graph_registry() = 0;
    };
}

#endif //AARI_GRAPH_REGISTRY_H
