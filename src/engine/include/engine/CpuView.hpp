#pragma once

#include <iterator>
#include <type_traits>

namespace engine {

// aka simplified std::ranges::views::strided_view from C++23
// NOTE: currently only works with pointers to const (T = const Foo)
template <typename T> struct CpuView {
    using CpuViewConst = CpuView<std::add_const_t<T>>;
    using BytePtr      = std::conditional_t<std::is_const<T>::value, uint8_t const*, uint8_t*>;
    using VoidPtr      = std::conditional_t<std::is_const<T>::value, void const*, void*>;

    BytePtr data;
    BytePtr dataEnd;
    size_t byteStride;

    explicit CpuView()
        : data(nullptr)
        , dataEnd(nullptr)
        , byteStride(0) { }

    explicit CpuView(T* data, std::size_t numElements, std::ptrdiff_t byteOffset = 0, std::size_t byteStride = sizeof(T))
        requires(!std::is_same_v<T, void const> && !std::is_same_v<T, void>)
        : CpuView(reinterpret_cast<VoidPtr>(data), numElements, byteOffset, byteStride) { }

    explicit CpuView(VoidPtr data, std::size_t numElements, std::ptrdiff_t byteOffset = 0, std::size_t byteStride = sizeof(T))
        : data(reinterpret_cast<BytePtr>(data) + byteOffset)
        , dataEnd(this->data + numElements * byteStride)
        , byteStride(byteStride) { }

    auto operator[](size_t idx) -> T* {
        auto* item = data + idx * byteStride;
        return (item < dataEnd) ? reinterpret_cast<T*>(item) : nullptr;
    }

    explicit operator bool() const { return data != nullptr; }
    operator CpuViewConst const&() const { return reinterpret_cast<CpuViewConst const&>(*this); }
    auto Begin() const -> T* { return reinterpret_cast<T*>(data); }
    auto End() const -> T* { return reinterpret_cast<T*>(dataEnd); }
    auto NumElements() const -> std::size_t { return (dataEnd - data) / byteStride; }
    auto NumBytes() const -> std::size_t { return dataEnd - data; }
    auto IsContiguous() const -> bool { return byteStride == sizeof(T); }
    auto IsEmpty() const -> bool { return data == nullptr | std::distance(data, dataEnd) == 0; }
};

// CpuMemory is same as CpuView, but contiguous (i.e. byteStride is equal to datatype)
template <typename T> struct CpuMemory : CpuView<T> {
    explicit CpuMemory(T* data, size_t numElements, ptrdiff_t byteOffset = 0)
        : CpuView<T>(data, numElements, byteOffset, sizeof(T)) { }
    explicit CpuMemory()
        : CpuView<T>() { }
};

template <> struct CpuMemory<void> : CpuView<void> {
    explicit CpuMemory(void* data, size_t numElements, ptrdiff_t byteOffset = 0)
        : CpuView<void>(data, numElements, byteOffset, 1) { }
    explicit CpuMemory()
        : CpuView<void>() { }
};

template <> struct CpuMemory<void const> : CpuView<void const> {
    explicit CpuMemory(void const* data, size_t numElements, ptrdiff_t byteOffset = 0)
        : CpuView<void const>(data, numElements, byteOffset, 1) { }
    explicit CpuMemory()
        : CpuView<void const>() { }
};

} // namespace engine