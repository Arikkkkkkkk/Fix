#pragma once
#include "ui/hoverrenderer.h"
#include "shulkerenderer/shulkerrenderer.h"
#include "hooks/minecraftuirendercontexthook.h"
#include "util/shulkerglobals.h"

using RenderHoverBoxFn = void (*)(void*, MinecraftUIRenderContext*, void*, void*, float);

inline RenderHoverBoxFn HoverRenderer_renderHoverBox_orig = nullptr;

inline void HoverRenderer_renderHoverBox_hook(
    void* selfPtr,
    MinecraftUIRenderContext* ctx,
    void* client,
    void* aabb,
    float someFloat)
{
    HoverRenderer_renderHoverBox_orig(selfPtr, ctx, client, aabb, someFloat);

    if (!ctx || !g_hasShulkerData || g_shulkerCacheIndex < 0)
        return;

    HoverRenderer* self = reinterpret_cast<HoverRenderer*>(selfPtr);
    if (!self)
        return;

    // If the touch/cursor position has moved since last frame, the finger
    // is being dragged and the game may not have re-validated what's
    // actually under it (see shulkerglobals.h). Don't trust the cached
    // data anymore in that case - drop it and skip rendering this frame.
    // A fresh tap on a shulker box will repopulate it.
    if (g_shulkerHasLastCursor) {
        float dx = self->mCursorX - g_shulkerLastCursorX;
        float dy = self->mCursorY - g_shulkerLastCursorY;
        if ((dx * dx + dy * dy) > kShulkerDragThresholdSq) {
            g_hasShulkerData      = false;
            g_shulkerCacheIndex   = -1;
            g_shulkerHasLastCursor = false;
            return;
        }
    }

    g_shulkerLastCursorX  = self->mCursorX;
    g_shulkerLastCursorY  = self->mCursorY;
    g_shulkerHasLastCursor = true;

    ActiveUIContext = ctx;

    float px = self->mCursorX + self->mOffsetX;
    float py = self->mCursorY + self->mOffsetY + self->mBoxHeight;

    ShulkerRenderer::render(ctx, px, py, g_shulkerCacheIndex, g_shulkerColorCode);
}
