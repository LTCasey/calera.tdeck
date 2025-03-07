/**
 * @file lv_conf.h
 * Configuration for LVGL for T-Deck UI
 */

 #ifndef LV_CONF_H
 #define LV_CONF_H
 
 #include <stdint.h>
 
 /* Color depth: 1 (1 byte per pixel), 8 (RGB332), 16 (RGB565), 32 (ARGB8888) */
 #define LV_COLOR_DEPTH 16
 
 /* Use a custom tick source for LVGL */
 #define LV_TICK_CUSTOM 1
 #if LV_TICK_CUSTOM
     #define LV_TICK_CUSTOM_INCLUDE "Arduino.h"
     #define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())
 #endif
 
 /* Default display refresh period - 5ms */
 #define LV_DISP_DEF_REFR_PERIOD 5
 
 /* Memory settings */
 #define LV_MEM_CUSTOM 0
 #if LV_MEM_CUSTOM == 0
     #define LV_MEM_SIZE (128U * 1024U)
     #define LV_MEM_ATTR
     #define LV_MEM_ADR 0
     #define LV_MEM_AUTO_DEFRAG 1
 #else
     #define LV_MEM_CUSTOM_INCLUDE "stdlib.h"
     #define LV_MEM_CUSTOM_ALLOC malloc
     #define LV_MEM_CUSTOM_FREE free
 #endif
 
 /* Use the standard `memcpy` and `memset` for memory operations */
 #define LV_MEMCPY_MEMSET_STD 0
 
 /* Enable the GPU interface, if available */
 #define LV_USE_GPU 0
 #define LV_USE_GPU_STM32_DMA2D 0
 #define LV_USE_GPU_NXP_PXP 0
 #define LV_USE_GPU_NXP_VG_LITE 0
 #define LV_USE_GPU_SDL 0
 
 /* Feature configuration */
 #define LV_USE_ANIMATION 1
 #define LV_USE_SHADOW 1
 #define LV_USE_BLEND_MODES 1
 #define LV_USE_OPA_SCALE 1
 #define LV_USE_IMG_TRANSFORM 1
 #define LV_USE_GROUP 1
 #define LV_USE_FLEX 1
 #define LV_USE_GRID 1
 #define LV_USE_THEME_DEFAULT 1
 #define LV_USE_THEME_BASIC 0
 #define LV_USE_THEME_MONO 0
 
 /* Font usage */
 #define LV_FONT_MONTSERRAT_8  0
 #define LV_FONT_MONTSERRAT_10 0
 #define LV_FONT_MONTSERRAT_12 1
 #define LV_FONT_MONTSERRAT_14 1
 #define LV_FONT_MONTSERRAT_16 1
 #define LV_FONT_MONTSERRAT_18 1
 #define LV_FONT_MONTSERRAT_20 1
 #define LV_FONT_MONTSERRAT_22 0
 #define LV_FONT_MONTSERRAT_24 1
 #define LV_FONT_MONTSERRAT_26 0
 #define LV_FONT_MONTSERRAT_28 0
 #define LV_FONT_MONTSERRAT_30 0
 #define LV_FONT_MONTSERRAT_32 0
 #define LV_FONT_MONTSERRAT_34 0
 #define LV_FONT_MONTSERRAT_36 0
 #define LV_FONT_MONTSERRAT_38 0
 #define LV_FONT_MONTSERRAT_40 0
 #define LV_FONT_MONTSERRAT_42 0
 #define LV_FONT_MONTSERRAT_44 0
 #define LV_FONT_MONTSERRAT_46 0
 #define LV_FONT_MONTSERRAT_48 0
 
 /* Symbol fonts */
 #define LV_FONT_DEJAVU_16_PERSIAN_HEBREW 0
 #define LV_FONT_SIMSUN_16_CJK 0
 #define LV_FONT_UNSCII_8 0
 #define LV_FONT_UNSCII_16 0
 #define LV_FONT_DEFAULT &lv_font_montserrat_14
 
 /* Enable drawing */
 #define LV_USE_DRAW_SW 1
 #if LV_USE_DRAW_SW
     #define LV_DRAW_SW_GRADIENT 1
     #define LV_DRAW_SW_COMPLEX 1
 #endif
 
 /* Widget usage - enable all widgets */
 #define LV_USE_ARC 1
 #define LV_USE_BAR 1
 #define LV_USE_BTN 1
 #define LV_USE_BTNMATRIX 1
 #define LV_USE_CANVAS 1
 #define LV_USE_CHECKBOX 1
 #define LV_USE_DROPDOWN 1
 #define LV_USE_IMG 1
 #define LV_USE_LABEL 1
 #define LV_USE_LINE 1
 #define LV_USE_ROLLER 1
 #define LV_USE_SLIDER 1
 #define LV_USE_SWITCH 1
 #define LV_USE_TEXTAREA 1
 #define LV_USE_TABLE 1
 #define LV_USE_CHART 1
 
 /* Container widgets */
 #define LV_USE_WIN 1
 #define LV_USE_TILEVIEW 1
 #define LV_USE_TABVIEW 1
 #define LV_USE_MSGBOX 1
 #define LV_USE_CALENDAR 1
 #define LV_USE_SPINBOX 1
 #define LV_USE_METER 1
 #define LV_USE_SPAN 1
 #define LV_USE_KEYBOARD 1
 
 /* Image decoder and cache */
 #define LV_USE_IMG_DECODER 1
 #define LV_IMG_CACHE_DEF_SIZE 32
 #define LV_USE_IMGFONT 0
 
 /* Logging */
 #define LV_USE_LOG 1
 #if LV_USE_LOG
     /* How important log should be added:
      * LV_LOG_LEVEL_TRACE  A lot of logs to give detailed information
      * LV_LOG_LEVEL_INFO   Log important events
      * LV_LOG_LEVEL_WARN   Log if something unwanted happened but didn't cause a problem
      * LV_LOG_LEVEL_ERROR  Only critical issue, when the system may fail
      * LV_LOG_LEVEL_USER   Only logs written by the user
      * LV_LOG_LEVEL_NONE   Do not log anything */
     #define LV_LOG_LEVEL LV_LOG_LEVEL_INFO
 
     /* 1: Print the log with 'printf';
      * 0: User need to register a callback with `lv_log_register_print_cb()` */
     #define LV_LOG_PRINTF 1
 
     /* Enable/disable LV_LOG_TRACE in modules that produces a huge number of logs */
     #define LV_LOG_TRACE_MEM 0
     #define LV_LOG_TRACE_TIMER 0
     #define LV_LOG_TRACE_INDEV 0
     #define LV_LOG_TRACE_DISP_REFR 0
     #define LV_LOG_TRACE_EVENT 0
     #define LV_LOG_TRACE_OBJ_CREATE 0
     #define LV_LOG_TRACE_LAYOUT 0
     #define LV_LOG_TRACE_ANIM 0
 #endif
 
 /* File system interface */
 #define LV_USE_FS_STDIO 0
 #define LV_USE_FS_POSIX 0
 #define LV_USE_FS_WIN32 0
 #define LV_USE_FS_FATFS 1
 #define LV_FS_STDIO_LETTER '\0'
 #define LV_FS_POSIX_LETTER 'A'
 #define LV_FS_WIN32_LETTER 'B'
 #define LV_FS_FATFS_LETTER 'S'
 
 /* Misc configuration */
 #define LV_ENABLE_GC 0
 #define LV_SPRINTF_CUSTOM 0
 #define LV_MEM_CUSTOM_CONSTRUCTOR 0
 #define LV_SPRINTF_DISABLE_FLOAT 0
 
 #endif /* LV_CONF_H */