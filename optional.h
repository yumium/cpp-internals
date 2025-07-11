#include <utility>
#include <stdexcept>

/*
Constructors
- SimpleOptional(nullopt_t)

has_value()
value()
operator*()
operator->()
*/


// A tag to represent the empty state
struct nullopt_t {};
inline constexpr nullopt_t nullopt;

template<typename T>
class SimpleOptional {
private:
    bool has_value_flag;
    // Use a union to manage storage without initializing the object
    union {
        T value_;
        char dummy_; // To have a default member for the union
    };

public:
    // Construct empty
    SimpleOptional() : has_value_flag(false), dummy_() {} // use of anonymous union here
    SimpleOptional(nullopt_t) : has_value_flag(false), dummy_() {}

    // Construct with a value
    SimpleOptional(const T& value) : has_value_flag(true), value_(value) {}
    SimpleOptional(T&& value) : has_value_flag(true), value_(std::move(value)) {}

    // Destructor
    ~SimpleOptional() {
        if (has_value_flag) {
            value_.~T(); // Manually destroy the object if it exists
        }
    }

    [[nodiscard]] bool has_value() const noexcept {
        return has_value_flag;
    }

    T& value() {
        if (!has_value_flag) {
            throw std::runtime_error("Bad optional access");
        }
        return value_;
    }

    T& operator*() {
        return value();
    }

    T* operator->() {
        return &value();
    }
};