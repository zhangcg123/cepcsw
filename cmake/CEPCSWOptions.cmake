# Define necessary options

##############################################################################
## CKF Belle
## EDM4CEPC
## ILCUTIL
## KalTest
## aidaTT
## DDKalTest
##
## CEPCSW_EXTERNAL_AS_COMPONENT
##   TRUE    install external project as component of CEPCSW in directory same
##           as CEPCSW
##   FALSE   install external project as external library in their speical
##           directory, optional through CEPCSW_EXTERNAL_ROOT
##############################################################################

option(CEPCSW_USE_SYSTEM_CKF_BELLE 
       "Use the existing installation of CKF BELLE. Otherwise the internal version will be used."
       FALSE)

option(CEPCSW_USER_SYSTEM_EDM4CEPC
       "Use the existing installation of EDM4CEPC. Otherwise the internal version will be used."
       FALSE)

option(CEPCSW_USER_SYSTEM_ILCUTIL
       "Use the existing installation of ILCUTIL. Otherwise the internal version will be used."
       FALSE)

option(CEPCSW_USER_SYSTEM_AIDATT
       "Use the existing installation of aidaTT. Otherwise the internal version will be used."
       FALSE)

option(CEPCSW_USER_SYSTEM_KALTEST
       "Use the existing installation of KalTest. Otherwise the internal version will be used."
       FALSE)

option(CEPCSW_USER_SYSTEM_DDKALTEST
       "Use the existing installation of DDKalTest. Otherwise the internal version will be used."
       FALSE)

option(CEPCSW_EXTERNAL_AS_COMPONENT
       "Use special option to install external into CEPCSW install directory."
       TRUE)

if (NOT CEPCSW_EXTERNAL_AS_COMPONENT)
  if (NOT DEFINED CEPCSW_EXTERNAL_ROOT)
    set(CEPCSW_EXTERNAL_ROOT ${CMAKE_INSTALL_PREFIX}/External)
  endif()
endif()
