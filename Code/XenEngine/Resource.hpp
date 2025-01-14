// Author: Jake Rieger
// Created: 1/13/2025.
//

#pragma once

namespace x {
    struct Resource {
        virtual void Release() = 0;
        virtual ~Resource()    = default;
    };
}  // namespace x
