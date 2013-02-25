/*
Minetest
Copyright (C) 2013 celeron55, Perttu Ahola <celeron55@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef GAME_HEADER
#define GAME_HEADER

#include "irrlichttypes_extrabloated.h"
#include <string>
#include <deque>
#include "keycode.h"
#include <list>

class KeyList : protected std::list<KeyPress>
{
	typedef std::list<KeyPress> super;
	typedef super::iterator iterator;
	typedef super::const_iterator const_iterator;

	virtual const_iterator find(const KeyPress &key) const
	{
		const_iterator f(begin());
		const_iterator e(end());
		while (f!=e) {
			if (*f == key)
				return f;
			++f;
		}
		return e;
	}

	virtual iterator find(const KeyPress &key)
	{
		iterator f(begin());
		iterator e(end());
		while (f!=e) {
			if (*f == key)
				return f;
			++f;
		}
		return e;
	}

public:
	void clear() { super::clear(); }

	void set(const KeyPress &key)
	{
		if (find(key) == end())
			push_back(key);
	}

	void unset(const KeyPress &key)
	{
		iterator p(find(key));
		if (p != end())
			erase(p);
	}

	void toggle(const KeyPress &key)
	{
		iterator p(this->find(key));
		if (p != end())
			erase(p);
		else
			push_back(key);
	}

	bool operator[](const KeyPress &key) const
	{
		return find(key) != end();
	}
};

class InputHandler
{
public:
	InputHandler()
	{
	}
	virtual ~InputHandler()
	{
	}

	virtual bool isKeyDown(const KeyPress &keyCode) = 0;
	virtual bool wasKeyDown(const KeyPress &keyCode) = 0;
	
	virtual v2s32 getMousePos() = 0;
	virtual void setMousePos(s32 x, s32 y) = 0;

	virtual bool getLeftState() = 0;
	virtual bool getRightState() = 0;

	virtual bool getLeftClicked() = 0;
	virtual bool getRightClicked() = 0;
	virtual void resetLeftClicked() = 0;
	virtual void resetRightClicked() = 0;

	virtual bool getLeftReleased() = 0;
	virtual bool getRightReleased() = 0;
	virtual void resetLeftReleased() = 0;
	virtual void resetRightReleased() = 0;
	
	virtual s32 getMouseWheel() = 0;

	virtual void step(float dtime) {};

	virtual void clear() {};
};

class ChatBackend;  /* to avoid having to include chat.h */
struct SubgameSpec;

void the_game(
	bool &kill,
	bool random_input,
	InputHandler *input,
	IrrlichtDevice *device,
	gui::IGUIFont* font,
	std::string map_dir,
	std::string playername,
	std::string password,
	std::string address, // If "", local server is used
	u16 port,
	std::wstring &error_message,
	ChatBackend &chat_backend,
	const SubgameSpec &gamespec, // Used for local game
	bool simple_singleplayer_mode
);

class RenderThread : public SimpleThread
{
public:

	RenderThread(irr::video::IVideoDriver* driver, std::deque<unsigned char*>* quene, std::deque<irr::video::IImage*>* dest, irr::core::dimension2d<u32> ss):
		m_dest(dest),
		m_quene(quene),
		m_ss(ss),
		m_driver(driver)
	{
	}

	void * Thread();
	
	irr::video::IVideoDriver* m_driver;

	std::deque<unsigned char*>* m_quene;

	std::deque<irr::video::IImage*>* m_dest;
	
	irr::core::dimension2d<u32> m_ss;
	
	int m_counter;
};

class SaveThread : public SimpleThread
{
public:

	SaveThread(irr::video::IVideoDriver* driver, std::deque<irr::video::IImage*>* quene, const char* videopath, const char* video_fmt):
		m_quene(quene),
		m_video_path(videopath),
		m_driver(driver),
		m_video_fmt(video_fmt)
	{
	}

	void * Thread();
	
	irr::video::IVideoDriver* m_driver;

	std::deque<irr::video::IImage*>* m_quene;

	const char* m_video_path;
	
	const char* m_video_fmt;
	
	int m_counter;
};

#endif

