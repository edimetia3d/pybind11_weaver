#ifndef PYBIND11_WEAVER_SAMPLE_H
#define PYBIND11_WEAVER_SAMPLE_H
namespace earth::creatures {

    /// This is Animal doc
    enum Animal {
        DOG, ///< Dog doc
        CAT, ///< Cat doc
    };

    enum class Plant {
        TREE,
        FLOWER,
    };

    enum class ValueSet {
        LOW = 100,
        MIDDLE = 1000,
        HIGH = 10000,
    };
    struct Home {
        enum Tool {
            PAN,
            ROPE,
        };
        enum class Food {
            MEAT,
            RICE,
        };
    };
}


#endif //PYBIND11_WEAVER_SAMPLE_H
