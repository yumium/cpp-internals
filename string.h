// This is not a complete implementation
// Details of design taken as is from libstdc++

itemplace <class _CharT, class _Traits, class _Allocator>
class _LIBCPP_TEMPLACE_VIS basic_string : private __basic_string_common<true> {
  // code omittted

  private:
    // 3*8 = 24 bytes long
    struct __long {
      size_t __cap_;
      size_t __size_;
      char* __data_;
    };

    struct __short {
      unsigned char __size_;
      char __data_[23];
    };

    // convenience constants used for checking mode
    static const size_t __short_mask = 0x01;
    static const size_t __long_mask = 0x1ul;

    // only used to calculate __n_words
    union __ulx {
      __long __lx;
      __short __lxx;
    };

    // __n_words = 3 in 64-bit systems
    enum { __n_words = sizeof (__ulx) / sizeof (size_t) };

    // used as convenience for methods that don't care long/short string mode (e.g., __zero())
    struct __raw {
      size_t __words[__n_words];
    };

    // std::string internal rep
    struct __rep {
      union {
        __long __l;
        __short __s;
        __raw __r;
      };
    };

    void __zero() {
      
    }

  public:
    size_t capacity() {
      if (__cap_ & 1) { // long string mode.
	// Clear last flag bit from __cap_
        size_t buffer_size = __cap_ & ~1ul;
	// Subtract 1 because the null terminator takes up one spot in
	// the character buffer.
	return buffer_size - 1;
      }

      // handle short string mode
    }

    size_t size() {
      if (__size_ & 1u == 0) {
        return __size_ >> 1;
      }

      // handle long string mode
    }

    // omitted code
}
