idf_component_register(
    SRCS "src/sys_identity.cpp" "src/sys_module.c"
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "src"
    REQUIRES NVSm_sys ui fatfs_controller
)
