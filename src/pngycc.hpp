/**
 * 
*/
#if !(defined hg_Theorize_PngYCbCr_h_)
#define hg_Theorize_PngYCbCr_h_

namespace theorize
{
    class ycbcr_box;

    /**
     * \
     */
    bool pngycc_read(char const* path, ycbcr_box& output);
}

#endif //hg_Theorize_PngYCbCr_h_