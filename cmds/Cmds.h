#pragma once
#include "CmdManager.h"

namespace cmds {
	class Cmds {
	public:
		Cmds();

		void init();
	private:
		Command sudoCommand;
	};
}