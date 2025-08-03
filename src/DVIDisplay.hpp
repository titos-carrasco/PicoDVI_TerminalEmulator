#ifndef _RCR_DVIDISPLAY_HPP_
#define _RCR_DVIDISPLAY_HPP_

#include <stdint.h>

// clang-format off
enum VideoMode {
    MODE_640x480_60Hz,
    MODE_720x480_60Hz,
    MODE_800x600_60Hz,
    MODE_960x540p_60Hz,
    MODE_1280x720_30Hz
};
// clang-format on

class DVIDisplay {
  public:
    static DVIDisplay &getInstance();
    void init(VideoMode mode, char *screen, uint16_t rows, uint16_t cols, uint8_t (*font8x8)[8]);
    void start();

    DVIDisplay(const DVIDisplay &)            = delete;
    DVIDisplay &operator=(const DVIDisplay &) = delete;

  private:
    DVIDisplay() = default;
    void completeVars(VideoMode mode);
};

#endif //_RCR_DVIDISPLAY_HPP_
