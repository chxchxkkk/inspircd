/*       +------------------------------------+
 *       | Inspire Internet Relay Chat Daemon |
 *       +------------------------------------+
 *
 *  InspIRCd: (C) 2002-2010 InspIRCd Development Team
 * See: http://wiki.inspircd.org/Credits
 *
 * This program is free but copyrighted software; see
 *            the file COPYING for details.
 *
 * ---------------------------------------------------
 */

#include "inspircd.h"

static void DisplayList(User* user, Channel* channel)
{
	std::stringstream items;
	for(ModeIDIter id; id; id++)
	{
		ModeHandler* mh = ServerInstance->Modes->FindMode(id);
		if (!mh || mh->IsListMode() || mh->GetModeType() != MODETYPE_CHANNEL)
			continue;
		if (!channel->IsModeSet(mh))
			continue;
		items << " +" << mh->name;
		if (mh->GetNumParams(true))
			items << " " << channel->GetModeParameter(mh);
		items << " " << item;
	}
	char pfx[MAXBUF];
	snprintf(pfx, MAXBUF, ":%s 961 %s %s", ServerInstance->Config->ServerName.c_str(), user->nick.c_str(), channel->name.c_str());
	user->SendText(std::string(pfx), items);
	user->WriteNumeric(960, "%s %s :End of mode list", user->nick.c_str(), channel->name.c_str());
}

class CommandProp : public Command
{
 public:
	CommandProp(Module* parent) : Command(parent, "PROP", 1)
	{
		syntax = "<user|channel> {[+-]<mode> [<value>]}*";
	}

	CmdResult Handle(const std::vector<std::string> &parameters, User *src)
	{
		if (parameters.size() == 1)
		{
			Channel* chan = ServerInstance->FindChan(parameters[0]);
			if (chan)
				DisplayList(src, chan);
			return CMD_SUCCESS;
		}
		Channel* chan = ServerInstance->FindChan(parameters[0]);
		User* user = ServerInstance->FindNick(parameters[0]);
		if (!chan && !user)
			return CMD_FAILURE;
		unsigned int i = 1;
		irc::modestacker modes;
		while (i < parameters.size())
		{
			std::string prop = parameters[i++];
			bool plus = prop[0] != '-';
			if (prop[0] == '+' || prop[0] == '-')
				prop.erase(prop.begin());

			ModeHandler* mh = ServerInstance->Modes->FindMode(prop);
			if (mh && mh->GetModeType() == MODETYPE_CHANNEL)
			{
				irc::modechange mc(mh->id);
				mc.adding = plus;
				if (mh->GetNumParams(plus))
				{
					if (i == parameters.size())
						return CMD_FAILURE;
					mc.value = parameters[i++];
				}
				modes.push(mc);
			}
		}
		ServerInstance->SendMode(src, chan ? (Extensible*)chan : (Extensible*)user, modes, true);
		return CMD_SUCCESS;
	}
};

class ModuleNamedModes : public Module
{
	CommandProp cmd;
 public:
	ModuleNamedModes() : cmd(this)
	{
	}

	void init()
	{
		ServerInstance->Modules->AddService(cmd);
	}

	Version GetVersion()
	{
		return Version("Provides the ability to manipulate modes via long names.",VF_VENDOR);
	}
};

MODULE_INIT(ModuleNamedModes)