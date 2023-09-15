//
//

#ifndef AARI_ASSERT_H
#define AARI_ASSERT_H
#include <exception>
#include <string>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define ASSERT(expr) \
    if (!(expr)) \
        throw AssertException("Assertion failed: " #expr, __FILE__, __LINE__);

class AssertException : public std::exception {
public:
    AssertException(const std::string& expr, const char* file, int line)
            : message("Assertion failed: " + expr + " in file " + file + " at line " + std::to_string(line)) {}

    const char* what() const noexcept override {
        return message.c_str();
    }

private:
    std::string message;
};

#endif //AARI_ASSERT_H
