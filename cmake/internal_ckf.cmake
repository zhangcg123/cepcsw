
include(FetchContent)

FetchContent_Declare(
  ckf_belle
  GIT_REPOSITORY https://code.ihep.ac.cn/cepc/externals/ckf_belle.git
  GIT_TAG        a80b052c5d3cce242b297c439b3f778e19714695
  )

FetchContent_MakeAvailable(ckf_belle)
