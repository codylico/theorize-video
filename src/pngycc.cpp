
#include "pngycc.hpp"
#include "yccbox.hpp"
#include "../deps/png-parts/src/api.h"
#include "../deps/png-parts/src/auxi.h"
#include <new>
#include <cmath>

namespace theorize {
    struct pngycc_gamma
    {
        double alpha;
        double beta;
        double delta;
        double epsilon;
    };
    struct pngycc_prime
    {
        double red;
        double green;
        double blue;
    };
    struct pngycc_ypbpr
    {
        double y;
        double pb;
        double pr;
    };
    struct pngycc_defrac_channel
    {
        double offset;
        double excursion;
    };
    struct pngycc_defrac
    {
        pngycc_defrac_channel y;
        pngycc_defrac_channel cb;
        pngycc_defrac_channel cr;
    };
    struct pngycc_kappa
    {
        constexpr pngycc_kappa(double r, double b)
            : kr(r), kb(b),
            two_m_2kr(2.0*(1.0-r)), two_m_2kb(2.0*(1.0-b)),
            frac_kr(r/(1.0-(b+r))), frac_kb(b/(1.0-(r+b)))
        {
        }
        double kr;
        double kb;
        double two_m_2kr;
        double two_m_2kb;
        double frac_kr;
        double frac_kb;
    };
    static
    int pngycc_start
        ( void* img, long int width, long int height, short bit_depth,
          short color_type, short compression, short filter, short interlace);
    static
    void pngycc_put
        ( void* img, long int x, long int y,
          unsigned int red, unsigned int green, unsigned int blue,
          unsigned int alpha);
    static
    double pngycc_apply_gamma(pngycc_gamma const& gamma, long int channel);
    static
    pngycc_ypbpr pngycc_to_ypbpr(pngycc_kappa const& kappa,
        pngycc_prime const& prime);
    static
    unsigned char pngycc_runout(pngycc_defrac_channel const& defrac,
        double channel);

    //BEGIN pngycc / static
    int pngycc_start
        ( void* img, long int width, long int height, short bit_depth,
          short color_type, short compression, short filter, short interlace)
    {
        constexpr long int max_size = 32767;
        if (width >= max_size || height >= max_size
        ||  width < 0 || height < 0)
            return PNGPARTS_API_BAD_PARAM;
        else try {
            ycbcr_box& box = *static_cast<ycbcr_box*>(img);
            box.resize(width, height);
        } catch(const std::bad_alloc&) {
            return PNGPARTS_API_MEMORY;
        }
        return PNGPARTS_API_OK;
    }

    void pngycc_put
        ( void* img, long int x, long int y,
          unsigned int red, unsigned int green, unsigned int blue,
          unsigned int alpha)
    {
        constexpr pngycc_gamma gamma = {1.0,1.0,0.1,0.001};
        constexpr pngycc_kappa kappa = {0.25,0.25};
        constexpr pngycc_defrac defrac = {
            {0.0,255.0}, {128.0,255.0}, {128.0,255.0}
        };
        ycbcr_box& box = *static_cast<ycbcr_box*>(img);
        pngycc_prime const prime = {
            pngycc_apply_gamma(gamma, red),
            pngycc_apply_gamma(gamma, green),
            pngycc_apply_gamma(gamma, blue)
        };
        pngycc_ypbpr const ypbpr = pngycc_to_ypbpr(kappa, prime);
        ycbcr const value = {
            pngycc_runout(defrac.y, ypbpr.y),
            pngycc_runout(defrac.cb, ypbpr.pb),
            pngycc_runout(defrac.cr, ypbpr.pr)
        };
        box.set(x,y,value);
        return;
    }

    inline
    double pngycc_apply_gamma(pngycc_gamma const& gamma, long int channel) {
        return (channel < gamma.delta*255)
            ? channel*gamma.alpha/255.0
            : (1+gamma.epsilon)*std::pow(channel/255.0, gamma.beta) - gamma.epsilon;
    }

    inline
    pngycc_ypbpr pngycc_to_ypbpr(pngycc_kappa const& kappa,
        pngycc_prime const& prime)
    {
        double const y = (prime.green + prime.red*kappa.frac_kr
            + prime.blue*kappa.frac_kb)/(1.0 + kappa.frac_kr + kappa.frac_kb);
        return pngycc_ypbpr{y, (prime.blue-y)/kappa.two_m_2kb,
            (prime.red-y)/kappa.two_m_2kr};
    }

    inline
    unsigned char pngycc_runout(pngycc_defrac_channel const& defrac,
        double channel)
    {
        double const pre = defrac.excursion*channel + defrac.offset;
        return pre < 0.0 ? 0u : (pre > 255.0
            ? 255u : static_cast<unsigned char>(pre)); 
    }
    //END   pngycc / static

    //BEGIN pngycc / namespace-local
    bool pngycc_read(char const* path, ycbcr_box& output) {
        pngparts_api_image img;
        img.cb_data = &output;
        img.start_cb = pngycc_start;
        img.put_cb = pngycc_put;
        int const result = pngparts_aux_read_png_8(&img, path);
        return result == PNGPARTS_API_OK;
    }
    //END   pngycc / namespace-local
}