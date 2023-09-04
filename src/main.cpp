
#include "yccbox.hpp"
#include "pngycc.hpp"
#include <theora/codec.h>
#include <theora/theoraenc.h>
#include <iostream>
#include <string>
#include <fstream>
#include <memory>
#include <cstdlib>

static
void scale(theorize::ycbcr_box& dst, theorize::ycbcr_box const& src);

static
bool write_packet(std::ostream &o, ogg_packet const& packet);

class encoder
{
private:
    th_enc_ctx* ptr;
public:
    encoder(int width, int height, int fps);
    encoder(encoder const&) = delete;
    encoder& operator=(encoder const&) = delete;
    ~encoder();
    operator th_enc_ctx*() const noexcept;
    explicit operator bool() const noexcept;
};

void scale(theorize::ycbcr_box& dst, theorize::ycbcr_box const& src) {
    for (unsigned dst_y = 0; dst_y < dst.height(); ++dst_y) {
        unsigned const src_y = dst_y*src.height()/dst.height();
        for (unsigned dst_x = 0; dst_x < dst.width(); ++dst_x) {
            unsigned const src_x = dst_x*src.width()/dst.width();
            dst.set(dst_x,dst_y,src.get(src_x,src_y));
        }
    }
    return;
}
encoder::encoder(int width, int height, int fps) {
    th_info info = {};
    th_info_init(&info);
    info.frame_width = width;
    info.frame_height = height;
    info.pic_width = width;
    info.pic_height = height;
    info.pic_x = 0;
    info.pic_y = 0;
    info.colorspace = TH_CS_ITU_REC_470M;
    info.pixel_fmt = TH_PF_444;
    //info.target_bitrate = 0;
    //info.quality = 0;
    info.fps_numerator = fps;
    info.fps_denominator = 1;
    info.aspect_numerator = 1;
    info.aspect_denominator = 1;
    ptr = th_encode_alloc(&info);
    if (!ptr) {
        std::cerr << "failed to initialize encoder.\n";
    }
    th_info_clear(&info);
}
encoder::~encoder() {
    if (ptr)
        th_encode_free(ptr);
    ptr = nullptr;
}
encoder::operator th_enc_ctx*() const noexcept {
    return ptr;
}
encoder::operator bool() const noexcept {
    return ptr;
}

bool write_packet(std::ostream &o, ogg_packet const& packet) {
    if (!o) {
        std::cerr << "error: unable to attempt packet write\n";
    }
    o.write(reinterpret_cast<char*>(packet.packet), packet.bytes);
    if (!o) {
        std::cerr << "error: failed to write packet\n";
    }
    return static_cast<bool>(o);
}

int main(int argc, char**argv)
{
    std::size_t lineno = 0;
    std::string output_path;
    int width = 640;
    int height = 480;
    int fps = 30;
    int result = EXIT_SUCCESS;
    std::unique_ptr<std::ifstream> input_ptr;
    if (argc > 1) {
        input_ptr.reset(new std::ifstream(argv[1]));
    }
    std::istream& input = (argc>1 ? *input_ptr.get() : std::cin);
    // acquire configuration
    {
        bool blank_text = false;
        std::string config_line;
        while (getline(input, config_line)) {
            lineno += 1;
            if (config_line.empty())
                break;
            else if (config_line.front() == '#')
                continue;
            else if (config_line.front() != '$') {
                std::cerr << lineno << ": warning: missing '$'"
                    " before variable name; ignoring\n";
                if ((!blank_text)
                &&  config_line.find_first_of("/\\.") == std::string::npos)
                {
                    blank_text = true;
                    std::cerr << lineno << ": note: blank line is needed"
                        " between configuration and frame file list\n";
                }
                continue;
            }
            std::size_t const equals_pos = config_line.find('=');
            if (!equals_pos) {
                std::cerr << lineno << ": warning: "
                    "missing equals sign; ignoring\n";
                continue;
            }
            std::string const key = config_line.substr(1,equals_pos-1);
            std::string const value = config_line.substr(equals_pos+1);
            if (key == "output")
                output_path = value;
            else if (key == "width")
                width = std::stoi(value);
            else if (key == "height")
                height = std::stoi(value);
            else if (key == "fps")
                fps = std::stoi(value);
        }
    }
    if (fps <= 0) {
        std::cerr << "error: fps must be positive\n";
        return EXIT_FAILURE;
    }
    if (width <= 0) {
        std::cerr << "error: width must be positive\n";
        return EXIT_FAILURE;
    }
    if (height <= 0) {
        std::cerr << "error: height must be positive\n";
        return EXIT_FAILURE;
    }
    // acquire frames
    {
        std::ofstream out(output_path, std::ios::out | std::ios::binary);
        if (!out) {
            std::cerr << "error: failed to open output file\n\t"
                << output_path << std::endl;
            return EXIT_FAILURE;
        }
        encoder enc(width, height, fps);
        if (!enc) {
            return EXIT_FAILURE;
        }
        th_comment comments = {};
        int last = 1;
        ogg_packet packet = {};
        while (last) {
            last = th_encode_flushheader(enc, &comments, &packet);
            if (last == 0)
                break;
            else if (last == TH_EFAULT) {
                std::cerr << "error: EFAULT encountered during header"
                    " flush\n";
                return EXIT_FAILURE;
            }
            if (!write_packet(out, packet))
                return EXIT_FAILURE;
        }
        std::string file_path;
        int repeat_count = 1;
        theorize::ycbcr_box box;
        theorize::ycbcr_box frame;
        frame.resize(width, height);
        th_ycbcr_buffer frame_source;
        frame_source[0].width = width;
        frame_source[0].height = height;
        frame_source[0].stride = frame.width();
        frame_source[0].data = frame.y_plane();
        frame_source[1].width = width;
        frame_source[1].height = height;
        frame_source[1].stride = frame.width();
        frame_source[1].data = frame.cb_plane();
        frame_source[2].width = width;
        frame_source[2].height = height;
        frame_source[2].stride = frame.width();
        frame_source[2].data = frame.cr_plane();
        while (getline(input, file_path)) {
            lineno += 1;
            if (file_path.empty()
            ||  file_path.front() == '#')
                continue;
            else if (file_path.front() == '*') {
                repeat_count = std::stoi(file_path.substr(1));
                continue;
            }
            // read frame
            bool const ok = theorize::pngycc_read(file_path.c_str(), box);
            if (!ok) {
                std::cerr << lineno << ": error: failed to load frame";
                frame.grey();
            }
            // resize frame
            scale(frame, box);
            for (int i = 0; i < repeat_count; ++i) {
                th_encode_ycbcr_in(enc, frame_source);
                last = 1;
                while (last) {
                    last = th_encode_packetout(enc, false, &packet);
                    if (last == TH_EFAULT) {
                        std::cerr << lineno << ":error: EFAULT encountered "
                            "during frame generation\n";
                        return EXIT_FAILURE;
                    } else if (last) {
                        if (!write_packet(out, packet))
                            return EXIT_FAILURE;
                    }
                }
            }
            repeat_count = 1;
        }
        // output last packets
        last = 1;
        while (last) {
            last = th_encode_packetout(enc, true, &packet);
            if (last == TH_EFAULT) {
                std::cerr << lineno << ":error: EFAULT encountered "
                    "during ending generation\n";
                return EXIT_FAILURE;
            } else if (last) {
                if (!write_packet(out, packet))
                    return EXIT_FAILURE;
            }
        }
    }
    return result;
}