/*
WeakPointer(const SharedPoiner<T>&)
~WeakPointer()
lock()
expired()
*/


template<typename T>
struct ControlBlock {
    T* ptr;
    size_t strong_count = 1;  // owned shared_ptr
    size_t weak_count = 0;    // weak_ptrs
};

template<typename T>
class WeakPointer;

// shared_ptr
template<typename T>
class SharedPointer {
public:
    T* ptr;
    ControlBlock<T>* ctrl;

    SharedPointer(T* raw) {
        ptr = raw;
        ctrl = new ControlBlock<T>{raw, 1, 0};
    }

    SharedPointer(const SharedPointer& other) {
        ptr = other.ptr;
        ctrl = other.ctrl;
        ctrl->strong_count++;
    }

    ~SharedPointer() {
        if (--ctrl->strong_count == 0) {
            delete ptr;
            if (ctrl->weak_count == 0) delete ctrl;
        }
    }

    size_t use_count() const { return ctrl->strong_count; }

    friend class WeakPointer<T>;

private:
    SharedPointer(ControlBlock<T>* existing_ctrl) {
        if (existing_ctrl && existing_ctrl->strong_count > 0) {
            ptr = existing_ctrl->ptr;
            ctrl = existing_ctrl;
            ctrl->strong_count++;
        } else {
            // This case handles locking an expired pointer,
            // resulting in a null SharedPtr.
            ptr = nullptr;
            ctrl = nullptr;
        }
    }
};


// weak_ptr
template<typename T>
class WeakPointer {
public:
    ControlBlock<T>* ctrl;

    WeakPointer(const SharedPointer<T>& shared) {
        ctrl = shared.ctrl;
        ctrl->weak_count++;
    }

    ~WeakPointer() {
        if (--ctrl->weak_count == 0 && ctrl->strong_count == 0)
            delete ctrl;
    }

    SharedPointer<T> lock() const {
        if (expired()) {
            return SharedPointer<T>(nullptr);
        }

        return SharedPointer<T>(ctrl);
    }

    bool expired() const {
        return ctrl->strong_count == 0;
    }
};
