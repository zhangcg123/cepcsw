# Howto scan the material

## Introduction
Use DD4hep tool [materialScan](https://dd4hep.web.cern.ch/dd4hep/reference/materialScan_8cpp_source.html) to get the material passing through for a geatino partical (i.e. a straight line) at a given angle.

## Material Scan

Select the dd4hep geometry files you wish to scan, and access the interactive interface of materialScan.

```bash
# Access to the interactive interface of materialScan
materialScan ./Detector/DetCRD/compact/TDR_o1_v01/TDR_o1_v01.xml -interactive

# if you want to scan the material for a specific detector, disable/enable the detectors in the xml file
materialScan ./Detector/DetCRD/compact/TDR_o1_v01/TDR_o1_v01-onlyBeamPipe.xml -interactive
```

copy the following code to the interactive interface, and press enter.
*bins* is the number of bins for theta and phi, *world_size* is the size of the scanning area (mm).

```bash
FILE* outFile = freopen("MaterialScanLogs.txt", "w", stdout);
int bins = 100;
double world_size = 10000;
for(int thetabin=0; thetabin<=bins; thetabin++){
    double theta = thetabin * M_PI / bins;
    double z = world_size*cos(theta);
    double tranverse = world_size*sin(theta);
    for(int phibin=-bins; phibin<=bins; phibin++){
        double phi = phibin * M_PI / bins;
        double x = tranverse*cos(phi);
        double y = tranverse*sin(phi);
        gMaterialScan->print(0,0,0,x,y,z);
    }
}
```

The "FILE* outFile" truncates the "stdout" output stream to the file "MaterialScanLogs.txt", so you can't see any output. Be patient and wait for the file to be created, it may take 1-2 minutes depending on the detectors & the number of bins.

The file "MaterialScanLogs.txt" contains all the material passing through for a straight line at the given angles with the size of bins of theta and phi.

## Draw the material budget for single xml file

We give an example script [options/extract_x0.py](options/extract_x0.py) to draw the material budget from the scan file generated above (e.x. MaterialScanLogs.txt).

```bash
# Draw the material budget for single xml file
# --input_file: the scan file generated above
# --output_file: the output file name (default: material_budget.png)
# --detector: the detector name to be used as the title of the plot (default: all_world)
# --bins: the bins of theta and phi (default: 100), the theta-bins is set to "bins"+1, the phi-bins is set to "bins"*2+1
# --bias: the bias used to cut the theta range to [bias, bins-bias] (default: 0)
python options/extract_x0.py \
    --input_file MaterialScanLogs.txt \
    --output_file material_budget.png \
    --detector all_world \
    --bins 100 \
    --bias 0
```

Make sure the property "bins" is the same as the one used in the [Material Scan](#material-scan).

The output is a png file shows the material budget at different theta and phi, and the average X0 for theta/phi.

## Draw the material budget for multiple xml files

For multiple xml files, we can use the script [options/extract_x0_multi.py](options/extract_x0_multi.py) to draw the material budget.

```bash
# Draw the material budget for multiple xml files
# --input_files: the scan files generated above, files names separated with spaces
# --labels: the labels name to be used as the legend of the plot
# --output_file: the output file name (default: accumulated_material_budget.png)
# --bins: the bins of theta and phi (default: 100), the theta-bins is set to "bins"+1, the phi-bins is set to "bins"*2+1
# --bias: the bias used to cut the theta range to [bias, bins-bias] (default: 0)
python options/extract_x0_multi.py \
    --input_file BeamPipe.txt Lumical.txt VXD.txt FTD.txt SIT.txt \
    --labels BeamPipe Lumical VXD FTD SIT \
    --output_file accumulated_material_budget.png \
    --bins 100 \
    --bias 0
```

The output is a png file shows the average X0 for theta/phi with different labels.