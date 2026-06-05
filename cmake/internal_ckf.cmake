
include(FetchContent)

FetchContent_Declare(
  ckf_belle
  GIT_REPOSITORY https://code.ihep.ac.cn/cepc/externals/ckf_belle.git
  GIT_TAG        212f90ae63e7a81f92bf8ff74328e50533f43432
  )

FetchContent_MakeAvailable(ckf_belle)
