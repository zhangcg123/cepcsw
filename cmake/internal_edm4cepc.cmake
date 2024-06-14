
include(FetchContent)

FetchContent_Declare(
  EDM4CEPC
  GIT_REPOSITORY https://code.ihep.ac.cn/cepc/externals/EDM4cepc.git
  GIT_TAG 8eb38894b7b055bf6fe99d4b156e97f7e9466a26
  )

FetchContent_MakeAvailable(EDM4CEPC)
