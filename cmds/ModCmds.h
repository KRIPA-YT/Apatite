#pragma once
#include "CmdManager.h"

namespace cmds {
	class ModCmds {
	public:
		ModCmds();

		void init();
	private:
		Command stopCommand;
		Command removeSudo;
		Command addSudo;
	};
}