#include "pl/Mod.hpp"

class ShulkerBoxPreviewMod {
public:
  bool load() {
    return true;
  }
};

PL_REGISTER_MOD(ShulkerBoxPreviewMod, ShulkerBoxPreviewMod{})
