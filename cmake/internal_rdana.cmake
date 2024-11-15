include(FetchContent)

FetchContent_Declare(
  RDAnalysis
  GIT_REPOSITORY https://code.ihep.ac.cn/cepc/externals/RDAnalysis.git
  GIT_TAG  4c24156a9ccec59c2d728ff9e1824743f1819850
  )

FetchContent_MakeAvailable(RDAnalysis)

