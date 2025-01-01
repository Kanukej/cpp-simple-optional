#include <stdexcept>
#include <utility>

// Исключение этого типа должно генерироватся при обращении к пустому optional
class BadOptionalAccess : public std::exception {
public:
    using exception::exception;

    virtual const char* what() const noexcept override {
        return "Bad optional access";
    }
};

template <typename T>
class Optional {
public:
    Optional() = default;
    Optional(const T& value) {
        *this = value;
    }
    Optional(T&& value) {
        *this = std::move(value);
    }
    Optional(const Optional& other) {
        *this = other;
    }
    Optional(Optional&& other) {
        *this = std::move(other);
    }

    Optional& operator=(const T& value) {
        if (is_initialized_) {
            *Ptr() = value;
        } else {
            new (data_) T(value);
        }
        is_initialized_ = true;
        return *this;
    }
    Optional& operator=(T&& rhs) {
        if (is_initialized_) {
            *Ptr() = std::move(rhs);
        } else {
            new (data_) T(std::move(rhs));
        }
        is_initialized_ = true;
        return *this;
    }
    Optional& operator=(const Optional& rhs) {
        if (rhs.is_initialized_) {
            if (is_initialized_) {
                *Ptr() = *rhs.Ptr();
            } else {
                new (data_) T(*rhs.Ptr());
            }
        } else {
            Reset();
        }
        this->is_initialized_ = rhs.is_initialized_;
        return *this;
    }
    Optional& operator=(Optional&& rhs) {
        if (rhs.is_initialized_) {
            if (is_initialized_) {
                *Ptr() = std::move(*rhs.Ptr());
            } else {
                new (data_) T(std::move(*rhs.Ptr()));
            }
        } else {
            Reset();
        }
        this->is_initialized_ = std::move(rhs.is_initialized_);
        return *this;
    }

    ~Optional() {
        Reset();
    }

    bool HasValue() const {
        return is_initialized_;
    }

    // Операторы * и -> не должны делать никаких проверок на пустоту Optional.
    // Эти проверки остаются на совести программиста
    T& operator*() {
        return *Ptr();
    }
    const T& operator*() const {
        return *Ptr();
    }
    T* operator->() {
        return Ptr();
    }
    const T* operator->() const {
        return Ptr();
    }

    // Метод Value() генерирует исключение BadOptionalAccess, если Optional пуст
    T& Value() {
        if (!is_initialized_) {
            throw BadOptionalAccess();
        }
        return *Ptr();
    }
    const T& Value() const {
        if (!is_initialized_) {
            throw BadOptionalAccess();
        }
        return *Ptr();
    }

    void Reset() {
        if (is_initialized_) {
            Ptr()->~T();
            is_initialized_ = false;
        }
    }

private:
    const T* Ptr() const {
        return reinterpret_cast<const T*>(&data_[0]);
    }
    
    T* Ptr() {
        return reinterpret_cast<T*>(&data_[0]);
    }
    // alignas нужен для правильного выравнивания блока памяти
    alignas(T) char data_[sizeof(T)];
    bool is_initialized_ = false;
};
