#include "DemoKawaiiTacticsState.h"

#include "../Component/BattleCamera.h"
#include "../Component/CharacterFacing.h"
#include "../Component/RotateAroundPoint.h"
#include "../Component/RotateItem.h"

#include <Engine/Scene/SceneSystem.h>

#include <Libs/Ecs/Component/CameraComponent.h>
#include <Libs/Ecs/Component/FrustumComponent.h>
#include <Libs/Ecs/Component/TransformComponent.h>
#include <Libs/Input/InputSystem.h>
#include <Libs/Rendering/RenderSystem.h>

#include <SDL.h>
#include <tuple>

namespace tactics {

namespace {
constexpr int kGridWidth = 10;
constexpr int kGridHeight = 10;
constexpr float kTileSize = 0.56f;
const glm::vec3 kGridOrigin{-2.52f, 0.0f, -2.52f}; // Bottom-left tile center in world space.

struct TileCoord {
	int x{};
	int y{};
	component::Facing facing = component::Facing::South;
};

glm::vec3 toWorldPosition(const TileCoord& tile) {
	return {kGridOrigin.x + static_cast<float>(tile.x) * kTileSize,
			0.0f,
			kGridOrigin.z + static_cast<float>(tile.y) * kTileSize};
	return {kGridOrigin.x + static_cast<float>(tile.x) * kTileSize, 0.0f, kGridOrigin.z + static_cast<float>(tile.y) * kTileSize};
}
} // namespace

FsmAction DemoKawaiiTacticsState::enter() {
	_createScene();
	_setupInput();
	return FsmAction::none();
}

FsmAction DemoKawaiiTacticsState::update() {
	using namespace component;
	auto& registry = getService<SceneSystem>().getRegistry();
	RotateAroundPointSystem::update(registry);
	RotateItemSystem::update(registry);
	CharacterFacingSystem::update(registry);
	BattleCameraSystem::update(registry);

	auto& inputSystem = getService<InputSystem>();
	if (inputSystem.checkAction("exitFromState")) {
		return FsmAction::transition("exit"_id);
	} else if (inputSystem.checkAction("loadNextMap")) {
		_mapIndex = (_mapIndex + 1) % 5;
		_createScene();
	} else if (inputSystem.checkAction("selectUnit")) {
		_trySelectUnitAtCursor();
	}

	return FsmAction::none();
}

void DemoKawaiiTacticsState::exit() {
	auto& sceneSystem = getService<SceneSystem>();
	sceneSystem.clearScene();

	component::BattleCameraSystem::uninit(sceneSystem.getRegistry());
	_units.clear();
	_selectedUnitIndex = -1;
}

void DemoKawaiiTacticsState::_createScene() {
	using namespace component;

	auto& sceneSystem = getService<SceneSystem>();
	BattleCameraSystem::init(sceneSystem.getRegistry());

	_createCamera("mapCamera"_id);

	sceneSystem.clearScene();
	auto mapName = fmt::format("map{:02d}", _mapIndex);
	sceneSystem.createEntity("map"_id, HashId(mapName));
	_units.clear();
	_selectedUnitIndex = -1;

	auto unitTiles = std::array{
		TileCoord{4, 4, Facing::East},
		TileCoord{3, 4, Facing::West},
		TileCoord{4, 9, Facing::South},
		TileCoord{3, 9, Facing::North},
	};

	for (auto [tileX, tileY, facing] : unitTiles) {
		if (tileX < 0 || tileX >= kGridWidth || tileY < 0 || tileY >= kGridHeight) {
			continue;
		}
		auto pos = toWorldPosition({tileX, tileY, facing});
		auto character = sceneSystem.createEntity("char"_id, "character"_id);
		auto& charTransform = character.getComponent<Transform>();
		charTransform.translate(pos);
		character.getComponent<CharacterFacing>().facing = facing;
		auto shadow = sceneSystem.createEntity("shadow"_id, "charShadow"_id);
		auto& shadowTransform = shadow.getComponent<Transform>();
		shadowTransform.translate(pos);
		_units.push_back({character, shadow});
	}

	_applySelectionHighlight();
}

void DemoKawaiiTacticsState::_setupInput() {
	auto& inputSystem = getService<InputSystem>();
	inputSystem.assignInputMap("mapDemoInput");
}

void DemoKawaiiTacticsState::_trySelectUnitAtCursor() {
	using namespace component;

	if (_units.empty()) {
		return;
	}

	auto& sceneSystem = getService<SceneSystem>();
	auto& registry = sceneSystem.getRegistry();
	auto cameraView = registry.view<CurrentCamera, Camera, Transform>();
	auto cameraView = registry.view<component::CurrentCamera, component::Camera, component::Transform>();
	if (cameraView.empty()) {
		return;
	}

	auto&& [cameraEntity, camera, cameraTransform] = *cameraView.each().begin();
	(void)cameraEntity;
	(void)cameraTransform;
	auto cameraTuple = *cameraView.each().begin();
	auto& camera = std::get<1>(cameraTuple);
	auto viewProjection = camera.projection * camera.view;

	int mouseX = 0;
	int mouseY = 0;
	SDL_GetMouseState(&mouseX, &mouseY);

	auto& renderSystem = getService<RenderSystem>();
	auto windowSize = renderSystem.getWindowSize();
	if (windowSize.x == 0 || windowSize.y == 0) {
		return;
	}

	glm::vec2 mouseNdc{(static_cast<float>(mouseX) / static_cast<float>(windowSize.x)) * 2.0f - 1.0f,
					   1.0f - (static_cast<float>(mouseY) / static_cast<float>(windowSize.y)) * 2.0f};
	glm::vec2 mouseNdc{
		(static_cast<float>(mouseX) / static_cast<float>(windowSize.x)) * 2.0f - 1.0f,
		1.0f - (static_cast<float>(mouseY) / static_cast<float>(windowSize.y)) * 2.0f};

	constexpr float selectionThreshold = 0.12f;
	float bestDistance = selectionThreshold * selectionThreshold;
	int bestIndex = -1;

	for (int i = 0; i < static_cast<int>(_units.size()); ++i) {
		auto& unit = _units[i];
		if (!unit.character) {
			continue;
		}

		auto& unitTransform = unit.character.getComponent<Transform>();
		auto clipPosition = viewProjection * glm::vec4(unitTransform.getPosition(), 1.0f);
		if (clipPosition.w == 0.0f) {
			continue;
		}

		auto ndcPosition = glm::vec2(clipPosition) / clipPosition.w;
		auto delta = ndcPosition - mouseNdc;
		auto distance = glm::dot(delta, delta);
		if (distance < bestDistance) {
			bestDistance = distance;
			bestIndex = i;
		}
	}

	_selectedUnitIndex = bestIndex;
	_applySelectionHighlight();
}

void DemoKawaiiTacticsState::_applySelectionHighlight() {
	using namespace component;

	for (int i = 0; i < static_cast<int>(_units.size()); ++i) {
		auto& unit = _units[i];
		if (!unit.character || !unit.shadow) {
			continue;
		}

		const auto isSelected = i == _selectedUnitIndex;
		auto& characterTransform = unit.character.getComponent<Transform>();
		auto& shadowTransform = unit.shadow.getComponent<Transform>();
		characterTransform.setScale(isSelected ? glm::vec3(1.15f, 1.20f, 1.15f) : glm::vec3(0.9f, 1.0f, 0.9f));
		shadowTransform.setScale(isSelected ? glm::vec3(0.80f, 0.80f, 1.0f) : glm::vec3(0.65f, 0.65f, 1.0f));
	}
}

} // namespace tactics
