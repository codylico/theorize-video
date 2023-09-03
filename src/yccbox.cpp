
#include "yccbox.hpp"
#include <new>
#include <limits>
#include <cstring>

namespace theorize
{
    static
    unsigned char* yccbox_alloc(unsigned width, unsigned height);

    //BEGIN yccbox / static
    static
    constexpr std::size_t yccbox_total(unsigned width, unsigned height) {
        return width * static_cast<std::size_t>(height) * 3;
    }
    static
    constexpr std::size_t yccbox_addr(std::size_t width,
            unsigned x, unsigned y)
    {
        return (width * y + x) * 3;
    }
    unsigned char* yccbox_alloc(unsigned width, unsigned height) {
        constexpr unsigned int max_size = 32767u;
        constexpr std::size_t max_total =
            std::numeric_limits<std::size_t>::max()/3;
        if (width == 0 || height == 0)
            return nullptr;
        else if (width > max_size || height > max_size)
            throw std::bad_alloc{};
        else if (width >= max_total/height)
            throw std::bad_alloc{};
        return new unsigned char[yccbox_total(width,height)];
    }
    //END   yccbox / static

    //BEGIN ycbcr_box / rule-of-six
    ycbcr_box::ycbcr_box(ycbcr_box const& other)
        : d(nullptr), w(0), h(0)
    {
        d = yccbox_alloc(other.w, other.h);
        std::memcpy(d, other.d, yccbox_total(other.w, other.h));
    }
    ycbcr_box& ycbcr_box::operator=(ycbcr_box const& other) {
        unsigned char* const new_ptr = yccbox_alloc(other.w, other.h);
        std::memcpy(new_ptr, other.d, yccbox_total(other.w, other.h));
        if (d)
            delete[] d;
        d = new_ptr;
        w = other.w;
        h = other.h;
        return *this;
    }
    //END   ycbcr_box / rule-of-six

    //BEGIN ycbcr_box / methods
    void ycbcr_box::resize(unsigned width, unsigned height) {
        unsigned char* const new_ptr = yccbox_alloc(width, height);
        if (d)
            delete[] d;
        d = new_ptr;
        w = width;
        h = height;
        return;
    }
    ycbcr ycbcr_box::get(unsigned int x, unsigned int y) const noexcept {
        unsigned char const* const ptr = d+yccbox_addr(w,x,y);
        return ycbcr{ptr[0],ptr[1],ptr[2]};
    }
    void ycbcr_box::set(unsigned int x, unsigned int y, ycbcr value) noexcept {
        unsigned char* const ptr = d+yccbox_addr(w,x,y);
        ptr[0] = value.y;
        ptr[1] = value.cb;
        ptr[2] = value.cr;
        return;
    }
    //END   ycbcr_box / methods
}