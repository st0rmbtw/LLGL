/*
 * D3D11StagingBufferPool.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11StagingBufferPool.h"


namespace LLGL
{


D3D11StagingBufferPool::D3D11StagingBufferPool(
    ID3D11Device*           device,
    ID3D11DeviceContext*    context,
    UINT                    chunkSize,
    UINT                    bindFlags,
    UINT                    miscFlags)
:
    device_           { device                                                },
    context_          { context                                               },
    chunkSize_        { chunkSize                                             },
    bindFlags_        { bindFlags                                             },
    miscFlags_        { miscFlags                                             },
    incrementOffsets_ { (context->GetType() == D3D11_DEVICE_CONTEXT_DEFERRED) }
{
}

void D3D11StagingBufferPool::Reset()
{
    for (auto& chunk : chunks_)
        chunk.Reset();
    chunkIdx_ = 0;
}

D3D11BufferRange D3D11StagingBufferPool::Write(const void* data, UINT dataSize)
{
    /* Check if a new chunk must be allocated */
    if (chunkIdx_ == chunks_.size())
        AllocChunk(dataSize);
    else if (!chunks_[chunkIdx_].Capacity(dataSize))
    {
        ++chunkIdx_;
        if (chunkIdx_ == chunks_.size())
            AllocChunk(dataSize);
    }

    /* Write data to current chunk */
    auto& chunk = chunks_[chunkIdx_];
    D3D11BufferRange range = { chunk.GetNative(), chunk.GetOffset(), dataSize };
    {
        if (incrementOffsets_)
            chunk.WriteAndIncrementOffset(context_, data, dataSize);
        else
            chunk.Write(context_, data, dataSize);
    }
    return range;
}


/*
 * ======= Private: =======
 */

void D3D11StagingBufferPool::AllocChunk(UINT minChunkSize)
{
    chunks_.emplace_back(device_, std::max(chunkSize_, minChunkSize), bindFlags_, miscFlags_);
    chunkIdx_ = chunks_.size() - 1;
}


} // /namespace LLGL



// ================================================================================