// Author: Jake Rieger
// Created: 1/13/2025.
//

#pragma once

#include "Types.hpp"
#include "GameState.hpp"
#include <optional>
#include <DirectXMath.h>

namespace x {
    class Scene {
    public:
        Scene(const str& name, const GameState& state) : _name(name), _state(state) {}

        struct SceneNode {
            EntityId entity;
            vector<shared_ptr<SceneNode>> children;
            weak_ptr<SceneNode> parent;
            DirectX::XMMATRIX localTransform;
            DirectX::XMMATRIX worldTransform;
        };

        EntityId CreateEntity(const std::optional<EntityId>& parent = Empty);
        void RemoveEntity(const EntityId& entity);

        void AttachEntity(EntityId child, EntityId parent);
        void DetachEntity(EntityId child);

        bool LoadFromFile(const str& filename);
        bool SaveToFile(const str& filename);
        void Unload();

        void SetWorldTransform(EntityId entity, const DirectX::XMMATRIX& transform);
        DirectX::XMMATRIX GetWorldTransform(EntityId entity) const;

    private:
        str _name;
        GameState _state;
        unordered_map<EntityId, shared_ptr<SceneNode>> _nodes;
        shared_ptr<SceneNode> _root;

        void UpdateWorldTransforms(const shared_ptr<SceneNode>& node,
                                   const DirectX::XMMATRIX& parentTransform);
    };
}  // namespace x
