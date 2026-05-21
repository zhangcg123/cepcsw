#ifndef DD4HEP2LCIO_H
#define DD4HEP2LCIO_H

namespace DD4hep2Lcio {
    namespace CEPCv4 {
        int getEcalLayer(int dd4hep_layer);
        int getEcalStave(int dd4hep_stave);
        int getEcalBarrelStave(int dd4hep_stave);
        int getEcalEndcapStave(int dd4hep_stave);
        int getHcalLayer(int dd4hep_layer);
        int getHcalStave(int dd4hep_stave);
        int getMuonLayer(int dd4hep_layer);
        int getMuonStave(int dd4hep_stave);
    }
}
#endif