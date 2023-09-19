# List

## Реализация
Класса List\<T, Allocator\> аналог std::list из stl.

* Базовая функциональность + итераторы
* Поддержка аллокаторов
* exception-safety

## Методы:

### Using'и

* value_type
* allocator_type
* iterator (удовлетворяет https://en.cppreference.com/w/cpp/named_req/BidirectionalIterator)

### Конструкторы

* List()
* List(size_t count, const T& value = T(), const Allocator& alloc = Allocator())
* explicit List(size_t count, const Allocator& alloc = Allocator())
* List(const list& other);
* List(std::initializer_list\<T\> init, const Allocator& alloc = Allocator())

### Деструктор

~List()

### Iterators (с поддержкой константных)

* begin()
* end()
* cbegin()
* cend()

### operator=

* List& operator=(const List& other)

### element access methods

* T& front()
* const T& front() const
* T& back()
* const T& back() const


### Capacity

* bool empty()
* size_t size()

### Modifiers

* push_back(front)(const T&)
* push_back(front)(T&&)
* pop_back(front)();

### Exception-safety

Общая концепция: если где-то выскочит исключение ваш контейнер должен вернуться в оригинальное состояние и пробросить исключение наверх.
