# Define necessary options

##############################################################################
## CKF Belle
##############################################################################

option(CEPCSW_USE_SYSTEM_CKF_BELLE 
       "Use the existing installation of CKF BELLE"
       TRUE)

if (CEPCSW_USE_SYSTEM_CKF_BELLE)
  message("Try to use an existing installation of CKF BELLE")
else()
  message("Try to use an internal installation of CKF BELLE")

  include(FetchContent)

  FetchContent_Declare(
    ckf_belle
    GIT_REPOSITORY https://code.ihep.ac.cn/cepc/externals/ckf_belle.git
    GIT_TAG        11624a552a9d2382bba3ce9ed9b0cf2d05a89b6e
  )

  FetchContent_MakeAvailable(ckf_belle)
endif()
