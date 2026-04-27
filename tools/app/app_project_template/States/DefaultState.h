#pragma once

#include <Libs/Fsm/FsmStateWithServices.h>

namespace tactics {

// clang-format off
class ##APP_NAME##State : public FsmStateWithServices {
public:
	using FsmStateWithServices::FsmStateWithServices;
	FsmAction enter() override;
	FsmAction update() override;
	void exit() override;
};
// clang-format on

} // namespace tactics
