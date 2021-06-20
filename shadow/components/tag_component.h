#pragma once

namespace Shadow {
class TagComponent {
public:
    TagComponent() = default;
    TagComponent(const Tag&) = default;
    explicit TagComponent(const std::string& tag)
            : Tag(tag) {}

    std::string Tag;
};
}
