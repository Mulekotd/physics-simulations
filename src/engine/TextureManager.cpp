#include <GL/gl.h>

#include "engine/TextureManager.hpp"

TextureManager::~TextureManager()
{
    clear();
}

TextureId TextureManager::createSolid(const std::string& name, std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a)
{
    std::vector<std::uint8_t> data(4, 0);

    data[0] = r;
    data[1] = g;
    data[2] = b;
    data[3] = a;

    return createTexture(name, 1, 1, data);
}

TextureId TextureManager::createChecker(const std::string& name,
                                        std::uint8_t r1, std::uint8_t g1, std::uint8_t b1, std::uint8_t a1,
                                        std::uint8_t r2, std::uint8_t g2, std::uint8_t b2, std::uint8_t a2,
                                        int size)
{
    if (size <= 0) size = 8;

    std::vector<std::uint8_t> data(static_cast<std::size_t>(size * size * 4), 0);

    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            bool even = ((x / (size / 2)) + (y / (size / 2))) % 2 == 0;

            std::uint8_t r = even ? r1 : r2;
            std::uint8_t g = even ? g1 : g2;
            std::uint8_t b = even ? b1 : b2;
            std::uint8_t a = even ? a1 : a2;

            std::size_t idx = static_cast<std::size_t>((y * size + x) * 4);

            data[idx + 0] = r;
            data[idx + 1] = g;
            data[idx + 2] = b;
            data[idx + 3] = a;
        }
    }

    return createTexture(name, size, size, data);
}

TextureId TextureManager::getByIndex(int index) const noexcept
{
    if (index < 0 || index >= static_cast<int>(m_entries.size()))
        return 0;

    return m_entries[static_cast<std::size_t>(index)].id;
}

void TextureManager::clear() {
    for (const auto &entry : m_entries)
        if (entry.id)
            glDeleteTextures(1, &entry.id);

    m_entries.clear();
}

TextureId TextureManager::createTexture(const std::string& name, int width, int height, const std::vector<std::uint8_t> &data)
{
    TextureId id = 0;

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

    m_entries.push_back({ name, id });

    return id;
}
