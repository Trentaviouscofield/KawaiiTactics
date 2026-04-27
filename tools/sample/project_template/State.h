#pragma once

#include "../SamplesUtils.h"

namespace tactics {

// clang-format off
class Demo##SAMPLE_NAME##State : public SampleState {
public:
	using SampleState::SampleState;
	FsmAction enter() override;
	FsmAction update() override;
	void exit() override;
};
// clang-format on

} // namespace tactics
