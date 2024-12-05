#pragma once

#ifndef CONFIG_PATH
#error "CONFIG_PATH must be set at compile-time"
#endif

#define CONFIG_RESOURCE_ID 1

#ifndef FILE_ICON_PATH
#error "FILE_ICON_PATH must be set at compile-time"
#endif

#define ICON_RESOURCE_ID 2

#ifndef WALLPAPER_PATH
#error "WALLPAPER_PATH must be set at compile-time"
#endif

#define WALLPAPER_RESOURCE_ID 3
