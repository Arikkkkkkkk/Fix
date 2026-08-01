#pragma once

inline int  g_shulkerCacheIndex = -1;
inline char g_shulkerColorCode  = '0';
inline bool g_hasShulkerData    = false;

inline bool  g_shulkerHasLastCursor = false;
inline float g_shulkerLastCursorX   = 0.0f;
inline float g_shulkerLastCursorY   = 0.0f;

constexpr float kShulkerDragThresholdSq = 8.0f;
