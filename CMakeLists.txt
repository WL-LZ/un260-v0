cmake_minimum_required(VERSION 3.1)

project(test_lvgl)

# Suppress cmake unused warning
set(ignore ${BUILD_DOC} ${BUILD_DOCS} ${BUILD_EXAMPLE} ${BUILD_EXAMPLES}
 ${BUILD_SHARED_LIBS}${BUILD_TEST}${BUILD_TESTING}${BUILD_TESTS})

add_compile_options(-Wall)

set(CMAKE_C_FLAGS "-O0 -g0")

include_directories(. ./aic_ui)
include_directories(. ./guider_lvgl_demo)
add_executable(${PROJECT_NAME} main.c
	aic_ui/aic_dec.c
	aic_ui/aic_ui.c
	aic_ui/cpu_mem.c
	aic_ui/font/ui_font_Big.c
	aic_ui/font/ui_font_Title.c
	aic_ui/font/ui_font_H1.c
	lv_fbdev.c
	lv_ge2d.c
	lv_port_indev.c
	lv_port_disp.c
    un260/lv_components/lv_components.c
    un260/lv_core/lv_page_event.c
    un260/lv_core/lv_page_manager.c
    un260/lv_core/page_01_main.c
    un260/lv_core/page_02_list.c
    un260/lv_core/page_03_menu.c
    un260/lv_core/page_05_set_password.c
    un260/lv_core/page_06_settings.c
    un260/lv_core/page_07_curr.c
    un260/lv_core/page_08_boot.c
    un260/lv_core/page_09_cis_cala.c
    un260/lv_refre/lvgl_refre.c
    un260/lv_resources/lv_img_init.c
    un260/lv_system/platform_app.c
    un260/lv_system/lv_str.c
    un260/lv_system/user_cfg.c
    un260/lv_drivers/lv_drivers.c
    #font
	un260/font/lv_font_montserrat_8.c 
    un260/font/lv_font_montserrat_10.c
    un260/font/lv_font_montserrat_12_subpx.c
    un260/font/lv_font_montserrat_12.c
    un260/font/lv_font_montserrat_14.c
    un260/font/lv_font_montserrat_16.c
    un260/font/lv_font_montserrat_18.c
    un260/font/lv_font_montserrat_20.c
    un260/font/lv_font_montserrat_22.c
    un260/font/lv_font_montserrat_24.c
    un260/font/lv_font_montserrat_26.c
    un260/font/lv_font_montserrat_28_compressed.c
    un260/font/lv_font_montserrat_28.c
    un260/font/lv_font_montserrat_30.c
    un260/font/lv_font_montserrat_32.c
    un260/font/lv_font_montserrat_34.c
    un260/font/lv_font_montserrat_36.c
    un260/font/lv_font_montserrat_38.c
    un260/font/lv_font_montserrat_40.c
    un260/font/lv_font_montserrat_42.c
    un260/font/lv_font_montserrat_44.c
    un260/font/lv_font_montserrat_46.c
    un260/font/lv_font_montserrat_48.c
    # img
    un260/lv_resources/page_01_back.c
    un260/lv_resources/page_01_collect_icon.c
    un260/lv_resources/page_01_curr_USD.c
    un260/lv_resources/page_01_esc_icon.c
    un260/lv_resources/page_01_list_icon.c
    un260/lv_resources/page_01_menu_icon.c
    un260/lv_resources/page_01_mode_icon.c
    un260/lv_resources/page_01_print_icon.c
    un260/lv_resources/page_01_set_icon.c
    un260/lv_resources/page_01_start_icon.c
    un260/lv_resources/page_01_usb_icon.c
    un260/lv_resources/page_02_home_icon.c
    un260/lv_resources/page_02_list_img.c
    un260/lv_resources/page_03_del_icon.c
    un260/lv_resources/page_03_menu_bg_img.c
    un260/lv_resources/page_03_ok_icon.c
    un260/lv_resources/page_05_set_password_bg_img.c
    un260/lv_resources/page_07_currency_bg_img.c
    #img_curr
    un260/lv_resources/lv_curr/CURR_CNY.c
    un260/lv_resources/lv_curr/CURR_EGP.c
    un260/lv_resources/lv_curr/CURR_EUR.c
    un260/lv_resources/lv_curr/CURR_GBP.c
    un260/lv_resources/lv_curr/CURR_ISK.c
    un260/lv_resources/lv_curr/CURR_PHP.c
    un260/lv_resources/lv_curr/CURR_SOS.c
    un260/lv_resources/lv_curr/CURR_TRY.c
    un260/lv_resources/lv_curr/CURR_USD.c

)

# Find libts for link
if ( ${USE_RTP_TSLIB} STREQUAL "yes")
	add_definitions(-DUSE_RTP_TSLIB)
	find_library(LIBTS libts.so)
	target_link_libraries(${PROJECT_NAME} ${LIBTS})
endif()

find_library(LIBMPP_GE libmpp_ge.so)
find_library(LIBMPP_BASE libmpp_base.so)
find_library(LIBMPP_VE libmpp_ve.so)
find_library(LIBMPP_DEC libmpp_decoder.so)
target_link_libraries(${PROJECT_NAME} ${LIBMPP_GE} ${LIBMPP_DEC} ${LIBMPP_BASE} ${LIBMPP_VE})
#target_link_libraries(${PROJECT_NAME} lvgl lvgl_examples lvgl_demos)
target_link_libraries(${PROJECT_NAME} lvgl )
target_link_libraries(${PROJECT_NAME} media_player)
target_link_libraries(${PROJECT_NAME} m)
target_link_libraries(${PROJECT_NAME} pthread)

set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "/usr/local/lib")

include(GNUInstallDirs)

if(DEFINED CMAKE_INSTALL_FULL_LIBDIR)
	install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "${CMAKE_INSTALL_FULL_BINDIR}")
endif()

add_definitions(-DLVGL_DIR="L:${CMAKE_INSTALL_FULL_DATAROOTDIR}/lvgl_data/")

if(DEFINED CMAKE_INSTALL_FULL_DATAROOTDIR)
	install(DIRECTORY aic_ui/lvgl_data/
		DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/lvgl_data)
endif() # CMAKE_INSTALL_FULL_DATAROOTDIR
