// Base case for the recursion: an empty tuple
template<typename... Ts>
class SimpleTuple {};

// Recursive definition
template<typename Head, typename... Tail>
class SimpleTuple<Head, Tail...> : public SimpleTuple<Tail...> {
private:
    Head head_;

public:
    SimpleTuple(const Head& head, const Tail&... tail)
        : SimpleTuple<Tail...>(tail...), head_(head) {}

    Head& get_head() { return head_; }
    const Head& get_head() const { return head_; }

    SimpleTuple<Tail...>& get_tail() { return *this; }
    const SimpleTuple<Tail...>& get_tail() const { return *this; }
};

// Helper function to get the Nth element
template<size_t I, typename Head, typename... Tail>
struct TupleGetter {
    static auto& get(SimpleTuple<Head, Tail...>& t) {
        return TupleGetter<I - 1, Tail...>::get(t.get_tail());
    }
};

// Base case for the getter recursion (when I is 0)
template<typename Head, typename... Tail>
struct TupleGetter<0, Head, Tail...> {
    static Head& get(SimpleTuple<Head, Tail...>& t) {
        return t.get_head();
    }
};

// User-facing get function
template<size_t I, typename... Ts>
auto& get(SimpleTuple<Ts...>& t) {
    return TupleGetter<I, Ts...>::get(t);
}