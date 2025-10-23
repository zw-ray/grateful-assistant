/*******************************************************************************
**     FileName: VADDetector.h
**    ClassName: VADDetector
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/03 10:16
**  Description:
*******************************************************************************/

#ifndef VADDETECTOR_H
#define VADDETECTOR_H

#include <cstdint>
#include <vector>
#include "webrtc_vad.h"

class VADDetector
{
public:
    VADDetector();
    ~VADDetector();

    bool initialize();

    bool isActiveAudio(const std::vector<int16_t> &audioBuffer) const;

protected:
    VadInst* m_inst;
}; // class VADDetector

#endif // VADDETECTOR_H
