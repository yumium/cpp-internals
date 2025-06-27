template<typename T>
struct ControlBlock {
    T* ptr;
    size_t strong_count = 1;  // owned shared_ptr
    size_t weak_count = 0;    // weak_ptrs
};

// shared_ptr
template<typename T>
class SharedPtr {
public:
    T* ptr;
    ControlBlock<T>* ctrl;

    SharedPtr(T* raw) {
        ptr = raw;
        ctrl = new ControlBlock<T>{raw, 1, 0};
    }

    SharedPtr(const SharedPtr& other) {
        ptr = other.ptr;
        ctrl = other.ctrl;
        ctrl->strong_count++;
    }

    ~SharedPtr() {
        if (--ctrl->strong_count == 0) {
            delete ptr;
            if (ctrl->weak_count == 0) delete ctrl;
        }
    }

    size_t use_count() const { return ctrl->strong_count; }
};


// weak_ptr
template<typename T>
class WeakPtr {
public:
    ControlBlock<T>* ctrl;

    WeakPtr(const SharedPtr<T>& shared) {
        ctrl = shared.ctrl;
        ctrl->weak_count++;
    }

    ~WeakPtr() {
        if (--ctrl->weak_count == 0 && ctrl->strong_count == 0)
            delete ctrl;
    }

    SharedPtr<T> lock() const {
        if (ctrl->strong_count > 0)
            return SharedPtr<T>(*ctrl);  // create new shared_ptr
        return SharedPtr<T>(nullptr);    // expired
    }

    bool expired() const {
        return ctrl->strong_count == 0;
    }
};



