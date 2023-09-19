#include <cstddef>
#include <iostream>
#include <iterator>
#include <memory>

template <class T, class Allocator = std::allocator<T>>
class List {
 private:
  struct Node;

 public:
  using node_alloc =
      typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;
  using node_alloc_traits = std::allocator_traits<node_alloc>;
  using value_type = T;
  using allocator_type = Allocator;
  List();
  List(size_t count, const T& value, const Allocator& alloc = Allocator());
  explicit List(size_t count, const Allocator& alloc = Allocator());
  List(const List& other);
  List(std::initializer_list<T> init, const Allocator& alloc = Allocator());
  List& operator=(const List& other);
  void clear(List& list);
  ~List();
  allocator_type get_allocator() const;
  T& front();
  const T& front() const;
  T& back();
  const T& back() const;
  bool empty() const;
  size_t size() const;
  void push_back(const T& value);
  void push_front(const T& value);
  void pop_front();
  void pop_back();
  void push_back(T&& value);
  void push_front(T&& value);

  template <bool IsConst>
  class CommonIterator
      : public std::iterator<std::bidirectional_iterator_tag,
                             std::conditional_t<IsConst, const T, T>> {
   public:
    CommonIterator();
    CommonIterator(std::conditional_t<IsConst, const Node*, Node*> other_ptr,
                   std::conditional_t<IsConst, const Node*, Node*> tail);
    CommonIterator(const CommonIterator& other);
    CommonIterator& operator=(const CommonIterator& other);
    CommonIterator& operator++();
    CommonIterator operator++(int val);
    CommonIterator& operator--();
    CommonIterator operator--(int val);
    bool operator==(const CommonIterator& other) const;
    bool operator!=(const CommonIterator& other) const;
    std::conditional_t<IsConst, const T&, T&> operator*();
    std::conditional_t<IsConst, const T*, T*> operator->();

   private:
    std::conditional_t<IsConst, const Node*, Node*> ptr_;
    std::conditional_t<IsConst, const Node*, Node*> tail_;
  };

  using iterator = CommonIterator<false>;
  using const_iterator = CommonIterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;

  iterator begin();
  iterator end();
  const_iterator begin() const;
  const_iterator end() const;
  const_iterator cbegin() const;
  const_iterator cend() const;
  reverse_iterator rbegin();
  reverse_iterator rend();

 private:
  struct Node {
    Node() : left(nullptr), right(nullptr) {}
    Node(Node* left, Node* right) : left(left), right(right) {}
    Node(Node* left, Node* right, const T& value)
        : left(left), right(right), value(value) {}
    Node(const T& value) : left(nullptr), right(nullptr), value(value) {}
    Node* left;
    Node* right;
    T value;
  };
  Node* head_;
  Node* tail_;
  size_t size_;
  node_alloc alloc_;
};

template <class T, class Allocator>
List<T, Allocator>::List() : head_(nullptr), tail_(nullptr), size_(0) {}

template <class T, class Allocator>
void List<T, Allocator>::clear(List<T, Allocator>& list) {
  if (list.empty()) {
    return;
  }
  Node* current = list.tail_->left;
  while (list.tail_ != nullptr) {
    node_alloc_traits::destroy(list.alloc_, list.tail_);
    node_alloc_traits::deallocate(list.alloc_, list.tail_, 1);
    list.tail_ = current;
    if (current != nullptr) {
      current->right = nullptr;
      current = current->left;
    }
  }
  list.size_ = 0;
}

template <class T, class Allocator>
List<T, Allocator>::List(size_t count, const T& value, const Allocator& alloc)
    : size_(count) {
  alloc_ = alloc;
  head_ = node_alloc_traits::allocate(alloc_, 1);
  try {
    node_alloc_traits::construct(alloc_, head_, value);
  } catch (...) {
    throw;
  }
  tail_ = head_;
  Node* current = tail_;
  for (size_t i = 1; i < size_; ++i) {
    tail_ = node_alloc_traits::allocate(alloc_, 1);
    try {
      node_alloc_traits::construct(alloc_, tail_, current, nullptr, value);
    } catch (...) {
      tail_ = current;
      node_alloc_traits::deallocate(alloc_, tail_, 1);
      clear(*this);
      throw;
    }
    current->right = tail_;
    current = tail_;
  }
}

