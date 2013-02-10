/*****************************************************************************************

Forever War - a NetHack-like FPS

Copyright (C) 2008 Thomas Schöps

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program;
if not, see <http://www.gnu.org/licenses/>.

*****************************************************************************************/

#ifndef _AI_STATE_MACHINE_H_
#define _AI_STATE_MACHINE_H_

template<class owner> class AIState
{
public:
	owner* myOwner;

	virtual ~AIState() {};

	virtual void init() = 0;
	virtual void update(float time) = 0;
	virtual void exit() = 0;
};

template<class owner> class AIStateMachine
{
public:
	owner*				myOwner;
	AIState<owner>*		currentState;

	AIStateMachine(owner* myOwner)
	{
		this->myOwner = myOwner;
		currentState = NULL;
	}
	~AIStateMachine()
	{
		changeState(NULL);
	}

	void update(float time)
	{
		if (currentState)
			currentState->update(time);
	}

	void changeState(AIState<owner>* newState)
	{
		if (currentState)
		{
			currentState->exit();
			delete currentState;
		}

		currentState = newState;

		if (currentState)
		{
			currentState->myOwner = myOwner;
			currentState->init();
		}
	}
};

#endif
