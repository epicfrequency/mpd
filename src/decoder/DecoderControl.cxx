/*
 * Copyright 2003-2017 The Music Player Daemon Project
 * http://www.musicpd.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "config.h"
#include "DecoderControl.hxx"
#include "DecoderError.hxx"
#include "MusicPipe.hxx"
#include "DetachedSong.hxx"

#include <stdexcept>

#include <assert.h>

DecoderControl::DecoderControl(Mutex &_mutex, Cond &_client_cond,
			       const AudioFormat _configured_audio_format,
			       const ReplayGainConfig &_replay_gain_config)
	:thread(BIND_THIS_METHOD(RunThread)),
	 mutex(_mutex), client_cond(_client_cond),
	 configured_audio_format(_configured_audio_format),
	 replay_gain_config(_replay_gain_config) {}

DecoderControl::~DecoderControl()
{
	ClearError();

	delete song;
}

void
DecoderControl::WaitForDecoder()
{
	assert(!client_is_waiting);
	client_is_waiting = true;

	client_cond.wait(mutex);

	assert(client_is_waiting);
	client_is_waiting = false;
}

void
DecoderControl::SetReady(const AudioFormat audio_format,
			 bool _seekable, SignedSongTime _duration)
{
	assert(state == DecoderState::START);
	assert(pipe != nullptr);
	assert(pipe->IsEmpty());
	assert(audio_format.IsDefined());
	assert(audio_format.IsValid());

	in_audio_format = audio_format;
	out_audio_format = audio_format.WithMask(configured_audio_format);
	
	if (out_audio_format.format!=SampleFormat::DSD) {
        if (out_audio_format.sample_rate==44100||out_audio_format.sample_rate==88200||out_audio_format.sample_rate==176400 )
            {  
	       out_audio_format.sample_rate=352800;
               out_audio_format.format=SampleFormat::S32;
                }
        
                        
        if (out_audio_format.sample_rate==96000||out_audio_format.sample_rate==48000||out_audio_format.sample_rate==192000)
           { 
	      out_audio_format.sample_rate=384000;
              out_audio_format.format=SampleFormat::S32;       
                }
	}
	seekable = _seekable;
	total_time = _duration;

	state = DecoderState::DECODE;
	client_cond.signal();
}

bool
DecoderControl::IsCurrentSong(const DetachedSong &_song) const
{
	switch (state) {
	case DecoderState::STOP:
	case DecoderState::ERROR:
		return false;

	case DecoderState::START:
	case DecoderState::DECODE:
		return song->IsSame(_song);
	}

	assert(false);
	gcc_unreachable();
}

void
DecoderControl::Start(DetachedSong *_song,
		      SongTime _start_time, SongTime _end_time,
		      MusicBuffer &_buffer, MusicPipe &_pipe)
{
	const std::lock_guard<Mutex> protect(mutex);

	assert(_song != nullptr);
	assert(_pipe.IsEmpty());

	delete song;
	song = _song;
	start_time = _start_time;
	end_time = _end_time;
	buffer = &_buffer;
	pipe = &_pipe;

	ClearError();
	SynchronousCommandLocked(DecoderCommand::START);
}

void
DecoderControl::Stop()
{
	const std::lock_guard<Mutex> protect(mutex);

	if (command != DecoderCommand::NONE)
		/* Attempt to cancel the current command.  If it's too
		   late and the decoder thread is already executing
		   the old command, we'll call STOP again in this
		   function (see below). */
		SynchronousCommandLocked(DecoderCommand::STOP);

	if (state != DecoderState::STOP && state != DecoderState::ERROR)
		SynchronousCommandLocked(DecoderCommand::STOP);
}

void
DecoderControl::Seek(SongTime t)
{
	const std::lock_guard<Mutex> protect(mutex);

	assert(state != DecoderState::START);
	assert(state != DecoderState::ERROR);

	switch (state) {
	case DecoderState::START:
	case DecoderState::ERROR:
		gcc_unreachable();

	case DecoderState::STOP:
		/* TODO: if this happens, the caller should be given a
		   chance to restart the decoder */
		throw std::runtime_error("Decoder is dead");

	case DecoderState::DECODE:
		break;
	}

	if (!seekable)
		throw std::runtime_error("Not seekable");

	seek_time = t;
	seek_error = false;
	SynchronousCommandLocked(DecoderCommand::SEEK);

	if (seek_error)
		throw std::runtime_error("Decoder failed to seek");
}

void
DecoderControl::Quit()
{
	assert(thread.IsDefined());

	quit = true;
	LockAsynchronousCommand(DecoderCommand::STOP);

	thread.Join();
}

void
DecoderControl::CycleMixRamp()
{
	previous_mix_ramp = std::move(mix_ramp);
	mix_ramp.Clear();
}
