#include <DD4hep/Detector.h>
#include <DDRec/MaterialScan.h>
#include <fstream>

void ScanMaterial(string infile, string outfile="MaterialScanLogs.txt", int bins=100, double world_size=1000){
    using namespace dd4hep;
    using namespace dd4hep::rec;

    if(infile.empty()){
        cout << "Usage: root -l 'ScanMaterial.cpp(\"input_file.xml\", \"output_file.txt\", bins, world_size)' " << endl;
        return;
    }
    
    Detector& description = Detector::getInstance();
    description.fromXML(infile.c_str());
    MaterialScan scan(description);

    std::ofstream clearFile(outfile.c_str(), std::ios::out | std::ios::trunc);
    clearFile.close();
    FILE* outFile = freopen(outfile.c_str(), "w", stdout);
    for(int thetabin=0; thetabin<=bins; thetabin++){
        double theta = thetabin * M_PI / bins;
        double z = world_size*cos(theta);
        double tranverse = world_size*sin(theta);
        for(int phibin=-bins; phibin<=bins; phibin++){
            double phi = phibin * M_PI / bins;
            double x = tranverse*cos(phi);
            double y = tranverse*sin(phi);
            scan.print(0,0,0,x,y,z);
        }
    }
    fclose(outFile);
    return;
}