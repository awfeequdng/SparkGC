//
// Created by kiva on 2019-01-03.
//
#pragma once

#include <vm/core.hpp>
#include <vm/exception.hpp>
#include <string>

/**
 * Ugly but useful
 */
#define READ_U2(v, p)  v = ((p)[0]<<8)|(p)[1];
#define READ_U4(v, p)  v = ((p)[0]<<24)|((p)[1]<<16)|((p)[2]<<8)|(p)[3];
#define READ_U8(v, p)  v = ((u8)(p)[0]<<56)|((u8)(p)[1]<<48)|((u8)(p)[2]<<40) \
                            |((u8)(p)[3]<<32)|((u8)(p)[4]<<24)|((u8)(p)[5]<<16) \
                            |((u8)(p)[6]<<8)|(u8)(p)[7];

namespace vm {
    /**
     *  Input stream for reading .class file
     *  The entire input stream is present in a buffer allocated by the caller.
     *  The caller is responsible for deallocating the buffer.
     */
    class byte_stream {
        using u1 = byte_t;
        using u2 = unsigned short;
        using u4 = unsigned int;
        using u8 = unsigned long long;
        using String = std::string;
    private:
        u1 *_bufferStart; // Buffer bottom
        u1 *_bufferEnd;   // Buffer top (one past last element)
        u1 *_current;      // Current buffer position
        String _source;    // Source of stream (directory name, ZIP/JAR archive name)
        bool _needVerify;  // True if verification is on for the class file

        void guaranteeMore(int size) {
            auto remaining = (size_t) (_bufferEnd - _current);
            auto usize = (unsigned int) size;
            if (usize > remaining) {
                vm_throw<std::logic_error>(20, "Unexpected EOF");
            }
        }

    public:
        byte_stream() {
            this->_current = nullptr;
            this->_bufferStart = nullptr;
            this->_bufferEnd = nullptr;
            this->_needVerify = true;
        }

        void init(u1 *buffer, size_t length) {
            this->_bufferStart = buffer;
            this->_bufferEnd = buffer + length;
            setNeedVerify(false);
            setCurrent(buffer);
        }

        // Buffer access
        u1 *getBufferStart() const { return _bufferStart; }

        size_t getLength() const { return static_cast<size_t>(_bufferEnd - _bufferStart); }

        u1 *getCurrent() const { return _current; }

        void setCurrent(u1 *pos) { _current = pos; }

        const String &getSource() const { return _source; }

        void setNeedVerify(bool flag) { _needVerify = flag; }

        void setSource(const String &source) { _source = source; }

        // Peek u1
        u1 peek1() const {
            return *_current;
        }

        u2 peek2() const {
            u2 res;
            READ_U2(res, _current);
            return res;
        }

        __attribute__((always_inline))
        u1 get1Fast() {
            return *_current++;
        }

        __attribute__((always_inline))
        u2 get2Fast() {
            u2 res;
            READ_U2(res, _current);
            _current += 2;
            return res;
        }

        __attribute__((always_inline))
        u4 get4Fast() {
            u4 res;
            READ_U4(res, _current);
            _current += 4;
            return res;
        }

        __attribute__((always_inline))
        u8 get8Fast() {
            u8 res;
            READ_U8(res, _current);
            _current += 8;
            return res;
        }

        // Read u1 from stream
        u1 get1() {
            if (_needVerify) {
                guaranteeMore(1);
            } else {
                assert(1 <= _bufferEnd - _current);
            }
            return get1Fast();
        }

        // Read u2 from stream
        u2 get2() {
            if (_needVerify) {
                guaranteeMore(2);
            } else {
                assert(2 <= _bufferEnd - _current);
            }
            return get2Fast();
        }

        // Read u4 from stream
        u4 get4() {
            if (_needVerify) {
                guaranteeMore(4);
            } else {
                assert(4 <= _bufferEnd - _current);
            }
            return get4Fast();
        }

        // Read u8 from stream
        u8 get8() {
            if (_needVerify) {
                guaranteeMore(8);
            } else {
                assert(8 <= _bufferEnd - _current);
            }
            return get8Fast();
        }

        // Copy `count` u1 bytes from current position to `to`
        void getBytes(u1 *to, int count) {
            u1 *from = asU1Buffer();
            skip1(count);

            int n = (count + 7) / 8;
#pragma clang diagnostic push
#pragma ide diagnostic ignored "missing_default_case"
            switch (count % 8) {
                case 0:
                    do {
                        *to++ = *from++;
                        case 7:
                            *to++ = *from++;
                        case 6:
                            *to++ = *from++;
                        case 5:
                            *to++ = *from++;
                        case 4:
                            *to++ = *from++;
                        case 3:
                            *to++ = *from++;
                        case 2:
                            *to++ = *from++;
                        case 1:
                            *to++ = *from++;
                    } while (--n > 0);
            }
#pragma clang diagnostic pop
        }

        // Get direct pointer into stream at current position.
        // Returns NULL if length elements are not remaining. The caller is
        // responsible for calling skip below if buffer contents is used.
        u1 *asU1Buffer() {
            return _current;
        }

        u2 *asU2Buffer() {
            return (u2 *) _current;
        }

        // Skip length u1 or u2 elements from stream
        void skip1(int length) {
            if (_needVerify) {
                guaranteeMore(length);
            }
            _current += length;
        }

        void skip1Fast(int length) {
            _current += length;
        }

        void skip4Fast(int length) {
            _current += 4 * length;
        }

        void skip2Fast(int length) {
            _current += 2 * length;
        }

        void skip2(int length) {
            if (_needVerify) {
                guaranteeMore(length * 2);
            }
            _current += length * 2;
        }

        void skip4(int length) {
            if (_needVerify) {
                guaranteeMore(length * 4);
            }
            _current += length * 4;
        }

        // Tells whether eos is reached
        bool isEnded() const { return _current == _bufferEnd; }
    };
}
