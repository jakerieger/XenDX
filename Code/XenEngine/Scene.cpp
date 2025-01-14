// Author: Jake Rieger
// Created: 1/13/2025.
//

#include "Scene.hpp"

#include <functional>

namespace x {
    using namespace DirectX;

    EntityId Scene::CreateEntity(const std::optional<EntityId>& parent) {
        const EntityId entity = _state.CreateEntity();
        const auto node       = make_shared<SceneNode>();
        node->entity          = entity;
        node->localTransform  = XMMatrixIdentity();
        node->worldTransform  = XMMatrixIdentity();

        if (!parent.has_value() || !parent->valid()) {
            if (!_root) {
                _root = node;
            } else {
                node->parent = _root;
                _root->children.push_back(node);
            }
        } else {
            const auto parentNode = _nodes[*parent];
            node->parent          = parentNode;
            parentNode->children.push_back(node);
        }

        _nodes[entity] = node;
        return entity;
    }

    void Scene::RemoveEntity(const EntityId& entity) {
        const auto it = _nodes.find(entity);
        if (it == _nodes.end()) return;

        const auto node = it->second;
        for (const auto& child : node->children) {
            RemoveEntity(child->entity);
        }

        if (const auto parent = node->parent.lock()) {
            auto& siblings = parent->children;
            std::erase_if(siblings, [entity](const auto& n) { return n->entity == entity; });
        }

        _nodes.erase(entity);
        _state.DestroyEntity(entity);
        if (_root && _root->entity == entity) _root.reset();
    }

    void Scene::AttachEntity(EntityId child, EntityId parent) {
        const auto childIt = _nodes.find(child);
        if (childIt == _nodes.end()) { return; }

        const auto parentIt = _nodes.find(parent);
        if (parentIt == _nodes.end()) { return; }

        const auto childNode  = childIt->second;
        const auto parentNode = parentIt->second;

        // Store the child's current world transform before we modify its hierarchy.
        // This lets us maintain its world position after reparenting.
        const XMMATRIX childWorldTransform = childNode->worldTransform;

        // if child is already attached to a different parent, remove it from that parent first.
        if (const auto oldParent = childNode->parent.lock()) {
            auto& oldParentChildren = oldParent->children;
            std::erase_if(oldParentChildren,
                          [child](const auto& node) { return node->entity == child; });
        }

        // update the hierarchy relationships
        childNode->parent = parentNode;
        parentNode->children.push_back(childNode);

        // Calculate the new local transform that will maintain the child's world position
        // worldTransform = parentWorldTransform * localTransform
        // Therefore: localTransform = inverse(parentWorldTransform) * worldTransform
        childNode->localTransform =
          XMMatrixInverse(None, parentNode->worldTransform) * childWorldTransform;

        // update the transforms for this node an all its children
        UpdateWorldTransforms(childNode, parentNode->worldTransform);
    }

    void Scene::DetachEntity(EntityId child) {
        const auto childIt = _nodes.find(child);
        if (childIt == _nodes.end()) { return; }

        const auto childNode  = childIt->second;
        const auto parentNode = childNode->parent.lock();
        if (!parentNode) { return; }

        const XMMATRIX worldTransform = childNode->worldTransform;
        auto& parentChildren          = parentNode->children;
        std::erase_if(parentChildren, [child](const auto& node) { return node->entity == child; });
        childNode->parent.reset();
        childNode->localTransform = worldTransform;

        // If we have a root node and this isn't it, make it a child of root
        if (_root && childNode != _root) {
            childNode->parent = _root;
            _root->children.push_back(childNode);
            childNode->localTransform =
              XMMatrixInverse(None, _root->worldTransform) * worldTransform;
            UpdateWorldTransforms(childNode, _root->worldTransform);
        } else {
            childNode->worldTransform = worldTransform;
            UpdateWorldTransforms(childNode, XMMatrixIdentity());
        }
    }

    bool Scene::LoadFromFile(const str& filename) {
        return false;
    }

    bool Scene::SaveToFile(const str& filename) {
        return false;
    }

    void Scene::Unload() {
        std::function<void(shared_ptr<SceneNode>)> destroyNode =
          [&](const shared_ptr<SceneNode>& node) {
              for (const auto& child : node->children) {
                  destroyNode(child);
              }
              _state.DestroyEntity(node->entity);
          };

        if (_root) destroyNode(_root);
        _nodes.clear();
        _root.reset();
    }

    void Scene::SetWorldTransform(EntityId entity, const XMMATRIX& transform) {
        const auto nodeIt = _nodes.find(entity);
        if (nodeIt == _nodes.end()) { return; }

        const auto node   = nodeIt->second;
        const auto parent = node->parent.lock();
        if (parent) {
            node->localTransform = XMMatrixInverse(None, parent->worldTransform) * transform;
        } else {
            node->localTransform = transform;
        }

        UpdateWorldTransforms(node, parent ? parent->worldTransform : XMMatrixIdentity());
    }

    XMMATRIX Scene::GetWorldTransform(EntityId entity) const {
        const auto it = _nodes.find(entity);
        if (it == _nodes.end()) return XMMatrixIdentity();
        return it->second->worldTransform;
    }

    void Scene::UpdateWorldTransforms(const shared_ptr<SceneNode>& node,
                                      const XMMATRIX& parentTransform) {
        node->worldTransform = XMMatrixMultiply(parentTransform, node->localTransform);
        if (auto* transform = _state.GetComponentMutable<TransformComponent>(node->entity)) {
            XMVECTOR scale, rotation, position;
            XMMatrixDecompose(&scale, &rotation, &position, node->worldTransform);

            XMFLOAT3 pos;
            XMStoreFloat3(&pos, position);
            XMFLOAT3 scl;
            XMStoreFloat3(&scl, scale);

            XMFLOAT4 rot;
            XMStoreFloat4(&rot, rotation);
            XMFLOAT3 eulerAngles;
            f32 pitch = asinf(-2.0f * (rot.x * rot.z - rot.w * rot.y));
            f32 yaw, roll;

            // Check for gimbal lock
            if (cosf(pitch) > 0.0001f) {
                yaw  = atan2f(2.0f * (rot.y * rot.z + rot.w * rot.x),
                             1.0f - 2.0f * (rot.x * rot.x + rot.y * rot.y));
                roll = atan2f(2.0f * (rot.x * rot.y + rot.w * rot.z),
                              1.0f - 2.0f * (rot.y * rot.y + rot.z * rot.z));
            } else {
                // Gimbal lock case
                yaw  = 0.0f;
                roll = atan2f(-2.0f * (rot.x * rot.y - rot.w * rot.z),
                              1.0f - 2.0f * (rot.x * rot.x + rot.z * rot.z));
            }

            // Convert to degrees
            eulerAngles.x = XMConvertToDegrees(pitch);
            eulerAngles.y = XMConvertToDegrees(yaw);
            eulerAngles.z = XMConvertToDegrees(roll);

            transform->SetPosition(pos);
            transform->SetRotation(eulerAngles);
            transform->SetScale(scl);
        }

        for (const auto& child : node->children) {
            UpdateWorldTransforms(child, node->worldTransform);
        }
    }
}  // namespace x