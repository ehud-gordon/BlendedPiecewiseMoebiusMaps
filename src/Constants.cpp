#include "Utils/Constants.h"

TextureType& operator++(TextureType& c) {
    c = static_cast<TextureType>((static_cast<int>(c) + 1) % static_cast<int>(TextureType::TYPES_COUNT));
    return c;
}

// Function to get the name of the TextureType
const char* GetTextureTypeName(TextureType type) {
    switch (type) {
        case TextureType::LINEAR: return "Linear";
        case TextureType::DIRECT_MOBIUS: return "Direct Mobius";
        case TextureType::BPM: return "BPM";
        default: return "Unknown";
    }
}