// Author: Jake Rieger
// Created: 1/13/2025.
//

#pragma once

#include "Types.hpp"
#include "EntityId.hpp"
#include "ComponentManager.hpp"
#include "TransformComponent.hpp"
#include <set>

namespace x {
    using std::set;

    class GameState {
    public:
        EntityId CreateEntity() {
            const auto newId = ++_nextId;
            return EntityId(newId);
        }

        void DestroyEntity(EntityId entity) {
            _transforms.RemoveComponent(entity);
        }

        GameState Clone() const {
            GameState newState;
            newState._nextId     = _nextId;
            newState._transforms = _transforms;
            return newState;
        }

        void ReleaseAllResources() {
            // TODO: Release component resources
        }

        template<typename T>
        const T* GetComponent(EntityId entity) const {
            if constexpr (std::is_same_v<T, TransformComponent>) {
                return _transforms.GetComponent(entity);
            }

            return None;
        }

        template<typename T>
        T* GetComponentMutable(EntityId entity) {
            if constexpr (std::is_same_v<T, TransformComponent>) {
                return _transforms.GetComponentMutable(entity);
            }

            return None;
        }

        template<typename T>
        T& AddComponent(EntityId entity) {
            if constexpr (std::is_same_v<T, TransformComponent>) {
                return _transforms.AddComponent(entity);
            }

            return None;
        }

        template<typename T>
        const ComponentManager<T>& GetComponents() const {
            if constexpr (std::is_same_v<T, TransformComponent>) { return _transforms; }
            return None;
        }

        template<typename T>
        ComponentManager<T>& GetComponents() {
            if constexpr (std::is_same_v<T, TransformComponent>) { return _transforms; }
            return None;
        }

    private:
        u64 _nextId = 0;
        ComponentManager<TransformComponent> _transforms;

        template<typename T>
        void ReleaseComponentResources() {
            if constexpr (detail::release_resources<T>::value) {
                GetComponents<T>().ReleaseResources();
            }
        }
    };
}  // namespace x
