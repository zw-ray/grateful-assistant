/*******************************************************************************
**     FileName: AwaitWakewordState.h
**    ClassName: AwaitWakewordState
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/03 16:36
**  Description:
*******************************************************************************/

#ifndef AWAITWAKEWORDSTATE_H
#define AWAITWAKEWORDSTATE_H

#include "AudioState.h"
#include <memory>

class AwaitWakewordState : public AudioState
{
public:
    using Ptr = std::shared_ptr<AwaitWakewordState>;
    AwaitWakewordState(AudioState::Ptr next);
    ~AwaitWakewordState();

    void handle() override;

protected:
}; // class AwaitWakewordState

#endif // AWAITWAKEWORDSTATE_H
