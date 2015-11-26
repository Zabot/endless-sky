/* MenuPanel.cpp
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "MenuPanel.h"

#include "Command.h"
#include "ConversationPanel.h"
#include "Files.h"
#include "Font.h"
#include "FontSet.h"
#include "Format.h"
#include "GameData.h"
#include "Interface.h"
#include "Information.h"
#include "LoadPanel.h"
#include "MainPanel.h"
#include "MultiplayerPanel.h"
#include "Planet.h"
#include "PlayerInfo.h"
#include "Point.h"
#include "PointerShader.h"
#include "PreferencesPanel.h"
#include "ShipyardPanel.h"
#include "Sprite.h"
#include "SpriteShader.h"
#include "StarField.h"
#include "System.h"
#include "UI.h"

using namespace std;

namespace {
	static float alpha = 1.;
	static const int scrollSpeed = 2;
}



MenuPanel::MenuPanel(PlayerInfo &player, UI &gamePanels, ListenServer*& serverPointer)
	: player(player), gamePanels(gamePanels), scroll(0), serverPointer(serverPointer)
{
	SetIsFullScreen(true);
	
	string data = Files::Read(Files::Resources() + "credits.txt");
	size_t pos = 0;
	while(pos < data.size())
	{
		size_t end = data.find('\n', pos);
		if(end == string::npos)
			end = data.size();
		
		credits.push_back(data.substr(pos, end - pos));
		pos = end + 1;
	}
}



void MenuPanel::Step()
{
	if(GetUI()->IsTop(this) && alpha < 1.)
	{
		++scroll;
		if(scroll >= (20 * credits.size() + 300) * scrollSpeed)
			scroll = 0;
	}
}



void MenuPanel::Draw() const
{
	glClear(GL_COLOR_BUFFER_BIT);
	GameData::Background().Draw(Point(), Point());
	
	Information info;
	if(player.IsLoaded() && !player.IsDead())
	{
		info.SetCondition("pilot loaded");
		info.SetString("pilot", player.FirstName() + " " + player.LastName());
		if(player.Flagship())
		{
			const Ship &flagship = *player.Flagship();
			info.SetSprite("ship sprite", flagship.GetSprite().GetSprite());
			info.SetString("ship", flagship.Name());
		}
		if(player.GetSystem())
			info.SetString("system", player.GetSystem()->Name());
		if(player.GetPlanet())
			info.SetString("planet", player.GetPlanet()->Name());
		info.SetString("credits", Format::Number(player.Accounts().Credits()));
		info.SetString("date", player.GetDate().ToString());
	}
	else if(player.IsLoaded())
	{
		info.SetCondition("no pilot loaded");
		info.SetString("pilot", player.FirstName() + " " + player.LastName());
		info.SetString("ship", "You have died.");
	}
	else
	{
		info.SetCondition("no pilot loaded");
		info.SetString("pilot", "No Pilot Loaded");
	}
	
	const Interface *menu = GameData::Interfaces().Get("main menu");
	menu->Draw(info);
	
	int progress = static_cast<int>(GameData::Progress() * 60.);
	
	if(progress == 60)
	{
		if(!gamePanels.Root())
			gamePanels.Push(new MainPanel(player));
		alpha -= .02f;
	}
	if(alpha > 0.f)
	{
		Angle da(6.);
		Angle a(0.);
		for(int i = 0; i < progress; ++i)
		{
			Color color(.5 * alpha, 0.f);
			PointerShader::Draw(Point(), a.Unit(), 8., 20., 140. * alpha, color);
			a += da;
		}
	}
	
	const Font &font = FontSet::Get(14);
	int y = 120 - scroll / scrollSpeed;
	for(const string &line : credits)
	{
		float fade = 1.f;
		if(y < -145)
			fade = max(0.f, (y + 165) / 20.f);
		else if(y > 95)
			fade = max(0.f, (115 - y) / 20.f);
		if(fade)
		{
			Color color(((line.empty() || line[0] == ' ') ? .2 : .4) * fade, 0.);
			font.Draw(line, Point(-470., y), color);
		}
		y += 20;
	}
}



// New player "conversation" callback.
void MenuPanel::OnCallback(int)
{
	GetUI()->Pop(this);
	gamePanels.Reset();
	Panel *panel = new MainPanel(player);
	gamePanels.Push(panel);
	// Tell the main panel to re-draw itself (and pop up the planet panel).
	panel->Step();
	gamePanels.Push(new ShipyardPanel(player));
}



bool MenuPanel::KeyDown(SDL_Keycode key, Uint16 mod, const Command &command)
{
	if(GameData::Progress() < 1.)
		return false;
	
	if(player.IsLoaded() && (key == 'e' || command.Has(Command::MENU)))
		GetUI()->Pop(this);
	else if(key == 'p')
		GetUI()->Push(new PreferencesPanel());
	else if(key == 'l')
		GetUI()->Push(new LoadPanel(player, gamePanels));
	else if(key == 'm')
		GetUI()->Push(new MultiplayerPanel(player, gamePanels, serverPointer));
	else if(key == 'n' || key == 'e')
	{
		// The "New Pilot" and "Enter Ship" buttons are in the same place.
		GameData::Revert();
		player.New();
		
		ConversationPanel *panel = new ConversationPanel(
			player, *GameData::Conversations().Get("intro"));
		GetUI()->Push(panel);
		panel->SetCallback(this, &MenuPanel::OnCallback);
	}
	else if(key == 'q')
		GetUI()->Quit();
	else
		return false;
	
	return true;
}



bool MenuPanel::Click(int x, int y)
{
	char key = GameData::Interfaces().Get("main menu")->OnClick(Point(x, y));
	if(key)
		return DoKey(key);
	
	return true;
}
