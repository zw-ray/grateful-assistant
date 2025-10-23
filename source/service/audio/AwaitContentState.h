/*******************************************************************************
**     FileName: AwaitContentState.h
**    ClassName: AwaitContentState
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/03 16:37
**  Description:
*******************************************************************************/

#ifndef AWAITCONTENTSTATE_H
#define AWAITCONTENTSTATE_H

#include "AudioState.h"
#include <memory>

class AwaitContentState : public AudioState
{
public:
    using Ptr = std::shared_ptr<AwaitContentState>;
    AwaitContentState(AudioState::Ptr next);
    ~AwaitContentState();

    void handle() override;

protected:
}; // class AwaitContentState

#endif // AWAITCONTENTSTATE_H
