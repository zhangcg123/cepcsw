
include(FetchContent)

FetchContent_Declare(
  ckf_belle
  GIT_REPOSITORY https://code.ihep.ac.cn/cepc/externals/ckf_belle.git
  GIT_TAG        72707998dfc4f7dbe4a91f995d03649cb217474d
  )

FetchContent_MakeAvailable(ckf_belle)
