#pragma once

#include <Engine/Core/Application.h>

namespace tactics {

// clang-format off
class ##APP_NAME##Application : public tactics::Application {
public:
	void setupComponentReflections() override;
	HashId initialize(ServiceLocator& serviceLocator, FsmBuilder& fsmBuilder) override;
};
// clang-format on

} // namespace tactics
