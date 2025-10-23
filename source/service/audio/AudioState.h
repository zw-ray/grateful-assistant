/*******************************************************************************
**     FileName: AudioState.h
**    ClassName: AudioState
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/03 16:34
**  Description:
*******************************************************************************/

#ifndef AUDIOSTATE_H
#define AUDIOSTATE_H

#include <memory>

class AudioState
{
public:
    using Ptr = std::shared_ptr<AudioState>;
    AudioState(Ptr next);
    virtual ~AudioState();

    virtual void handle() = 0;

protected:
    Ptr m_next;
}; // class AudioState

#endif // AUDIOSTATE_H
