#pragma once

#include "shadow/renderer/buffer_layout.h"

namespace Shadow {

class VertexBuffer {
public:
    explicit VertexBuffer(uint32_t size);
    VertexBuffer(float* vertices, uint32_t size);
    ~VertexBuffer();

    void Bind() const;
    void Unbind() const;

    const BufferLayout& GetLayout() const { return layout; }
    void SetLayout(BufferLayout const& lo) { layout = lo; }

    void SetData(const void* data, uint32_t size);
private:
    uint32_t rendererId;
    BufferLayout layout;
};

class IndexBuffer {
public:
    IndexBuffer(uint32_t* indices, uint32_t count);
    ~IndexBuffer();

    void Bind() const;
    void Unbind() const;

    uint32_t GetCount() const { return indicesCount; };
private:
    uint32_t rendererId;
    uint32_t indicesCount;
};


class ShaderStorageBuffer {
public:
    ShaderStorageBuffer(float* vertices, uint32_t size);
    ~ShaderStorageBuffer();

    void Bind() const;
    void Unbind() const;

    const BufferLayout& GetLayout() const { return layout; }
    void SetLayout(BufferLayout const& lo) { layout = lo; }
private:
    uint32_t rendererId;
    BufferLayout layout;
};

}


