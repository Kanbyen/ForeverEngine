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

#include "precompiled.h"
#include "weaponControlsInfo.h"
#include "componentWeapon.h"
#include "windowManager.h"
#include "util.h"

void WeaponControlsInfo::set(ComponentWeapon* weapon)
{
	setTime = currentTime;
	expireTime = setTime + 2.0f;
	
	if (weapon->creator->nameLMB.size())
	{
		string str = "[LMB] " + weapon->creator->nameLMB;
		text[0].setText(str.c_str());
	}
	else
		text[0].setText("");
	if (weapon->creator->nameRMB.size())
	{
		string str = "[RMB] " + weapon->creator->nameRMB;
		text[1].setText(str.c_str());
	}
	else
		text[1].setText("");
	
	for (int i = 0; i < 2; i++)
	{
		text[i].setAlpha(0);
		float textExtent = windowManager.getFont()->getTextExtent(text[0].getText());
		float lineSpacing = windowManager.getFont()->getLineSpacing();
		text[i].setArea(cegui_reldim(0.5) + cegui_absdim(-(textExtent / 2)),
						cegui_reldim(0.5) + cegui_absdim((4 + i) * lineSpacing),
						cegui_absdim(textExtent),
						cegui_absdim(lineSpacing));
	}
}

void WeaponControlsInfo::init()
{
	currentTime = 0;
	setTime = -100;
	expireTime = 0;
	
	for (int i = 0; i < 2; i++)
	{
		text[i].create("", cegui_absdim(0), cegui_absdim(0), cegui_absdim(0), cegui_absdim(0), CEGUI::WindowManager::getSingleton().getWindow("HUDRoot"),
					   "DDDDDD", TEXTALIGN_CENTER);
		text[i].setAlpha(0);
	}
}
void WeaponControlsInfo::update(float time)
{
	const float fadeTime = 0.1f;

	currentTime += time;

	if (currentTime >= expireTime)
	{
		for (int i = 0; i < 2; i++)
			text[i].setAlpha(0.0f);
	}
	else
	{
		for (int i = 0; i < 2; i++)
		{
			if (currentTime < setTime + fadeTime)
				text[i].setAlpha((currentTime - setTime) / fadeTime);
			else if (currentTime > expireTime - fadeTime)
				text[i].setAlpha((expireTime - currentTime) / fadeTime);
			else
				text[i].setAlpha(1.0f);
		}
	}
}
void WeaponControlsInfo::exit()
{
	for (int i = 0; i < 2; i++)
		text[i].destroy();
}
