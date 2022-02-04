# The following four lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly

idf_component_register(SRCS "lora.c" 
                    INCLUDE_DIRS "./include")