template <class T, class Allocator>
List<T, Allocator>::List(size_t count, const Allocator& alloc) : size_(count) {
  alloc_ = alloc;
  head_ = node_alloc_traits::allocate(alloc_, 1);
  try {
    node_alloc_traits::construct(alloc_, head_);
  } catch (...) {
    throw;
  }
  tail_ = head_;
  Node* current = tail_;
  for (size_t i = 1; i < size_; ++i) {
    tail_ = node_alloc_traits::allocate(alloc_, 1);
    try {
      node_alloc_traits::construct(alloc_, tail_, current, nullptr);
    } catch (...) {
      node_alloc_traits::deallocate(alloc_, tail_, 1);
      tail_ = current;
      clear(*this);
      throw;
    }
    current->right = tail_;
    current = tail_;
  }
}

template <class T, class Allocator>
List<T, Allocator>::List(const List<T, Allocator>& other) : size_(other.size_) {
  alloc_ =
      node_alloc_traits::select_on_container_copy_construction(other.alloc_);
  head_ = node_alloc_traits::allocate(alloc_, 1);
  try {
    node_alloc_traits::construct(alloc_, head_, other.head_->value);
  } catch (...) {
    throw;
  }
  tail_ = head_;
  Node* current = tail_;
  Node* current_value = other.head_->right;
  for (size_t i = 1; i < size_; ++i) {
    tail_ = node_alloc_traits::allocate(alloc_, 1);
    try {
      node_alloc_traits::construct(alloc_, tail_, current, nullptr,
                                   current_value->value);
    } catch (...) {
      node_alloc_traits::deallocate(alloc_, tail_, 1);
      tail_ = current;
      clear(*this);
      throw;
    }
    current->right = tail_;
    current = tail_;
    current_value = current_value->right;
  }
}

template <class T, class Allocator>
List<T, Allocator>::List(std::initializer_list<T> init, const Allocator& alloc)
    : size_(init.size()), alloc_(alloc) {
  auto itt = init.begin();
  head_ = node_alloc_traits::allocate(alloc_, 1);
  try {
    node_alloc_traits::construct(alloc_, head_, *itt);
  } catch (...) {
    throw;
  }
  tail_ = head_;
  Node* current = tail_;
  ++itt;
  for (; itt != init.end(); ++itt) {
    tail_ = node_alloc_traits::allocate(alloc_, 1);
    try {
      node_alloc_traits::construct(alloc_, tail_, current, nullptr, *itt);
    } catch (...) {
      node_alloc_traits::deallocate(alloc_, tail_, 1);
      tail_ = current;
      clear(*this);
      throw;
    }
    current->right = tail_;
    current = tail_;
  }
}

template <class T, class Allocator>
List<T, Allocator>& List<T, Allocator>::operator=(
    const List<T, Allocator>& other) {
  if (this == &other) {
    return *this;
  }
  List<T, Allocator> list;
  list.alloc_ = alloc_;
  if (node_alloc_traits::propagate_on_container_copy_assignment::value) {
    list.alloc_ = other.alloc_;
  }
  list.size_ = other.size_;
  list.head_ = node_alloc_traits::allocate(list.alloc_, 1);
  try {
    node_alloc_traits::construct(list.alloc_, list.head_, other.head_->value);
  } catch (...) {
    throw;
  }
  list.tail_ = list.head_;
  Node* current = list.tail_;
  Node* current_value = other.head_->right;
  for (size_t i = 1; i < list.size_; ++i) {
    list.tail_ = node_alloc_traits::allocate(list.alloc_, 1);
    try {
      node_alloc_traits::construct(list.alloc_, list.tail_, current, nullptr,
                                   current_value->value);
    } catch (...) {
      node_alloc_traits::deallocate(alloc_, list.tail_, 1);
      list.tail_ = current;
      clear(list);
      throw;
    }
    current->right = list.tail_;
    current = list.tail_;
    current_value = current_value->right;
  }
  clear(*this);
  alloc_ = list.alloc_;
  size_ = list.size_;
  head_ = list.head_;
  tail_ = list.tail_;
  list.head_ = nullptr;
  list.tail_ = nullptr;
  list.size_ = 0;
  return *this;
}

