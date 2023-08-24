#ifndef AARI_FIELDMAP_H
#define AARI_FIELDMAP_H
#include <iostream>
#include <array>
#include <string_view>
#include <cstdint>


template<const char*... Names>
struct NameArray {
    static constexpr std::array<std::string_view, sizeof...(Names)> names = { Names... };
};

template<int... Ints>
struct IntArray {
    static constexpr std::array<int, sizeof...(Ints)> values = { Ints... };
};

template<typename NameType, typename IntType>
class FieldMap;

template<const char*... Names, int... Ints>
class FieldMap<NameArray<Names...>, IntArray<Ints...>> {
    static_assert(sizeof...(Names) == sizeof...(Ints), "Mismatched number of names and integers!");

public:
    static constexpr int get(const char* key) {
        std::string_view keyView(key);
        for (size_t i = 0; i < NameArray<Names...>::names.size(); ++i) {
            if (NameArray<Names...>::names[i] == keyView) {
                return IntArray<Ints...>::values[i];
            }
        }
        // At compile time, if the condition is false, a compilation error will be triggered
        static_assert(sizeof...(Names) == 0, "Key not found in the FieldMap!");

        // At runtime, this exception will be thrown
        throw std::runtime_error("Key not found in the FieldMap!");
    }

    // Function to get the name based on the index
    static constexpr std::string_view getName(size_t index) {
        return (index < NameArray<Names...>::names.size()) ? NameArray<Names...>::names[index] : "";
    }
};



#endif //AARI_FIELDMAP_H
