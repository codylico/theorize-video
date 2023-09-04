/**
 * 
*/
#if !(defined hg_Theorize_YCbCrBox_h_)
#define hg_Theorize_YCbCrBox_h_

namespace theorize
{
    struct ycbcr {
        unsigned char y;
        unsigned char cb;
        unsigned char cr;
    };
    /**
     *
     */
    class ycbcr_box {
    private:
        unsigned char* d;
        unsigned int w;
        unsigned int h;
    public:
        constexpr ycbcr_box() noexcept
            : d(nullptr), w(0), h(0)
        {
        }
        ycbcr_box(ycbcr_box const& other);
        ycbcr_box& operator=(ycbcr_box const& other);
        ~ycbcr_box();
        void resize(unsigned width, unsigned height);
        ycbcr get(unsigned int x, unsigned int y) const noexcept;
        void set(unsigned int x, unsigned int y, ycbcr value) noexcept;
        unsigned int width() const noexcept { return w; }
        unsigned int height() const noexcept { return h; }
        void grey() noexcept;
        unsigned char* y_plane() noexcept;
        unsigned char const* y_plane() const noexcept;
        unsigned char* cb_plane() noexcept;
        unsigned char const* cb_plane() const noexcept;
        unsigned char* cr_plane() noexcept;
        unsigned char const* cr_plane() const noexcept;
    };
}

#endif //hg_Theorize_YCbCrBox_h_