#pragma once

#include "../SamplesUtils.h"

#include <vector>

namespace tactics {

class DemoKawaiiTacticsState : public SampleState {
public:
	using SampleState::SampleState;
	FsmAction enter() override;
	FsmAction update() override;
	void exit() override;

private:
	struct UnitSelectionEntry {
		Entity character;
		Entity shadow;
	};

	void _createScene();
	void _setupInput();
	void _trySelectUnitAtCursor();
	void _applySelectionHighlight();

	unsigned int _mapIndex{};
	std::vector<UnitSelectionEntry> _units;
	int _selectedUnitIndex{-1};
};

} // namespace tactics
