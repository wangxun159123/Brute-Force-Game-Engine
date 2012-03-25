/*    ___  _________     ____          __         
     / _ )/ __/ ___/____/ __/___ ___ _/_/___ ___ 
    / _  / _// (_ //___/ _/ / _ | _ `/ // _ | -_)
   /____/_/  \___/    /___//_//_|_, /_//_//_|__/ 
                               /___/             

This file is part of the Brute-Force Game Engine, BFG-Engine

For the latest info, see http://www.brute-force-games.com

Copyright (c) 2012 Brute-Force Games GbR

The BFG-Engine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

The BFG-Engine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with the BFG-Engine. If not, see <http://www.gnu.org/licenses/>.
*/

#include <Audio/OpenALStream.h>
#include <Base/CLogger.h>
#include <Audio/FileBase.h>

namespace BFG {
namespace Audio {

	OpenALStream::OpenALStream(boost::shared_ptr<AudioFile> file,
		                       boost::function<void (void)> onStreamFinished,
	                           ALuint sourceId):
		Stream(file, onStreamFinished),
		mNUM_BUFFER(6),
		mSourceId(sourceId)
    {
		mBufferIds.reset(new ALuint[mNUM_BUFFER]);
		preload();
    }

	OpenALStream::~OpenALStream()
	{
		alDeleteSources(1, &mSourceId);
	}

	void OpenALStream::preload()
	{
 		for (s32 i = 0; i < mNUM_BUFFER; ++i)
		{
			mAudioFile->read(mBufferIds[i]);
		}

		//dbglog << "Stream: "+mName+" preloaded";
    }

	void OpenALStream::nextStreamStep()
	{
		ALint processedBuffers = 0;
		ALuint tempBufferId;

		// Get the numbers of buffers that are already processed.
		alGetSourcei(mSourceId, AL_BUFFERS_PROCESSED, &processedBuffers);

		if (processedBuffers >= mNUM_BUFFER)
		{
			dbglog << "Stream '"+mAudioFile->toString()+"' finished.";
			alSourceUnqueueBuffers(mSourceId, mNUM_BUFFER, mBufferIds.get());

			// callback
			mOnStreamFinished();

			return;
		}

		while (processedBuffers > 0)
		{
			alSourceUnqueueBuffers(mSourceId, 1, &tempBufferId);
			mAudioFile->read(tempBufferId);
			alSourceQueueBuffers(mSourceId, 1, &tempBufferId);
			alGetSourcei(mSourceId, AL_BUFFERS_PROCESSED, &processedBuffers);
		}
	}

} // namespace Audio
} // namespace BFG
