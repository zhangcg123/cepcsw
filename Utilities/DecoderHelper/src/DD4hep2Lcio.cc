#include "DecoderHelper/DD4hep2Lcio.h"

int DD4hep2Lcio::CEPCv4::getEcalLayer(int dd4hep_layer){
    return dd4hep_layer - 1 ;
}
int DD4hep2Lcio::CEPCv4::getEcalStave(int dd4hep_stave){
    int lcio_stave = dd4hep_stave <=2 ? dd4hep_stave+5 : dd4hep_stave-3 ;
    return lcio_stave ;
}
int DD4hep2Lcio::CEPCv4::getEcalBarrelStave(int dd4hep_stave){
    int lcio_stave = dd4hep_stave <=2 ? dd4hep_stave+5 : dd4hep_stave-3 ;
    return lcio_stave ;
}
int DD4hep2Lcio::CEPCv4::getEcalEndcapStave(int dd4hep_stave){
    int lcio_stave = dd4hep_stave <=2 ? dd4hep_stave+1 : dd4hep_stave-3 ;
    return lcio_stave ;
}
int DD4hep2Lcio::CEPCv4::getHcalLayer(int dd4hep_layer){
    return dd4hep_layer - 1 ;
}
int DD4hep2Lcio::CEPCv4::getHcalStave(int dd4hep_stave){

    int lcio_stave = dd4hep_stave ==0 ? dd4hep_stave+7 : dd4hep_stave-1 ;
    /*
                1                     0
               ****                  ****
            2 *    * 0            1 *    * 7
             *      *              *      *
            3*      * 7  --->     2*      * 6
             *      *              *      *
            4 *    * 6            3 *    * 5
               ****                  ****
                5                     4


    */
    return lcio_stave ;
}
int DD4hep2Lcio::CEPCv4::getMuonLayer(int dd4hep_layer){
    return dd4hep_layer - 1 ;
}
int DD4hep2Lcio::CEPCv4::getMuonStave(int dd4hep_stave){
    return 12 - dd4hep_stave ;
}