template <class T, class Allocator>
List<T, Allocator>::~List() {
  clear(*this);
}

template <class T, class Allocator>
typename List<T, Allocator>::allocator_type List<T, Allocator>::get_allocator()
    const {
  return alloc_;
}

template <class T, class Allocator>
T& List<T, Allocator>::front() {
  if (!empty()) {
    return head_->value;
  }
  return 0;
}

template <class T, class Allocator>
const T& List<T, Allocator>::front() const {
  if (!empty()) {
    return head_->value;
  }
  return 0;
}

template <class T, class Allocator>
T& List<T, Allocator>::back() {
  if (!empty()) {
    return tail_->value;
  }
  return 0;
}

template <class T, class Allocator>
const T& List<T, Allocator>::back() const {
  if (!empty()) {
    return tail_->value;
  }
  return 0;
}

template <class T, class Allocator>
bool List<T, Allocator>::empty() const {
  return size_ == 0;
}

template <class T, class Allocator>
size_t List<T, Allocator>::size() const {
  return size_;
}

template <class T, class Allocator>
void List<T, Allocator>::push_back(const T& value) {
  ++size_;
  Node* current = tail_;
  if (current == nullptr) {
    tail_ = node_alloc_traits::allocate(alloc_, 1);
    node_alloc_traits::construct(alloc_, tail_, nullptr, nullptr, value);
    head_ = tail_;
  } else {
    tail_ = node_alloc_traits::allocate(alloc_, 1);
    node_alloc_traits::construct(alloc_, tail_, current, nullptr, value);
    current->right = tail_;
  }
}

template <class T, class Allocator>
void List<T, Allocator>::push_front(const T& value) {
  ++size_;
  Node* current = head_;
  if (current == nullptr) {
    head_ = node_alloc_traits::allocate(alloc_, 1);
    node_alloc_traits::construct(alloc_, head_, nullptr, nullptr, value);
    tail_ = head_;
  } else {
    head_ = node_alloc_traits::allocate(alloc_, 1);
    node_alloc_traits::construct(alloc_, head_, nullptr, current, value);
    current->left = head_;
  }
}

template <class T, class Allocator>
void List<T, Allocator>::pop_front() {
  if (empty()) {
    return;
  }
  --size_;
  Node* current = head_;
  head_ = head_->right;
  node_alloc_traits::destroy(alloc_, current);
  node_alloc_traits::deallocate(alloc_, current, 1);
  if (head_ != nullptr) {
    head_->left = nullptr;
  }
}

template <class T, class Allocator>
void List<T, Allocator>::pop_back() {
  if (empty()) {
    return;
  }
  --size_;
  Node* current = tail_;
  tail_ = tail_->left;
  node_alloc_traits::destroy(alloc_, current);
  node_alloc_traits::deallocate(alloc_, current, 1);
  if (tail_ != nullptr) {
    tail_->right = nullptr;
  }
}

template <class T, class Allocator>
void List<T, Allocator>::push_back(T&& value) {
  ++size_;
  Node* current = tail_;
  if (current == nullptr) {
    tail_ = node_alloc_traits::allocate(alloc_, 1);
    node_alloc_traits::construct(alloc_, tail_, nullptr, nullptr,
                                 std::move(value));
    head_ = tail_;
  } else {
    tail_ = node_alloc_traits::allocate(alloc_, 1);
    node_alloc_traits::construct(alloc_, tail_, current, nullptr,
                                 std::move(value));
    current->right = tail_;
  }
}

template <class T, class Allocator>
void List<T, Allocator>::push_front(T&& value) {
  ++size_;
  Node* current = head_;
  if (current == nullptr) {
    head_ = node_alloc_traits::allocate(alloc_, 1);
    node_alloc_traits::construct(alloc_, head_, nullptr, nullptr,
                                 std::move(value));
    tail_ = head_;
  } else {
    head_ = node_alloc_traits::allocate(alloc_, 1);
    node_alloc_traits::construct(alloc_, head_, nullptr, current,
                                 std::move(value));
    current->left = head_;
  }
}

template <class T, class Allocator>
template <bool IsConst>
List<T, Allocator>::CommonIterator<IsConst>::CommonIterator()
    : ptr_(nullptr), tail_(nullptr) {}

