#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <GL/gl.h>

using TextureId = GLuint;

struct TextureEntry {
    std::string name;
    TextureId id;
};

class TextureManager {
public:
    TextureManager() = default;
    ~TextureManager();

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    TextureId createSolid(const std::string& name, std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a);
    TextureId createChecker(const std::string& name,
                            std::uint8_t r1, std::uint8_t g1, std::uint8_t b1, std::uint8_t a1,
                            std::uint8_t r2, std::uint8_t g2, std::uint8_t b2, std::uint8_t a2,
                            int size);

    const std::vector<TextureEntry>& entries() const noexcept { return m_entries; }
    TextureId getByIndex(int index) const noexcept;

    void clear();

private:
    std::vector<TextureEntry> m_entries;

    TextureId createTexture(const std::string& name, int width, int height, const std::vector<std::uint8_t>& data);
};
