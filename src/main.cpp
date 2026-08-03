#include <cstdint>
#include <cstring>
#include <android/log.h>
#include "pl/Mod.hpp"
#include "pl/memory/Signature.hpp"
#include "pl/memory/Vtable.hpp"
#include "pl/memory/Hook.hpp"
#include "main.h"
#include "modmenu.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  "[ShulkerPreview]", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "[ShulkerPreview]", __VA_ARGS__)

BaseActorRenderContext_ctor_t      BaseActorRenderContext_ctor      = nullptr;
ItemRenderer_renderGuiItemNew_t    ItemRenderer_renderGuiItemNew    = nullptr;

static constexpr const char* MCPE_LIB = "libminecraftpe.so";

static void* resolve(const char* sig, const char* name) {
    uintptr_t addr = pl::memory::resolveSignature(sig, MCPE_LIB);
    if (!addr) { LOGE("not found: %s", name); return nullptr; }
    LOGI("found %s @ 0x%lx", name, (unsigned long)addr);
    return reinterpret_cast<void*>(addr);
}

static bool hookVtable(const char* cls, void** outOrig, void* hookFn, int slot) {
    uintptr_t addr = pl::memory::resolveVtableFunction(cls, slot, MCPE_LIB);
    if (!addr) { LOGE("hookVtable: resolve failed for %s", cls); return false; }

    int rc = pl::memory::hook(reinterpret_cast<void*>(addr), hookFn, outOrig);
    if (rc != 0) { LOGE("hookVtable: hook failed for %s (rc=%d)", cls, rc); return false; }

    LOGI("hooked %s slot[%d]", cls, slot);
    return true;
}

static void* g_Item_appendHover_orig = nullptr;
using Item_appendHover_t = void(*)(void*, ItemStackBase*, void*, std::string&, bool);

static void Item_appendFormattedHovertext_hook(
    void* self, ItemStackBase* stack, void* level, std::string& out, bool flag)
{
    g_hasShulkerData = false;
    if (g_Item_appendHover_orig)
        ((Item_appendHover_t)g_Item_appendHover_orig)(self, stack, level, out, flag);
}

class ShulkerBoxPreviewMod {
public:
  bool load() {
    Nbt_treeFind = (Nbt_treeFind_t)resolve(
        "? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 FD 03 00 91 F3 03 00 AA ? ? ? F8 ? ? ? B4 ? ? ? A9 F5 03 13 AA ? ? ? 14 ? ? ? 52 ? ? ? 71 ? ? ? 54 ? ? ? 91 ? ? ? F9 ? ? ? B4 ? ? ? 39 ? ? ? 36 ? ? ? F9 ? ? ? 36 ? ? ? F9 1F 03 16 EB E0 03 14 AA 02 33 96 9A ? ? ? 94 ? ? ? 34 ? ? ? 37 ? ? ? 52 ? ? ? 71 ? ? ? 54 ? ? ? 14 ? ? ? 91 ? ? ? 37 ? ? ? D3 1F 03 16 EB E0 03 14 AA 02 33 96 9A ? ? ? 94 ? ? ? 35 DF 02 18 EB ? ? ? 54 E8 03 1F 2A ? ? ? 71 ? ? ? 54 F5 03 17 AA ? ? ? F9 ? ? ? B5 ? ? ? 14 ? ? ? 54 ? ? ? 17 BF 02 13 EB ? ? ? 54 ? ? ? 39 ? ? ? A9 E0 03 14 AA ? ? ? D3 ? ? ? 72 ? ? ? 91 01 01 8B 9A 57 01 89 9A FF 02 16 EB E2 32 96 9A ? ? ? 94 DF 02 17 EB E8 27 9F 1A 1F 00 00 71 E9 A7 9F 1A 08 01 89 1A 1F 01 00 71 73 12 95 9A E0 03 13 AA ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A8 C0 03 5F D6 ? ? ? A9",
        "Nbt_treeFind");

    ItemStackBase_loadItem = (ItemStackBase_loadItem_t)resolve(
        "? ? ? D1 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? 91 ? ? ? D5 F3 03 00 AA ? ? ? ? ? ? ? 91 ? ? ? F9 F5 03 01 AA",
        "ItemStackBase_loadItem");

    ItemStackBase_getDamageValue = (ItemStackBase_getDamageValue_t)resolve(
        "? ? ? D1 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? 91 ? ? ? D5 ? ? ? F9 ? ? ? F8 ? ? ? F9 ? ? ? B4 ? ? ? F9 ? ? ? B4 ? ? ? F9 ? ? ? B4",
        "ItemStackBase_getDamageValue");

    BaseActorRenderContext_ctor = (BaseActorRenderContext_ctor_t)resolve(
        "? ? ? A9 ? ? ? A9 ? ? ? A9 FD 03 00 91 ? ? ? ? ? ? ? 91 ? ? ? A9 ? ? ? A9 F3 03 00 AA F4 03 02 AA",
        "BaseActorRenderContext_ctor");

    ItemRenderer_renderGuiItemNew = (ItemRenderer_renderGuiItemNew_t)resolve(
        "FF C3 05 D1 EC 73 00 FD EB 2B 0F 6D E9 23 10 6D FD 7B 11 A9 FC 6F 12 A9 FA 67 13 A9 F8 5F 14 A9 F6 57 15 A9 F4 4F 16 A9 FD 43 04 91 5B D0 3B D5",
        "ItemRenderer_renderGuiItemNew");

    hookVtable("4Item",
        &g_Item_appendHover_orig,
        (void*)Item_appendFormattedHovertext_hook,
        55);

    hookVtable("19ShulkerBoxBlockItem",
        (void**)&ShulkerBoxBlockItem_appendFormattedHovertext_orig,
        (void*)ShulkerBoxBlockItem_appendFormattedHovertext_hook,
        55);

    hookVtable("17HoverTextRenderer",
        (void**)&HoverRenderer_renderHoverBox_orig,
        (void*)HoverRenderer_renderHoverBox_hook,
        17);

    hookVtable("24MinecraftUIRenderContext",
        (void**)&MinecraftUIRenderContext_drawText_orig,
        (void*)MinecraftUIRenderContext_drawText_hook,
        6);

    LOGI("initialized");
    SP_initModMenu();
    return true;
  }
};

PL_REGISTER_MOD(ShulkerBoxPreviewMod, ShulkerBoxPreviewMod{})
