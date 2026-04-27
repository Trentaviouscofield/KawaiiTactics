#include "##APP_NAME##Application.h"
#include "States/##APP_NAME##State.h"

namespace tactics {

// clang-format off
void ##APP_NAME##Application::setupComponentReflections() {}

HashId ##APP_NAME##Application::initialize(ServiceLocator& serviceLocator, FsmBuilder& fsmBuilder) {
	fsmBuilder
		.state<##APP_NAME##State>("Default", serviceLocator)
		.on("proceed").exitFsm()
		.onAppExitRequest().exitFsm();

	return "Default"_id;
}
// clang-format on

} // namespace tactics
