include(FetchContent)

# fetch and build "embedded_utils" code repo
FetchContent_Populate(
  embedded_utils
  GIT_REPOSITORY https://github.com/cracked-machine/embedded_utils.git
  GIT_TAG main
  SOURCE_DIR embedded_utils
)



# # fetch and build "stm32_interrupt_managers" code repo
# FetchContent_Populate(
#   stm32_interrupt_managers
#   GIT_REPOSITORY https://github.com/cracked-machine/stm32_interrupt_managers.git
#   GIT_TAG main
#   SOURCE_DIR stm32_interrupt_managers
# )