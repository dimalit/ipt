file( GLOB sources "*.cpp" "*.h")

add_library(lighting STATIC ${sources})
target_include_directories(lighting PUBLIC ..)      # PUBLIC allows to apply this rule to referring targets (src in this case)


