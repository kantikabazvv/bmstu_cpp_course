#pragma once
#include <cstdint>
#include <exception>
#include <type_traits>
#include <utility>

namespace bmstu {

// Специальный тип для обозначения пустого optional
struct nullopt_t {
    constexpr explicit nullopt_t(int) {}
};
inline constexpr nullopt_t nullopt{0};

// Исключение для случаев доступа к пустому optional
class bad_optional_access : public std::exception {
   public:
    using exception::exception;

    const char* what() const noexcept override { 
        return "Bad optional access"; 
    }
};

template <typename T>
class optional {
   public:
    // Конструктор по умолчанию - создает пустой optional
    optional() = default;

    // Конструктор от lvalue-ссылки - инициализирует optional значением
    optional(const T& value) {
        is_initialized_ = true;
        new (&data_[0]) T(value);  // Размещающее new
    }

    // Конструктор от rvalue-ссылки - инициализирует optional с перемещением
    optional(T&& value) {
        is_initialized_ = true;
        new (&data_[0]) T(std::move(value));  // Перемещаем значение
    }

    // Конструктор копирования - копирует состояние другого optional
    optional(const optional& other) : is_initialized_(other.is_initialized_) {
        if (other.is_initialized_) {
            new (&data_[0]) T(other.value());  // Копируем значение
        }
    }

    // Конструктор перемещения - перемещает состояние другого optional
    optional(optional&& other) noexcept : is_initialized_(other.is_initialized_) {
        if (other.is_initialized_) {
            new (&data_[0]) T(std::move(other.value()));  // Перемещаем значение
            // Помечаем исходный объект как пустой после перемещения
            other.is_initialized_ = false;
        }
    }

    // Оператор присваивания от lvalue-ссылки
    optional& operator=(const T& value) {
        if (is_initialized_) {
            this->value() = value;  // Присваиваем значение, если optional уже инициализирован
        } else {
            new (&data_[0]) T(value);  // Создаем новое значение
            is_initialized_ = true;
        }
        return *this;
    }

    // Оператор присваивания от rvalue-ссылки
    optional& operator=(T&& value) {
        if (is_initialized_) {
            this->value() = std::move(value);  // Перемещаем значение
        } else {
            new (&data_[0]) T(std::move(value));  // Создаем значение с перемещением
            is_initialized_ = true;
        }
        return *this;
    }

    // Оператор присваивания копированием
    optional& operator=(const optional& other) {
        if (this != &other) {  // Защита от самоприсваивания
            if (other.is_initialized_) {
                if (is_initialized_) {
                    value() = other.value();  // Присваиваем значение
                } else {
                    new (&data_[0]) T(other.value());  // Создаем копию
                    is_initialized_ = true;
                }
            } else {
                reset();  // Сбрасываем, если other пуст
            }
        }
        return *this;
    }

    // Оператор присваивания перемещением
    optional& operator=(optional&& other) noexcept {
        if (this != &other) {  // Защита от самоприсваивания
            if (other.is_initialized_) {
                if (is_initialized_) {
                    value() = std::move(other.value());  // Перемещаем значение
                } else {
                    new (&data_[0]) T(std::move(other.value()));  // Создаем с перемещением
                    is_initialized_ = true;
                }
                // Помечаем исходный объект как пустой после перемещения
                other.is_initialized_ = false;
            } else {
                reset();  // Сбрасываем, если other пуст
            }
        }
        return *this;
    }

    // Оператор разыменования для lvalue
    T& operator*() & {
        return value();
    }

    // Оператор разыменования для const lvalue
    const T& operator*() const& {
        return value();
    }

    // Оператор разыменования для rvalue
    T&& operator*() && {
        return

std::move(value());
    }

    // Оператор доступа к членам для non-const объектов
    T* operator->() {
        return &value();
    }

    // Оператор доступа к членам для const объектов
    const T* operator->() const {
        return &value();
    }

    // Получение ссылки на значение (lvalue)
    T& value() & {
        if (!is_initialized_) {
            throw bad_optional_access();  // Бросаем исключение если optional пуст
        }
        return *reinterpret_cast<T*>(&data_[0]);
    }

    // Получение const ссылки на значение
    const T& value() const& {
        if (!is_initialized_) {
            throw bad_optional_access();  // Бросаем исключение если optional пуст
        }
        return *reinterpret_cast<const T*>(&data_[0]);
    }

    // Получение rvalue ссылки на значение
    T&& value() && {
        if (!is_initialized_) {
            throw bad_optional_access();  // Бросаем исключение если optional пуст
        }
        return std::move(*reinterpret_cast<T*>(&data_[0]));
    }

    // Создание значения на месте с заданными аргументами
    template <typename... Args>
    void emplace(Args&&... args) {
        reset();  // Сначала сбрасываем текущее значение
        new (&data_[0]) T(std::forward<Args>(args)...);  // Создаем новое значение
        is_initialized_ = true;
    }

    // Сброс optional в пустое состояние
    void reset() {
        if (is_initialized_) {
            value().~T();  // Явно вызываем деструктор
            is_initialized_ = false;
        }
    }

    // Деструктор - освобождает ресурсы
    ~optional() { 
        reset(); 
    }

    // Проверка наличия значения
    bool has_value() const { 
        return is_initialized_; 
    }

    // Явное преобразование в bool (проверка наличия значения)
    explicit operator bool() const noexcept {
        return is_initialized_;
    }

    // Получение значения или значения по умолчанию (lvalue версия)
    template <typename U>
    T value_or(U&& default_value) const& {
        return is_initialized_ ? value() : static_cast<T>(std::forward<U>(default_value));
    }

    // Получение значения или значения по умолчанию (rvalue версия)
    template <typename U>
    T value_or(U&& default_value) && {
        return is_initialized_ ? std::move(value()) : static_cast<T>(std::forward<U>(default_value));
    }

   private:
    // Буфер для хранения значения (выравнивание как у типа T)
    alignas(T) uint8_t data_[sizeof(T)];
    // Флаг инициализации
    bool is_initialized_ = false;
};

}  // namespace bmstu