template <class T, class Allocator>
template <bool IsConst>
List<T, Allocator>::CommonIterator<IsConst>::CommonIterator(
    std::conditional_t<IsConst, const Node*, Node*> other_ptr,
    std::conditional_t<IsConst, const Node*, Node*> tail)
    : ptr_(other_ptr), tail_(tail) {}

template <class T, class Allocator>
template <bool IsConst>
List<T, Allocator>::CommonIterator<IsConst>::CommonIterator(
    const CommonIterator& other)
    : ptr_(other.ptr_), tail_(other.tail_) {}

template <class T, class Allocator>
template <bool IsConst>
typename List<T, Allocator>::template CommonIterator<IsConst>&
List<T, Allocator>::CommonIterator<IsConst>::operator=(
    const CommonIterator& other) {
  ptr_ = other.ptr_;
  tail_ = other.tail_;
  return *this;
}

template <class T, class Allocator>
template <bool IsConst>
typename List<T, Allocator>::template CommonIterator<IsConst>&
List<T, Allocator>::CommonIterator<IsConst>::operator++() {
  if (ptr_ == nullptr) {
    return *this;
  }
  ptr_ = ptr_->right;
  return *this;
}

template <class T, class Allocator>
template <bool IsConst>
typename List<T, Allocator>::template CommonIterator<IsConst>
List<T, Allocator>::CommonIterator<IsConst>::operator++(int val) {
  CommonIterator itt(*this);
  val = 0;
  ptr_->value += val;
  ++(*this);
  return itt;
}

template <class T, class Allocator>
template <bool IsConst>
typename List<T, Allocator>::template CommonIterator<IsConst>&
List<T, Allocator>::CommonIterator<IsConst>::operator--() {
  if (ptr_ == nullptr) {
    if (tail_ != nullptr) {
      ptr_ = tail_;
      return *this;
    }
    return *this;
  }
  ptr_ = ptr_->left;
  return *this;
}

template <class T, class Allocator>
template <bool IsConst>
typename List<T, Allocator>::template CommonIterator<IsConst>
List<T, Allocator>::CommonIterator<IsConst>::operator--(int val) {
  CommonIterator itt(*this);
  val = 0;
  ptr_->value -= val;
  ++(*this);
  return itt;
}

template <class T, class Allocator>
template <bool IsConst>
bool List<T, Allocator>::CommonIterator<IsConst>::operator==(
    const CommonIterator& other) const {
  return ptr_ == other.ptr_;
}

template <class T, class Allocator>
template <bool IsConst>
bool List<T, Allocator>::CommonIterator<IsConst>::operator!=(
    const CommonIterator& other) const {
  return !(*this == other);
}

template <class T, class Allocator>
template <bool IsConst>
std::conditional_t<IsConst, const T&, T&>
List<T, Allocator>::CommonIterator<IsConst>::operator*() {
  return ptr_->value;
}

template <class T, class Allocator>
template <bool IsConst>
std::conditional_t<IsConst, const T*, T*>
List<T, Allocator>::CommonIterator<IsConst>::operator->() {
  return &ptr_->value;
}

template <class T, class Allocator>
typename List<T, Allocator>::iterator List<T, Allocator>::begin() {
  return iterator(head_, tail_);
}

template <class T, class Allocator>
typename List<T, Allocator>::iterator List<T, Allocator>::end() {
  return iterator(tail_->right, tail_);
}

template <class T, class Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::begin() const {
  return const_iterator(head_, tail_);
}

template <class T, class Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::end() const {
  return const_iterator(tail_->right, tail_);
}

template <class T, class Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::cbegin() const {
  return const_iterator(head_), tail_;
}

template <class T, class Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::cend() const {
  return const_iterator(tail_->right, tail_);
}

template <class T, class Allocator>
typename List<T, Allocator>::reverse_iterator List<T, Allocator>::rbegin() {
  return reverse_iterator(iterator(tail_->right, tail_));
}

template <class T, class Allocator>
typename List<T, Allocator>::reverse_iterator List<T, Allocator>::rend() {
  return reverse_iterator(iterator(head_, tail_));
}

int main() {

  return 0;
}